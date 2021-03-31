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
    static bool single = false;
    uint8_t msg[4];
    int n_data;

    while(n_data = tud_midi_n_available(0, 0)) {
	msg[0] = 0; msg[1] = 0; msg[2] = 0; msg[3] = 0;
	if (tud_midi_n_read(0, 0, msg, 4)) {
	    printf("%02x%02x%02x%02x | ", msg[0], msg[1], msg[2], msg[3]);
	    if (single) {
		printf("      |           : ");
		for (int i = 0; i < 3; i++) {
		    printf("%02x ", msg[i]);
		    if (msg[i] == 0xf7) {
			single = false;
			break;
		    }
		}
		printf("\n");
		continue;
	    }
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
		if (msg[1] == 0xf0) {
		    printf("SYSEX start");
		} else {
		    printf("...");
		}
		break;
	    case 5:
	    case 6:
	    case 7:
		printf("SYSEX end");
		break;
	    case 8:
		printf("NOTE OFF  : key=%d velocity=%d", msg[1], msg[2]);
		break;
	    case 9:
		printf("NOTE ON   : key=%d velocity=%d", msg[1], msg[2]);
		break;
	    case 10:
		printf("PolyPress :");
		break;
	    case 11:
		printf("Control   : %02x %02x", msg[1], msg[2]);
		break;
	    case 12:
		printf("Program   : %02x", msg[1]);
		break;
	    case 13:
		printf("ChPress   : %02x", msg[1]);
		break;
	    case 14:
		printf("PitchBend : %d", msg[1] + 128 * msg[2] - 8192);
		break;
	    case 15:
		printf("Singlebyte: %02x %02x %02x", msg[0], msg[1], msg[2]);
		single = true;
		break;
	    default:
		break;
	    }
	    printf("\n");
	}
    }
}
