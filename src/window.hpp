#pragma once

#include <stdint.h>

namespace raycaster {

enum class Key : uint32_t { W, A, S, D, Q, E, SPACE, COUNT };
const uint32_t key_count = static_cast<uint32_t>(Key::COUNT);

using KeyPressCallback = void (*)();

struct WindowDescriptor {
  uint32_t width;
  uint32_t height;
};

struct Window_t;

using Window = Window_t *;

const uint32_t MAX_WINDOWS = 16;

bool init_window_system();

bool create_window(const WindowDescriptor *desc, Window *window);

void pump_window(Window window);

bool window_should_close(Window window);

void register_key_press_callback(Window window, Key key,
                                 KeyPressCallback callback);

void present_window(Window window, const uint8_t *framebuffer,
                    uint32_t framebuffer_size);

void shutdown_window_system();

} // namespace raycaster