#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"

#define VGA_MODE vga_mode_640x480_60
#define MIN_RUN 3
#define BUF_ROW_WORDS 20
#define BUF_LINES 480

// black and white bitmap 32bit x 20 x 480
uint32_t framebuffer[BUF_ROW_WORDS * BUF_LINES];

void clear_screen() {
    for(int i; i < BUF_ROW_WORDS * BUF_LINES; i++){
	framebuffer[i] = 0;
    }
}

void set_pixel_xor(int x, int y) {
    framebuffer[x / 32 + y * BUF_ROW_WORDS] ^= (1 << (x % 32));
}

#define abs(x) (x) > 0 ? (x) : -(x)

void line(int x0, int y0, int x1, int y1) {
    int dx = abs(x1-x0);
    int dy = abs(y1-y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    while((x0 != x1) || (y0 != y1)) {
	set_pixel_xor(x0,y0);
	int e2 = 2 * err;
	if (e2 > -dy) {
	    err = err - dy;
	    x0 = x0 + sx;
	}
	if (e2 < dx) {
	    err = err + dx;
	    y0 = y0 + sy;
	}
    }
}

// scanline
uint32_t twopixels[4];

void set_color_table(uint32_t fg_color, uint32_t bg_color) {
    twopixels[0] = bg_color;
    twopixels[1] = bg_color << 16 | fg_color;
    twopixels[2] = fg_color << 16 | bg_color;
    twopixels[3] = fg_color << 16 | fg_color;
}

int32_t inline __time_critical_func(single_scanline)(uint32_t *buf, size_t buf_length, uint32_t *data) {
    uint16_t *p16 = (uint16_t *) buf;
    uint32_t *p = buf + 1;

    for(int i = 0; i < BUF_ROW_WORDS; i++) {
	uint32_t w = *(data + i);
	for(int b = 0; b < 16; b++) {
	    *p++ = twopixels[(w >> (b+b)) & 3];
	}
    }
    p16[0] = COMPOSABLE_RAW_RUN;
    p16[1] = p16[2];
    p16[2] = (VGA_MODE.width + 1) - MIN_RUN;
    p16[VGA_MODE.width + 1] = 0;
    p16[VGA_MODE.width + 2] = COMPOSABLE_EOL_ALIGN;

    return (VGA_MODE.width / 2) + 2;
}

// update the scene
void frame_update_logic(int num) {
    static int count = 0;
    int x0 = rand() % 1280;
    int y0 = 0;
    int x1 = 0;
    int y1 = rand() % 960;
    if (x0 >= 640) {
	x0 -= 640;
	y0 = 480;
    }
    if (y1 >= 480) {
	y1 -= 480;
	x1 = 640;
    }
    line(x0, y0, x1, y1);

    if (count++ > 600) {
	clear_screen();
	count = 0;
    }
}

static void inline render_scanline(struct scanvideo_scanline_buffer *dest) {
    uint32_t *buf = dest->data;
    size_t buf_length = dest->data_max;
    int line_num = scanvideo_scanline_number(dest->scanline_id);

    dest->data_used = single_scanline(buf, buf_length, &framebuffer[BUF_ROW_WORDS * line_num]);

    dest->status = SCANLINE_OK;
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
	    /*
	    for(int i = 0; i < 40; i++){
		int s = (i * 12 + frame_num % 12) * BUF_ROW_WORDS;
		for(int j = 0; j < BUF_ROW_WORDS; j++) {
		    framebuffer[s + j] = rand() + rand();
		}
	    }
	    */
        }

        render_scanline(scanline_buffer);
	
        scanvideo_end_scanline_generation(scanline_buffer);
    }
}

int main(void) {
    stdio_init_all();
    scanvideo_setup(&VGA_MODE);
    scanvideo_timing_enable(true);
    set_color_table(PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff,0xff,0xff), 0);
    render_loop();
    return 0;
}
