#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"

void midi_task(void);

int main() {
    stdio_init_all();
    board_init();
    tusb_init();

    sleep_ms(500);
    printf("*** MIDI MONITOR ***\n");

    while (1)
	{
	    tud_task(); // tinyusb device task
	    midi_task();
	}

    return 0;
}

void midi_task(void)
{
    uint8_t msg[4];
    int n_data;

    while(n_data = tud_midi_n_available(0, 0)) {
	if (tud_midi_n_read(0, 0, msg, 4)) {
	    printf("Ch:%02d | ", msg[0] & 0xf);
	    switch ((msg[0] >> 4) & 0xf) {
	    case 0:
		// reserved
		break;
	    case 1:
		// reserved
		break;
	    case 2:
		printf("SYS: %02x %02x", msg[1], msg[2]);
		break;
	    case 3:
		printf("SYS: %02x %02x %02x", msg[1], msg[2], msg[3]);
		break;
	    case 4:
	    case 5:
	    case 6:
	    case 7:
		printf("SYSEX");
		break;
	    case 8:
		printf("NOTE OFF: key=%02x velocity=%d", msg[1], msg[2]);
		break;
	    case 9:
		printf("NOTE ON:  key=%02x velocity=%d", msg[1], msg[2]);
		break;
	    case 10:
		printf("Poly key pressure");
		break;
	    case 11:
		printf("Control:  %02x %02x", msg[1], msg[2]);
		break;
	    case 12:
		printf("Prog chg: %02x", msg[1]);
		break;
	    case 13:
		printf("Channel Pressure");
		break;
	    case 14:
		printf("Pitch bend: %02x %02x", msg[1], msg[2]);
		break;
	    case 15:
		printf("Single byte: %02x", msg[0]);
		break;
	    default:
		break;
	    }
	    printf("\n");
	}
    }
}
