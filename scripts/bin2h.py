#!/usr/bin/env python3

import argparse
import pathlib

def bin2h(fpathin: pathlib.Path, fout:pathlib.Path = None):
  '''Generates a C header file from a binary file.'''
  with open(fout.joinpath("generated.h"), 'w') as output:
    for fin in fpathin.glob("*.bin"):
      var_name = fin.stem.replace(".", "_").replace(" ", "_")
      
      with open(fin, 'rb') as input:
        data_len = 0
        header_guard = var_name.upper() + "_H_"
        output.write(f'''
// Generated from {fin.name} by bin2header (python impl)
#ifndef {header_guard}
#define {header_guard}
#ifdef __cplusplus
extern "C" {{
#endif //__cplusplus
const unsigned char {var_name}[] = {{
''')
        # writes the previous line ending after the next line is read
        # so we don't end up with a trailing comma
        write_line_ending = False
        while True:
          data = input.read(1024 * 1024)
          if not data:
            break
          data_len += len(data)
          lines = [data[i:i+16] for i in range(0, len(data), 16)]
          for line in lines:
            if write_line_ending:
              output.write(f',\n')
            line = ','.join([f"0x{byte:02x}" for byte in line])
            output.write(f'{line}')
            write_line_ending = True
        output.write(f'}};\n')
        output.write(f"unsigned int {var_name}_len = {data_len};\n")
        output.write(f"""
#ifdef __cplusplus
}}
#endif //__cplusplus
#endif // {header_guard}\n""")

def main(fin: str, fout: str):
  fin = pathlib.Path(fin)
  fout = pathlib.Path(fout)
  bin2h(fin, fout)

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description='Generates a header file from the binary file provided. Outputs to the same file path as the input by default')
  parser.add_argument('fin', metavar='in', type=str,
                      help='Path to the file to convert to a header file')
  parser.add_argument('-o', '--output', type=str,
                      help='(Optional) Path to the directory to output the header file')
  args = parser.parse_args()
  main(args.fin, args.output)
