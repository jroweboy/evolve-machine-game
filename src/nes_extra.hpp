
#pragma once

__attribute__((leaf)) void pal_fade_to(char from, char to, char duration);

// pad 0 or 1, use AFTER pad_poll() to get the trigger / new button presses
// more efficient than pad_trigger, which runs the entire pad_poll code again
__attribute__((leaf)) char get_pad_new(char pad);

// use this internal value to time events, this ticks up every frame
__attribute__((leaf)) char get_frame_count(void);
