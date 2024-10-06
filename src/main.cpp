
#include <6502.h>
#include <neslib.h>
#include <peekpoke.h>
#include <mapper.h>

#include "common.hpp"
#include "dungeon_generator.hpp"
#include "game.hpp"
#include "map_loader.hpp"
#include "music.hpp"
#include "object.hpp"
#include "soa.h"
#include "sprite_render.hpp"
#include "title_screen.hpp"

MainMode main_mode;
MainMode prev_main_mode;

__attribute__((section(".zp"))) _BitInt(24) global_timer;

// Reserve 3 bytes of RAM as an editable IRQ handler. 
// The linker is expecting this to be named exactly irq
noinit void (*irq_pointer)();
noinit u8 irq_counter;
noinit bool has_epsm;

// Define the global object arrays
noinit soa::Array<Object, OBJECT_COUNT> objects;
noinit soa::Array<SolidObject, SOLID_OBJECT_COUNT> solid_objects;
noinit u8 solid_object_offset;

noinit ObjectType equipped_weapon;
noinit ObjectType weapon_inventory[4];


// IRQ handler that will just increment a counter and return
extern "C" void irq_detection();
extern "C" void irq_default();
asm(R"ASM(
.section .text.irqdetect,"ax",@progbits
.globl irq
irq:
    pha
        ; Ack the IRQ source on the EPSM
        lda #$27
        sta $401c
        lda #$30
        sta $401d
        jmp (irq_pointer)

.globl irq_detection
irq_detection:
        ; inform the main thread that we did something
        inc irq_counter
    pla
    rti

.globl irq_default
irq_default:
    pla
    rti

)ASM");

extern "C" void __putchar(char c) { POKE(0x401b, c); }

static void main_init() {    
    prev_main_mode = (MainMode) 0xff;
    
    // Disable the PPU so we can freely modify its state
    ppu_off();

    set_chr_bank(3);
    vram_adr(0);
    vram_fill(0xff, 0x2000);
    // u32 magic;
    // vram_adr(0x2000 - 4);
    // vram_read(&magic, 4);
    // if (magic != 0x600dc0de) {
        // vram_adr(0);
        // vram_fill(0xff, 0x2000);
        // vram_adr(0x2000 - 4);
        // u32 magic = 0x600dc0de;
        // vram_write(&magic, 4);
        // warm_boot = true; // todo figure out what to warm boot
    // }

    set_mirroring(MIRROR_VERTICAL);

    // set 8x16 sprite mode
    oam_size(1);

    // Use lower half of PPU memory for background tiles
    bank_bg(0);

    // Clear out all the objects
    for (auto obj : objects) {
        obj.state = State::Dead;
    }

    set_prg_bank(2);
    
    move_all_sprites_offscreen();
}

void irq_detect() {
    // Switch the IRQ handler to increment a counter
    irq_pointer = irq_detection;

    u8 current_counter = irq_counter;

    CLI();
    
    POKE(0x401c, 0x29); // Enable IRQ
    POKE(0x401d, 0x8f);
    POKE(0x401c, 0x25); // Timer A Lo
    POKE(0x401d, 0x00);
    POKE(0x401c, 0x24); // Timer A Hi
    POKE(0x401d, 0xff);
    POKE(0x401c, 0x27); // Load and Enable Timer A IRQ
    POKE(0x401d, 0x05);

    ppu_wait_nmi();

    // Reset IRQ in case it conflicts with a cart mapper and did not get reset earlier.
    POKE(0x401c, 0x27);
    POKE(0x401d, 0x30);

    SEI();

    has_epsm = current_counter != irq_counter;

    // Reset IRQ back to the default "just return" handler
    irq_pointer = irq_default;
}

prg_rom_2 noinline static void run_gamemode() {
    // I split this out so we can turn off inlining to put it in its own bank
    // Run this frame of the game mode
    switch (main_mode) {
    case MainMode::TitleScreen:
        Titlescreen::update();
        break;
    case MainMode::GamePlay:
        Game::update();
        break;
    default:
        break;
    }
}

int main() {
    main_init();
    irq_detect();

    while (true) {
        POKE(0x4123, 1);
        ppu_wait_nmi();
        POKE(0x4123, 0);

        pad_poll(0);
        u8 start_id = 0;

        // If we changed game modes, initialze the new one
        if (main_mode != prev_main_mode) {
            switch (main_mode) {
            case MainMode::TitleScreen:
                Titlescreen::init();
                break;
            case MainMode::GenerateDungeon: {
                set_prg_bank(2);
                u8 total_rooms = 0;
                do {
                    const auto[room_id, room_count] = Dungeon::generate_dungeon();
                    total_rooms = room_count;
                    start_id = room_id;
                } while (total_rooms < Dungeon::ROOM_MINIMUM);
                
                main_mode = MainMode::GamePlay;
                // FALLTHROUGH
                equipped_weapon = ObjectType::None;
                MapLoader::load_map(start_id);
                Game::init();
                next_song = Song::DangerousCitty;
            }
            case MainMode::GamePlay:
                break;
            default:
                break;
            }
            prev_main_mode = main_mode;
        }
        set_prg_bank(CODE_BANK);
        run_gamemode();
    }
}