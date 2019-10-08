#pragma once

#include "texture_types.hpp"

namespace raycaster {

enum class Cell : uint8_t {
  BLANK = 0,
  WALL_0 = 1,
  WALL_1,
  WALL_2,
  WALL_3,
  COUNT
};

const uint32_t WALL_TYPE_COUNT = static_cast<uint32_t>(Cell::COUNT) - 1;

const uint32_t MAX_MAP_SIZE = 512; // Max map size 512x512
const uint32_t MAX_MAP_INDEX = MAX_MAP_SIZE - 1;
const float MAX_MAP_VALUE = MAX_MAP_SIZE;

struct Map {
  texture textures[WALL_TYPE_COUNT] = {};
  Cell **cells = nullptr;
};

} // namespace raycaster