#ifdef __linux__

#include "window.hpp"

#include <assert.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <xcb/xcb.h>
#include <xcb/shm.h>
#include <xcb/xcb_image.h>

namespace raycaster {

const uint32_t MAX_FRAMEBUFFER_COUNT = 2;

struct Window_t {
  xcb_window_t handle = {};
  xcb_gcontext_t graphics_context = {};
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t current_frame = 0;
  xcb_pixmap_t framebuffers [MAX_FRAMEBUFFER_COUNT] = {};
  xcb_shm_segment_info_t framebuffer_memory_info[MAX_FRAMEBUFFER_COUNT] = {};
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

  xcb_window_t handle = xcb_generate_id(connection);
  xcb_create_window(connection, XCB_COPY_FROM_PARENT, handle, screen->root, 0,
                    0, desc->width, desc->height, 10,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, 0,
                    nullptr);

  if (handle == 0) {
    assert(false);
    return false;
  }

  // Create graphics context
  uint32_t value_mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
  uint32_t value_list[2] = {screen->black_pixel, 0};

  xcb_gcontext_t graphics_context = xcb_generate_id(connection);
  xcb_create_gc(connection, graphics_context, handle, value_mask, value_list);
  if (graphics_context == 0) {
    assert(false);
    return false;
  }

  xcb_map_window(connection, handle);

  // Make sure we can use the shared memory extension
  xcb_shm_query_version_reply_t* reply = xcb_shm_query_version_reply(
        connection,
        xcb_shm_query_version(connection),
        nullptr
  );

  if(!reply || !reply->shared_pixmaps){
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
  for (uint32_t i = 0; i < MAX_FRAMEBUFFER_COUNT; ++i) {
    xcb_shm_segment_info_t info = {};
    info.shmid = shmget(IPC_PRIVATE, desc->width * desc->height * 4, IPC_CREAT | 0600);
    info.shmaddr = (uint8_t*)shmat(info.shmid, 0, 0);
    info.shmseg = xcb_generate_id(connection);
  
    xcb_shm_attach(connection, info.shmseg, info.shmid, 0);
    shmctl(info.shmid, IPC_RMID, 0);

    xcb_pixmap_t pixmap = xcb_generate_id(connection);
    xcb_shm_create_pixmap(connection, pixmap, handle, desc->width,
                      desc->height, screen->root_depth, info.shmseg, 0);

    (*window)->framebuffer_memory_info[i] = info; 
    (*window)->framebuffers[i] = pixmap;
  }

  // TODO: Setup cookie for shutdown event

  // Flush commands to show the window
  xcb_flush(connection);

  return true;
}

bool window_should_close(Window window) {
  xcb_generic_event_t *event = nullptr;
  while((event = xcb_poll_for_event(connection)) != nullptr){
    // TODO: Handle events
    delete event;
  }

  xcb_copy_area(connection, window->framebuffers[window->current_frame],
                      window->handle, window->graphics_context, 0, 0, 0, 0,
                      window->width, window->height);

  window->current_frame++;
  window->current_frame = window->current_frame % MAX_FRAMEBUFFER_COUNT;
  
  xcb_flush(connection);

  return false;
}

void present_window(Window window, const uint8_t *framebuffer,
                    uint32_t framebuffer_size) {

  uint32_t next_frame_idx = (window->current_frame + 1) % MAX_FRAMEBUFFER_COUNT;

  xcb_shm_segment_info_t& framebuffer_memory_info = window->framebuffer_memory_info[next_frame_idx];

  uint8_t* framebuffer_data = framebuffer_memory_info.shmaddr;
  memcpy(framebuffer_data, framebuffer, framebuffer_size);
}

void shutdown_window_system() {
  for (uint32_t i = 0; i < window_count; ++i) {
    Window_t &window = windows[i];

    for (uint32_t i = 0; i < MAX_FRAMEBUFFER_COUNT; ++i) {
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