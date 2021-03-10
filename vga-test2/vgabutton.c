#include <stdio.h>
#include "pico/stdlib.h"
#include "pico.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"
#include "hardware/irq.h"

// the code for reading button state is from pico-playground/apps/popcorn

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

#define VGA_MODE vga_mode_640x480_60
#define MIN_RUN 3

// rgb bar display
// BAR_HEIGHT * RINGBUF_LEN = VGA_MODE.height
// BAR_WIDTH * 3 + SCREEN_MARGIN * 2 + 2(for dots) = VGA_MODE.width
#define SCREEN_MARGIN 19
#define BAR_WIDTH 200
#define BAR_HEIGHT 8
#define RINGBUF_LEN 60
#define RINGBUF_DOT 5
#define DOT_MASK 8

uint8_t disp_buf[RINGBUF_LEN];
uint8_t disp_buf_tail;

void disp_buf_clear() {
    for(int i = 0; i < RINGBUF_LEN; i++) {
	disp_buf[i] = (i % 5) ? 0 : DOT_MASK;
    }
    disp_buf_tail = RINGBUF_LEN - 1;
}

// render color bar
int32_t single_scanline(uint32_t *buf, size_t buf_length, uint8_t data) {
    assert(buf_length >= 10);

    int r = (data & 1) ? PICO_SCANVIDEO_PIXEL_FROM_RGB8(0xff, 0 ,0) : 0;
    int g = (data & 2) ? PICO_SCANVIDEO_PIXEL_FROM_RGB8(0, 0xff, 0) : 0;
    int b = (data & 4) ? PICO_SCANVIDEO_PIXEL_FROM_RGB8(0, 0, 0xff) : 0;
    int w = (data & DOT_MASK) ? PICO_SCANVIDEO_PIXEL_FROM_RGB8(0x40, 0x40, 0x40) : 0;

    buf[0] = COMPOSABLE_COLOR_RUN    | 0;
    buf[1] = SCREEN_MARGIN - MIN_RUN | COMPOSABLE_RAW_1P << 16;
    buf[2] = w                       | COMPOSABLE_COLOR_RUN << 16;
    buf[3] = r                       | (BAR_WIDTH - MIN_RUN) << 16;
    buf[4] = COMPOSABLE_COLOR_RUN    | g << 16;
    buf[5] = BAR_WIDTH - MIN_RUN     | COMPOSABLE_COLOR_RUN << 16;
    buf[6] = b                       | (BAR_WIDTH - MIN_RUN) << 16;
    buf[7] = COMPOSABLE_RAW_1P       | w << 16;
    buf[8] = COMPOSABLE_COLOR_RUN    | 0;
    buf[9] = SCREEN_MARGIN - MIN_RUN | COMPOSABLE_EOL_ALIGN << 16;
    return 10;
}

void render_scanline(struct scanvideo_scanline_buffer *dest) {
    uint32_t *buf = dest->data;
    size_t buf_length = dest->data_max;
    int line_num = scanvideo_scanline_number(dest->scanline_id);
    int disp_buf_head = (disp_buf_tail + 1) % RINGBUF_LEN;
    int buf_pos = (disp_buf_head + (line_num / BAR_HEIGHT)) % RINGBUF_LEN;

    dest->data_used = single_scanline(buf, buf_length, disp_buf[buf_pos]);

    dest->status = SCANLINE_OK;
}

// update display buffer
void frame_update_logic(int num) {
    disp_buf_tail = ++disp_buf_tail % RINGBUF_LEN;
    int prev_state = disp_buf[disp_buf_tail] & DOT_MASK;
    disp_buf[disp_buf_tail] = prev_state | button_state;
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
    disp_buf_clear();
    vga_board_init_buttons();
    scanvideo_setup(&VGA_MODE);
    scanvideo_timing_enable(true);
    render_loop();
    return 0;
}
