cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

# We want at least -Os for everything, even debug. Otherwise the emitted assembly
# can easily overflow code sections

set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -Os -flto")
set(CMAKE_CXX_FLAGS_DEBUG "-Os")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-DNDEBUG -Os -flto")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Os -flto")
# Assembler complains when -DNDEBUG is here
set(CMAKE_ASM_FLAGS_RELEASE "-Os -flto")
set(CMAKE_ASM_FLAGS_DEBUG "-Os")
set(CMAKE_ASM_FLAGS_MINSIZEREL "-Os -flto")
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO "-Os -flto")

# Configure the project to use llvm-mos targeting the NES with the MMC1 mapper
set(LLVM_MOS_PLATFORM nes-action53)
find_package(llvm-mos-sdk REQUIRED)
find_program(CA65_BIN ca65 REQUIRED HINTS ${CMAKE_SOURCE_DIR}/tools/${CMAKE_HOST_SYSTEM_NAME}/cc65)

project(evolve_machine CXX ASM)

add_compile_options(
    -g -gdwarf-4           # We want debug info generated for all builds
    # -Wall -Wextra -Werror  # Compiler's got your back
    -std=c++23             # We _can_ have nice things!
)

add_link_options(
    -g -gdwarf-4
    # -Wall -Wextra -Werror # Same goes for the linker
    -mreserve-zp=10

    # make functions non-recursive by default
    -fnonreentrant
    # -Map generates a file with information about how the final binary's memory
    # is laid out. This will eventually be used to show the remaining space in
    # CHR-ROM, PRG-ROM, PRG-RAM, System RAM, and ZEROPAGE
    
    # --lto-whole-program-visibility helps ensure clang does as many
    # optimizations as possible
    -Wl,-Map,${CMAKE_PROJECT_NAME}.map,--lto-whole-program-visibility
)

# Gather all project source and header files to compile into the final ROM image
# set(SRC ${CMAKE_SOURCE_DIR}/src)

find_package(Python3 REQUIRED)
# find the exe for nestile. we just downloaded it and committed it to the repo because why not.
if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
  find_file(FAMISTUDIO_DLL FamiStudio.dll REQUIRED HINTS ${CMAKE_SOURCE_DIR}/tools/${CMAKE_HOST_SYSTEM_NAME}/famistudio)
  set(FAMISTUDIO dotnet ${FAMISTUDIO_DLL})
else()
  find_program(FAMISTUDIO famistudio REQUIRED HINTS ${CMAKE_SOURCE_DIR}/tools/${CMAKE_HOST_SYSTEM_NAME}/famistudio)
endif()

set(FAMISTUDIO_CMD ${FAMISTUDIO} 
    ${CMAKE_SOURCE_DIR}/audio/evolve_machine.fms 
    famistudio-asm-export 
    ${CMAKE_CURRENT_BINARY_DIR}/gen/audio/evolve_machine.s 
    -famistudio-asm-format:ca65
)

find_program(CA65_BIN ca65 REQUIRED HINTS ${CMAKE_SOURCE_DIR}/tools/${CMAKE_HOST_SYSTEM_NAME}/cc65)
find_program(OD65_BIN od65 REQUIRED HINTS ${CMAKE_SOURCE_DIR}/tools/${CMAKE_HOST_SYSTEM_NAME}/cc65)
find_program(LD65_BIN ld65 REQUIRED HINTS ${CMAKE_SOURCE_DIR}/tools/${CMAKE_HOST_SYSTEM_NAME}/cc65)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gen/audio/evolve_machine.s
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/gen/audio
    COMMAND ${FAMISTUDIO_CMD}
    DEPENDS ${CMAKE_SOURCE_DIR}/audio/evolve_machine.fms
)

function(add_ca65_object out src)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gen/obj/${out}
    COMMAND ${CA65_BIN} --bin-include-dir ${CMAKE_CURRENT_BINARY_DIR}/gen/audio --include-dir ${CMAKE_CURRENT_BINARY_DIR}/gen/audio -o ${CMAKE_CURRENT_BINARY_DIR}/gen/obj/${out} ${CMAKE_SOURCE_DIR}/ca65bin/${src}
    DEPENDS ${CMAKE_SOURCE_DIR}/ca65bin/${src}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/gen/audio/evolve_machine.s
    VERBATIM)
endfunction()

add_ca65_object(donut.o donut.s)
add_ca65_object(music.o music.s)

set(ROM ${CMAKE_PROJECT_NAME}.nes)

add_executable(${ROM}
    src/ca65.s
    src/game.s
    src/graphics.s
    src/sprite_render.s
    src/vector_math.s

    src/common.hpp
    src/dungeon_generator.cpp
    src/dungeon_generator.hpp
    src/game.cpp
    src/game.hpp
    src/main.cpp
    src/map_loader.cpp
    src/map_loader.hpp
    src/music.hpp
    src/nes_extra.cpp
    src/nes_extra.hpp
    src/object.cpp
    src/object.hpp
    src/rand.cpp
    src/rand.hpp
    src/sprite_render.hpp
    src/sprite_render.cpp
    src/sprite_render.hpp
    src/title_screen.cpp
    src/title_screen.hpp

    ${CMAKE_CURRENT_BINARY_DIR}/gen/obj/donut.o
    ${CMAKE_CURRENT_BINARY_DIR}/gen/obj/music.o
)
target_compile_definitions(${ROM} PRIVATE NES)

target_include_directories(${ROM} PRIVATE gen)

include(BuildMetasprites)
build_metasprites(TARGET ${CMAKE_SOURCE_DIR}/src/sprite_render.s SRC ${CMAKE_SOURCE_DIR}/chr DEST ${CMAKE_CURRENT_BINARY_DIR}/gen)
include(BuildNESTiles)
build_nes_tiles(TARGET ${CMAKE_SOURCE_DIR}/src/graphics.s SRC ${CMAKE_SOURCE_DIR}/chr DEST ${CMAKE_CURRENT_BINARY_DIR}/gen)
target_include_directories(${ROM} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gen)

target_link_options(${ROM} PRIVATE
    -Wl,-od65-path=${OD65_BIN}
    -Wl,-ld65-path=${LD65_BIN}
)
target_link_options(${ROM} PRIVATE
    -Tlink.ld
    -T${CMAKE_SOURCE_DIR}/combined.ld
)

# -Wl,-ld65-path=${LD65_BIN}

target_link_libraries(${ROM} PRIVATE
    GraphicsAssets
    # GeneratedCA65Binaries
    Metasprites
    # Second-party, included in llvm-mos
    neslib
    # skip using NESDOUG since we don't want to include the extra 128 bytes of ram for its VRAM buffer
    # nesdoug
)
