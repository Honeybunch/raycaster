#include "map.hpp"

#include "texture.hpp"

#include <string.h> // for memset

namespace raycaster {

void destroy_map(Map *map) {
  // Destroy "thing" textures and reset the things array
  for (uint32_t i = 0; i < THING_TYPE_COUNT; ++i) {
    destroy_texture(map->thing_textures[i]);
    map->thing_textures[i] = INVALID_TEXTURE_HANDLE;
  }
  memset(map->things, 0, sizeof(Thing) * map->thing_count);

  // Destroy wall textures and map geometry
  for (uint32_t i = 0; i < WALL_TYPE_COUNT; ++i) {
    destroy_texture(map->wall_textures[i]);
    map->wall_textures[i] = INVALID_TEXTURE_HANDLE;
  }

  for (uint32_t i = 0; i < MAX_MAP_SIZE; ++i) {
    delete[] map->cells[i];
  }
  delete[] map->cells;
  map->cells = nullptr;
}

} // namespace raycaster