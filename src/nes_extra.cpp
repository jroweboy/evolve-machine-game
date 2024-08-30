
/// File containing manually ported code from libnesdoug

#include <neslib.h>

#include "nes_extra.hpp"

void pal_fade_to(char from, char to, char duration) {
    while (from != to) {
        from = (from < to) ? from + 1 : from - 1;
        pal_bright(from);
        delay(duration);
    }
    // Wait one additional frame to make sure the last fade happens before exiting
    delay(1);
}

extern char PAD_STATET[];
char get_pad_new(char pad) { return PAD_STATET[(unsigned int)pad]; }

extern volatile char FRAME_CNT1;
char get_frame_count(void) { return FRAME_CNT1; }