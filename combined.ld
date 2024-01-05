

MEMORY {
    prg_rom_0_extended : ORIGIN = 0x8000, LENGTH = 0x8000 - 0x20
}

SECTIONS {
    .prg_rom_0_extended : { *(.prg_rom_0); . = (. < 0x8000 ? 0x8000 : .); *(.prg_rom_fixed) } > prg_rom_0
}
