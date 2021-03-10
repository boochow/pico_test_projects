#include <stdio.h>
#include "pico/stdlib.h"
#include "pico.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"

#include "image.h"
#define VGA_MODE vga_mode_320x240_60
#define MIN_RUN 3

// RAW_RUN |color 1| n+1-3 |..|color n| 0(black) | EOL
int num_token = 2 + image_width + 1 + 1;

// render pixels
int32_t single_scanline(uint32_t *buf, size_t buf_length, const uint16_t *data) {
    assert(buf_length >= num_words);

    buf[0] = COMPOSABLE_RAW_RUN        | (*data++ << 16);
    buf[1] = (num_token - 3) - MIN_RUN | (*data++ << 16);
    uint16_t *p = (uint16_t *) (buf + 2);

    // -2 is for pixels in buf[0] and buf[1]
    for(int i = 0; i < (num_token - 4 - 2) ; i++) { 
	*p++ = *data++;
    }
    *p++ = 0; // last pixel must be black
    *p++ = (num_token % 2) ? COMPOSABLE_EOL_ALIGN : COMPOSABLE_EOL_SKIP_ALIGN;

    return (num_token + 1) / 2;
}

void render_scanline(struct scanvideo_scanline_buffer *dest) {
    uint32_t *buf = dest->data;
    size_t buf_length = dest->data_max;
    int line_num = scanvideo_scanline_number(dest->scanline_id);

    dest->data_used = single_scanline(buf, buf_length, &image[line_num * image_width]);

    dest->status = SCANLINE_OK;
}

// main loop
void render_loop() {
    static uint32_t last_frame_num = 0;

    while (true) {
        struct scanvideo_scanline_buffer *scanline_buffer = scanvideo_begin_scanline_generation(true);

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
