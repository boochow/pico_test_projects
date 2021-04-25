#include <stdio.h>
#include "pico/stdlib.h"
#include "pico.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"

#define VGA_MODE vga_mode_640x480_60
#define MIN_RUN 3

// render single line
int32_t single_scanline(uint32_t *buf, size_t buf_length, uint16_t color) {
    assert(buf_length >= 3);
    
    buf[0] = COMPOSABLE_COLOR_RUN     | (color << 16);
    buf[1] = VGA_MODE.width - MIN_RUN | (COMPOSABLE_RAW_1P << 16);
    buf[2] = 0                        | (COMPOSABLE_EOL_ALIGN << 16);
    return 3;
}

// main loop
void render_loop() {
    while (true) {
        struct scanvideo_scanline_buffer *buf = scanvideo_begin_scanline_generation(true);
        uint32_t frame_num = scanvideo_frame_number(buf->scanline_id);
	int line_num = scanvideo_scanline_number(buf->scanline_id);
	uint16_t color;

        if (line_num == (frame_num & 0x1ff)) {
	    color = PICO_SCANVIDEO_PIXEL_FROM_RGB8((line_num >> 1) & 0xe0, (line_num << 2) & 0xe0, (line_num << 5) & 0xe0);
        } else {
	    color = 0;
	}
        buf->data_used = single_scanline(buf->data, buf->data_max, color);
	buf->status = SCANLINE_OK;
        scanvideo_end_scanline_generation(buf);
    }
}

int main(void) {
    stdio_init_all();
    scanvideo_setup(&VGA_MODE);
    scanvideo_timing_enable(true);
    render_loop();
    return 0;
}
