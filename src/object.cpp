
// TODO: move this to a spreadsheet or something i dunno.

#include "object.hpp"

soa::Array<ObjectInitData, (u8)ObjectType::Count>  object_init_data = {{
    {
        .hitbox = { .x = 3, .y = 3, .width = 8, .height = 8},
    },
}};