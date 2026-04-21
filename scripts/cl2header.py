#!/usr/bin/env python3
"""Convert an OpenCL .cl file into a C++ header with the source as a string constant."""
import sys

def main():
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} input.cl output.h VARIABLE_NAME", file=sys.stderr)
        sys.exit(1)

    input_file, output_file, var_name = sys.argv[1], sys.argv[2], sys.argv[3]

    with open(input_file, 'r') as f:
        source = f.read()

    # Escape for C++ string literal
    source = source.replace('\\', '\\\\').replace('"', '\\"')
    lines = source.split('\n')

    with open(output_file, 'w') as f:
        f.write(f'// Auto-generated from {input_file} — do not edit\n')
        f.write(f'#pragma once\n\n')
        f.write(f'static const char {var_name}[] =\n')
        for i, line in enumerate(lines):
            suffix = ';' if i == len(lines) - 1 else ''
            f.write(f'  "{line}\\n"{suffix}\n')

if __name__ == '__main__':
    main()
