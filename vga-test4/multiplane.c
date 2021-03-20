#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"

#define VGA_MODE vga_mode_640x480_60
#define MIN_RUN 3

// background animation
static int graylevel = 0;

// moving box
static int box_x = 4;
static int delta_x = 5;
static int box_y = 0;
static int delta_y = 5;
static int box_color = 0;
#define BOX_SIZE  40
#define W_MINUS_BOX  (VGA_MODE.width - BOX_SIZE)
#define IS_BETWEEN(x,a,b)  ((a <= x) && (x <= b))

void box_step() {
    box_x += delta_x;
    if (!IS_BETWEEN(box_x, 0, VGA_MODE.width - BOX_SIZE)) {
	delta_x = -delta_x;
	box_x += delta_x;
    }
    
    box_y += delta_y;
    if (!IS_BETWEEN(box_y, 0, VGA_MODE.height - BOX_SIZE)) {
	delta_y = -delta_y;
	box_y += delta_y;
    }
}

int32_t single_color_line(uint32_t *buf, size_t buf_length, uint16_t color) {
    assert(buf_length >= 3);
    
    buf[0] = COMPOSABLE_COLOR_RUN     | (color << 16);
    buf[1] = VGA_MODE.width - MIN_RUN | (COMPOSABLE_RAW_1P << 16);
    buf[2] = 0                        | (COMPOSABLE_EOL_ALIGN << 16);
    return 3;
}

int32_t box(uint32_t *buf, size_t buf_length, uint16_t line_num, uint16_t color) {
    assert(buf_length >= 5);

    if (!IS_BETWEEN(line_num, box_y, box_y + BOX_SIZE)) {
	return single_color_line(buf, buf_length, 0);
    } else {
	buf[0] = COMPOSABLE_COLOR_RUN | 0;
	buf[1] = box_x - MIN_RUN      | (COMPOSABLE_COLOR_RUN << 16);
	buf[2] = color                | (BOX_SIZE - MIN_RUN)<< 16;
	buf[3] = COMPOSABLE_RAW_1P    | 0;
	buf[4] = COMPOSABLE_EOL_SKIP_ALIGN;
	return 5;
    }
}

void render_scanline(struct scanvideo_scanline_buffer *dest) {
    uint32_t *buf = dest->data;
    size_t buf_length = dest->data_max;
    int l = scanvideo_scanline_number(dest->scanline_id);

    uint16_t c = (graylevel + (l > 240 ? 480 - l : l)) / 3;
    c = PICO_SCANVIDEO_PIXEL_FROM_RGB8(c, c, c);
    dest->data_used = single_color_line(buf, buf_length, c);

    buf = dest->data2;
    buf_length = dest->data2_max;
    dest->data2_used = box(buf, buf_length, l, box_color);

    dest->status = SCANLINE_OK;
}

// update the scene
void frame_update_logic(int num) {
    num = num % 960;
    graylevel = num > 480 ? 960 - num : num;

    box_step();
    box_color = (rand() & 0xffff) | PICO_SCANVIDEO_ALPHA_MASK;
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
    scanvideo_setup(&VGA_MODE);
    scanvideo_timing_enable(true);
    render_loop();
    return 0;
}
