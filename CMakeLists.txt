cmake_minimum_required(VERSION 3.20)

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

project(evolve_machine CXX ASM)

add_compile_options(
    -g -gdwarf-4           # We want debug info generated for all builds
    # -Wall -Wextra -Werror  # Compiler's got your back
    -std=c++23             # We _can_ have nice things!
)

add_link_options(
    -g -gdwarf-4
    # -Wall -Wextra -Werror # Same goes for the linker
    -mreserve-zp=19

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

set(ROM ${CMAKE_PROJECT_NAME}.nes)
add_executable(${ROM}
    src/ca65.s
    src/chr.cpp
    src/chr.hpp
    src/chr.s
    src/common.hpp
    src/map_loader.cpp
    src/map_loader.hpp
    src/main.cpp
    src/rand.cpp
    src/rand.hpp
    src/title_screen.cpp
    src/title_screen.hpp
)

target_include_directories(${ROM} PRIVATE gen)

include(CompressCHR)
compress_chr(SRC ${CMAKE_SOURCE_DIR}/chr/raw DEST ${CMAKE_SOURCE_DIR}/gen/chr)
include(GenerateCA65Binary)
generate_ca65_binary(SRC ${CMAKE_SOURCE_DIR}/ca65bin DEST ${CMAKE_SOURCE_DIR}/gen/ca65)

target_link_libraries(${ROM} PRIVATE
    # first-party
    CompressedCHR
    GeneratedCA65Binaries
    # Second-party, included in llvm-mos
    neslib
    nesdoug
)
