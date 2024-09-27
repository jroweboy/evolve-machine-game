#!/usr/bin/env python3

import argparse
import math
import struct
import sys
import subprocess
from PIL import Image # type: ignore
from pathlib import Path
from struct import *
from donut import compress

def run_nes_tiler(nestiler: Path, *args):
  cmd = [str(nestiler / "nestiler"), *args]
  done = subprocess.run(cmd, stderr=subprocess.STDOUT, stdout=subprocess.PIPE, text=True)
  if done.stderr != None:
    print(done.stderr)
  print(f"nestiler CMD ({' '.join(cmd)}) output:\n{done.stdout}")
  # print(done.stdout)
  return done.stdout

def run_huffmunch(huffmunch: Path, outpath: Path):
  cmd = [str(huffmunch / "huffmunch.exe"),
     "-V", # -V verbose
     "-X", "0", # number of attempt, 0 is infinite
     "-S", "6", # width of data from 2 - 16 (6 seems to be good compression for our data)
     "-L", str(huffmunch / "huffmunch_list.txt"),
     str(outpath / "archive.hfm")
    ]
  done = subprocess.run(cmd, stderr=subprocess.STDOUT, stdout=subprocess.PIPE, text=True)
  if done.stderr != None:
    print(done.stderr)
  print(f"huffmunch CMD ({' '.join(cmd)}) output:\n{done.stdout}")
  return done.stdout

def make_input_params(i: int, input: Path, outnmt: Path, outatr: Path, outpal: Path):
  file = [f"-i{i}", str(input)]
  nmt =   [f"-a{i}", str(outnmt / f"{input.stem}.nmt")]
  attr =  [f"-u{i}", str(outatr / f"{input.stem}.atr")]
  out = []
  out += file
  out += nmt
  out += attr
  return out

def make_palette_params(input: Path, outpal: Path):
  return [
    f"-t0", str(outpal / f"{input.stem}_0.pal"),
    f"-t1", str(outpal / f"{input.stem}_1.pal"),
    f"-t2", str(outpal / f"{input.stem}_2.pal"),
    f"-t3", str(outpal / f"{input.stem}_3.pal"),
  ]

def nestiler_params_bg(nestiler: Path, outchr: Path, opts = []):
  return [
    "-c", str(nestiler / 'nestiler-colors.json'),
    "--mode", "bg", "--lossy", "1",
    "--share-pattern-table",
    "--out-pattern-table-0", str(outchr)] + opts

def nestiler_params_spr(nestiler: Path, outchr: Path, bg_color: int):
  return [
    "-c", str(nestiler / 'nestiler-colors.json'),
    "--mode", "sprites8x16", "--lossy", "1",
    "--bg-color", f"#{hex(bg_color)[2:]}",
    "--palette-0",  "#44009C"   # 0x03
    ","             "#747474"   # 0x00
    ","             "#FC9838",  # 0x27
    "--share-pattern-table",
    "--out-pattern-table-0", str(outchr)]

def concat_palette(name: str, fin: Path, fout: Path):
  with open(fin / f"{name}_0.pal", 'rb') as pal0:
    with open(fin / f"{name}_1.pal", 'rb') as pal1:
      with open(fin / f"{name}_2.pal", 'rb') as pal2:
        with open(fin / f"{name}_3.pal", 'rb') as pal3:
          with open(fout / f"{name}.pal", 'wb') as out:
            out.write(pal0.read())
            out.write(pal1.read())
            out.write(pal2.read())
            out.write(pal3.read())

def main(nestiler: Path, huffmunch: Path, fin: Path, fout: Path):
  # make all the output directories first
  room_path = fin / "rooms"
  obj_path = fin / "objects"
  special_path = fin / "special"
  header_path = fout / "header"
  rawatr_path = fout / "raw" / "atr"
  rawchr_path = fout / "raw" / "chr"
  rawnmt_path = fout / "raw" / "nmt"
  rawpal_path = fout / "raw" / "pal"
  rawtmp_path = fout / "raw" / "tmp"
  outspr_path = fout / "sprites"
  outchr_path = fout / "graphics" / "chr"
  outnmt_path = fout / "graphics" / "nmt"
  outatr_path = fout / "graphics" / "atr"
  outpal_path = fout / "graphics" / "pal"
  outcompress_path = fout / "compressed"
  for p in [rawatr_path,rawchr_path,rawnmt_path,rawpal_path,rawtmp_path,outchr_path,outnmt_path,outatr_path,outpal_path,outcompress_path]:
    p.mkdir(parents=True, exist_ok=True)

  # store a list of the chr tile count and offset
  # so we can import them as linker symbols
  chr_offset = {}
  chr_count = {}
  nmt_width = {}
  nmt_height = {}
  atr_width = {}
  atr_height = {}
  
  # split leftright.bmp and updown.bmp into separate files
  with Image.open(room_path / "leftright.bmp") as im:
    im.crop((0,0,256,240)).save(rawtmp_path / "left.bmp")
    im.crop((256,0,256+256,240)).save(rawtmp_path / "right.bmp")
  with Image.open(room_path / "updown.bmp") as im:
    im.crop((0,0,256,240)).save(rawtmp_path / "up.bmp")
    im.crop((0,240,256,240+240)).save(rawtmp_path / "down.bmp")
  with Image.open(room_path / "start.bmp") as im:
    im.crop((0,0,256,240)).save(rawtmp_path / "startup.bmp")
    im.crop((0,240,256,240+240)).save(rawtmp_path / "startdown.bmp")
  
  # Now run nestiler on all of the split room bmps (leftright, updown)
  for pair in [[rawtmp_path / "up.bmp", rawtmp_path / "down.bmp", []],
               [rawtmp_path / "left.bmp", rawtmp_path / "right.bmp", []],
               [rawtmp_path / "startup.bmp", rawtmp_path / "startdown.bmp", []]]:
    first, second, opts = pair
    chr_path = rawchr_path / f"{first.stem}{second.stem}.chr"
    params = []
    params += nestiler_params_bg(nestiler, rawchr_path / f"{first.stem}{second.stem}.chr", opts)
    params += make_input_params(0, first, rawnmt_path, rawatr_path, rawpal_path)
    params += make_input_params(1, second, rawnmt_path, rawatr_path, rawpal_path)
    params += make_palette_params(chr_path, rawpal_path)
    run_nes_tiler(nestiler, *params)
    concat_palette(f"{first.stem}{second.stem}", rawpal_path, outpal_path)

  # and then run it on all the single room screens
  for screen in [room_path / "single.bmp"]:
    params = []
    chr_path = rawchr_path / f"{screen.stem}.chr"
    params += nestiler_params_bg(nestiler, chr_path)
    params += make_input_params(0, screen, rawnmt_path, rawatr_path, rawpal_path)
    params += make_palette_params(chr_path, rawpal_path)
    run_nes_tiler(nestiler, *params)
    concat_palette(f"{screen.stem}", rawpal_path, outpal_path)

  # now process any special screens we have like the title screen or cutscenes
  # we save on space by concatting the attribute data to the nametable data
  # and compress them together
  for screen in [special_path / "titlescreen.bmp"]:
    params = []
    chr_path = rawchr_path / f"{screen.stem}.chr"
    params += nestiler_params_bg(nestiler, chr_path)
    # write the screen nmt and attr to the tmp path so we can concat them
    # since these screens don't have any object mixins, we can save space by compressing
    # the attribute tables as part of the nametable
    params += make_input_params(0, screen, rawtmp_path, rawtmp_path, rawpal_path)
    params += make_palette_params(chr_path, rawpal_path)
    run_nes_tiler(nestiler, *params)
    concat_palette(f"{screen.stem}", rawpal_path, outpal_path)
    
    # now concat the nametable and attr together
    with open(rawtmp_path / f"{screen.stem}.nmt", 'rb') as nmt, open(rawtmp_path / f"{screen.stem}.atr", 'rb') as atr:
      with open(rawnmt_path / f"{screen.stem}_atr.nmt", 'wb') as out:
        out.write(nmt.read())
        out.write(atr.read())

  # run it on the HUD font
  for font in [special_path / "hudfont.bmp"]:
    params = []
    chr_path = rawchr_path / f"{font.stem}.chr"
    params += nestiler_params_spr(nestiler, chr_path, 0xFF000000)
    params += [f"-i0", str(font)]
    run_nes_tiler(nestiler, *params)

  # next up, run it on the objects
  objs = [
    obj_path / "door_down.bmp",
    obj_path / "door_left.bmp",
    obj_path / "door_right.bmp",
    obj_path / "door_up.bmp",
  ]
  for obj in objs:
    params = []
    img = Image.open(obj)
    w,h = img.size
    nmt_width[obj.stem] = w // 8
    nmt_height[obj.stem] = h // 8
    atr_width[obj.stem] = int(math.ceil(w / 8))
    atr_height[obj.stem] = int(math.ceil(h / 8))
    chr_path = rawchr_path / f"{obj.stem}.chr"
    params += nestiler_params_bg(nestiler, chr_path)
    params += make_input_params(0, obj, rawnmt_path, rawatr_path, rawpal_path)
    params += make_palette_params(chr_path, rawpal_path)
    run_nes_tiler(nestiler, *params)
    concat_palette(f"{obj.stem}", rawpal_path, outpal_path)

  # run_huffmunch(huffmunch, outcompress_path)

  # compress all the chr files
  for chr in rawchr_path.glob("*.chr"):
    with open(chr, 'rb') as f:
      raw_chr = f.read()
      byts = compress(raw_chr, allow_partial=True)
      r = len(raw_chr)
      w = len(byts)
      chr_offset[chr.stem] = r
      chr_count[chr.stem] = r // 16
      ratio = 1 - (w / r)
      print("<total> :{:>6.1%} ({} => {} bytes, {})".format(ratio, r, w, chr.stem), file=sys.stderr)
      # byts += b"\xff\xff" # add a terminator bit here
      with open(outchr_path / f"{chr.stem}.chr.dnt", 'wb') as o:
        o.write(byts)

  # # compress all the nmt files too
  for nmt in rawnmt_path.glob("*.nmt"):
    with open(nmt, 'rb') as f:
      byts = compress(f.read(), allow_partial=True)
      # byts += b"\xff\xff" # add a terminator bit here
      with open(outnmt_path / f"{nmt.stem}.nmt.dnt", 'wb') as o:
        o.write(byts)
  
  # compress all the atr files too
  for atr in rawatr_path.glob("*.atr"):
    with open(atr, 'rb') as f:
      byts = compress(f.read(), allow_partial=True)
      # byts += b"\xff\xff" # add a terminator bit here
      with open(outatr_path / f"{atr.stem}.atr.dnt", 'wb') as o:
        o.write(byts)

  # compress all the pal files too cause why not at this point
  for p in outpal_path.glob("*.pal"):
    with open(p, 'rb') as f:
      byts = compress(f.read(), allow_partial=True)
      # byts += b"\xff\xff" # add a terminator bit here
      with open(outpal_path / f"{p.stem}.pal.dnt", 'wb') as o:
        o.write(byts)

  # Combine everything into a single output file
  # The metasprite building code runs first
  with open(outcompress_path / f"archive.raw.dnt", 'wb') as o:
    file_count = 0
    file_size = []
    file_name = []
    for p in [outatr_path, outchr_path, outnmt_path, outpal_path, outspr_path]:
      for f in sorted(p.glob("*.dnt")):
        with open(f, 'rb') as i:
          byts = i.read() + b"\xff\xff"
          o.write(byts)
          file_count += 1
          file_size += [len(byts)]
          file_name += [f.stem]
          print(f" file {f.stem} size {len(byts)}")
      
  # add the sizes to the front of the file
  with open(
        outcompress_path / f"archive.raw.dnt", 'rb') as o,open(
        outcompress_path / f"archive.dnt", 'wb') as n:
    offset = file_count * 2
    for i in range(file_count):
      n.write(struct.pack("<H", offset))
      offset += file_size[i]

    n.write(o.read())

  # write the asm file with all the CHR tile sizes and offsets
  asm_file = """
;;; Generated by process_background.py

; CHR offsets - ie the total number of bytes used by this block
"""
  for name,off in chr_offset.items():
    asm_file += f"""
.set {name}_chr_offset, {off}"""

  asm_file += """

; NMT width/height
"""
  for name,width in nmt_width.items():
    asm_file += f"""
.set {name}_nmt_width, {width}
.set {name}_nmt_height, {nmt_height[name]}"""

  asm_file += """

; att width/height
"""
  for name,width in atr_width.items():
    asm_file += f"""
.set {name}_atr_width, {width}
.set {name}_atr_height, {atr_height[name]}"""

  asm_file += """

; CHR count - ie the number of 8x8 tiles used by this block
"""
  for name,count in chr_count.items():
    asm_file += f"""
.set {name}_chr_count, {count}"""
  hpp_file = """
/// Generated by process_background.py

#pragma once
"""


  hpp_file += f"""
// generated Archive lookup

enum class Archive : unsigned char {{
"""
  for name in file_name:
    hpp_file += f"""
    {name.replace(".", "_")},"""
  hpp_file += f"""
    Count,
}};
"""

  hpp_file += """
// CHR offsets - ie the total number of bytes used by this block
"""
  for name,off in chr_offset.items():
    hpp_file += f"""
constexpr unsigned short {name}_chr_offset = {off};"""
  hpp_file += """

// CHR count - ie the number of 8x8 tiles used by this block
"""
  for name,count in chr_count.items():
    hpp_file += f"""
constexpr unsigned char {name}_chr_count = {count};"""

  with open(header_path / "graphics_constants.s", 'w') as out:
    out.write(asm_file)
  with open(header_path / "graphics_constants.hpp", 'w') as out:
    out.write(hpp_file)

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Processes all background tiles with nestiler')
  
  parser.add_argument('nestiler', metavar='nestiler', type=str,
                      help='Path to nestiler')
  parser.add_argument('huffmunch', metavar='huffmunch', type=str,
                      help='Path to huffmunch')
  parser.add_argument('fin', metavar='in', type=str,
                      help='CHR Directory')
  parser.add_argument('fout', metavar='out', type=str,
                      help='Gen Directory')
                      
  args = parser.parse_args()
  main(Path(args.nestiler).resolve(),
       Path(args.huffmunch).resolve(),
       Path(args.fin).resolve(),
       Path(args.fout).resolve())
