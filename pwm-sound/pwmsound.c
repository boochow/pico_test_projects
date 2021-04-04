#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/regs/rosc.h"

#define PWM_AUDIO_L    (27)
#define PWM_AUDIO_R    (28)
#define RANDOMBIT      (*((uint *)(ROSC_BASE + ROSC_RANDOMBIT_OFFSET)) & 1)

#define PWM_RANGE_BITS (10)
#define PWM_RANGE      (1<<PWM_RANGE_BITS)
#define NUM_PSG        (4)
#define PSG_VOL_MAX    (PWM_RANGE / NUM_PSG - 1)
#define SAMPLE_RATE    (125000000 / PWM_RANGE)
#define OMEGA_UNIT     (FIXED_1_0 / SAMPLE_RATE)

#define FIXED_0_5      (0x40000000)
#define FIXED_1_0      (0x7fffffff)
typedef uint32_t fixed;       // 1.0=0x7fffffff, 0.0=0x0

enum psg_type {OSC_SQUARE, OSC_SAW, OSC_TRI, OSC_NOISE};
		 
struct psg_t {
    volatile fixed phi;       // 0..FIXED_1_0
    fixed step;               // 0..FIXED_1_0
    volatile int sound_vol;   // 0..PSG_VOL_MAX
    enum psg_type type;
};

static struct psg_t psg[NUM_PSG];

void psg_freq(int i, float freq) {
    assert(i < NUM_PSG);
    psg[i].step = freq * OMEGA_UNIT; 
}

void psg_vol(int i, int value) {
    assert(i < NUM_PSG);
    if (value < 0) {
	value = 0;
    }
    psg[i].sound_vol = value % (PSG_VOL_MAX + 1);
}

void psg_type(int i, enum psg_type type) {
    assert(i < NUM_PSG);
    psg[i].type = type;
}

static inline uint psg_value(int i) {
    assert(i < NUM_PSG);
    uint result;
    if (psg[i].type == OSC_SQUARE) {
	result = (psg[i].phi > FIXED_0_5) ? psg[i].sound_vol : 0;
    } else if (psg[i].type == OSC_SAW) {
	result = ((psg[i].phi >> (31 - PWM_RANGE_BITS)) * psg[i].sound_vol) >> PWM_RANGE_BITS;
    } else if (psg[i].type == OSC_TRI) {
	result = ((psg[i].phi >> (30 - PWM_RANGE_BITS)) * psg[i].sound_vol) >> PWM_RANGE_BITS;
	result = (result < psg[i].sound_vol) ? result : psg[i].sound_vol * 2 - result;
    } else { // OSC_NOISE
	result = RANDOMBIT * psg[i].sound_vol;
    }
    return result;
}

static inline void psg_next() {
    for(int i = 0; i < NUM_PSG; i++) {
	psg[i].phi += psg[i].step;
	if (psg[i].phi > FIXED_1_0) {
	    psg[i].phi -= FIXED_1_0;
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

void psg_init() {
    for(int i = 0; i < NUM_PSG; i++) {
	psg[i].phi = 0;
	psg[i].step = 0;
	psg[i].sound_vol = PSG_VOL_MAX / 4;
	psg[i].type = OSC_SQUARE;
    }
    psg_pwm_config();
}

void psg_all_vol(int value) {
    for(int i = 0 ; i < NUM_PSG; i++) {
	psg_vol(i, value);
    }
}

void demo1() {
    psg_type(0, OSC_SQUARE);
    psg_type(1, OSC_SAW);
    psg_type(2, OSC_TRI);
    psg_type(3, OSC_NOISE);
    psg_all_vol(0);
    
    for(int j = 0; j < 3; j++) {
	psg_vol(j, PSG_VOL_MAX / 4);
	float f0 = 100.f;
	for(int i = 0; i < 400; i++) {
	    psg_freq(j, f0);
	    f0 = f0 * 1.007;
	    sleep_ms(3);
	}
	psg_vol(j, 0);
    }

    for(int i = 10; i >= 0; i--){
	psg_vol(3, PSG_VOL_MAX / 100 * i * i);
	sleep_ms(100);
    }
}

void demo2() {
    const int pat[4][17] = {
			    {1,0,0,0, 1,0,1,0, 1,0,0,0, 1,0,1,1, 0},
			    {1,1,0,0, 0,0,1,1, 1,1,0,0, 1,0,1,0, 0},
			    {0,0,1,0, 1,1,0,1, 0,0,1,1, 1,1,0,0, 0},
			    {1,0,0,1, 0,0,1,0, 0,1,0,0, 1,0,0,0, 0}};

    psg_type(0, OSC_NOISE);
    psg_type(1, OSC_SQUARE);
    psg_type(2, OSC_SAW);
    psg_type(3, OSC_TRI);
    psg_freq(1, 110.f);
    psg_freq(2, 440.f);
    psg_freq(3, 880.f);
    psg_all_vol(PSG_VOL_MAX/4);

    for(int repeat = 0; repeat < 4; repeat++) {
	for(int i = 0; i < 16; i++) {
	    for(int t = 1; t < 5; t++) {
		for (int j = 0; j < 4; j++) {
		    if (pat[j][i + 1]) {
			psg_vol(j, pat[j][i] * (PSG_VOL_MAX / 4));
		    } else {
			psg_vol(j, pat[j][i] * (PSG_VOL_MAX / 4) / t);
		    }
		}
		sleep_ms(30);
	    }
	}
    }
    psg_all_vol(0);
}

int main() {
    stdio_init_all();
    psg_init();

    sleep_ms(1000);
    demo1();
    sleep_ms(500);
    demo2();
    sleep_ms(500);
    
    while (1)
        tight_loop_contents();
}
