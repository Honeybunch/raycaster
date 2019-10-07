#ifdef __linux__

#include "window.hpp"

struct Window {
  int handle;
};

bool create_window(const struct WindowDescriptor *desc, Window *window) {
  return true;
}

#endif