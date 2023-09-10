
function(compress_chr)
  set(options)
  set(oneValueArgs SRC DEST)
  set(multiValueArgs)
  cmake_parse_arguments(COMPRESS_CHR "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT COMPRESS_CHR_SRC)
    message(FATAL_ERROR "Compressed CHR Gen SRC folder is required!")
  endif()
  if (NOT COMPRESS_CHR_DEST)
    message(FATAL_ERROR "Compressed CHR Gen DEST folder is required!")
  endif()
  
  file(GLOB infiles "${COMPRESS_CHR_SRC}/*.chr")
  file(GLOB outputfiles "${COMPRESS_CHR_SRC}/*.chr")
  list(TRANSFORM outputfiles REPLACE "${COMPRESS_CHR_SRC}/(.*)\.chr" "${COMPRESS_CHR_DEST}/\\1\.bin")

  find_package(Python3 REQUIRED)

  find_file(compressor_script NAMES donut donut.py)
  if (NOT compressor_script)
    message(FATAL_ERROR "Cannot generate compressed CHR: Unable to find compressor script donut.py")
  endif()
  # find_file(bin2h NAMES bin2h bin2h.py)
  # if (NOT bin2h)
  #   message(FATAL_ERROR "Cannot generate compressed CHR: Unable to find bin2h.py")
  # endif()

  add_custom_command(
    OUTPUT ${outputfiles}
    # Compress all of the CHR files into bin files
    COMMAND ${PYTHON_EXECUTABLE} ${compressor_script} -f ${COMPRESS_CHR_SRC} ${COMPRESS_CHR_DEST}
    # Then run each file through bin2h
    # COMMAND ${PYTHON_EXECUTABLE} ${bin2h} ${CMAKE_CURRENT_BINARY_DIR}/chr -o ${compressed_chr}
    DEPENDS ${compressor_script} ${infiles} # ${bin2h} 
    COMMENT "Generating compressed CHR files into gen folder"
  )
  add_library(CompressedCHR ${outputfiles})
  set_target_properties(CompressedCHR PROPERTIES LINKER_LANGUAGE CXX)
  target_include_directories(CompressedCHR PUBLIC ${COMPRESS_CHR_DEST})
endfunction()
