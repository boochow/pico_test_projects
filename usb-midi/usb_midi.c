#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"

bool print_midi_event(const uint8_t msg[3]) {
    int ch;
    int event;

    ch = msg[0] & 0xf;
    event = (msg[0] >> 4) & 0xf;
    if (event != 0xf) {
	printf("Ch: %02d | ", ch);
    } else {
	printf("System | ");
    }

    switch (event) {
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
	printf("Control   : num=%d value=%d", msg[1], msg[2]);
	break;
    case 12:
	printf("Program   : %d", msg[1]);
	break;
    case 13:
	printf("ChPress   : %d", msg[1]);
	break;
    case 14:
	printf("PitchBend : %d", msg[1] + 128 * msg[2] - 8192);
	break;
    case 15:
	if (ch == 0) {
	    printf("SysEx     : %02x %02x %02x", msg[0], msg[1], msg[2]);
	} else if (ch == 1) {
	    printf("MIDI TC   : %d", msg[1]);
	} else if (ch == 2) {
	    printf("SongPos   : %d", msg[1] + 128 * msg[2]);
	} else if (ch == 3) {
	    printf("SongSelect: %d", msg[1]);
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
	} else if (ch == 14) {
	    printf("ActiveSensing");
	} else if (ch == 15) {
	    printf("RESET");
	}
	break;
    default:
	break;
    }
    printf("\n");
}

void dump_message(const uint8_t msg[3], int n_data) {
    int i;
    
    for(i = 0; i < n_data; i++) {
	printf("%02x", msg[i]);
    }
    for(; i < 3; i++) {
	printf("--");
    }
}

void midi_task(void)
{
    static bool sysex = false;
    uint8_t msg[3];
    int n_data;

    while(n_data = tud_midi_n_available(0, 0)) {
	msg[0] = 0; msg[1] = 0; msg[2] = 0;
	if (n_data = tud_midi_n_read(0, 0, msg, 3)) {
	    dump_message(msg, n_data);
	    printf(" | ");

	    if (sysex) {
		printf("       |           : ");
		for (int i = 0; i < 3; i++) {
		    printf("%02x ", msg[i]);
		    if (msg[i] == 0xf7) {
			sysex = false;
			break;
		    }
		}
		printf("\n");
	    } else {
		sysex = (msg[0] == 0xf0);
		print_midi_event(msg);
	    }
	}
    }
}

int main() {
    stdio_init_all();
    board_init();
    tusb_init();

    sleep_ms(500);
    printf("*** MIDI MONITOR ***\n");

    while (true) {
	tud_task();
	midi_task();
    }

    return 0;
}
