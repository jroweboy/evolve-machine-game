
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

  set(NESTILER_DIR ${CMAKE_SOURCE_DIR}/tools/${CMAKE_HOST_SYSTEM_NAME}/nestiler)
  set(HUFFMUNCH_DIR ${CMAKE_SOURCE_DIR}/tools/)

  if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    find_file(NESTILER_EXE nestiler REQUIRED HINTS ${NESTILER_DIR})
    set(NESTILER ${NESTILER_EXE})
  else()
    find_program(NESTILER nestiler REQUIRED HINTS ${NESTILER_DIR})
  endif()
  find_package(Python3 REQUIRED)
  # find the exe for nestile. we just downloaded it and committed it to the repo because why not.
  # set(CMAKE_FIND_DEBUG_MODE TRUE)
  # set(CMAKE_FIND_DEBUG_MODE FALSE)

  find_file(compressor_script NAMES donut donut.py)
  if (NOT compressor_script)
    message(FATAL_ERROR "Cannot generate compressed CHR: Unable to find compressor script donut.py")
  endif()

  find_file(process_script NAMES process_background process_background.py)
  if (NOT process_script)
    message(FATAL_ERROR "Cannot generate BG data: Unable to find builder script process_background.py")
  endif()

  find_file(tiled_script NAMES process_tiled_maps process_tiled_maps.py)
  if (NOT tiled_script)
    message(FATAL_ERROR "Cannot generate BG data: Unable to find builder script process_tiled_maps.py")
  endif()

  # build the room graphics
  # basepath is the input folder
  set(room_basepath ${TILES_SRC}/rooms)
  set(object_basepath ${TILES_SRC}/objects)
  set(special_basepath ${TILES_SRC}/special)
  set(rawchr_path ${TILES_DEST}/raw/chr)
  set(rawnmt_path ${TILES_DEST}/raw/nmt)
  set(rawtmp_path ${TILES_DEST}/raw/tmp)
  set(rawpal_path ${TILES_DEST}/raw/pal)
  set(out_chrpath ${TILES_DEST}/graphics/chr)
  set(out_nmtpath ${TILES_DEST}/graphics/nmt)
  set(out_atrpath ${TILES_DEST}/graphics/atr)
  set(out_palpath ${TILES_DEST}/graphics/pal)
  set(out_asmpath ${TILES_DEST}/header)
  set(out_compresspath ${TILES_DEST}/compressed)

  set(all_in_files
    # all room bmps
    ${room_basepath}/leftright.bmp
    ${room_basepath}/updown.bmp
    ${room_basepath}/single.bmp
    ${room_basepath}/start.bmp

    # all special bmps
    ${special_basepath}/hudfont.bmp
    ${special_basepath}/titlescreen.bmp

    # all objects
    ${object_basepath}/door_down.bmp
    ${object_basepath}/door_left.bmp
    ${object_basepath}/door_right.bmp
    ${object_basepath}/door_up.bmp
  )

  set(all_out_files

  # ${out_compresspath}/archive.hfm
    ${out_compresspath}/archive.dnt

    # room compressed chr
    # ${out_chrpath}/leftright.chr.dnt
    # ${out_chrpath}/updown.chr.dnt
    # ${out_chrpath}/startupstartdown.chr.dnt
    # ${out_chrpath}/single.chr.dnt

    # ${out_chrpath}/hudfont.chr.dnt
    # ${out_chrpath}/titlescreen.chr.dnt

    # compressed nametables
    # ${out_nmtpath}/down.nmt.dnt
    # ${out_nmtpath}/left.nmt.dnt
    # ${out_nmtpath}/right.nmt.dnt
    # ${out_nmtpath}/single.nmt.dnt
    # ${out_nmtpath}/startup.nmt.dnt
    # ${out_nmtpath}/startdown.nmt.dnt
    # ${out_nmtpath}/up.nmt.dnt
    # ${out_nmtpath}/titlescreen_atr.nmt.dnt

    # attributes for the nmts that aren't special
    ${out_atrpath}/down.atr
    ${out_atrpath}/left.atr
    ${out_atrpath}/right.atr
    ${out_atrpath}/single.atr
    ${out_atrpath}/startup.atr
    ${out_atrpath}/startdown.atr
    ${out_atrpath}/up.atr

    # all objects
    # ${out_nmtpath}/door_down.nmt.dnt
    # ${out_nmtpath}/door_left.nmt.dnt
    # ${out_nmtpath}/door_right.nmt.dnt
    # ${out_nmtpath}/door_up.nmt.dnt
    ${out_atrpath}/door_down.atr
    ${out_atrpath}/door_left.atr
    ${out_atrpath}/door_right.atr
    ${out_atrpath}/door_up.atr

    # asm constants
    ${out_asmpath}/graphics_constants.hpp
    ${out_asmpath}/graphics_constants.s
    ${out_asmpath}/room_collision.s
  )
  
  add_custom_command(
    OUTPUT ${all_out_files}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${out_chrpath}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${out_nmtpath}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${out_atrpath}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${out_palpath}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${out_compresspath}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${out_asmpath}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${rawchr_path}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${rawtmp_path}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${rawnmt_path}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${rawpal_path}
    COMMAND ${Python3_EXECUTABLE} ${process_script} ${NESTILER_DIR} ${HUFFMUNCH_DIR} ${TILES_SRC} ${TILES_DEST}
    COMMAND ${Python3_EXECUTABLE} ${tiled_script} ${TILES_SRC} ${TILES_DEST}
    DEPENDS ${all_in_files} ${process_script} ${compressor_script} ${tiled_script}
  )

  add_library(GraphicsAssets ${all_out_files})
  set_target_properties(GraphicsAssets PROPERTIES LINKER_LANGUAGE CXX)
  target_include_directories(GraphicsAssets PUBLIC ${TILES_DEST})
  
  set_property(SOURCE ${TILES_TARGET} PROPERTY OBJECT_DEPENDS ${all_out_files})

  # i'm sick of writing cmake so i'm just hardcoding the file paths for now
  # nestiler expects a list of input names in a certain format so use hardcode that too
  
  # set(room_updown_opts
  #   -i0 ${room_basepath}/bottom.bmp
  #   -i1 ${room_basepath}/top.bmp
  #   -a0 ${rawnmt_path}/bottom.nmt
  #   -a1 ${rawnmt_path}/top.nmt
  #   -u0 ${out_atrpath}/bottom.attr
  #   -u1 ${out_atrpath}/top.attr
  #   -t0 ${rawpal_path}/updown_0.pal
  #   -t1 ${rawpal_path}/updown_1.pal
  #   -t2 ${rawpal_path}/updown_2.pal
  #   -t3 ${rawpal_path}/updown_3.pal
  # )

  # set(room_leftright_opts
  #   -i0 ${room_basepath}/left.bmp
  #   -i1 ${room_basepath}/right.bmp
  #   -a0 ${rawnmt_path}/left.nmt
  #   -a1 ${rawnmt_path}/right.nmt
  #   -u0 ${out_atrpath}/left.attr
  #   -u1 ${out_atrpath}/right.attr
  #   -t0 ${rawpal_path}/leftright_0.pal
  #   -t1 ${rawpal_path}/leftright_1.pal
  #   -t2 ${rawpal_path}/leftright_2.pal
  #   -t3 ${rawpal_path}/leftright_3.pal
  # )

  # set(room_single_opts
  #   -i0 ${room_basepath}/single.bmp
  #   -a0 ${rawnmt_path}/single.nmt
  #   -u0 ${out_atrpath}/single.attr
  #   -t0 ${rawpal_path}/single_0.pal
  #   -t1 ${rawpal_path}/single_1.pal
  #   -t2 ${rawpal_path}/single_2.pal
  #   -t3 ${rawpal_path}/single_3.pal
  # )

  # set(room_start_opts
  #   -i0 ${room_basepath}/start.bmp
  #   -a0 ${rawnmt_path}/start.nmt
  #   -u0 ${out_atrpath}/start.attr
  #   -t0 ${rawpal_path}/start_0.pal
  #   -t1 ${rawpal_path}/start_1.pal
  #   -t2 ${rawpal_path}/start_2.pal
  #   -t3 ${rawpal_path}/start_3.pal
  # )

  # set(object_infiles
  #   ${object_basepath}/door_up.bmp
  #   ${object_basepath}/door_down.bmp
  #   ${object_basepath}/door_left.bmp
  #   ${object_basepath}/door_right.bmp
  # )
  # set(object_infiles_indexed
  #   -i0 ${object_basepath}/door_up.bmp
  #   -i1 ${object_basepath}/door_down.bmp
  #   -i2 ${object_basepath}/door_left.bmp
  #   -i3 ${object_basepath}/door_right.bmp
  # )
  # set(object_nametable
  #   -a0 ${rawnmt_path}/door_up.nmt
  #   -a1 ${rawnmt_path}/door_down.nmt
  #   -a2 ${rawnmt_path}/door_left.nmt
  #   -a3 ${rawnmt_path}/door_right.nmt
  # )
  # set(object_attrs
  #   -u0 ${out_atrpath}/door_up.attr
  #   -u1 ${out_atrpath}/door_down.attr
  #   -u2 ${out_atrpath}/door_left.attr
  #   -u3 ${out_atrpath}/door_right.attr
  # )
  
  # set(room_updown ${room_basepath}/bottom.bmp ${room_basepath}/top.bmp)
  # set(room_leftright ${room_basepath}/left.bmp ${room_basepath}/right.bmp)
  # set(room_single ${room_basepath}/single.bmp)
  # set(room_start ${room_basepath}/start.bmp)
  # set(all_bmp_files ${room_updown} ${room_leftright} ${room_single} ${room_start})
  # set(all_chr_files ${all_bmp_files})
  # list(TRANSFORM all_chr_files REPLACE "${room_basepath}/(.*)\.bmp" "${out_nmtpath}/\\1\.bin")

  # set(NESTILER_OPTS
  #   -c ${NESTILER_DIR}/nestiler-colors.json
  #   --mode bg --lossy 1
  #   --share-pattern-table
  #   --out-pattern-table-0
  # )
  # set(NESTILER_ROOM_UPDOWN ${NESTILER}
  #   ${NESTILER_OPTS}
  #   ${rawchr_path}/room_updown_chr.chr
  #   ${room_updown_opts}
  # )
  # set(NESTILER_ROOM_LEFTRIGHT ${NESTILER}
  #   ${NESTILER_OPTS}
  #   ${rawchr_path}/room_leftright_chr.chr
  #   ${room_leftright_opts}
  # )
  # set(NESTILER_ROOM_SINGLE ${NESTILER}
  #   ${NESTILER_OPTS}
  #   ${rawchr_path}/room_single_chr.chr
  #   ${room_single_opts}
  # )
  # set(NESTILER_ROOM_START ${NESTILER}
  #   ${NESTILER_OPTS}
  #   ${rawchr_path}/room_start_chr.chr
  #   ${room_start_opts}
  # )

  # set(NESTILER_OBJECTS ${NESTILER}
  #   ${NESTILER_OPTS}
  #   ${rawchr_path}/object_chr.chr
  #   ${object_infiles_indexed}
  #   ${object_nametable}
  #   ${object_attrs}
  # )
  # set(NESTILER_SPECIAL ${NESTILER}
  #   ${NESTILER_OPTS}
  #   ${rawchr_path}/titlescreen_chr.chr
  #   -b "#000000"
  #   -p0 "#3CBCFC,#C4D4FC,#FFFFFF"
  #   -p1 "#BCBCBC,#D82800,#FC9838"
  #   -i0 ${special_basepath}/titlescreen.bmp
  #   -a0 ${rawtmp_path}/titlescreen.nmt
  #   -u0 ${rawtmp_path}/titlescreen.attr
  #   -t0 ${rawpal_path}/title_0.pal
  #   -t1 ${rawpal_path}/title_1.pal
  #   -t2 ${rawpal_path}/title_2.pal
  #   -t3 ${rawpal_path}/title_3.pal
  # )

  # add_custom_command(
  #   OUTPUT ${all_chr_files}

  #   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  #   COMMAND ${CMAKE_COMMAND} -E make_directory ${out_chrpath}
  #   COMMAND ${CMAKE_COMMAND} -E make_directory ${out_nmtpath}
  #   COMMAND ${CMAKE_COMMAND} -E make_directory ${out_atrpath}
  #   COMMAND ${CMAKE_COMMAND} -E make_directory ${out_palpath}
  #   COMMAND ${CMAKE_COMMAND} -E make_directory ${rawchr_path}
  #   COMMAND ${CMAKE_COMMAND} -E make_directory ${rawtmp_path}
  #   COMMAND ${CMAKE_COMMAND} -E make_directory ${rawnmt_path}
  #   COMMAND ${CMAKE_COMMAND} -E make_directory ${rawpal_path}

  #   # nestiler all the rooms
  #   COMMAND ${NESTILER_ROOM_UPDOWN}
  #   COMMAND ${NESTILER_ROOM_LEFTRIGHT}
  #   COMMAND ${NESTILER_ROOM_SINGLE}
  #   COMMAND ${NESTILER_ROOM_START}
  #   COMMAND ${NESTILER_OBJECTS}
  #   # and then the special ones individually
  #   COMMAND ${NESTILER_SPECIAL}

  #   # for the titlescreen, we don't care about mixing attributes, so just compress them too
  #   COMMAND ${CMAKE_COMMAND} -E cat ${rawtmp_path}/titlescreen.nmt ${rawtmp_path}/titlescreen.attr > ${rawnmt_path}/titlescreen.nmt
  #   COMMAND ${CMAKE_COMMAND} -E cat ${rawpal_path}/title_0.pal ${rawpal_path}/title_1.pal ${rawpal_path}/title_2.pal ${rawpal_path}/title_3.pal > ${out_palpath}/title_pal.bin

  #   # combine all the palettes for each of the rooms into one file
  #   COMMAND ${CMAKE_COMMAND} -E cat ${rawpal_path}/updown_0.pal ${rawpal_path}/updown_1.pal ${rawpal_path}/updown_2.pal ${rawpal_path}/updown_3.pal > ${out_palpath}/updown_pal.bin
  #   COMMAND ${CMAKE_COMMAND} -E cat ${rawpal_path}/leftright_0.pal ${rawpal_path}/leftright_1.pal ${rawpal_path}/leftright_2.pal ${rawpal_path}/leftright_3.pal > ${out_palpath}/leftright_pal.bin
  #   COMMAND ${CMAKE_COMMAND} -E cat ${rawpal_path}/single_0.pal ${rawpal_path}/single_1.pal ${rawpal_path}/single_2.pal ${rawpal_path}/single_3.pal > ${out_palpath}/single_pal.bin
  #   COMMAND ${CMAKE_COMMAND} -E cat ${rawpal_path}/start_0.pal ${rawpal_path}/start_1.pal ${rawpal_path}/start_2.pal ${rawpal_path}/start_3.pal > ${out_palpath}/start_pal.bin

  #   # Compress all the CHR files and Nametables
  #   COMMAND ${Python3_EXECUTABLE} ${compressor_script} -f ${rawchr_path} ${out_chrpath}
  #   COMMAND ${Python3_EXECUTABLE} ${compressor_script} -f ${rawnmt_path} ${out_nmtpath}

  #   DEPENDS ${all_bmp_files} ${special_basepath}/titlescreen.bmp
  #   COMMENT "Running nestiler on the room backgrounds"
  # )

endfunction()
