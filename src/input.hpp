#pragma once

#include <stdint.h>

namespace raycaster {
enum class Key : uint32_t { W, A, S, D, Q, E, SPACE, COUNT };

using KeyPressCallback = void (*)();

bool init_input_system();

void register_key_press_callback(Key key, KeyPressCallback callback);

void poll_input();

void shutdown_input_system();
} // namespace raycaster