
#include <6502.h>
#include <neslib.h>
#include <peekpoke.h>
#include <bank.h>

#include "common.hpp"
#include "dungeon_generator.hpp"
#include "game.hpp"
#include "object.hpp"
#include "map_loader.hpp"
#include "soa.h"
#include "title_screen.hpp"

GameMode game_mode;
GameMode prev_game_mode;

__attribute__((section(".zp"))) _BitInt(24) global_timer;

// Reserve 3 bytes of RAM as an editable IRQ handler. 
// The linker is expecting this to be named exactly irq
__attribute__((section(".noinit.late"))) void (*irq_pointer)();
__attribute__((section(".noinit.late"))) u8 irq_counter;
__attribute__((section(".noinit.late"))) bool has_epsm;

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


// Define the global object array
soa::Array<Object, OBJECT_COUNT> objects;

static void main_init() {    
    prev_game_mode = (GameMode) 0xff;
    
    // Disable the PPU so we can freely modify its state
    ppu_off();

    set_mirroring(MIRROR_VERTICAL);

    // set 8x16 sprite mode
    oam_size(1);

    // Use lower half of PPU memory for background tiles
    bank_bg(0);

    // Clear out all the objects
    for (auto obj : objects) {
        obj.state = State::Dead;
    }

    // Initialize the player object
    auto player = objects[0];
    player.metasprite = 0;
    player.animation_frame = 0;
    player.state = State::Hidden;
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

int main() {
    main_init();
    irq_detect();

    while (true) {
        POKE(0x4123, 1);
        ppu_wait_nmi();
        POKE(0x4123, 0);

        pad_poll(0);

        // If we changed game modes, initialze the new one
        if (game_mode != prev_game_mode) {
        switch (game_mode) {
            case GameMode::TitleScreen:
            Titlescreen::init();
            break;
            case GameMode::GenerateDungeon:
            generate_dungeon();
            game_mode = GameMode::MapLoader;
            // FALLTHROUGH
            case GameMode::MapLoader:
            MapLoader::init();
            game_mode = GameMode::GamePlay;
            // FALLTHROUGH
            case GameMode::GamePlay:
                // Game::init();
            default:
            break;
        }
        prev_game_mode = game_mode;
        }
        
        // Run this frame of the game mode
        switch (game_mode) {
        case GameMode::TitleScreen:
            Titlescreen::update();
            break;
        case GameMode::MapLoader:
            MapLoader::update();
            break;
        case GameMode::GamePlay:
            Game::update();
            break;
        default:
            break;
        }
    }
}