

#include "common.hpp"
#include "game.hpp"
#include "object.hpp"
#include "sprite_render.hpp"

extern volatile char FRAME_CNT1;

// marks if the previous frame was a lag frame.
static bool lag_frame;

static void move_player() {
    auto player = objects[0];

    
}

namespace Game {
    void update() {
        u8 frame = FRAME_CNT1;

        // Skip drawing sprites this frame if we lagged on the previous frame.
        // This should help us get caught up if we process too much in one frame
        if (lag_frame) {
            Sprite::draw_objects();
        }
        lag_frame = frame == FRAME_CNT1;
    }
}