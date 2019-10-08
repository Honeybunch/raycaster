#pragma once

#include <stdint.h>

#include "map_types.hpp"

namespace raycaster {

bool init_renderer(uint32_t width, uint32_t height);

void set_view_pos(float x, float y);
void set_view_angle(float angle);

void render(const Map *map);

uint32_t get_buffer_size();
const uint8_t *get_render_target();

void shutdown_renderer();

} // namespace raycaster