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

bool print_midi_event(const uint8_t msg[4]) {
    bool single = false;
    int ch;
    int cin;

    ch = msg[0] & 0xf;
    cin = (msg[0] >> 4) & 0xf;
    if (cin != 0xf) {
	printf("Ch: %02d | ", ch);
    } else {
	printf("Common | ");
    }

    switch (cin) {
    case 0:
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
	    printf("SysEx start");
	} else {
	    printf("...");
	}
	break;
    case 5:
    case 6:
    case 7:
	printf("SysEx end");
	break;
    case 8:
	printf("Note Off  : key=%d velocity=%d", msg[1], msg[2]);
	break;
    case 9:
	printf("Note On   : key=%d velocity=%d", msg[1], msg[2]);
	break;
    case 10:
	printf("PolyPress : key=%d velocity=%d", msg[1], msg[2]);
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
	if (ch == 0) {
	    printf("SysEx     : %02x %02x %02x", msg[0], msg[1], msg[2]);
	    single = true;
	} else if (ch == 1) {
	    printf("MIDI TC   : %02x", msg[1]);
	} else if (ch == 2) {
	    printf("SongPos   : %02x %02x", msg[1], msg[2]);
	} else if (ch == 3) {
	    printf("SongSelect: %02x", msg[1]);
	} else if (ch == 6) {
	    printf("TuneReqest");
	} else if (ch == 8) {
	    printf("TimingClock");
	} else if (ch == 10) {
	    printf("START");
	} else if (ch == 11) {
	    printf("CONTINUE");
	} else if (ch == 12) {
	    printf("STOP");
	} else if (ch == 15) {
	    printf("RESET");
	}
	break;
    default:
	break;
    }
    printf("\n");
    return single;
}

void midi_task(void)
{
    static bool single = false;
    uint8_t msg[4];
    int n_data;
    int cin;
    int ch;

    while(n_data = tud_midi_n_available(0, 0)) {
	msg[0] = 0; msg[1] = 0; msg[2] = 0; msg[3] = 0;
	if (tud_midi_n_read(0, 0, msg, 4)) {
	    printf("%02x%02x%02x%02x | ", msg[0], msg[1], msg[2], msg[3]);

	    if (single) {
		printf("       |           : ");
		for (int i = 0; i < 3; i++) {
		    printf("%02x ", msg[i]);
		    if (msg[i] == 0xf7) {
			single = false;
			break;
		    }
		}
		printf("\n");
	    } else {
		single = print_midi_event(msg);
	    }
	}
    }
}
