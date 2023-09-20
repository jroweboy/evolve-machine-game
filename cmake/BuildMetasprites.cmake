
function(build_metasprites)
  set(options)
  set(oneValueArgs TARGET SRC DEST)
  set(multiValueArgs)
  cmake_parse_arguments(META "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT META_TARGET)
    message(FATAL_ERROR "Metasprite TARGET folder is required!")
  endif()
  if (NOT META_SRC)
    message(FATAL_ERROR "Metasprite SRC folder is required!")
  endif()
  if (NOT META_DEST)
    message(FATAL_ERROR "Metasprite DEST folder is required!")
  endif()

  find_package(Python3 REQUIRED)

  find_file(metasprite_script NAMES process_metasprite process_metasprite.py)
  if (NOT metasprite_script)
    message(FATAL_ERROR "Cannot build metasprites: Unable to find processing script process_metasprite.py")
  endif()
  
  file(GLOB infiles "${META_SRC}/*")
  file(GLOB outfiles "${META_SRC}/*")
  list(TRANSFORM outfiles REPLACE "${META_SRC}/(.*).chr" "${META_DEST}/\\1_chr.bin")
  list(TRANSFORM outfiles REPLACE "${META_SRC}/(.*).msb" "${META_DEST}/\\1_metasprite.bin")

  set(cmd ${Python3_EXECUTABLE} ${metasprite_script} ${META_SRC} ${META_DEST})
  add_custom_command(
    OUTPUT ${outfiles}

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${META_DEST}

    COMMAND ${cmd}

    DEPENDS ${infiles}
    DEPENDS ${metasprite_script}
  )
  
  add_library(Metasprites ${outfiles})
  set_target_properties(Metasprites PROPERTIES LINKER_LANGUAGE CXX)
  target_include_directories(Metasprites PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gen)

  set_property(SOURCE ${META_TARGET} PROPERTY OBJECT_DEPENDS ${outfiles})

endfunction()