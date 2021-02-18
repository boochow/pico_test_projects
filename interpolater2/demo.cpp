#include <string.h>
#include <math.h>
#include <cstdlib>

#include "pico/stdlib.h"
#include "hardware/interp.h"
#include "pico_display.hpp"

constexpr float scale_min = 0.0078125;
constexpr float scale_max = 1.0;
constexpr float skew_min = -0.5;
constexpr float skew_max = 0.5;
constexpr float skew_step = 0.015625;

uint8_t pi_logo[] = {0,0,0,1,1,1,1,0,1,1,1,1,0,0,0,0,
		     0,0,1,2,2,2,1,1,1,2,2,2,1,0,0,0,
		     0,0,1,2,2,1,2,1,2,1,2,2,1,0,0,0,
		     0,0,0,1,2,2,1,1,1,2,2,1,0,0,0,0,
		     0,0,0,0,1,2,1,3,1,2,1,0,0,0,0,0,
		     0,0,0,1,3,1,3,3,3,1,3,1,0,0,0,0,
		     0,0,1,3,3,3,3,3,3,3,3,3,1,0,0,0,
		     0,1,1,3,3,4,3,4,3,4,3,3,1,1,0,0,
		     0,1,3,3,4,4,4,4,4,4,4,3,3,1,0,0,
		     0,1,3,3,4,4,4,4,4,4,4,3,3,1,0,0,
		     0,1,1,3,4,1,4,4,4,1,4,3,1,1,0,0,
		     0,0,1,4,4,4,4,4,4,4,4,4,1,0,0,0,
		     0,0,1,1,4,4,3,3,3,4,4,1,1,0,0,0,
		     0,0,0,1,1,4,4,4,4,4,1,1,0,0,0,0,
		     0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
		     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

uint8_t color_balls[][64] = {
			  {0,1,0,0,0,1,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,1,0,0,0,1,
			   0,0,0,0,0,0,0,0,
			   0,1,0,0,0,1,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,1,0,0,0,1,
			   0,0,0,0,0,0,0,0},
			  
			  {0,0,0,1,1,1,0,0,
			   0,0,1,0,1,1,1,0,
			   0,1,0,1,1,1,1,1,
			   0,1,0,1,1,1,1,1,
			   0,1,1,1,1,1,1,1,
			   0,0,1,1,1,1,1,0,
			   0,0,0,1,1,1,0,0,
			   0,0,0,0,0,0,0,0},
			  
			  {0,0,0,2,2,2,0,0,
			   0,0,2,0,2,2,2,0,
			   0,2,0,2,2,2,2,2,
			   0,2,0,2,2,2,2,2,
			   0,2,2,2,2,2,2,2,
			   0,0,2,2,2,2,2,0,
			   0,0,0,2,2,2,0,0,
			   0,0,0,0,0,0,0,0},
			  
			  {0,0,0,3,3,3,0,0,
			   0,0,3,0,3,3,3,0,
			   0,3,0,3,3,3,3,3,
			   0,3,0,3,3,3,3,3,
			   0,3,3,3,3,3,3,3,
			   0,0,3,3,3,3,3,0,
			   0,0,0,3,3,3,0,0,
			   0,0,0,0,0,0,0,0},
			  
			  {0,0,0,4,0,0,0,0,
			   0,0,0,4,0,0,0,0,
			   0,0,0,4,0,0,0,0,
			   0,0,0,4,0,0,0,0,
			   0,0,0,4,0,0,0,0,
			   0,0,0,4,0,0,0,0,
			   0,0,0,4,0,0,0,0,
			   0,0,0,0,0,0,0,0}};
/*			  
			  {0,0,0,4,4,4,0,0,
			   0,0,4,0,4,4,4,0,
			   0,4,0,4,4,4,4,4,
			   0,4,0,4,4,4,4,4,
			   0,4,4,4,4,4,4,4,
			   0,0,4,4,4,4,4,0,
			   0,0,0,4,4,4,0,0,
			   0,0,0,0,0,0,0,0}};
*/
void texture_mapping_setup(uint8_t *texture, uint texture_width_bits, uint texture_height_bits, uint uv_fractional_bits, uint tile_width_bits, uint tile_height_bits) {
    interp_config cfg = interp_default_config();

    interp_config_set_add_raw(&cfg, true);
    interp_config_set_shift(&cfg, uv_fractional_bits);
    interp_config_set_mask(&cfg, 0, texture_width_bits - 1);
    interp_set_config(interp0, 0, &cfg);

    interp_config_set_shift(&cfg, uv_fractional_bits - texture_width_bits);
    interp_config_set_mask(&cfg, texture_width_bits, texture_width_bits + texture_height_bits - 1);
    interp_set_config(interp0, 1, &cfg);

    interp0->base[2] = (uintptr_t) texture;

    
    interp_config_set_add_raw(&cfg, true);
    interp_config_set_shift(&cfg, uv_fractional_bits - tile_width_bits);
    interp_config_set_mask(&cfg, 0, tile_width_bits - 1);
    interp_set_config(interp1, 0, &cfg);

    interp_config_set_shift(&cfg, uv_fractional_bits - tile_width_bits - tile_height_bits);
    interp_config_set_mask(&cfg, tile_width_bits, tile_width_bits + tile_height_bits - 1);
    interp_set_config(interp1, 1, &cfg);

    interp1->base[2] = 0;
}

using namespace pimoroni;

uint16_t buffer[PicoDisplay::WIDTH * PicoDisplay::HEIGHT];
PicoDisplay pico_display(buffer);

const uint16_t black = pico_display.create_pen(0, 0, 0);
const uint16_t dark_grey = pico_display.create_pen(20, 40, 60);
const uint16_t rp_leaf = pico_display.create_pen(107,192,72);
const uint16_t rp_berry = pico_display.create_pen(196,25,73);
const uint16_t rp_skin = pico_display.create_pen(246,178,107);
const uint16_t colors[] = {dark_grey, black, rp_leaf, rp_berry, rp_skin};

const uint16_t bg_color = dark_grey;

void texture_fill_line(uint16_t *output, uint32_t u, uint32_t v, uint32_t du, uint32_t dv, uint count) {
  interp0->accum[0] = u;
  interp0->base[0] = du;
  interp0->accum[1] = v;
  interp0->base[1] = dv;

  interp1->accum[0] = u;
  interp1->base[0] = du;
  interp1->accum[1] = v;
  interp1->base[1] = dv;

  uint k = (v >> 10) & 0x38;

  for (uint i = 0; i < count; i++) {
    uint8_t t = *(uint8_t *) interp0->pop[2];
    uint8_t c = *(uint8_t *) interp1->pop[2];
    output[i] = colors[color_balls[t][k + (c & 7)]];
  }
}

void fill_buffer(int w, int h, float xscale, float yscale, float skew) {
  texture_mapping_setup(pi_logo, 4, 4, 16, 3, 3);
  uint32_t dx = 65536 * xscale;
  uint32_t dy = 65536 * yscale;
  int32_t s = 65536 * skew;
  for(int l = 0 ; l < h ; l++) {
    texture_fill_line(&buffer[l * PicoDisplay::WIDTH], 0, l * dy, dx, s, w);
  }
}

float xscale;
float d_xscale;
float yscale;
float d_yscale;
float skew;
float d_skew;
  
void init_params() {
  xscale = 0.125;
  d_xscale = 0.8;
  yscale = 0.125;
  d_yscale = 0.8;
  skew = 0;
  d_skew = skew_step;
  xscale = 0.0625;
  yscale = 0.0625;
}

void step_skew() {
  skew += d_skew;
  if (skew < skew_min) {
    skew = skew_min;
    d_skew = skew_step;
  } else if  (skew > skew_max) {
    skew = skew_max;
    d_skew = -skew_step;
  }
}

void step_xscale() {
  xscale = xscale * d_xscale;
  if (xscale < scale_min) {
    xscale = scale_min;
    d_xscale = 1.25;
  } else if (xscale > scale_max) {
    xscale = scale_max;
    d_xscale = 0.8;
  }
}

void step_yscale() {
  yscale = yscale * d_yscale;
  if (yscale < scale_min) {
    yscale = scale_min;
    d_yscale = 1.25;
  } else if (yscale > scale_max) {
    yscale = scale_max;
    d_yscale = 0.8;
  }
}

int main() {
  stdio_init_all();
  pico_display.init();
  pico_display.set_backlight(80);
  init_params();
  
  while(true) {
    pico_display.clear();
    fill_buffer(PicoDisplay::WIDTH, PicoDisplay::HEIGHT, xscale, yscale, skew);
    pico_display.update();

    if (pico_display.is_pressed(pico_display.X)) {
      step_skew();
    }
    if (pico_display.is_pressed(pico_display.Y)) {
      init_params();
    }
    if (pico_display.is_pressed(pico_display.A)) {
      step_xscale();
    }
    if (pico_display.is_pressed(pico_display.B)) {
      step_yscale();
    }
  }
  return 0;
}
