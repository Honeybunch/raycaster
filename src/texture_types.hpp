#pragma once

#include <stdint.h>

namespace raycaster {

// SUPER temporary; need a good texture alloc strategy
const uint32_t MAX_TEXTURES = 64;

struct texture_t;
using texture = texture_t *;

const texture INVALID_TEXTURE_HANDLE = nullptr;

} // namespace raycaster