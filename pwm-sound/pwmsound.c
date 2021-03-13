#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/regs/rosc.h"

#define PWM_AUDIO_L    27
#define PWM_AUDIO_R    28
#define RANDOMBIT      (*((uint *)(ROSC_BASE + ROSC_RANDOMBIT_OFFSET)) & 1)

#define PWM_RANGE_BITS 8
#define PWM_RANGE      (1<<PWM_RANGE_BITS)
#define VOL_MAX        (PWM_RANGE / 4 - 1)
#define SAMPLE_RATE    (125000000 / PWM_RANGE)
#define OMEGA_UNIT     (FIXED_1_0 / SAMPLE_RATE)
#define NUM_PSG        4

#define FIXED_0_5      0x40000000
#define FIXED_1_0      0x7fffffff
typedef uint32_t fixed; // 1.0=0x7fffffff, 0.0=0x0

enum psg_type {OSC_SQUARE, OSC_SAW, OSC_TRI, OSC_NOISE};
		 
struct psg_t {
    volatile fixed phase;       // 0..FIXED_1_0
    fixed step;                 // 0..FIXED_1_0
    volatile int sound_vol;     // 0..VOL_MAX
    enum psg_type type;
};

static struct psg_t psg[NUM_PSG];

void psg_init() {
    for(int i = 0; i < NUM_PSG; i++) {
	psg[i].phase = 0;
	psg[i].step = 0;
	psg[i].sound_vol = VOL_MAX / 4;
	psg[i].type = OSC_SQUARE;
    }
}

void psg_freq(int i, uint freq) {
    psg[i].step = freq * OMEGA_UNIT; 
}

void psg_vol(int i, int value) {
    if (value < 0) {
	value = 0;
    }
    psg[i].sound_vol = value % (VOL_MAX + 1);
}

void psg_type(int i, enum psg_type type) {
    psg[i].type = type;
}

static inline uint psg_value(int i) {
    uint result;
    if (psg[i].type == OSC_SQUARE) {
	result = (psg[i].phase > FIXED_0_5) ? psg[i].sound_vol : 0;
    } else if (psg[i].type == OSC_SAW) {
	result = ((psg[i].phase >> (31 - PWM_RANGE_BITS)) * psg[i].sound_vol) >> PWM_RANGE_BITS;
    } else if (psg[i].type == OSC_TRI) {
	result = ((psg[i].phase >> (30 - PWM_RANGE_BITS)) * psg[i].sound_vol) >> PWM_RANGE_BITS;
	result = (result < psg[i].sound_vol) ? result : psg[i].sound_vol * 2 - result;
    } else { // OSC_NOISE
	result = RANDOMBIT * psg[i].sound_vol;
    }
    return result;
}

static inline void psg_next() {
    for(int i = 0; i < NUM_PSG; i++) {
	psg[i].phase += psg[i].step;
	if (psg[i].phase > FIXED_1_0) {
	    psg[i].phase -= FIXED_1_0;
	}
    }
}

void on_pwm_wrap() {
    uint sum = 0;
    
    pwm_clear_irq(pwm_gpio_to_slice_num(PWM_AUDIO_L));
#ifdef PWM_AUDIO_R
    pwm_clear_irq(pwm_gpio_to_slice_num(PWM_AUDIO_R));
#endif
    psg_next();
    for(int i = 0; i < NUM_PSG; i++) {
	sum += psg_value(i);
    }
    pwm_set_gpio_level(PWM_AUDIO_L, sum);
#ifdef PWM_AUDIO_R
    pwm_set_gpio_level(PWM_AUDIO_R, sum);
#endif
}

void psg_pwm_config() {
    gpio_set_function(PWM_AUDIO_L, GPIO_FUNC_PWM);
#ifdef PWM_AUDIO_R
    gpio_set_function(PWM_AUDIO_R, GPIO_FUNC_PWM);
#endif
    uint slice_num = pwm_gpio_to_slice_num(PWM_AUDIO_L);
    pwm_clear_irq(slice_num);
    pwm_set_irq_enabled(slice_num, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, on_pwm_wrap);
    irq_set_enabled(PWM_IRQ_WRAP, true);
    
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&config, 1);
    pwm_config_set_wrap(&config, PWM_RANGE);
    pwm_init(slice_num, &config, true);
#ifdef PWM_AUDIO_R
    slice_num = pwm_gpio_to_slice_num(PWM_AUDIO_R);
    pwm_init(slice_num, &config, true);
#endif
}

int main() {
    stdio_init_all();
    psg_pwm_config();
    psg_init();

    psg_type(0, OSC_SQUARE);
    psg_type(1, OSC_SAW);
    psg_type(2, OSC_TRI);

    float f0 = 100.f;
    float f1 = 100.f;
    for(int i = 0; i < 400; i++) {
	psg_freq(0, f0);
	psg_freq(1, f1 * 2 / 3);
	psg_freq(2, f0 + f1 * 3 / 4);
    	f0 = f0 * 1.007;
    	f1 = f1 * 1.005;
    	sleep_ms(10);
    }

    psg_type(0, OSC_NOISE);
    psg_type(1, OSC_NOISE);
    psg_type(2, OSC_NOISE);
    for(int i = VOL_MAX; i >= 0; i--){
	psg_vol(0, i);
	psg_vol(1, i);
	psg_vol(2, i);
	sleep_ms(30);
    }

    while (1)
        tight_loop_contents();
}
