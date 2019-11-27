#ifdef _WIN32

#include "window.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <assert.h>

#include <stdio.h>

namespace raycaster {

const uint32_t max_framebuffer_count = 2;
const uint32_t max_callbacks = 4;

// Maps from our Key type to the win32 VK_KEY value
const BYTE win32_vk_key_map[key_count] = {
    'W', 'A', 'S', 'D', 'Q', 'E', VK_SPACE,
};

struct KeyPressCallbackTracker {
  uint32_t callback_count = 0;
  KeyPressCallback callbacks[max_callbacks] = {};
};

struct Window_t {
  HWND handle = NULL;
  bool should_close = false;
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t current_frame = 0;
  HDC framebuffers[max_framebuffer_count] = {};
  KeyPressCallbackTracker key_press_callbacks[key_count] = {};
};

Window_t windows[MAX_WINDOWS] = {};
uint32_t window_count = 0;

const wchar_t class_name[] = L"TestWindowClass";
const wchar_t window_name[] = L"Raycaster";

void key_pressed(Window window, Key key) {
  uint32_t key_idx = static_cast<uint32_t>(key);
  KeyPressCallbackTracker &tracker = window->key_press_callbacks[key_idx];
  for (uint32_t i = 0; i < tracker.callback_count; ++i) {
    tracker.callbacks[i]();
  }
}

void poll_input(Window window) {
  static const uint32_t max_win32_vk = 256;
  BYTE vk_state[max_win32_vk] = {};

  BOOL err = GetKeyboardState(vk_state);
  (void)err;
  assert(err != 0);

  for (uint32_t i = 0; i < key_count; ++i) {
    Key key = Key(i);
    BYTE win32_key = win32_vk_key_map[i];

    // Check only the high order bit
    // Low order bit is used for toggle keys
    if ((vk_state[win32_key] >> 7)) {
      key_pressed(window, key);
    }
  }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                            LPARAM lParam) {
  Window window = (Window)GetWindowLongPtr(hwnd, GWLP_USERDATA);

  switch (uMsg) {
  case WM_DESTROY:
  case WM_CLOSE:
  case WM_QUIT:
    window->should_close = true;
    break;
  case WM_PAINT: {
    PAINTSTRUCT paint = {};
    HDC dc = BeginPaint(hwnd, &paint);
    BitBlt(dc, 0, 0, int32_t(window->width), int32_t(window->height),
           window->framebuffers[window->current_frame], 0, 0, SRCCOPY);
    EndPaint(hwnd, &paint);
  } break;
  default:
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
  }

  return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

bool init_window_system() {
  WNDCLASSEXW window_class = {};
  window_class.cbSize = sizeof(WNDCLASSEXW);
  window_class.lpfnWndProc = WindowProc;
  window_class.hInstance = GetModuleHandle(NULL);
  window_class.lpszClassName = class_name;

  ATOM result = RegisterClassExW(&window_class);
  if (result == 0) {
    return false;
  }

  return true;
}

bool create_window(const WindowDescriptor *desc, Window *window) {
  assert(window_count <= MAX_WINDOWS);

  const uint32_t width = desc->width;
  const uint32_t height = desc->height;

  *window = &windows[window_count];

  // We need to determine how big to create the window
  // if we want a "client-area" of the given size
  RECT rect = {0, 0, (LONG)width, (LONG)height};
  AdjustWindowRectEx(&rect, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE, 0);
  int32_t adjustedWidth = rect.right - rect.left;
  int32_t adjustedHeight = rect.bottom - rect.top;

  HWND handle =
      CreateWindowExW(0, class_name, window_name, WS_OVERLAPPEDWINDOW,
                      CW_USEDEFAULT, CW_USEDEFAULT, adjustedWidth,
                      adjustedHeight, NULL, NULL, GetModuleHandle(NULL), NULL);

  if (handle == NULL) {
    return false;
  }

  Window window_ptr = *window;

  window_ptr->handle = handle;
  window_ptr->width = width;
  window_ptr->height = height;

  BITMAPINFO bmi = {};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = (LONG)width;
  bmi.bmiHeader.biHeight = (LONG)height;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  uint8_t *framebuffer_pixels = nullptr;

  HDC dc = GetDC(handle);
  for (uint32_t i = 0; i < max_framebuffer_count; ++i) {

    HBITMAP hDib = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS,
                                    (void **)&framebuffer_pixels, 0, 0);
    assert(framebuffer_pixels);

    window_ptr->framebuffers[i] = CreateCompatibleDC(dc);
    SelectObject(window_ptr->framebuffers[i], hDib);
  }
  ReleaseDC(handle, dc);

  window_count++;

  // Set custom data on the Win32 window to the window object here
  SetWindowLongPtr(handle, GWLP_USERDATA, (LONG_PTR)window_ptr);

  ShowWindow(handle, SW_SHOWDEFAULT);

  return true;
}

void pump_window(Window window) {
  MSG msg = {};

  bool okay = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
  (void)okay;
  assert(okay);

  TranslateMessage(&msg);
  DispatchMessage(&msg);

  poll_input(window);
}

bool window_should_close(Window window) { return window->should_close; }

void register_key_press_callback(Window window, Key key,
                                 KeyPressCallback callback) {
  const uint32_t key_idx = static_cast<uint32_t>(key);

  KeyPressCallbackTracker &tracker = window->key_press_callbacks[key_idx];

  assert(tracker.callback_count <= max_callbacks);

  tracker.callbacks[tracker.callback_count] = callback;
  tracker.callback_count++;
}

void present_window(Window window, const uint8_t *framebuffer,
                    uint32_t framebuffer_size) {
  // * 4 because we assume 4 bytes per pixel
  (void)framebuffer_size;
  assert(window->width * window->height * 4 >= framebuffer_size);

  const uint32_t next_frame =
      (window->current_frame + 1) % max_framebuffer_count;
  HDC current_dc = window->framebuffers[next_frame];

  BITMAPINFO bmi = {};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = (LONG)window->width;
  bmi.bmiHeader.biHeight = (LONG)window->height;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  int res =
      SetDIBitsToDevice(current_dc, 0, 0, window->width, window->height, 0, 0,
                        0, window->height, framebuffer, &bmi, DIB_RGB_COLORS);
  (void)res;
  assert(res != GDI_ERROR);

  // Must invalidate window otherwise the updates won't be
  // visible until the user moves/resizes the window.
  // Does NOT force a WM_PAINT event; just tells the system to actually
  // update the window canvas.
  InvalidateRect(window->handle, nullptr, false);

  // Force a WM_PAINT event
  RedrawWindow(window->handle, NULL, NULL, RDW_INTERNALPAINT);

  window->current_frame++;
  window->current_frame = window->current_frame % max_framebuffer_count;
}

void shutdown_window_system() {
  memset(windows, 0, sizeof(struct Window_t) * MAX_WINDOWS);
  window_count = 0;
}

} // namespace raycaster

#endif