
function(compress_chr)
  set(options)
  set(oneValueArgs TARGET SRC DEST)
  set(multiValueArgs)
  cmake_parse_arguments(COMPRESS_CHR "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT COMPRESS_CHR_TARGET)
    message(FATAL_ERROR "Compressed CHR Gen TARGET is required!")
  endif()
  if (NOT COMPRESS_CHR_SRC)
    message(FATAL_ERROR "Compressed CHR Gen SRC folder is required!")
  endif()
  if (NOT COMPRESS_CHR_DEST)
    message(FATAL_ERROR "Compressed CHR Gen DEST folder is required!")
  endif()
  
  file(GLOB infiles "${COMPRESS_CHR_SRC}/*")
  file(GLOB outfiles "${COMPRESS_CHR_SRC}/*")
  list(TRANSFORM outfiles REPLACE "${COMPRESS_CHR_SRC}/(.*)" "${COMPRESS_CHR_DEST}/\\1")
  find_package(Python3 REQUIRED)

  find_file(compressor_script NAMES donut donut.py)
  if (NOT compressor_script)
    message(FATAL_ERROR "Cannot generate compressed CHR: Unable to find compressor script donut.py")
  endif()

  add_custom_command(
    OUTPUT ${outfiles}
    # Compress all of the CHR files into bin files
    COMMAND ${Python3_EXECUTABLE} ${compressor_script} -f ${COMPRESS_CHR_SRC} ${COMPRESS_CHR_DEST}
    DEPENDS ${compressor_script}
    DEPENDS ${infiles}
    COMMENT "Generating compressed CHR files into gen folder"
  )

  
if(NOT CMAKE_SCRIPT_MODE_FILE)
  add_library(CompressedCHR ${outfiles})
  set_target_properties(CompressedCHR PROPERTIES LINKER_LANGUAGE CXX)
  target_include_directories(CompressedCHR PUBLIC ${COMPRESS_CHR_DEST})

  foreach(FILE IN ITEMS ${outfiles})
    set_source_files_properties(${COMPRESS_CHR_TARGET} PROPERTIES OBJECT_DEPENDS ${FILE})
  endforeach()
endif()

endfunction()

if(CMAKE_SCRIPT_MODE_FILE AND NOT CMAKE_PARENT_LIST_FILE)
  message ("running compress?")
  compress_chr(TARGET ${CMAKE_ARGV3} SRC ${CMAKE_ARGV4} DEST ${CMAKE_ARGV5})
endif()