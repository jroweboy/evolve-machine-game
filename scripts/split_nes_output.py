
import re
import sys

def main():
    prgc_end = 0
    segments = False
    with open(f'{sys.argv[1]}/map.txt','r') as map:
        for line in map.readlines():
            if line.startswith("Segment"):
                segments = True
            if segments and line.startswith("BANKED"):
                d=re.findall(r'BANKED \s*([\dA-Fa-f]*)\s*([\dA-Fa-f]*)\s',line)
                prg8_start = int(d[0][0], 16)
                prg8_end = int(d[0][1], 16)
            if segments and line.startswith("LOWCODE"):
                d=re.findall(r'LOWCODE \s*([\dA-Fa-f]*)\s*([\dA-Fa-f]*)\s',line)
                prgc_start = int(d[0][0], 16)
                # if theres no dpcm set this as a backup?
                if prgc_end == 0:
                    prgc_end = int(d[0][1], 16)
            if segments and line.startswith("DPCM"):
                d=re.findall(r'DPCM \s*([\dA-Fa-f]*)\s*([\dA-Fa-f]*)\s',line)
                prgc_end = int(d[0][1], 16)
    prg8_len = prg8_end-prg8_start
    prgc_len = prgc_end-prgc_start
    print(f"Size of Audio bank in bytes: {prg8_len}")
    print(f"Size of Compressor and DPCM in bytes: {prgc_len}")
    with open(f'{sys.argv[1]}/empty.nes','rb') as nes:
        bin = nes.read()
        with open(f'{sys.argv[1]}/prg8.bin','wb') as prg8:
            prg8.write(bin[0x10 : 0x10 + prg8_len])
        with open(f'{sys.argv[1]}/prgc.bin','wb') as prgc:
            prgc.write(bin[0xc000 + 0x10 : 0xc000 + 0x10 + prg8_len])


if __name__ == "__main__":
    main()