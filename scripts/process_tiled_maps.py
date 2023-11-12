#!/usr/bin/env python3

import argparse
import sys
import xml.etree.ElementTree as ET
from pathlib import Path
from struct import *

def parse_xml_room_object(root: ET.Element):
  objcollision = root.find('./objectgroup[@name="collision"]')
  collision = []
  for obj in objcollision.findall('object'):
    collision.append([
      int(obj.attrib["x"]),
      int(obj.attrib["y"]),
      int(obj.attrib["width"]),
      int(obj.attrib["height"]),
    ])
  return collision

def parse_xml_section_object(root: ET.Element, section: str):
  objexits = root.find('./objectgroup[@name="exit_spawns"]')
  section_exits = []
  for objname in [f"{section}_up", f"{section}_right", f"{section}_down", f"{section}_left"]:
    obj = objexits.find(f'object[@name="{objname}"]')
    if obj is not None:
      section_exits.append([
        int(obj.attrib["x"])//8%32,
        int(obj.attrib["y"])//8%30,
        int(obj.attrib["width"])//8,
        int(obj.attrib["height"])//8,
      ])
    else:
      section_exits.append([0,0,0,0])
  return section_exits
  # for objname in ["side_up", "side_right", "side_down", "side_left"]:
  #   obj = objexits.find(f'object[@name="{objname}"]')
  #   if obj is not None:
  #     print(ET.tostring(obj, encoding='unicode'))
  #     side_exits[i].append([
  #       int(obj.attrib["x"])//8%32,
  #       int(obj.attrib["y"])//8%30,
  #       int(obj.attrib["width"])//8,
  #       int(obj.attrib["height"])//8,
  #     ])
  #   else:
  #     side_exits[i].append([0,0,0,0])

def main(fin: Path, fout: Path):
  fout.mkdir(parents=True, exist_ok=True)

  file_order = []
  room_objects = []
  
  i = 0

  for filename in ["leftright.tmx", "updown.tmx"]:
    # split out the files that have multiple boundaries
    root = ET.parse(fin / "tiled" / filename).getroot()
  
  room_objects = []
  lead_exits = []
  side_exits = []

  for file in (fin / "tiled").rglob('*.tmx'):
    file_order.append(f"{file.stem}")
    root = ET.parse(file).getroot()
    room_objects.append( parse_xml_room_object(root) )
    lead_exits.append( parse_xml_section_object(root, "lead") )
    side_exits.append( parse_xml_section_object(root, "side") )


  nl = "\n"
  with open(fout / "header" / "room_collision.hpp", 'w') as header:
    hppfile = """
/// Generated by process_background.py

#pragma once
"""
    for i, f in enumerate(file_order):
      hppfile += f"""
constexpr unsigned char room_object_collision_{f}_count = {len(room_objects[i])};"""
    header.write(hppfile)


  with open(fout / "header" / f"room_collision.s", 'w') as asm:
    asm.write(f"""
;;; Generated by process_tiled_maps.py
.section .prg_rom_1.room_collision_lut,"aR",@progbits
.globl room_collision_lut
room_collision_lut:
;; Room collision data
; x lo
{nl.join([".byte " + ",".join([f"({row[0]}&0xff)" for row in col]) for col in room_objects])}
; x hi
{nl.join([".byte " + ",".join([f"(({row[0]}>>8)&0xff)" for row in col]) for col in room_objects])}
; y lo
{nl.join([".byte " + ",".join([f"({row[1]}&0xff)" for row in col]) for col in room_objects])}
; y hi
{nl.join([".byte " + ",".join([f"(({row[1]}>>8)&0xff)" for row in col]) for col in room_objects])}
; width
{nl.join([".byte " + ",".join([f"{row[2]}" for row in col]) for col in room_objects])}
; height
{nl.join([".byte " + ",".join([f"{row[3]}" for row in col]) for col in room_objects])}

;; lead exits
.section .prg_rom_1.lead_exit_lut,"aR",@progbits
.globl lead_exit_lut
lead_exit_lut:
; x (nmt position)
{nl.join([".byte " + ",".join([f"{row[0]}" for row in col]) for col in lead_exits])}
; y (nmt position)
{nl.join([".byte " + ",".join([f"{row[1]}" for row in col]) for col in lead_exits])}
; width
{nl.join([".byte " + ",".join([f"{row[2]}" for row in col]) for col in lead_exits])}
; height
{nl.join([".byte " + ",".join([f"{row[3]}" for row in col]) for col in lead_exits])}

;; side exits
.section .prg_rom_1.side_exit_lut,"aR",@progbits
.globl side_exit_lut
side_exit_lut:
; x (nmt position)
{nl.join([".byte " + ",".join([f"{row[0]}" for row in col]) for col in side_exits])}
; y (nmt position)
{nl.join([".byte " + ",".join([f"{row[1]}" for row in col]) for col in side_exits])}
; width
{nl.join([".byte " + ",".join([f"{row[2]}" for row in col]) for col in side_exits])}
; height
{nl.join([".byte " + ",".join([f"{row[3]}" for row in col]) for col in side_exits])}

""")


if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Processes Metasprites and outputs to header files')
  
  parser.add_argument('fin', metavar='in', type=str,
                      help='Input Directory of msb files to build the song data from')
  parser.add_argument('fout', metavar='out', type=str,
                      help='Build Directory to write the output files to.')
                      
  args = parser.parse_args()
  main(Path(args.fin).resolve(), Path(args.fout).resolve())
