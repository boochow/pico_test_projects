#include <stdio.h>
#include "pico/stdlib.h"
#include "pico.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"

#define VGA_MODE vga_mode_640x480_60
#define MIN_RUN 3

// color table
#define COLORTABLE_SIZE 216
uint16_t color[COLORTABLE_SIZE];

void init_colors() {
    static const uint8_t scale[6] = {0, 0x33, 0x66, 0x99, 0xcc, 0xff};
    for(int i = 0; i < COLORTABLE_SIZE; i++) {
	int r = i % 6;
	int g = (i / 6) % 6;
	int b = (i / 36) % 6;
	color[i] = PICO_SCANVIDEO_PIXEL_FROM_RGB8(scale[r], scale[g], scale[b]);
    }
}

// display buffer
#define DISPBUF_W 40
#define DISPBUF_H 30
#define DISPBUF_SIZE (DISPBUF_W * DISPBUF_H)
#define PIXEL_W 16
#define PIXEL_W_BITS 4
#define PIXEL_H_MASK 0xf

uint8_t disp_buf[DISPBUF_SIZE];

void disp_buf_draw_color_table(int offset) {
    for(int i = 0; i < DISPBUF_SIZE; i++) {
	disp_buf[i] = (i + offset) % COLORTABLE_SIZE;
    }
}

// render big pixels
int32_t single_black_line(uint32_t *buf, size_t buf_length) {
    assert(buf_length >= 2);
    
    buf[0] = COMPOSABLE_COLOR_RUN     | 0;
    buf[1] = VGA_MODE.width - MIN_RUN | (COMPOSABLE_EOL_ALIGN << 16);
    return 2;
}

int32_t single_scanline(uint32_t *buf, size_t buf_length, uint8_t *data) {
    assert(buf_length >= 1 + 3 * DISPBUF_W);

    for(int i = 0; i < DISPBUF_W; i++) {
	buf[0] = COMPOSABLE_COLOR_RUN  | (color[*data++] << 16);
	buf[1] = PIXEL_W - 1 - MIN_RUN | (COMPOSABLE_RAW_1P_SKIP_ALIGN << 16);
	buf[2] = 0;                  //| -- the last token is ignored --
	buf += 3;
    }
    buf[0] = COMPOSABLE_EOL_SKIP_ALIGN | 0;
    return 3 * DISPBUF_W + 1;
}

void render_scanline(struct scanvideo_scanline_buffer *dest) {
    uint32_t *buf = dest->data;
    size_t buf_length = dest->data_max;
    int line_num = scanvideo_scanline_number(dest->scanline_id);

    if ((line_num & PIXEL_H_MASK) == PIXEL_H_MASK) {
	dest->data_used = single_black_line(buf, buf_length);
    } else {
	int buf_pos = DISPBUF_W * (line_num >> PIXEL_W_BITS);
	dest->data_used = single_scanline(buf, buf_length, &disp_buf[buf_pos]);
    }
    dest->status = SCANLINE_OK;
}

// update display buffer
void frame_update_logic(int num) {
    disp_buf_draw_color_table(num % DISPBUF_SIZE);
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
        render_scanline(scanline_buffer);
	
        scanvideo_end_scanline_generation(scanline_buffer);
    }
}

int main(void) {
    stdio_init_all();
    init_colors();
    disp_buf_draw_color_table(0);
    scanvideo_setup(&VGA_MODE);
    scanvideo_timing_enable(true);
    render_loop();
    return 0;
}
