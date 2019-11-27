#include "float2.hpp"
#include "renderer.hpp"
#include "window.hpp"

#include "map.hpp"
#include "map_1.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

#include <assert.h>
#include <stdio.h>

namespace raycaster {

const float pi_f = float(M_PI);
const float pi_2_f = float(M_PI_2);

Window window = nullptr;

struct Player {
  float view_angle = 4.7f;
  float2 pos = {3.0f, 5.5f};
  float move_speed = 0.01f;
  float turn_rate = 0.005f;
};

Player player = {};
Map *map = nullptr;

bool pos_intersects_wall(float2 pos, Map *cur_map) {
  assert(cur_map);

  uint32_t x = (uint32_t)pos.x;
  uint32_t y = (uint32_t)pos.y;

  return cur_map->cells[x][y] != Cell::BLANK;
}

void on_turn_left() {
  player.view_angle += player.turn_rate;

  if (player.view_angle > pi_f * 2.0f) {
    player.view_angle = 0.0f;
  } else if (player.view_angle < 0.0f) {
    player.view_angle = pi_f * 2.0f - player.view_angle;
  }
}

void on_turn_right() {
  player.view_angle -= player.turn_rate;

  if (player.view_angle > pi_f * 2.0f) {
    player.view_angle = 0.0f;
  } else if (player.view_angle < 0.0f) {
    player.view_angle = pi_f * 2.0f - player.view_angle;
  }
}

void on_move_left() {
  float right_angle = player.view_angle - pi_2_f;
  float2 right = {cosf(right_angle), sinf(right_angle)};

  float2 next_pos =
      float2_sub(player.pos, float2_mul(right, player.move_speed));

  if (pos_intersects_wall(next_pos, map)) {
    // Place the player next to the wall but not through it
  } else {
    player.pos = next_pos;
  }

  if (player.pos.x < 0.0f) {
    player.pos.x = 0.0f;
  }
  if (player.pos.y < 0.0f) {
    player.pos.y = 0.0f;
  }
}

void on_move_right() {
  float right_angle = player.view_angle - pi_2_f;
  float2 right = {cosf(right_angle), sinf(right_angle)};

  float2 next_pos =
      float2_add(player.pos, float2_mul(right, player.move_speed));

  if (pos_intersects_wall(next_pos, map)) {
    // Place the player next to the wall but not through it
  } else {
    player.pos = next_pos;
  }

  if (player.pos.x < 0.0f) {
    player.pos.x = 0.0f;
  }
  if (player.pos.y < 0.0f) {
    player.pos.y = 0.0f;
  }
}

void on_move_forward() {
  float2 forward = {cosf(player.view_angle), sinf(player.view_angle)};

  float2 next_pos =
      float2_add(player.pos, float2_mul(forward, player.move_speed));

  if (pos_intersects_wall(next_pos, map)) {
    // Place the player next to the wall but not through it
  } else {
    player.pos = next_pos;
  }

  if (player.pos.x < 0.0f) {
    player.pos.x = 0.0f;
  }
  if (player.pos.y < 0.0f) {
    player.pos.y = 0.0f;
  }
}

void on_move_backward() {
  float2 forward = {cosf(player.view_angle), sinf(player.view_angle)};

  float2 next_pos =
      float2_sub(player.pos, float2_mul(forward, player.move_speed));

  if (pos_intersects_wall(next_pos, map)) {
    // Place the player next to the wall but not through it
  } else {
    player.pos = next_pos;
  }

  if (player.pos.x < 0.0f) {
    player.pos.x = 0.0f;
  }
  if (player.pos.y < 0.0f) {
    player.pos.y = 0.0f;
  }
}

bool start() {
  uint32_t width = 1280;
  uint32_t height = 720;

  WindowDescriptor window_desc = {width, height};

  if (!init_window_system()) {
    return false;
  }

  if (!create_window(&window_desc, &window)) {
    return false;
  }

  if (!init_renderer(width, height)) {
    return false;
  }

  // register input callbacks
  register_key_press_callback(window, Key::W, on_move_forward);
  register_key_press_callback(window, Key::A, on_turn_left);
  register_key_press_callback(window, Key::S, on_move_backward);
  register_key_press_callback(window, Key::D, on_turn_right);
  register_key_press_callback(window, Key::Q, on_move_left);
  register_key_press_callback(window, Key::E, on_move_right);

  // Load map
  map = load_map_1();

  return true;
}

void update() {
  while (!window_should_close(window)) {
    pump_window(window);

    renderer_set_view(player.pos, player.view_angle);

    render(map);

    uint32_t buffer_size = get_buffer_size();
    const uint8_t *buffer = get_render_target();

    present_window(window, buffer, buffer_size);
  }
}

void end() {
  destroy_map(map);
  shutdown_renderer();
  shutdown_window_system();
}

} // namespace raycaster

int main() {
  if (!raycaster::start()) {
    return -1;
  }

  raycaster::update();

  raycaster::end();

  return 0;
}