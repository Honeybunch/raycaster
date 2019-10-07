#pragma once

#include <stdint.h>

namespace raycaster {

bool init_renderer(uint32_t width, uint32_t height);

void set_view_pos(float x, float y);
void set_view_angle(float angle);

void render();

uint32_t get_buffer_size();
const uint8_t *get_render_target();

void shutdown_renderer();

} // namespace raycaster