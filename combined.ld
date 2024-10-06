INCLUDE link.ld

SECTIONS {
  CLASS(c) { *(.rodata.earlier) }
  .rodata { *(.rodata) CLASS(c) (*.rodata.later) } >prg_rom_0
  .rodata2 { CLASS(c) } >prg_rom_fixed
}