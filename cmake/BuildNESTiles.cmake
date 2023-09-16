
include(CompressCHR)

function(build_nes_tiles)
  set(options)
  set(oneValueArgs TARGET SRC DEST)
  set(multiValueArgs)
  cmake_parse_arguments(TILES "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT TILES_TARGET)
    message(FATAL_ERROR "CA65 Build TARGET is required!")
  endif()
  if (NOT TILES_SRC)
    message(FATAL_ERROR "CA65 Build SRC folder is required!")
  endif()
  if (NOT TILES_DEST)
    message(FATAL_ERROR "CA65 Build DEST folder is required!")
  endif()

  set(nestiler_dir ${CMAKE_SOURCE_DIR}/tools/${CMAKE_HOST_SYSTEM_NAME}/nestiler)

  if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    find_file(nestiler_exe nestiler REQUIRED HINTS ${nestiler_dir})
    set(NESTILER "dotnet ${nestiler_exe}")
  else()
    find_program(NESTILER nestiler REQUIRED HINTS ${nestiler_dir})
  endif()
  find_package(Python3 REQUIRED)
  # find the exe for nestile. we just downloaded it and committed it to the repo because why not.
  set(CMAKE_FIND_DEBUG_MODE TRUE)
  set(CMAKE_FIND_DEBUG_MODE FALSE)

  find_file(compressor_script NAMES donut donut.py)
  if (NOT compressor_script)
    message(FATAL_ERROR "Cannot generate compressed CHR: Unable to find compressor script donut.py")
  endif()

  # build the room graphics
  # basepath is the input folder
  set(room_basepath ${TILES_SRC}/rooms)
  set(special_basepath ${TILES_SRC}/special)
  set(tmp_rawpath ${TILES_DEST}/raw)
  set(tmp_catpath ${TILES_DEST}/raw/concat)
  set(outpath ${TILES_DEST}/graphics)
  
  # i'm sick of writing cmake so i'm just hardcoding the file paths for now
  set(room_infiles
    ${room_basepath}/bottom.bmp
    ${room_basepath}/left.bmp
    ${room_basepath}/right.bmp
    ${room_basepath}/single.bmp
    ${room_basepath}/start.bmp
    ${room_basepath}/top.bmp
  )
  # nestiler expects a list of input names in a certain format so use hardcode that too
  set(room_infiles_indexed 
    -i0 ${room_basepath}/bottom.bmp
    -i1 ${room_basepath}/left.bmp
    -i2 ${room_basepath}/right.bmp
    -i3 ${room_basepath}/single.bmp
    -i4 ${room_basepath}/start.bmp
    -i5 ${room_basepath}/top.bmp
  )
  set(room_nametable 
    -a0 ${tmp_rawpath}/bottom.nmt
    -a1 ${tmp_rawpath}/left.nmt
    -a2 ${tmp_rawpath}/right.nmt
    -a3 ${tmp_rawpath}/single.nmt
    -a4 ${tmp_rawpath}/start.nmt
    -a5 ${tmp_rawpath}/top.nmt
  )
  set(room_attrs
    -u0 ${tmp_rawpath}/bottom.attr
    -u1 ${tmp_rawpath}/left.attr
    -u2 ${tmp_rawpath}/right.attr
    -u3 ${tmp_rawpath}/single.attr
    -u4 ${tmp_rawpath}/start.attr
    -u5 ${tmp_rawpath}/top.attr
  )
  
  set(room_outfiles ${room_infiles})
  list(TRANSFORM room_outfiles REPLACE "${room_basepath}/(.*)\.bmp" "${outpath}/\\1\.bin")

  add_custom_command(
    OUTPUT ${room_outfiles}

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${outpath}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${tmp_catpath}

    # nestiler all the rooms
    COMMAND ${NESTILER} -c ${nestiler_dir}/nestiler-colors.json --mode bg --lossy 1 --share-pattern-table --out-pattern-table-0 ${tmp_catpath}/room_chr.chr ${room_infiles_indexed} ${room_nametable} ${room_attrs}
    
    # and then the special ones individually
    COMMAND ${NESTILER} -c ${nestiler_dir}/nestiler-colors.json  --mode bg --lossy 1 --out-pattern-table-0 ${tmp_catpath}/titlescreen_chr.bin -i0 ${special_basepath}/titlescreen.bmp -a0 ${tmp_rawpath}/titlescreen.nmt -u0 ${tmp_rawpath}/titlescreen.attr
    
    COMMAND ${CMAKE_COMMAND} -E cat ${tmp_rawpath}/bottom.nmt  ${tmp_rawpath}/bottom.attr  > ${tmp_catpath}/bottom.bin
    COMMAND ${CMAKE_COMMAND} -E cat ${tmp_rawpath}/left.nmt    ${tmp_rawpath}/left.attr    > ${tmp_catpath}/left.bin
    COMMAND ${CMAKE_COMMAND} -E cat ${tmp_rawpath}/right.nmt   ${tmp_rawpath}/right.attr   > ${tmp_catpath}/right.bin
    COMMAND ${CMAKE_COMMAND} -E cat ${tmp_rawpath}/single.nmt  ${tmp_rawpath}/single.attr  > ${tmp_catpath}/single.bin
    COMMAND ${CMAKE_COMMAND} -E cat ${tmp_rawpath}/start.nmt   ${tmp_rawpath}/start.attr   > ${tmp_catpath}/start.bin
    COMMAND ${CMAKE_COMMAND} -E cat ${tmp_rawpath}/top.nmt     ${tmp_rawpath}/top.attr     > ${tmp_catpath}/top.bin
    
    COMMAND ${CMAKE_COMMAND} -E cat ${tmp_rawpath}/titlescreen.nmt     ${tmp_rawpath}/titlescreen.attr     > ${tmp_catpath}/titlescreen.bin

    COMMAND ${Python3_EXECUTABLE} ${compressor_script} -f ${tmp_catpath} ${outpath}

    DEPENDS ${room_infiles} ${special_basepath}/titlescreen.bmp
    COMMENT "Running nestiler on the room backgrounds"
  )

  add_library(GraphicsAssets ${room_outfiles})
  set_target_properties(GraphicsAssets PROPERTIES LINKER_LANGUAGE CXX)
  target_include_directories(GraphicsAssets PUBLIC ${TILES_DEST})
  
  foreach(FILE IN ITEMS ${room_outfiles})
    set_source_files_properties(${TILES_TARGET} PROPERTIES OBJECT_DEPENDS ${FILE})
  endforeach()
endfunction()
