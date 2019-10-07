#ifdef _WIN32

#include "window.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <assert.h>

#include <stdio.h>

namespace raycaster {

const uint32_t MAX_FRAMEBUFFER_COUNT = 2;

struct Window_t {
  HWND handle = NULL;
  bool should_close = false;
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t current_frame = 0;
  HDC framebuffers[MAX_FRAMEBUFFER_COUNT] = {};
};

struct Window_t windows[MAX_WINDOWS] = {};
uint32_t window_count = 0;

const wchar_t class_name[] = L"TestWindowClass";
const wchar_t window_name[] = L"Raycaster";

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
    BitBlt(dc, 0, 0, window->width, window->height,
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
  uint32_t adjustedWidth = rect.right - rect.left;
  uint32_t adjustedHeight = rect.bottom - rect.top;

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
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = height;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  uint8_t *framebuffer_pixels = nullptr;

  HDC dc = GetDC(handle);
  for (uint32_t i = 0; i < MAX_FRAMEBUFFER_COUNT; ++i) {

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

bool window_should_close(Window window) {
  MSG msg = {};
  bool okay = GetMessage(&msg, NULL, 0, 0);

  assert(okay);

  TranslateMessage(&msg);
  DispatchMessage(&msg);

  return window->should_close;
}

void present_window(Window window, const uint8_t *framebuffer,
                    uint32_t framebuffer_size) {
  // * 4 because we assume 4 bytes per pixel
  assert(window->width * window->height * 4 >= framebuffer_size);

  const uint32_t next_frame =
      (window->current_frame + 1) % MAX_FRAMEBUFFER_COUNT;
  HDC current_dc = window->framebuffers[next_frame];

  BITMAPINFO bmi = {};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = window->width;
  bmi.bmiHeader.biHeight = window->height;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  int res =
      SetDIBitsToDevice(current_dc, 0, 0, window->width, window->height, 0, 0,
                        0, window->height, framebuffer, &bmi, DIB_RGB_COLORS);
  assert(res != GDI_ERROR);

  // Must invalidate window otherwise the updates won't be
  // visible until the user moves/resizes the window.
  // Does NOT force a WM_PAINT event; just tells the system to actually
  // update the window canvas.
  InvalidateRect(window->handle, nullptr, false);

  // Force a WM_PAINT event
  RedrawWindow(window->handle, NULL, NULL, RDW_INTERNALPAINT);

  window->current_frame++;
  window->current_frame = window->current_frame % MAX_FRAMEBUFFER_COUNT;
}

void shutdown_window_system() {
  memset(windows, 0, sizeof(struct Window_t) * MAX_WINDOWS);
  window_count = 0;
}

} // namespace raycaster

#endif