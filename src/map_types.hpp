#pragma once

#include "float2.hpp"
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

enum class ThingType : uint8_t { PICKUP_0, PICKUP_1, ENEMY_0, ENEMY_1, COUNT };

struct Thing {
  ThingType type = {};
  float2 pos = {};
};

const uint32_t WALL_TYPE_COUNT = static_cast<uint32_t>(Cell::COUNT) - 1;
const uint32_t THING_TYPE_COUNT = static_cast<uint32_t>(ThingType::COUNT);

const uint32_t MAX_MAP_SIZE = 512; // Max map size 512x512
const uint32_t MAX_MAP_INDEX = MAX_MAP_SIZE - 1;
const float MAX_MAP_VALUE = MAX_MAP_SIZE;

const uint32_t MAX_THINGS = 512;

struct Map {
  texture wall_textures[WALL_TYPE_COUNT] = {};
  Cell **cells = nullptr;

  texture thing_textures[THING_TYPE_COUNT] = {};
  Thing things[MAX_THINGS] = {};
  uint32_t thing_count = 0;
};

} // namespace raycaster