#ifdef __linux__

#include "window.hpp"

#include <assert.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <xcb/shm.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>

namespace raycaster {

#define KEY_ESCAPE 0x9
#define KEY_F1 0x43
#define KEY_F2 0x44
#define KEY_F3 0x45
#define KEY_F4 0x46
#define KEY_Q 0x18
#define KEY_W 0x19
#define KEY_E 0x1A
#define KEY_A 0x26
#define KEY_S 0x27
#define KEY_D 0x28
#define KEY_P 0x21
#define KEY_SPACE 0x41
#define KEY_KPADD 0x56
#define KEY_KPSUB 0x52
#define KEY_B 0x38
#define KEY_F 0x29
#define KEY_L 0x2E
#define KEY_N 0x39
#define KEY_O 0x20
#define KEY_T 0x1C

// Maps from our Key type to the xcb key value
const xcb_keycode_t linux_key_map[key_count] = {KEY_W, KEY_A, KEY_S,    KEY_D,
                                                KEY_Q, KEY_E, KEY_SPACE};

const uint32_t max_framebuffer_count = 2;
const uint32_t max_key_callbacks = 4;

struct KeyPressCallbackTracker {
  uint32_t callback_count = 0;
  KeyPressCallback callbacks[max_key_callbacks] = {};
};

struct Window_t {
  xcb_window_t handle = {};
  xcb_gcontext_t graphics_context = {};
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t current_frame = 0;
  xcb_pixmap_t framebuffers[max_framebuffer_count] = {};
  xcb_shm_segment_info_t framebuffer_memory_info[max_framebuffer_count] = {};
  xcb_intern_atom_reply_t *delete_window_reply = nullptr;
  KeyPressCallbackTracker key_press_callbacks[key_count] = {};
  bool should_close = false;
};

Window_t windows[MAX_WINDOWS] = {};
uint32_t window_count = 0;

xcb_connection_t *connection = nullptr;
xcb_screen_t *screen = nullptr;

bool init_window_system() {
  connection = xcb_connect(nullptr, nullptr);
  if (connection == nullptr) {
    assert(false);
    return false;
  }

  // Just get the first screen for now
  const xcb_setup_t *setup = xcb_get_setup(connection);
  xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(setup);
  screen = screen_iter.data;
  if (screen == nullptr) {
    assert(false);
    return false;
  }

  return true;
}

bool create_window(const struct WindowDescriptor *desc, Window *window) {
  if (window_count >= MAX_WINDOWS) {
    assert(false);
    return false;
  }

  uint32_t window_value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t window_value_list[2] = {
      screen->black_pixel,
      XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS |
          XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
          XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
          XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE};

  xcb_window_t handle = xcb_generate_id(connection);
  xcb_create_window(connection, XCB_COPY_FROM_PARENT, handle, screen->root, 0,
                    0, desc->width, desc->height, 10,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual,
                    window_value_mask, window_value_list);

  if (handle == 0) {
    assert(false);
    return false;
  }

  // Create graphics context
  uint32_t gc_value_mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
  uint32_t gc_value_list[2] = {screen->black_pixel, 0};

  xcb_gcontext_t graphics_context = xcb_generate_id(connection);
  xcb_create_gc(connection, graphics_context, handle, gc_value_mask,
                gc_value_list);
  if (graphics_context == 0) {
    assert(false);
    return false;
  }

  xcb_map_window(connection, handle);

  // Make sure we can use the shared memory extension
  xcb_shm_query_version_reply_t *reply = xcb_shm_query_version_reply(
      connection, xcb_shm_query_version(connection), nullptr);

  if (!reply || !reply->shared_pixmaps) {
    assert(false);
    return false;
  }

  (*window) = &windows[window_count];
  window_count++;

  (*window)->handle = handle;
  (*window)->graphics_context = graphics_context;
  (*window)->width = desc->width;
  (*window)->height = desc->height;

  // Create pixmaps
  for (uint32_t i = 0; i < max_framebuffer_count; ++i) {
    xcb_shm_segment_info_t info = {};
    info.shmid =
        shmget(IPC_PRIVATE, desc->width * desc->height * 4, IPC_CREAT | 0600);
    info.shmaddr = (uint8_t *)shmat(info.shmid, 0, 0);
    info.shmseg = xcb_generate_id(connection);

    xcb_shm_attach(connection, info.shmseg, info.shmid, 0);
    shmctl(info.shmid, IPC_RMID, 0);

    xcb_pixmap_t pixmap = xcb_generate_id(connection);
    xcb_shm_create_pixmap(connection, pixmap, handle, desc->width, desc->height,
                          screen->root_depth, info.shmseg, 0);

    (*window)->framebuffer_memory_info[i] = info;
    (*window)->framebuffers[i] = pixmap;
  }

  /* Magic code that will send notification when window is destroyed */
  xcb_intern_atom_cookie_t cookie =
      xcb_intern_atom(connection, 1, 12, "WM_PROTOCOLS");
  xcb_intern_atom_reply_t *protocols_reply =
      xcb_intern_atom_reply(connection, cookie, 0);

  xcb_intern_atom_cookie_t cookie2 =
      xcb_intern_atom(connection, 0, 16, "WM_DELETE_WINDOW");
  (*window)->delete_window_reply =
      xcb_intern_atom_reply(connection, cookie2, 0);

  xcb_change_property(connection, XCB_PROP_MODE_REPLACE, handle,
                      (*protocols_reply).atom, 4, 32, 1,
                      &(*(*window)->delete_window_reply).atom);
  delete protocols_reply;

  // Flush commands to show the window
  xcb_flush(connection);

  return true;
}

void pump_window(Window window) {
  xcb_generic_event_t *event = nullptr;
  while ((event = xcb_poll_for_event(connection)) != nullptr) {
    switch (event->response_type & ~0x80) {
    /* Handle the ButtonPress event type */
    case XCB_KEY_PRESS: {
      xcb_key_press_event_t *key_event = (xcb_key_press_event_t *)event;

      // Convert the keycode from XCB key code to our key code
      int32_t key_code = -1;
      for (int32_t i = 0; i < (int32_t)key_count; ++i) {
        if (linux_key_map[i] == key_event->detail) {
          key_code = i;
        }
      }

      // If no code was found, break
      if (key_code == -1) {
        break;
      }

      // Otherwise go trigger the appropriate callbacks
      KeyPressCallbackTracker &tracker = window->key_press_callbacks[key_code];
      for (uint32_t i = 0; i < tracker.callback_count; ++i) {
        tracker.callbacks[i]();
      }

      break;
    }
    case XCB_CLIENT_MESSAGE:
      if (((xcb_client_message_event_t *)event)->data.data32[0] ==
          (window->delete_window_reply)->atom) {
        window->should_close = true;
      }
      break;
    }
    delete event;
  }

  xcb_copy_area(connection, window->framebuffers[window->current_frame],
                window->handle, window->graphics_context, 0, 0, 0, 0,
                window->width, window->height);

  window->current_frame++;
  window->current_frame = window->current_frame % max_framebuffer_count;

  xcb_flush(connection);
}

bool window_should_close(Window window) { return window->should_close; }

void register_key_press_callback(Window window, Key key,
                                 KeyPressCallback callback) {
  KeyPressCallbackTracker &tracker =
      window->key_press_callbacks[static_cast<uint32_t>(key)];

  assert(tracker.callback_count < max_key_callbacks);

  tracker.callbacks[tracker.callback_count] = callback;
  tracker.callback_count++;
}

void present_window(Window window, const uint8_t *framebuffer,
                    uint32_t framebuffer_size) {

  uint32_t next_frame_idx = (window->current_frame + 1) % max_framebuffer_count;

  xcb_shm_segment_info_t &framebuffer_memory_info =
      window->framebuffer_memory_info[next_frame_idx];

  uint8_t *framebuffer_data = framebuffer_memory_info.shmaddr;
  memcpy(framebuffer_data, framebuffer, framebuffer_size);
}

void shutdown_window_system() {
  for (uint32_t i = 0; i < window_count; ++i) {
    Window_t &window = windows[i];

    for (uint32_t i = 0; i < max_framebuffer_count; ++i) {
      // Shutdown shared memory ext
      xcb_shm_detach(connection, window.framebuffer_memory_info[i].shmseg);
      shmdt(window.framebuffer_memory_info[i].shmaddr);

      xcb_free_pixmap(connection, window.framebuffers[i]);
    }

    xcb_free_gc(connection, window.graphics_context);
  }

  screen = nullptr;
  xcb_disconnect(connection);
  connection = nullptr;
}

} // namespace raycaster

#endif