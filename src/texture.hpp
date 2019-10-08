#pragma once

#include "texture_types.hpp"

namespace raycaster {

texture load_texture(const char *filepath);

const uint8_t *texture_get_image(texture t);
int32_t texture_get_width(texture t);
int32_t texture_get_height(texture t);
int32_t texture_get_channels(texture t);

void destroy_texture(texture t);

} // namespace raycaster