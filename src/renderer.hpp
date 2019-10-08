#pragma once

#include "float2.hpp"
#include "map_types.hpp"

#include <stdint.h>

namespace raycaster {

bool init_renderer(uint32_t width, uint32_t height);

void renderer_set_view(float2 view_pos, float angle);

void render(const Map *map);

uint32_t get_buffer_size();
const uint8_t *get_render_target();

void shutdown_renderer();

} // namespace raycaster