#include <string.h>
#include <math.h>
#include <cstdlib>

#include "pico/stdlib.h"
#include "hardware/interp.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"
#include "hardware/irq.h"
#include "map.h"

// scanning for the button states
// this code is from pico-playground/apps/popcorn

#define BUTTON_LEFT 1
#define BUTTON_MIDDLE 2
#define BUTTON_RIGHT 4

volatile uint32_t button_state = 0;
static const uint button_pins[] = {0, 6, 11};

const uint VSYNC_PIN = PICO_SCANVIDEO_COLOR_PIN_BASE + PICO_SCANVIDEO_COLOR_PIN_COUNT + 1;

// set pins to input. On deassertion, sample and set back to output.
void vga_board_button_irq_handler() {
    int vsync_current_level = gpio_get(VSYNC_PIN);
    gpio_acknowledge_irq(VSYNC_PIN, vsync_current_level ? GPIO_IRQ_EDGE_RISE : GPIO_IRQ_EDGE_FALL);

    // Note v_sync_polarity == 1 means active-low because anything else would be confusing
    if (vsync_current_level != scanvideo_get_mode().default_timing->v_sync_polarity) {
        for (int i = 0; i < count_of(button_pins); ++i) {
            gpio_pull_down(button_pins[i]);
            gpio_set_oeover(button_pins[i], GPIO_OVERRIDE_LOW);
        }
    } else {
        uint32_t state = 0;
        for (int i = 0; i < count_of(button_pins); ++i) {
            state |= gpio_get(button_pins[i]) << i;
            gpio_set_oeover(button_pins[i], GPIO_OVERRIDE_NORMAL);
        }
        button_state = state;
    }
}

void vga_board_init_buttons() {
    gpio_set_irq_enabled(VSYNC_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
    irq_set_exclusive_handler(IO_IRQ_BANK0, vga_board_button_irq_handler);
    irq_set_enabled(IO_IRQ_BANK0, true);
}

// mode7
static uint8_t tiles[][256] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
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

//#define VGA_MODE vga_mode_213x160_60
#define VGA_MODE vga_mode_320x240_60
#define MIN_RUN 3

const uint16_t dark_grey  = PICO_SCANVIDEO_PIXEL_FROM_RGB8(20, 40, 60);
const uint16_t black      = PICO_SCANVIDEO_PIXEL_FROM_RGB8(0, 0, 0);
const uint16_t dark_green = PICO_SCANVIDEO_PIXEL_FROM_RGB8(32, 128, 0);
const uint16_t lt_blue    = PICO_SCANVIDEO_PIXEL_FROM_RGB8(66, 133, 244);
const uint16_t lt_yellow  = PICO_SCANVIDEO_PIXEL_FROM_RGB8(255, 229, 153);
const uint16_t lt_green   = PICO_SCANVIDEO_PIXEL_FROM_RGB8(0, 255, 0);
const uint16_t brown      = PICO_SCANVIDEO_PIXEL_FROM_RGB8(120, 63, 4);
const uint16_t white      = PICO_SCANVIDEO_PIXEL_FROM_RGB8(255, 255, 255);
const uint16_t sky_blue   = PICO_SCANVIDEO_PIXEL_FROM_RGB8(96, 192, 255);

const uint16_t colors[] = {dark_grey, black, dark_green, lt_blue, lt_yellow, lt_green, brown, white};

struct camera {
    int32_t x;
    int32_t y;
    int32_t z;
    int32_t screen_distance; 
    int32_t rot;
    float   rcos;
    float   rsin;
    float   speed;
    int32_t jump_count;
};

camera camera = {128<<16,
		 128<<16,
		 3<<16,
		 200,
		 0,
		 1.f,
		 0.f,
		 1.0,
		 0};

void inline map_fill_line(uint16_t *output, uint32_t u, uint32_t v, uint32_t du, uint32_t dv, uint count) {
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

int32_t inline rle_sky_line(uint32_t *buf) {
    buf[0] = COMPOSABLE_COLOR_RUN     | (sky_blue << 16);
    buf[1] = VGA_MODE.width - MIN_RUN | (COMPOSABLE_RAW_1P << 16);
    buf[2] = 0                        | (COMPOSABLE_EOL_ALIGN << 16);
    return 3;
}

void single_scanline(struct scanvideo_scanline_buffer *dest) {
    uint32_t *buf = dest->data;
    size_t buf_length = dest->data_max;
    int line_num = scanvideo_scanline_number(dest->scanline_id);

    uint n_run = VGA_MODE.width + 1 - 3;
    uint last_pixel = (n_run + 4) / 2;

    float rcos = camera.rcos;
    float rsin = camera.rsin;
    int32_t n = camera.z / line_num;
    int32_t s = camera.screen_distance * n;
    int32_t t2 = VGA_MODE.width * n;
    int32_t x = camera.x + (t2 / 2 * rcos + s * rsin);
    int32_t y = camera.y + ((-t2 / 2) * rsin + s * rcos);
    int32_t du = -rcos * n;
    int32_t dv = rsin * n;
    if ((s > (400<<16)) || (line_num < 4)) {
	dest->data_used = rle_sky_line(buf);
    } else {
	map_fill_line((uint16_t *)(buf + 1), x, y, du, dv, n_run + 2);
	uint16_t first_pixel = buf[1] & 0xffff;
	buf[1] = buf[1] & 0xffff0000;
	buf[0] = COMPOSABLE_RAW_RUN  | (first_pixel << 16);
	buf[1] = n_run               | buf[1];
	buf[last_pixel] = 0          | (COMPOSABLE_EOL_ALIGN<<16);
	dest->data_used = last_pixel + 1;
    }
    
    dest->status = SCANLINE_OK;
}

void frame_update_logic(int num) {
    int32_t step = (camera.speed - 1) * 65536;
    camera.x += step * camera.rsin;
    camera.y += step * camera.rcos;

    if (camera.jump_count > 0) {
	if (camera.jump_count < 50) {
	    camera.z += (1 + (camera.jump_count++ / 5)) << 14;
	} else if (camera.jump_count < 99) {
	    camera.z -= (1 + ((camera.jump_count++ - 49) / 5)) << 14;
	} else {
	    camera.jump_count = 0;
	}
    }

    if (button_state & BUTTON_RIGHT) {
	camera.rot -= 1;
	if (camera.rot < 0) {
	    camera.rot += 360;
	}
	camera.rcos = cos(M_PI * camera.rot / 180);
	camera.rsin = sin(M_PI * camera.rot / 180);
    }
    if (button_state & BUTTON_LEFT) {
	camera.rot += 1;
	if (camera.rot >= 360) {
	    camera.rot -= 360;
	}
	camera.rcos = cos(M_PI * camera.rot / 180);
	camera.rsin = sin(M_PI * camera.rot / 180);
    }

    if ((button_state & BUTTON_LEFT) && (button_state & BUTTON_RIGHT)) {
	if (camera.jump_count == 0) {
	    camera.jump_count = 1;
	}
    }
  
    if (button_state & BUTTON_MIDDLE) {
	camera.speed = camera.speed * 1.005;
    } else {
	camera.speed = camera.speed * 0.995;
	if (camera.speed < 1.0) {
	    camera.speed = 1.0;
	}
    }
}

// main loop

void render_loop() {
    static uint32_t last_frame_num = 0;

    while (true) {
        struct scanvideo_scanline_buffer *scanline_buffer = scanvideo_begin_scanline_generation(true);

        uint32_t frame_num = scanvideo_frame_number(scanline_buffer->scanline_id);
        if (frame_num != last_frame_num) {
            last_frame_num = frame_num;
	    frame_update_logic(frame_num);
        }
        single_scanline(scanline_buffer);
	
        scanvideo_end_scanline_generation(scanline_buffer);
    }
}

int main() {
    set_sys_clock_khz(200000, true);
    stdio_init_all();
    interp_setup(map, map_width, map_height, 16, 4, 4);
    vga_board_init_buttons();
    scanvideo_setup(&VGA_MODE);
    scanvideo_timing_enable(true);
    render_loop();
    return 0;
}
