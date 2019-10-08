#include "texture.hpp"

#include "stb_image.h"

#include <assert.h>

namespace raycaster {

struct texture_t {
  uint8_t *image = nullptr;
  int32_t width = 0;
  int32_t height = 0;
  int32_t channels = 0;
};

texture_t textures[MAX_TEXTURES] = {};
uint32_t texture_count = 0;

texture load_texture(const char *filepath) {
  assert(filepath);
  if (texture_count >= MAX_TEXTURES) {
    assert(false);
    return INVALID_TEXTURE_HANDLE;
  }

  texture t = &textures[texture_count];
  texture_count++;

  const int32_t desired_channels = STBI_rgb_alpha;

  t->image = stbi_load(filepath, &t->width, &t->height, &t->channels,
                       desired_channels);

  assert(t->image);

  return t;
}

const uint8_t *texture_get_image(texture t) {
  assert(t);
  return t->image;
}

int32_t texture_get_width(texture t) {
  assert(t);
  return t->width;
}

int32_t texture_get_height(texture t) {
  assert(t);
  return t->height;
}

int32_t texture_get_channels(texture t) {
  assert(t);
  return t->channels;
}

uint32_t texture_get_size(texture t) {
  assert(t);
  return t->width * t->height * t->channels;
}

void destroy_texture(texture t) {
  if (t == INVALID_TEXTURE_HANDLE) {
    return;
  }

  stbi_image_free(t->image);

  t->image = nullptr;
  t->width = 0;
  t->height = 0;
  t->channels = 0;
}

} // namespace raycaster