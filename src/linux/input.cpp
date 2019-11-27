#ifdef __linux__

#include "input.hpp"

#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

namespace raycaster {

const char *keyboard_device = "/dev/input/event0";
int32_t keyboard_file = 0;
input_event *keyboard_event = {};

const uint32_t key_count = static_cast<uint32_t>(Key::COUNT);

// Maps from our Key type to the win32 VK_KEY value
const uint8_t linux_key_map[key_count] = {'W', 'A', 'S', 'D', 'Q', 'E', ' '};

bool init_input_system() {
  keyboard_file = open(keyboard_device, O_RDONLY | O_NONBLOCK);
  if (keyboard_file == 0) {
    return false;
  }

  return true;
}

void register_key_press_callback(Key key, KeyPressCallback callback) {
  (void)linux_key_map;
}

void poll_input() {
  int32_t bytes = read(keyboard_file, keyboard_event, sizeof(input_event *));
  if (bytes > 0) {
    if (keyboard_event->type & EV_KEY) {
    }
  }
}

void shutdown_input_system() {}

} // namespace raycaster

#endif