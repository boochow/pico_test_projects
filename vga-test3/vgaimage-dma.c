#include <stdio.h>
#include "pico/stdlib.h"
#include "pico.h"
#include "pico/scanvideo.h"
#include "pico/scanvideo/composable_scanline.h"
#include "hardware/structs/dma.h"
#include "hardware/structs/ssi.h"

#include "image-vga.h"
//#define VGA_MODE vga_mode_320x240_60
#define VGA_MODE vga_mode_640x480_60
#define MIN_RUN 3

// flash_bulk_read() is from pco-playground/scanvideo/flashstream

// Use direct SSI DMA for maximum transfer speed (but cannot execute from
// flash at the same time)
void __no_inline_not_in_flash_func(flash_bulk_read)(uint32_t *rxbuf, uint32_t flash_offs, size_t len, uint dma_chan) {
    ssi_hw->ssienr = 0;
    ssi_hw->ctrlr1 = len - 1; // NDF, number of data frames (32b each)
    ssi_hw->dmacr = SSI_DMACR_TDMAE_BITS | SSI_DMACR_RDMAE_BITS;
    ssi_hw->ssienr = 1;

    dma_hw->ch[dma_chan].read_addr = (uint32_t) &ssi_hw->dr0;
    dma_hw->ch[dma_chan].write_addr = (uint32_t) rxbuf;
    dma_hw->ch[dma_chan].transfer_count = len;
    // Must enable DMA byteswap because non-XIP 32-bit flash transfers are
    // big-endian on SSI (we added a hardware tweak to make XIP sensible)
    dma_hw->ch[dma_chan].ctrl_trig =
            DMA_CH0_CTRL_TRIG_BSWAP_BITS |
            DREQ_XIP_SSIRX << DMA_CH0_CTRL_TRIG_TREQ_SEL_LSB |
            dma_chan << DMA_CH0_CTRL_TRIG_CHAIN_TO_LSB |
            DMA_CH0_CTRL_TRIG_INCR_WRITE_BITS |
            DMA_CH0_CTRL_TRIG_DATA_SIZE_VALUE_SIZE_WORD << DMA_CH0_CTRL_TRIG_DATA_SIZE_LSB |
            DMA_CH0_CTRL_TRIG_EN_BITS;

    // Now DMA is waiting, kick off the SSI transfer (mode continuation bits in LSBs)
    ssi_hw->dr0 = (flash_offs << 8) | 0xa0;

    while (dma_hw->ch[dma_chan].ctrl_trig & DMA_CH0_CTRL_TRIG_BUSY_BITS)
        tight_loop_contents();

    ssi_hw->ssienr = 0;
    ssi_hw->ctrlr1 = 0;
    ssi_hw->dmacr = 0;
    ssi_hw->ssienr = 1;
}

// RAW_RUN |color 1| n-3 |..|color n| 0 | EOL
int num_token = 2 + image_width + 1 + 1;

// render pixels
int32_t __time_critical_func(single_scanline)(uint32_t *buf, size_t buf_length, const uint16_t *data) {
    assert(buf_length >= num_words);

    uint16_t *p16 = (uint16_t *) buf;
    
    flash_bulk_read(&buf[1], (uint32_t) data, image_width / 2, 11);
    p16[0] = COMPOSABLE_RAW_RUN;
    p16[1] = p16[2];
    //    p16[2] = (num_token - 3) - MIN_RUN;
    p16[2] = (image_width + 1) - MIN_RUN;
    buf[num_token / 2 - 1] = 0 | COMPOSABLE_EOL_ALIGN << 16;

    return (num_token + 1) / 2;
}

static void inline render_scanline(struct scanvideo_scanline_buffer *dest) {
    uint32_t *buf = dest->data;
    size_t buf_length = dest->data_max;
    int line_num = scanvideo_scanline_number(dest->scanline_id);

    dest->data_used = single_scanline(buf, buf_length, &image[line_num * image_width]);

    dest->status = SCANLINE_OK;
}

// main loop
void __time_critical_func(render_loop)() {
    static uint32_t last_frame_num = 0;

    while (true) {
        struct scanvideo_scanline_buffer *scanline_buffer = scanvideo_begin_scanline_generation(true);

        render_scanline(scanline_buffer);

        scanvideo_end_scanline_generation(scanline_buffer);
    }
}

int main(void) {
    set_sys_clock_khz(200000, true);
    stdio_init_all();
    scanvideo_setup(&VGA_MODE);
    scanvideo_timing_enable(true);
    render_loop();
    return 0;
}
