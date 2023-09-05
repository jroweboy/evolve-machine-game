
function(generate_ca65_binary)
  set(options)
  set(oneValueArgs SRC DEST)
  set(multiValueArgs)
  cmake_parse_arguments(GEN_BINARY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT GEN_BINARY_SRC)
    message(FATAL_ERROR "CA65 Build SRC folder is required!")
  endif()
  if (NOT GEN_BINARY_DEST)
    message(FATAL_ERROR "CA65 Build DEST folder is required!")
  endif()

  # set(ca65_artifacts ${GEN_BINARY_DEST}/ca65_build_output.h)
  # file(GLOB outputfiles "${GEN_BINARY_SRC}/*.chr")
  # list(TRANSFORM outputfiles REPLACE "${GEN_BINARY_SRC}/(.*)\.chr" "${GEN_BINARY_DEST}/\\1\.bin")
  set(outputfiles ${GEN_BINARY_DEST}/prg8.bin ${GEN_BINARY_DEST}/prgc.bin)

  find_package(PythonInterp 3 REQUIRED)
  # find_file(bin2h NAMES bin2h bin2h.py)
  # if (NOT bin2h)
  #   message(FATAL_ERROR "Cannot generate ca65 output: Unable to find bin2h.py")
  # endif()

  add_custom_command(
    OUTPUT ${outputfiles}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/ca65
    # build the files into a raw dump. the linker file is setup to spit out the banks into different files
    COMMAND ${GEN_BINARY_SRC}/cl65 --no-target-lib -C ${GEN_BINARY_SRC}/llvm.cfg ${GEN_BINARY_SRC}/music.s ${GEN_BINARY_SRC}/donut.s 
    # Then run each file through bin2h
    # COMMAND ${PYTHON_EXECUTABLE} ${bin2h} ${CMAKE_CURRENT_BINARY_DIR}/ca65 -o ${ca65_artifacts}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/ca65 ${GEN_BINARY_DEST}
    DEPENDS ${compressor_script} ${GEN_BINARY_SRC} ${GEN_BINARY_SRC}/llvm.cfg ${GEN_BINARY_SRC}/music.s ${GEN_BINARY_SRC}/donut.s # ${bin2h} 
    COMMENT "Generating compressed CHR files into header file"
  )
  add_library(GeneratedCA65Binaries ${outputfiles})
  set_target_properties(GeneratedCA65Binaries PROPERTIES LINKER_LANGUAGE CXX)
  # target_include_directories(GeneratedCA65Binaries PUBLIC ${GEN_BINARY_DEST})
endfunction()
