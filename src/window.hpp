#pragma once

#include <stdint.h>

namespace raycaster {

struct WindowDescriptor {
  uint32_t width;
  uint32_t height;
};

struct Window_t;

using Window = Window_t *;

const uint32_t MAX_WINDOWS = 16;

bool init_window_system();

bool create_window(const WindowDescriptor *desc, Window *window);

bool window_should_close(Window window);

void present_window(Window window, const uint8_t *framebuffer,
                    uint32_t framebuffer_size);

void shutdown_window_system();

} // namespace raycaster