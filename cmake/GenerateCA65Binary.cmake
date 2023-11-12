
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

  # set(outfiles ${DEST}/prg8.bin ${DEST}/prgc.bin)

  find_package(Python3 REQUIRED)
  # find the exe for nestile. we just downloaded it and committed it to the repo because why not.
  if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    find_file(FAMISTUDIO_DLL FamiStudio.dll REQUIRED HINTS ${CMAKE_SOURCE_DIR}/tools/${CMAKE_HOST_SYSTEM_NAME}/famistudio)
    set(FAMISTUDIO dotnet ${FAMISTUDIO_DLL})
  else()
    find_program(FAMISTUDIO famistudio REQUIRED HINTS ${CMAKE_SOURCE_DIR}/tools/${CMAKE_HOST_SYSTEM_NAME}/famistudio)
  endif()

  set(music_in ${CMAKE_SOURCE_DIR}/audio/evolve_machine.fms)
  set(music_outpath ${GEN_BINARY_DEST}/audio)
  set(music_outfiles ${music_outpath}/evolve_machine.s ${music_outpath}/evolve_machine.dmc)

  set(FAMISTUDIO_CMD ${FAMISTUDIO} ${music_in} famistudio-asm-export ${music_outpath}/evolve_machine.s -famistudio-asm-format:ca65)

  set(out ${CMAKE_CURRENT_BINARY_DIR}/ca65build)

  find_program(CA65_BIN ca65 REQUIRED HINTS ${CMAKE_SOURCE_DIR}/tools/${CMAKE_HOST_SYSTEM_NAME}/cc65)

  function(add_ca65_object out src)
    add_custom_command(
      OUTPUT ${GEN_BINARY_DEST}/obj/${out}
      COMMAND ${CA65_BIN} --bin-include-dir ${GEN_BINARY_DEST}/audio --include-dir ${GEN_BINARY_DEST}/audio -o ${GEN_BINARY_DEST}/obj/${out} ${GEN_BINARY_SRC}/${src}
      DEPENDS ${GEN_BINARY_SRC}/${src}
      VERBATIM)
  endfunction()

  # add_custom_command(
  #   OUTPUT ${outfiles}
  #   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  #   COMMAND ${CMAKE_COMMAND} -E make_directory ${out}
  #   COMMAND ${CMAKE_COMMAND} -E make_directory ${DEST}
  #   COMMAND ${CMAKE_COMMAND} -E make_directory ${music_outpath}
  #   # run famistudio to spit out the audio files from the fms
  #   COMMAND ${FAMISTUDIO_CMD}
  #   # build the files into a raw dump. the linker file is setup to spit out the banks into different files
  #   # COMMAND ${GENCA65BIN_CA65} -g -m ${out}/map.txt --bin-include-dir ${GEN_BINARY_DEST}/audio --asm-include-dir ${GEN_BINARY_DEST}/audio --ld-args --dbgfile,${out}/out.dbg --no-target-lib -C ${GEN_BINARY_SRC}/llvm.cfg ${GEN_BINARY_SRC}/music.s ${GEN_BINARY_SRC}/donut.s 
  #   # Then run each file through bin2h
  #   # COMMAND ${PYTHON_EXECUTABLE} ${bin2h} ${CMAKE_CURRENT_BINARY_DIR}/ca65 -o ${ca65_artifacts}
  #   # COMMAND ${CMAKE_COMMAND} -E copy ${out}/prg8.bin ${out}/prgc.bin ${DEST}
  #   # DEPENDS ${compressor_script}
  #   # DEPENDS ${GEN_BINARY_SRC}/famistudio_ca65.s
  #   # DEPENDS ${GEN_BINARY_SRC}/llvm.cfg
  #   # DEPENDS ${GEN_BINARY_SRC}/music.s
  #   # DEPENDS ${GEN_BINARY_SRC}/donut.s
  #   DEPENDS ${music_in}
  #   COMMENT "Building CA65 artifacts and copying to gen folder"
  # )

  
  add_library(GeneratedCA65Binaries ${GEN_BINARY_DEST}/obj/donut.o ${GEN_BINARY_DEST}/obj/music.o)
  set_target_properties(GeneratedCA65Binaries PROPERTIES LINKER_LANGUAGE CXX)
  target_include_directories(GeneratedCA65Binaries PUBLIC ${GEN_BINARY_DEST})
  
  foreach(FILE IN ITEMS ${outfiles})
    set_source_files_properties(${GEN_BINARY_TARGET} PROPERTIES OBJECT_DEPENDS ${FILE})
  endforeach()
endfunction()
