
#include "common.hpp"
#include "rand.hpp"

__attribute__((section(".zp"))) u32 seed;

asm(R"ASM(
.section .text.rand,"axR",@progbits
galois32o:
	; rotate the middle bytes left
	ldy seed+2 ; will move to seed+3 at the end
	lda seed+1
	sta seed+2
	; compute seed+1 ($C5>>1 = %1100010)
	lda seed+3 ; original high byte
	lsr
	sta seed+1 ; reverse: 100011
	lsr
	lsr
	lsr
	lsr
	eor seed+1
	lsr
	eor seed+1
	eor seed+0 ; combine with original low byte
	sta seed+1
	; compute seed+0 ($C5 = %11000101)
	lda seed+3 ; original high byte
	asl
	eor seed+3
	asl
	asl
	asl
	asl
	eor seed+3
	asl
	asl
	eor seed+3
	sty seed+3 ; finish rotating byte 2 into 3
	sta seed+0
	rts

.section .text.rand,"axR",@progbits
.globl galois_rand8
galois_rand8:
    jmp galois32o

.section .text.rand,"axR",@progbits
.globl galois_rand16
galois_rand16:
    jsr galois32o
    tax
    jmp galois32o
    ; rts
)ASM"
);

extern "C" u8 galois_rand8();
extern "C" u16 galois_rand16();

namespace Rng {
    void seed(const u8* buffer) {
        if (buffer == nullptr) {
            u8* cast = reinterpret_cast<u8*>(&::seed);
            cast[0] = global_timer[0];
            cast[1] = global_timer[1];
            cast[2] = global_timer[2];
            return;
        }
        // TODO: handle the case where the user provides a seed!
    }

    u8 rand8() {
        return galois_rand8();
    }

    u16 rand16() {
       return galois_rand16();
    }
}
