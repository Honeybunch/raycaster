#ifdef __linux__

#include "window.hpp"

namespace raycaster {

struct Window_t {
  int handle;
};

bool init_window_system() { return true; }

bool create_window(const struct WindowDescriptor *desc, Window *window) {
  return true;
}

bool window_should_close(Window window) { return false; }

void present_window(Window window, const uint8_t *framebuffer,
                    uint32_t framebuffer_size) {}

void shutdown_window_system() {}

} // namespace raycaster

#endif