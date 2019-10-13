#include "renderer.hpp"

#include "float2.hpp"
#include "texture.hpp"

#include <assert.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>

namespace raycaster {

uint32_t width = 0;
uint32_t height = 0;
const uint32_t bytes_pp = 4;

// Stores the wall height at each column
// Used during draw_things to determine if a column needs to be clipped
uint32_t *occlusion_buffer = nullptr;

uint32_t backbuffer_size = 0;
uint8_t *backbuffer = nullptr;

float2 view_pos = {0, 0};
float view_angle = 0.0f;

uint32_t pack_rgb(uint8_t r, uint8_t g, uint8_t b) {
  return 0xFF000000 + (r << 16) + (g << 8) + b;
}

uint32_t pack_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return (a << 24) + (r << 16) + (g << 8) + b;
}

void set_pixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b) {
  assert(x < width);
  assert(y < height);

  uint32_t i = (y * height) + x;
  uint32_t *backbuffer_rgba = reinterpret_cast<uint32_t *>(backbuffer);

  backbuffer_rgba[i] = pack_rgb(r, g, b);
}

void draw_floor() {
  uint32_t floor_rgb = pack_rgb(50, 50, 50);
  uint32_t *backbuffer_rgba = reinterpret_cast<uint32_t *>(backbuffer);

  const uint32_t half_size = (width * height) / 2;
  for (uint32_t i = 0; i < half_size; ++i) {
    backbuffer_rgba[i] = floor_rgb;
  }
}

void draw_ceiling() {
  uint32_t ceiling_rgb = pack_rgb(150, 150, 255);
  uint32_t *backbuffer_rgba = reinterpret_cast<uint32_t *>(backbuffer);

  const uint32_t half_size = width * height / 2;
  for (uint32_t i = half_size; i < width * height; ++i) {
    backbuffer_rgba[i] = ceiling_rgb;
  }
}

uint32_t calc_column_height(float view_angle, float view_x, float view_y,
                            float x_intercept, float y_intercept) {
  // To avoid a fisheye effect we avoid a formula like:
  // height = sqrt(sq(x2 - x1) + sq(y2 - y1))
  // And opt instead for:
  // height = ((x2 - x1) * cos(view_angle)) - ((y2 -y1) * sin(view_angle))

  float dx = x_intercept - view_x;
  float dy = y_intercept - view_y;

  float proj_dx = dx * cosf(view_angle);
  float proj_dy = dy * sinf(view_angle);

  float dist = fabsf(proj_dx + proj_dy);

  static const float min_dist = 0.1f;
  static const float scale_factor = height * 0.1f;

  if (dist < min_dist) {
    dist = min_dist;
  }

  float column_height = scale_factor / dist;

  return static_cast<uint32_t>(column_height);
}

void draw_column(uint32_t column_idx, uint32_t column_height, uint32_t color) {
  if (column_height > height) {
    column_height = height;
  }

  const uint32_t half_height = (height * 0.5f) * width;
  const uint32_t half_column_height = column_height * 0.5f;

  uint32_t *backbuffer_rgbx = reinterpret_cast<uint32_t *>(backbuffer);

  for (uint32_t i = 0; i < half_column_height; ++i) {
    uint32_t offset = (i * width);

    uint32_t upper_idx = half_height + offset + column_idx;
    uint32_t lower_idx = half_height - offset + column_idx;

    backbuffer_rgbx[upper_idx] = color;
    backbuffer_rgbx[lower_idx] = color;
  }
}

void draw_column(uint32_t column_idx, uint32_t column_height, float intersect,
                 texture tex) {
  if (column_height > height) {
    column_height = height;
  }

  const uint32_t tex_width = texture_get_width(tex);
  const uint32_t tex_height = texture_get_height(tex);
  const uint32_t tex_size = texture_get_size(tex);

  // Want to select column of texture based on where we intersected the wall
  // This way we will repeat some columns so the texture fills the entire wall.
  const uint32_t tex_column = (intersect - uint32_t(intersect)) * tex_width;

  const uint8_t *tex_img = texture_get_image(tex);

  const uint32_t half_height = height * 0.5f;
  const uint32_t half_column_height = column_height * 0.5f;
  const uint32_t start_height_idx = (half_height - half_column_height) * width;

  uint32_t *backbuffer_rgbx = reinterpret_cast<uint32_t *>(backbuffer);

  float tex_y_factor = float(tex_height) / float(column_height);

  for (uint32_t i = 0; i < column_height; ++i) {
    uint32_t backbuffer_idx = start_height_idx + (i * width) + column_idx;

    // Need to scale texture on Y to match column height
    uint32_t tex_y = uint32_t(i * tex_y_factor) % tex_height;

    // * 4 because the texture is assumed to either have 4
    // channels or have padding.
    uint32_t tex_idx = ((tex_y * tex_width) + tex_column) * 4;

    const uint8_t r = tex_img[tex_idx + 0];
    const uint8_t g = tex_img[tex_idx + 1];
    const uint8_t b = tex_img[tex_idx + 2];
    const uint8_t a = tex_img[tex_idx + 3];

    backbuffer_rgbx[backbuffer_idx] = pack_rgba(r, g, b, a);
  }

  int i = 0;
}

void draw_world(const Map *map) {
  const float fov = M_PI_2; // 90 degree FoV
  const uint32_t ray_count = width;

  const float view_angle_start = view_angle + (fov * 0.5f);
  const float view_angle_itr = fov / static_cast<float>(ray_count);

  const float view_angle_cos = cosf(view_angle);
  const float view_angle_sin = sinf(view_angle);

  // For every column of pixels, shoot out a ray from the player
  for (uint32_t i = 0; i < ray_count; ++i) {
    const float ray_angle = view_angle_start - (view_angle_itr * i);

    const float ray_angle_cos = cosf(ray_angle);
    const float ray_angle_sin = sinf(ray_angle);

    // Want to march a ray though the grid
    // Instead of keeping track of each intersection point's x and y coordinate
    // think of marching the ray as extending its length
    // *how much* in the direction of the ray do we need to march before we
    // intersect the grid?

    // Next we need to find how much to extend the rays each iteration
    float x_intersect_step = fabsf(1 / ray_angle_cos);
    float y_intersect_step = fabsf(1 / ray_angle_sin);

    // Need to find starting length
    // Note that we calcuate the length to the first intersection
    // What if that intersection is a collision?
    // We use the ray length to inform how we iterate over the map cells.
    // If we find a colliding cell we simply walk the ray back one length to
    // find the actual intersection point.
    float x_intersect_len = 0.0f;
    float y_intersect_len = 0.0f;

    // How much do we step when just counting cells?
    int32_t cell_step_x = 1.0f;
    int32_t cell_step_y = 1.0f;

    // Are we facing positive or negative along the X axis?
    if (ray_angle_cos > 0) {
      x_intersect_len = ((uint32_t(view_pos.x) + 1) - view_pos.x);
    } else {
      x_intersect_len = (uint32_t(view_pos.x) - view_pos.x);
      cell_step_x = -1.0f;
    }
    x_intersect_len = fabsf(x_intersect_len / ray_angle_cos);

    // Are we facing positive or negative along the Y axis?
    if (ray_angle_sin > 0) {
      y_intersect_len = ((uint32_t(view_pos.y) + 1) - view_pos.y);
    } else {
      y_intersect_len = (uint32_t(view_pos.y) - view_pos.y);
      cell_step_y = -1.0f;
    }
    y_intersect_len = fabsf(y_intersect_len / ray_angle_sin);

    uint32_t cell_x = uint32_t(view_pos.x);
    uint32_t cell_y = uint32_t(view_pos.y);

    uint8_t side = 0; // 0 for X, 1, for Y

    // March ray via DDA
    // Note that while we march the ray, alternating on X and Y axis comparison
    // lengths, we also step a cell counter. This helps keep track of if we hit
    // a wall cell but collided with its positive side.
    Cell cell = Cell::BLANK;
    while (true) {
      if (x_intersect_len < y_intersect_len) {
        x_intersect_len += x_intersect_step;
        cell_x += cell_step_x;
        side = 0;
      } else {
        y_intersect_len += y_intersect_step;
        cell_y += cell_step_y;
        side = 1;
      }

      if (cell_x < 0) {
        cell_x = 0;
      } else if (cell_x > MAX_MAP_INDEX) {
        cell_x = MAX_MAP_INDEX;
      }
      if (cell_y < 0) {
        cell_y = 0;
      } else if (cell_y > MAX_MAP_INDEX) {
        cell_y = MAX_MAP_INDEX;
      }

      if (cell_x == 0 || cell_y == 0 || cell_x == MAX_MAP_INDEX ||
          cell_y == MAX_MAP_INDEX) {
        break;
      }

      cell = map->cells[cell_x][cell_y];
      if (cell != Cell::BLANK) {
        break;
      }
    }

    float intersect_x = 0.0f;
    float intersect_y = 0.0f;

    if (side == 0) {
      // Step back once
      x_intersect_len -= x_intersect_step;

      // Find actual intersection point
      intersect_x = ray_angle_cos * x_intersect_len;
      intersect_y = ray_angle_sin * x_intersect_len;
    } else {
      // Step back once
      y_intersect_len -= y_intersect_step;

      // Find actual intersection point
      intersect_x = ray_angle_cos * y_intersect_len;
      intersect_y = ray_angle_sin * y_intersect_len;
    }

    intersect_x += view_pos.x;
    intersect_y += view_pos.y;

    uint32_t height = calc_column_height(view_angle, view_pos.x, view_pos.y,
                                         intersect_x, intersect_y);

    occlusion_buffer[i] = height;

    // We have reached the end of the world
    // Draw a color
    if (cell == Cell::BLANK) {
      draw_column(i, height, pack_rgb(50, 10, 10));
    } else {
      texture wall_texture = map->wall_textures[uint32_t(cell) - 1];

      if (side == 0) {
        draw_column(i, height, intersect_y, wall_texture);
      } else {
        draw_column(i, height, intersect_x, wall_texture);
      }
    }
  }
}

void draw_things(const Map *map) {}

bool init_renderer(uint32_t w, uint32_t h) {
  width = w;
  height = h;

  backbuffer_size = width * height * bytes_pp;
  backbuffer = new uint8_t[backbuffer_size];

  occlusion_buffer = new uint32_t[width];

  return true;
}

void renderer_set_view(float2 pos, float angle) {
  view_pos = pos;
  view_angle = angle;
}

void render(const Map *map) {
  assert(occlusion_buffer);

  draw_floor();
  draw_ceiling();
  draw_world(map);
  draw_things(map);
}

uint32_t get_buffer_size() { return backbuffer_size; }
const uint8_t *get_render_target() { return backbuffer; }

void shutdown_renderer() {
  width = 0;
  height = 0;

  delete[] backbuffer;
  backbuffer = nullptr;

  delete[] occlusion_buffer;
  occlusion_buffer = nullptr;
}

} // namespace raycaster