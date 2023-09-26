#!/usr/bin/env python3

import argparse
import sys
from pathlib import Path
from struct import *
from bin2h import bin2array
from donut import get_cblocks_from_bytes

def main(fin: Path, fout: Path):
  fout.mkdir(parents=True, exist_ok=True)

  for file in (fin / "metasprites").rglob('*.msb'):
    
    # metasprites = ""
    # filename = file.stem.replace(".", "_").replace(" ", "_")
    with open(file, 'rb') as f:
      byts = f.read()
      grid_x = byts[0]
      grid_y = byts[1]
      out = []
      for metasprite in range(0, 256):
        sprites = []
        for sprite in range(0, 64):
          # load the four bytes for this sprite from the bytes
          spr_idx = 2 + metasprite * 256 + sprite * 4
          spr = byts[spr_idx:spr_idx+4]

          # y, tile, attr, x
          if (spr[0] == 0xff):
            break

          spr_y = spr[0] - grid_y
          # since these are 8x16 sprites, we need to offset the tile id properly
          tile_id = spr[1] * 2 + 1
          attr = spr[2]
          spr_x = spr[3] - grid_x
          # NOTICE: we change the order slightly to allow skipping a sprite if its offscreen
          # print(f" {attr}, {tile_id}, {spr_y}, {spr_x}")
          sprites.append(pack("<BBbb", attr, tile_id, spr_y, spr_x))
        
        if len(sprites) == 0:
          continue
        o = b"\x7f" # terminal byte
        for spr in sprites:
          o += spr
        out.append(o)
        
        with open((fout / "sprites" / f"{file.stem}_metasprite_{metasprite}.bin"), 'wb') as single:
          single.write(o)

      with open((fout / "sprites" / f"{file.stem}_metasprite.msb"), 'wb') as o:
        offset = len(out)
        for byt in out:
          offset += len(byt)

          if (offset > 255):
            print(f"<ERROR> Total length of metasprites for {file.stem} is greater than 255", file=sys.stderr)
            exit(1)
          o.write(pack("<B", offset))
        for byt in out:
          o.write(byt)

    with open(fin / "metasprites" / f"{file.stem}.chr", 'rb') as f:
      rawchr = f.read()
      compressed_pages = []
      for block in get_cblocks_from_bytes(rawchr, allow_partial=True):
        compressed_pages.append(block)
      compressed = b''.join(compressed_pages)
      # chr = bin2array(filename + "_chr", compressed)
      r = len(rawchr)
      w = len(compressed)
      ratio = 1 - (w / r)
      print("<total> :{:>6.1%} ({} => {} bytes, {})".format(ratio, r, w, file.stem), file=sys.stderr)
      with open((fout / "sprites" / f"{file.stem}.chr.dnt"), 'wb') as o:
        o.write(compressed)
      with open((fout / "raw" / "sprites" / f"{file.stem}.chr"), 'wb') as o:
        o.write(rawchr)

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Processes Metasprites and outputs to header files')
  
  parser.add_argument('fin', metavar='in', type=str,
                      help='Input Directory of msb files to build the song data from')
  parser.add_argument('fout', metavar='out', type=str,
                      help='Build Directory to write the output files to.')
                      
  args = parser.parse_args()
  main(Path(args.fin).resolve(), Path(args.fout).resolve())
