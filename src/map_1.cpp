#include "map_1.hpp"

#include "texture.hpp"

#include <string.h>

namespace raycaster {

Map map_1 = {};

Map *load_map_1() {
  // Load textures
  map_1.textures[0] = load_texture("assets/textures/ice.png");
  map_1.textures[1] = load_texture("assets/textures/rock.png");
  map_1.textures[2] = load_texture("assets/textures/sand.png");
  map_1.textures[3] = load_texture("assets/textures/snow.png");

  // Setup map
  map_1.cells = new Cell *[MAX_MAP_SIZE];
  for (uint32_t i = 0; i < MAX_MAP_SIZE; ++i) {
    map_1.cells[i] = new Cell[MAX_MAP_SIZE];
    memset(map_1.cells[i], 0, sizeof(Cell) * MAX_MAP_SIZE);
  }

  // Init dummy map data
  map_1.cells[2][2] = Cell::WALL_0;
  map_1.cells[2][3] = Cell::WALL_1;
  map_1.cells[3][2] = Cell::WALL_2;
  map_1.cells[3][3] = Cell::WALL_3;

  return &map_1;
}

} // namespace raycaster