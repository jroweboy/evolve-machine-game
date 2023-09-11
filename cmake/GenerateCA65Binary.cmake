
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

  # set(ca65_artifacts ${GEN_BINARY_DEST}/ca65_build_output.h)
  # file(GLOB outputfiles "${GEN_BINARY_SRC}/*.chr")
  # list(TRANSFORM outputfiles REPLACE "${GEN_BINARY_SRC}/(.*)\.chr" "${GEN_BINARY_DEST}/\\1\.bin")
  set(outfiles ${DEST}/prg8.bin ${DEST}/prgc.bin)

  find_package(Python3 REQUIRED)
  # find_file(bin2h NAMES bin2h bin2h.py)
  # if (NOT bin2h)
  #   message(FATAL_ERROR "Cannot generate ca65 output: Unable to find bin2h.py")
  # endif()

  set(out ${CMAKE_CURRENT_BINARY_DIR}/ca65build)

  find_program(GENCA65BIN_CL65 cl65 REQUIRED HINTS ${GEN_BINARY_SRC})

  add_custom_command(
    OUTPUT ${outfiles}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${out}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DEST}
    # build the files into a raw dump. the linker file is setup to spit out the banks into different files
    COMMAND ${GENCA65BIN_CL65} -g -m ${out}/map.txt --ld-args --dbgfile,${out}/out.dbg --no-target-lib -C ${GEN_BINARY_SRC}/llvm.cfg ${GEN_BINARY_SRC}/music.s ${GEN_BINARY_SRC}/donut.s 
    # Then run each file through bin2h
    # COMMAND ${PYTHON_EXECUTABLE} ${bin2h} ${CMAKE_CURRENT_BINARY_DIR}/ca65 -o ${ca65_artifacts}
    COMMAND ${CMAKE_COMMAND} -E copy ${out}/prg8.bin ${out}/prgc.bin ${DEST}
    DEPENDS ${compressor_script}
    DEPENDS ${GEN_BINARY_SRC}/evolve_machine.s
    DEPENDS ${GEN_BINARY_SRC}/famistudio_ca65.s
    DEPENDS ${GEN_BINARY_SRC}/llvm.cfg
    DEPENDS ${GEN_BINARY_SRC}/music.s
    DEPENDS ${GEN_BINARY_SRC}/donut.s
    COMMENT "Building CA65 artifacts and copying to gen folder"
  )

  add_library(GeneratedCA65Binaries ${outfiles})
  set_target_properties(GeneratedCA65Binaries PROPERTIES LINKER_LANGUAGE CXX)
  target_include_directories(GeneratedCA65Binaries PUBLIC ${GEN_BINARY_DEST})
  
  foreach(FILE IN ITEMS ${outfiles})
    set_source_files_properties(${GEN_BINARY_TARGET} PROPERTIES OBJECT_DEPENDS ${FILE})
  endforeach()
endfunction()
