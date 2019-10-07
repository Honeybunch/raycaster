#ifdef __linux__

#include "input.hpp"

namespace raycaster {

void register_key_press_callback(Key key, KeyPressCallback callback) {}

void poll_input() {}

} // namespace raycaster

#endif