
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

  set(OUT ${COMPRESS_CHR_DEST}/chr)
  
  file(GLOB infiles "${COMPRESS_CHR_SRC}/*.chr")
  file(GLOB outfiles "${COMPRESS_CHR_SRC}/*.chr")
  list(TRANSFORM outfiles REPLACE "${COMPRESS_CHR_SRC}/(.*)\.chr" "${OUT}/\\1\.bin")
  find_package(Python3 REQUIRED)

  find_file(compressor_script NAMES donut donut.py)
  if (NOT compressor_script)
    message(FATAL_ERROR "Cannot generate compressed CHR: Unable to find compressor script donut.py")
  endif()

  add_custom_command(
    OUTPUT ${outfiles}
    # Compress all of the CHR files into bin files
    COMMAND ${Python3_EXECUTABLE} ${compressor_script} -f ${COMPRESS_CHR_SRC} ${OUT}
    DEPENDS ${compressor_script}
    DEPENDS ${infiles}
    COMMENT "Generating compressed CHR files into gen folder"
  )
  add_library(CompressedCHR ${outfiles})
  set_target_properties(CompressedCHR PROPERTIES LINKER_LANGUAGE CXX)
  target_include_directories(CompressedCHR PUBLIC ${COMPRESS_CHR_DEST})
endfunction()
