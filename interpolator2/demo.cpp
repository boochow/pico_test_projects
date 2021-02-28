#include <string.h>
#include <math.h>
#include <cstdlib>

#include "pico/stdlib.h"
#include "hardware/interp.h"
#include "pico_display.hpp"
#include "map.h"
uint8_t tiles[][256] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,2,2,5,5,5,5,5,2,5,5,2,5,2,2,1,
			 2,2,5,2,5,2,5,5,5,2,2,2,2,5,1,1,
			 2,2,5,5,2,5,2,5,2,1,5,5,2,2,1,2,
			 2,5,2,5,2,5,2,2,2,5,2,5,2,5,2,1,
			 2,5,5,2,5,2,1,2,2,1,1,2,5,2,1,2,
			 2,5,2,2,5,2,2,2,5,2,1,1,6,6,1,1,
			 2,5,5,2,2,1,5,2,2,1,1,1,6,6,1,1,
			 2,5,2,1,5,2,2,1,2,2,1,2,2,5,5,2,
			 2,5,2,5,2,5,2,2,1,5,2,1,5,5,2,1,
			 5,2,5,2,2,2,1,5,1,2,1,5,5,2,5,2,
			 2,2,2,2,2,2,2,1,2,1,5,5,5,2,2,2,
			 2,2,1,1,2,5,2,1,1,5,5,2,5,2,5,1,
			 2,1,2,1,1,2,1,2,2,5,2,5,2,5,2,2,
			 1,2,1,2,1,6,6,1,5,5,5,2,2,2,2,2,
			 2,2,1,2,1,6,6,1,2,5,2,5,2,5,2,1,
			 2,1,2,1,2,6,6,1,5,5,2,2,5,2,1,2},
			{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
			 3,3,3,3,3,3,3,3,3,3,3,3,7,3,3,3,
			 3,3,3,3,3,3,3,3,3,3,7,3,3,3,3,3,
			 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
			 3,3,3,3,7,3,3,3,3,3,3,3,3,3,3,3,
			 3,3,7,7,3,3,3,3,3,3,3,3,3,3,3,3,
			 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
			 3,3,3,3,3,3,7,3,3,3,7,3,3,3,3,3,
			 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
			 3,3,3,3,3,3,3,3,3,7,3,3,3,3,3,3,
			 3,3,3,3,3,3,3,7,3,3,3,3,3,3,3,3,
			 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
			 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
			 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
			 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
			 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
			{4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			 4,4,6,4,4,4,4,4,4,4,4,4,4,4,4,4,
			 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			 4,4,4,4,4,4,4,4,4,6,4,4,4,4,4,4,
			 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			 4,4,4,4,4,6,4,4,4,4,4,4,4,4,4,4,
			 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			 4,4,4,4,4,4,4,4,4,4,4,4,4,6,4,4,
			 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
			{2,5,5,5,2,5,5,4,5,5,2,5,5,5,5,5,
			 5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,5,
			 5,5,4,5,5,5,5,5,5,5,5,4,5,2,5,5,
			 5,5,5,5,5,5,5,5,2,5,5,5,5,5,5,5,
			 5,5,2,5,5,5,4,5,5,5,5,5,5,5,5,5,
			 5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,5,
			 4,5,5,5,5,5,5,5,4,5,5,2,5,5,5,5,
			 5,5,5,4,5,5,2,5,5,5,5,5,5,5,5,2,
			 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
			 5,5,5,5,5,5,5,5,5,2,5,5,4,5,5,5,
			 5,5,5,5,5,5,4,5,5,5,5,5,5,5,5,5,
			 5,4,5,5,2,5,5,5,5,5,5,5,5,2,5,5,
			 5,5,5,5,5,5,5,5,5,4,5,5,5,5,5,5,
			 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
			 5,2,5,5,5,4,5,5,5,5,2,5,5,5,4,5,
			 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
			{6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
			 1,6,6,6,6,6,6,6,4,4,6,6,6,6,6,4,
			 1,6,6,6,6,6,6,4,1,6,1,6,6,6,6,4,
			 6,1,6,6,6,6,4,6,6,6,1,1,6,6,6,4,
			 6,1,6,6,6,6,4,6,6,6,6,1,6,6,4,6,
			 1,6,1,6,6,4,6,6,6,4,6,1,1,6,4,6,
			 1,6,1,6,4,6,6,6,4,6,6,1,1,1,6,6,
			 6,1,6,4,6,6,6,4,6,6,6,1,6,1,6,6,
			 6,1,4,4,6,6,6,4,1,6,6,1,6,1,1,6,
			 1,6,4,6,6,6,6,6,1,1,1,6,1,1,1,6,
			 6,4,4,6,6,4,4,1,6,1,6,1,6,4,1,1,
			 6,6,6,6,4,6,6,1,1,6,1,6,4,1,1,1,
			 1,6,6,6,6,6,6,1,1,1,6,4,1,6,1,1,
			 1,6,6,6,6,6,1,6,6,1,4,1,6,1,1,6,
			 6,1,4,6,6,6,1,6,1,1,4,1,6,1,1,1,
			 1,1,4,6,6,6,1,1,6,1,1,6,1,6,1,1}};

void interp_setup(const uint8_t *map, uint map_width_bits, uint map_height_bits, uint uv_fractional_bits, uint tile_width_bits, uint tile_height_bits) {
    interp_config cfg = interp_default_config();

    interp_config_set_add_raw(&cfg, true);
    interp_config_set_shift(&cfg, uv_fractional_bits);
    interp_config_set_mask(&cfg, 0, map_width_bits - 1);
    interp_set_config(interp0, 0, &cfg);

    interp_config_set_shift(&cfg, uv_fractional_bits - map_width_bits);
    interp_config_set_mask(&cfg, map_width_bits, map_width_bits + map_height_bits - 1);
    interp_set_config(interp0, 1, &cfg);

    interp0->base[2] = (uintptr_t) map;

    
    interp_config_set_add_raw(&cfg, true);
    interp_config_set_shift(&cfg, uv_fractional_bits - tile_width_bits);
    interp_config_set_mask(&cfg, 0, tile_width_bits - 1);
    interp_set_config(interp1, 0, &cfg);

    interp_config_set_shift(&cfg, uv_fractional_bits - tile_height_bits - tile_width_bits);
    interp_config_set_mask(&cfg, tile_width_bits, tile_height_bits + tile_width_bits - 1);
    interp_set_config(interp1, 1, &cfg);

    interp1->base[2] = 0;
}

using namespace pimoroni;

uint16_t buffer[PicoDisplay::WIDTH * PicoDisplay::HEIGHT];
PicoDisplay pico_display(buffer);

const uint16_t dark_grey = pico_display.create_pen(20, 40, 60);
const uint16_t black = pico_display.create_pen(0, 0, 0);
const uint16_t dark_green = pico_display.create_pen(32, 128, 0);
const uint16_t lt_blue = pico_display.create_pen(66, 133, 244);
const uint16_t lt_yellow = pico_display.create_pen(255, 229, 153);
const uint16_t lt_green = pico_display.create_pen(0, 255, 0);
const uint16_t brown = pico_display.create_pen(120, 63, 4);
const uint16_t white = pico_display.create_pen(255, 255, 255);
const uint16_t sky_blue = pico_display.create_pen(96, 192, 255);

const uint16_t colors[] = {dark_grey, black, dark_green, lt_blue, lt_yellow, lt_green, brown, white};

const uint16_t bg_color = dark_grey;

struct camera {
  int32_t x;
  int32_t y;
  int32_t z;
  int32_t screen_distance; 
  int rot;
};

camera camera = {128<<16, 128<<16, 3<<16, 200, 0};

void map_fill_line(uint16_t *output, uint32_t u, uint32_t v, uint32_t du, uint32_t dv, uint count) {
  interp0->accum[0] = u;
  interp0->base[0] = du;
  interp0->accum[1] = v;
  interp0->base[1] = dv;

  interp1->accum[0] = u;
  interp1->base[0] = du;
  interp1->accum[1] = v;
  interp1->base[1] = dv;

  for (uint i = 0; i < count; i++) {
    uint8_t t = *(uint8_t *) interp0->pop[2];
    uint8_t c = interp1->pop[2];
    output[i] = colors[tiles[t][c]];
  }
}

void fill_buffer(int w, int h) {
  interp_setup(map, map_width, map_height, 16, 4, 4);

  float rcos = cos(camera.rot / 180.0 * M_PI);
  float rsin = sin(camera.rot / 180.0 * M_PI);
  for(uint32_t q = 1 ; q < h ; q++) {
    int32_t n = camera.z / q;
    int32_t s = camera.screen_distance * n;
    int32_t t2 = w * n;
    int32_t x = camera.x  + t2 / 2 * rcos + s * rsin;
    int32_t y = camera.y  + (-t2 / 2) * rsin + s * rcos;
    int32_t du = -rcos * n;
    int32_t dv = rsin * n;
    if (s > (200<<16)) {
      continue;
    }
    map_fill_line(&buffer[q * PicoDisplay::WIDTH], x, y, du, dv, w);
  }
}

int main() {
  float speed = 1.0;
  int jump_count = 0;
  
  stdio_init_all();
  pico_display.init();
  pico_display.set_backlight(80);
  
  while(true) {
    pico_display.set_pen(sky_blue);
    pico_display.clear();
    fill_buffer(PicoDisplay::WIDTH, PicoDisplay::HEIGHT);
    pico_display.update();

    int32_t step = (speed - 1) * 65536;
    camera.x += step * sin(M_PI * camera.rot / 180.0);
    camera.y += step * cos(M_PI * camera.rot / 180.0);

    if (jump_count > 0) {
      if (jump_count < 50) {
	camera.z += (1 + (jump_count++ / 5)) << 13;
      } else if (jump_count < 99) {
	camera.z -= (1 + ((jump_count++ - 49) / 5)) << 13;
      } else {
	jump_count = 0;
      }
    }
    
    if (pico_display.is_pressed(pico_display.Y)) {
      camera.rot -= 5;
    }
    if (pico_display.is_pressed(pico_display.B)) {
      camera.rot += 5;
    }
    if (camera.rot > 360) {
      camera.rot -= 360;
    } else if (camera.rot < 0) {
      camera.rot += 360;
    }

    if (pico_display.is_pressed(pico_display.A)) {
      if (jump_count == 0) {
	jump_count = 1;
      }
    }
  
    if (pico_display.is_pressed(pico_display.X)) {
      speed = speed * 1.02;
    } else {
      speed = speed * 0.98;
      if (speed < 1.0) {
	speed = 1.0;
      }
    }
    sleep_ms(32);
  }
  return 0;
}
