#!/usr/bin/env python3

import argparse
import sys
import subprocess
from PIL import Image
from pathlib import Path
from struct import *
from donut import compress

def run_nes_tiler(nestiler: Path, *args):
  cmd = [str(nestiler / "nestiler"), *args]
  done = subprocess.run(cmd, stderr=subprocess.STDOUT, stdout=subprocess.PIPE, text=True)
  # print(f"nestiler CMD ({' '.join(cmd)}) output:\n{done.stdout}")
  if done.stderr != None:
    print(done.stderr)
  print(done.stdout)
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
    "--mode", "bg", "--lossy", "0",
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

def main(nestiler: Path, fin: Path, fout: Path):
  # make all the output directories first
  room_path = fin / "rooms"
  obj_path = fin / "objects"
  special_path = fin / "special"
  header_path = fout / "header"
  rawchr_path = fout / "raw" / "chr"
  rawnmt_path = fout / "raw" / "nmt"
  rawpal_path = fout / "raw" / "pal"
  rawtmp_path = fout / "raw" / "tmp"
  outchr_path = fout / "graphics" / "chr"
  outnmt_path = fout / "graphics" / "nmt"
  outatr_path = fout / "graphics" / "atr"
  outpal_path = fout / "graphics" / "pal"
  for p in [rawchr_path,rawnmt_path,rawpal_path,rawtmp_path,outchr_path,outnmt_path,outatr_path,outpal_path]:
    p.mkdir(parents=True, exist_ok=True)

  # store a list of the chr tile count and offset
  # so we can import them as linker symbols
  chr_offset = {}
  chr_count = {}
  
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
    params += make_input_params(0, first, rawnmt_path, outatr_path, rawpal_path)
    params += make_input_params(1, second, rawnmt_path, outatr_path, rawpal_path)
    params += make_palette_params(chr_path, rawpal_path)
    run_nes_tiler(nestiler, *params)
    concat_palette(f"{first.stem}{second.stem}", rawpal_path, outpal_path)

  # and then run it on all the single room screens
  for screen in [room_path / "single.bmp"]:
    params = []
    chr_path = rawchr_path / f"{screen.stem}.chr"
    params += nestiler_params_bg(nestiler, chr_path)
    params += make_input_params(0, screen, rawnmt_path, outatr_path, rawpal_path)
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
    chr_path = rawchr_path / f"{obj.stem}.chr"
    params += nestiler_params_bg(nestiler, chr_path)
    params += make_input_params(0, obj, rawnmt_path, outatr_path, rawpal_path)
    params += make_palette_params(chr_path, rawpal_path)
    run_nes_tiler(nestiler, *params)
    concat_palette(f"{obj.stem}", rawpal_path, outpal_path)

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
      byts += b"\xff\xff" # add a terminator bit here
      with open(outchr_path / f"{chr.stem}.chr.dnt", 'wb') as o:
        o.write(byts)

  # compress all the nmt files too
  for nmt in rawnmt_path.glob("*.nmt"):
    with open(nmt, 'rb') as f:
      byts = compress(f.read(), allow_partial=True)
      byts += b"\xff\xff" # add a terminator bit here
      with open(outnmt_path / f"{nmt.stem}.nmt.dnt", 'wb') as o:
        o.write(byts)

  # write the asm file with all the CHR tile sizes and offsets
  asm_file = """
;;; Generated by process_background.py

; CHR offsets - ie the total number of bytes used by this block
"""
  for name,off in chr_offset.items():
    asm_file += f"""
.set {name}_chr_offset, {off}"""
  asm_file += """

; CHR count - ie the number of 8x8 tiles used by this block
"""
  for name,count in chr_count.items():
    asm_file += f"""
.set {name}_chr_count, {count}"""
  hpp_file = """
/// Generated by process_background.py

#pragma once
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
  parser.add_argument('fin', metavar='in', type=str,
                      help='CHR Directory')
  parser.add_argument('fout', metavar='out', type=str,
                      help='Gen Directory')
                      
  args = parser.parse_args()
  main(Path(args.nestiler).resolve(), Path(args.fin).resolve(), Path(args.fout).resolve())
