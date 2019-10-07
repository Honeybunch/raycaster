#ifdef _WIN32
#include "input.hpp"

#include <assert.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdio.h>

namespace raycaster {

const uint32_t key_count = static_cast<uint32_t>(Key::COUNT);
const uint32_t max_callbacks = 4;

// Maps from our Key type to the win32 VK_KEY value
const BYTE win32_vk_key_map[key_count] = {
    'W', 'A', 'S', 'D', 'Q', 'E', VK_SPACE,
};

struct KeyPressCallbackTracker {
  uint32_t callback_count = 0;
  KeyPressCallback callbacks[max_callbacks] = {};
};

bool key_state[key_count] = {};
KeyPressCallbackTracker key_press_callbacks[key_count] = {};

void key_pressed(Key key) {
  uint32_t key_idx = static_cast<uint32_t>(key);
  KeyPressCallbackTracker &tracker = key_press_callbacks[key_idx];
  for (uint32_t i = 0; i < tracker.callback_count; ++i) {
    tracker.callbacks[i]();
  }
}

void register_key_press_callback(Key key, KeyPressCallback callback) {
  const uint32_t key_idx = static_cast<uint32_t>(key);

  KeyPressCallbackTracker &tracker = key_press_callbacks[key_idx];

  assert(tracker.callback_count <= max_callbacks);

  tracker.callbacks[tracker.callback_count] = callback;
  tracker.callback_count++;
}

void poll_input() {
  static const uint32_t max_win32_vk = 256;
  BYTE key_state[max_win32_vk] = {};

  BOOL err = GetKeyboardState(key_state);
  assert(err != 0);

  for (uint32_t i = 0; i < key_count; ++i) {
    Key key = Key(i);
    BYTE win32_key = win32_vk_key_map[i];

    // Check only the high order bit
    // Low order bit is used for toggle keys
    if ((key_state[win32_key] >> 7)) {
      key_pressed(key);
    }
  }
}

} // namespace raycaster
#endif