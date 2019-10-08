#include "input.hpp"
#include "renderer.hpp"
#include "window.hpp"

#include "map.hpp"
#include "map_1.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

#include <stdio.h>

namespace raycaster {

Window window = nullptr;

struct Player {
  float view_angle = 5.133181f;
  float pos_x = 1.5f;
  float pos_y = 4.5f;
  const float move_speed = 0.01f;
  const float turn_rate = 0.05f;
};

Player player = {};
Map *map = nullptr;

void on_turn_left() {
  player.view_angle += player.turn_rate;

  if (player.view_angle > M_PI * 2) {
    player.view_angle = 0;
  } else if (player.view_angle < 0) {
    player.view_angle = M_PI * 2 - player.view_angle;
  }
}

void on_turn_right() {
  player.view_angle -= player.turn_rate;

  if (player.view_angle > M_PI * 2) {
    player.view_angle = 0;
  } else if (player.view_angle < 0) {
    player.view_angle = M_PI * 2 - player.view_angle;
  }
}

void on_move_left() {
  float right_angle = player.view_angle - M_PI_2;
  float right_x = cosf(right_angle);
  float right_y = sinf(right_angle);

  player.pos_x -= right_x * player.move_speed;
  player.pos_y -= right_y * player.move_speed;

  if (player.pos_x < 0) {
    player.pos_x = 0;
  }
  if (player.pos_y < 0) {
    player.pos_y = 0;
  }
}

void on_move_right() {
  float right_angle = player.view_angle - M_PI_2;
  float right_x = cosf(right_angle);
  float right_y = sinf(right_angle);

  player.pos_x += right_x * player.move_speed;
  player.pos_y += right_y * player.move_speed;

  if (player.pos_x < 0) {
    player.pos_x = 0;
  }
  if (player.pos_y < 0) {
    player.pos_y = 0;
  }
}

void on_move_forward() {
  float forward_x = cosf(player.view_angle);
  float forward_y = sinf(player.view_angle);

  player.pos_x += forward_x * player.move_speed;
  player.pos_y += forward_y * player.move_speed;

  if (player.pos_x < 0) {
    player.pos_x = 0;
  }
  if (player.pos_y < 0) {
    player.pos_y = 0;
  }
}

void on_move_backward() {
  float forward_x = cosf(player.view_angle);
  float forward_y = sinf(player.view_angle);

  player.pos_x -= forward_x * player.move_speed;
  player.pos_y -= forward_y * player.move_speed;

  if (player.pos_x < 0) {
    player.pos_x = 0;
  }
  if (player.pos_y < 0) {
    player.pos_y = 0;
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
  register_key_press_callback(Key::W, on_move_forward);
  register_key_press_callback(Key::A, on_turn_left);
  register_key_press_callback(Key::S, on_move_backward);
  register_key_press_callback(Key::D, on_turn_right);
  register_key_press_callback(Key::Q, on_move_left);
  register_key_press_callback(Key::E, on_move_right);

  // Load map
  map = load_map_1();

  return true;
}

void update() {
  while (!window_should_close(window)) {
    poll_input();

    set_view_pos(player.pos_x, player.pos_y);
    set_view_angle(player.view_angle);

    printf("Pos: %f, %f ; View: %f\n", player.pos_x, player.pos_y,
           player.view_angle);

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