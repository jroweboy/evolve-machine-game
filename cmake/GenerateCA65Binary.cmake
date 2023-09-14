
function(generate_ca65_binary)
  set(options)
  set(oneValueArgs TARGET SRC DEST)
  set(multiValueArgs)
  cmake_parse_arguments(GEN_BINARY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT GEN_BINARY_TARGET)
    message(FATAL_ERROR "CA65 Build TARGET is required!")
  endif()
  if (NOT GEN_BINARY_SRC)
    message(FATAL_ERROR "CA65 Build SRC folder is required!")
  endif()
  if (NOT GEN_BINARY_DEST)
    message(FATAL_ERROR "CA65 Build DEST folder is required!")
  endif()

  set(DEST ${GEN_BINARY_DEST}/ca65)

  set(outfiles ${DEST}/prg8.bin ${DEST}/prgc.bin)

  find_package(Python3 REQUIRED)

  # find the exe for nestile. we just downloaded it and committed it to the repo because why not.
  find_program(FAMISTUDIO famistudio REQUIRED HINTS ${CMAKE_SOURCE_DIR}/tools/${CMAKE_HOST_SYSTEM_NAME}/famistudio)

  set(music_in ${CMAKE_SOURCE_DIR}/audio/evolve_machine.fms)
  set(music_outpath ${GEN_BINARY_DEST}/audio)
  set(music_outfiles ${music_outpath}/evolve_machine.s ${music_outpath}/evolve_machine.dmc)

  set(FAMISTUDIO_CMD ${FAMISTUDIO} ${music_in} famistudio-asm-export
    ${music_outpath}/evolve_machine.s
    -famistudio-asm-format:ca65
  )
  if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    set(FAMISTUDIO_CMD xvfb-run --auto-servernum mono ${FAMISTUDIO_CMD})
  endif()

  set(out ${CMAKE_CURRENT_BINARY_DIR}/ca65build)

  find_program(GENCA65BIN_CL65 cl65 REQUIRED HINTS ${GEN_BINARY_SRC})

  set(BUILD_CL65_CMD ${GENCA65BIN_CL65} -g 
    -o ${out}/empty.nes -m ${out}/map.txt 
    --bin-include-dir ${GEN_BINARY_DEST}/audio 
    --asm-include-dir ${GEN_BINARY_DEST}/audio 
    --ld-args --dbgfile,${out}/ca65.dbg 
    --no-target-lib -C ${GEN_BINARY_SRC}/llvm.cfg 
    ${GEN_BINARY_SRC}/music.s ${GEN_BINARY_SRC}/donut.s
  )

  # hacky to get source level debugging to work, i'm trying to split out the prg manually
  
  find_file(split_nes NAMES split_nes_output split_nes_output.py)
  if (NOT split_nes)
    message(FATAL_ERROR "Cannot split ca65 rom: Unable to find script split_nes_output.py")
  endif()
  set(SPLIT_PRG_CMD ${Python3_EXECUTABLE} ${split_nes} ${out})

  message(${FAMISTUDIO_CMD})
  add_custom_command(
    OUTPUT ${outfiles} ${music_outfiles}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${out}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DEST}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${music_outpath}
    # run famistudio to spit out the audio files from the fms
    COMMAND echo Running Famistudio export
    COMMAND ${FAMISTUDIO_CMD}
    # build the files into a raw dump.
    COMMAND echo Building CA65 sources into binaries
    COMMAND ${BUILD_CL65_CMD}
    COMMAND echo Splitting out the compiled sources from the ROM into banks
    COMMAND ${SPLIT_PRG_CMD}
    COMMAND ${CMAKE_COMMAND} -E copy ${out}/prg8.bin ${out}/prgc.bin ${DEST}
    DEPENDS ${compressor_script}
    DEPENDS ${split_nes}
    DEPENDS ${GEN_BINARY_SRC}/famistudio_ca65.s
    DEPENDS ${GEN_BINARY_SRC}/llvm.cfg
    DEPENDS ${GEN_BINARY_SRC}/music.s
    DEPENDS ${GEN_BINARY_SRC}/donut.s
    DEPENDS ${music_in}
    COMMENT "Building CA65 artifacts and copying to gen folder"
  )

  add_library(GeneratedCA65Binaries ${outfiles})
  set_target_properties(GeneratedCA65Binaries PROPERTIES LINKER_LANGUAGE CXX)
  target_include_directories(GeneratedCA65Binaries PUBLIC ${GEN_BINARY_DEST})
  
  foreach(FILE IN ITEMS ${outfiles})
    set_source_files_properties(${GEN_BINARY_TARGET} PROPERTIES OBJECT_DEPENDS ${FILE})
  endforeach()
  foreach(FILE IN ITEMS ${music_outfiles})
    set_source_files_properties(${GEN_BINARY_TARGET} PROPERTIES OBJECT_DEPENDS ${FILE})
  endforeach()
endfunction()
