#include "map.hpp"

#include "texture.hpp"

namespace raycaster {

void destroy_map(Map *map) {
  for (uint32_t i = 0; i < WALL_TYPE_COUNT; ++i) {
    destroy_texture(map->textures[i]);
    map->textures[i] = INVALID_TEXTURE_HANDLE;
  }

  for (uint32_t i = 0; i < MAX_MAP_SIZE; ++i) {
    delete[] map->cells[i];
  }
  delete[] map->cells;
  map->cells = nullptr;
}

} // namespace raycaster