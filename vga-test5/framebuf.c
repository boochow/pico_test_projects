#include <stdlib.h>
#include <string.h>
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"

#define VGA_MODE vga_mode_640x480_60
#define MIN_RUN 3
#define BUF_ROW_WORDS 20
#define BUF_LINES 480

static semaphore_t video_initted;

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

static void inline render_scanline(struct scanvideo_scanline_buffer *dest) {
    uint32_t *buf = dest->data;
    size_t buf_length = dest->data_max;
    int line_num = scanvideo_scanline_number(dest->scanline_id);

    dest->data_used = single_scanline(buf, buf_length, &framebuffer[BUF_ROW_WORDS * line_num]);

    dest->status = SCANLINE_OK;
}

void render_loop() {
    static uint32_t last_frame_num = 0;

    while (true) {
        struct scanvideo_scanline_buffer *scanline_buffer = scanvideo_begin_scanline_generation(true);
        uint32_t frame_num = scanvideo_frame_number(scanline_buffer->scanline_id);
        if (frame_num != last_frame_num) {
            last_frame_num = frame_num;
        }

        render_scanline(scanline_buffer);
	
        scanvideo_end_scanline_generation(scanline_buffer);
    }
}

// main loop (scanvideo)
void core1_main() {
    scanvideo_setup(&VGA_MODE);
    scanvideo_timing_enable(true);
    sem_release(&video_initted);
    set_color_table(PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff,0xff,0xff), 0);
    render_loop();
}

// main loop (graphics)
void draw_random_line() {
    int x0 = rand() % (VGA_MODE.width * 2);
    int y0 = 0;
    int x1 = 0;
    int y1 = rand() % (VGA_MODE.height * 2);
    if (x0 >= VGA_MODE.width) {
	x0 -= VGA_MODE.width;
	y0 = VGA_MODE.height;
    }
    if (y1 >= VGA_MODE.height) {
	y1 -= VGA_MODE.height;
	x1 = VGA_MODE.width;
    }
    line(x0, y0, x1, y1);
}

void core0_main() {
    int mode = 0;
    
    while(true) {
        int c = getchar_timeout_us(0);
        switch (c) {
            case ' ':
                mode = 1 - mode;
                clear_screen();
                break;
        }
	if (mode) {
	    scanvideo_wait_for_vblank();
	    draw_random_line();
	} else {
	    for(int i = 0; i < BUF_ROW_WORDS * BUF_LINES; i++){
		framebuffer[i] = rand() + rand();
	    }
	}
    }
}

int main(void) {
    stdio_init_all();
    sem_init(&video_initted, 0, 1);
    multicore_launch_core1(core1_main);
    sem_acquire_blocking(&video_initted);
    core0_main();
    return 0;
}
