


#include "enemy.hpp"
#include "common.hpp"
#include "object.hpp"


extern volatile char FRAME_CNT1;

prg_rom_2 void process_hamster(u8 slot) {
    auto obj = objects[slot];
    if (obj.state == State::HamsterChasing) {
        obj.angle = arctan2(0, slot);
    } else {
        auto dist = distance(0, slot);
        if (dist < 50) {
            obj.state = State::HamsterChasing;
        } else {
            // Spin in a circle
            if ((FRAME_CNT1 & 0b11) == 0b11) {
                obj.angle += 1;
                if (obj.angle > 32) {
                    obj.angle = 0;
                }
            }
        }
    }
    obj.direction = angle_to_direction_lut[obj->angle];
    Objects::move_object_with_solid_collision(slot);
    if (FRAME_CNT1 & 1) {
        obj.metasprite = (Metasprite)((u8)obj->metasprite + 4);
        obj.attribute = PALETTE_WEAPON;
    } else {
        obj.attribute = PALETTE_ENEMY1;
    }
}
