
function(add_ca65_source)
  set(options)
  set(oneValueArgs TARGET SRC)
  set(multiValueArgs)
  cmake_parse_arguments(CA65 "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT CA65_TARGET)
    message(FATAL_ERROR "CA65 Build TARGET is required!")
  endif()
  if (NOT CA65_SRC)
    message(FATAL_ERROR "CA65 Build SRC folder is required!")
  endif()

  if(NOT CA65_BIN)
    find_program(CA65_BIN ca65 REQUIRED HINTS ${CMAKE_SOURCE_DIR}/tools/${CMAKE_HOST_SYSTEM_NAME}/cc65)
    find_program(OD65_BIN od65 REQUIRED HINTS ${CMAKE_SOURCE_DIR}/tools/${CMAKE_HOST_SYSTEM_NAME}/cc65)
    find_program(LD65_BIN ld65 REQUIRED HINTS ${CMAKE_SOURCE_DIR}/tools/${CMAKE_HOST_SYSTEM_NAME}/cc65)
  endif()
  target_link_options(${CA65_TARGET} PRIVATE
      -Wl,-od65-path=${OD65_BIN}
      -Wl,-ld65-path=${LD65_BIN}
      -Wno-error=unused-command-line-argument
  )
  cmake_path(GET CA65_SRC STEM filestem)
  cmake_path(GET CA65_SRC PARENT_PATH filepath)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gen/obj/${filestem}.o
    COMMAND ${CA65_BIN} --bin-include-dir ${filepath} --include-dir ${filepath} -o ${CMAKE_CURRENT_BINARY_DIR}/gen/obj/${filestem}.o ${CA65_SRC}
    DEPENDS ${CA65_SRC}
    VERBATIM
  )
  target_sources(${CA65_TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gen/obj/${filestem}.o)
endfunction()
