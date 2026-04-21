#!/usr/bin/env python3
"""Generate meson files() blocks for each ICL module by scanning icl/*/."""
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BACKEND_SUFFIXES = ('_Ipp.cpp', '_Accelerate.cpp', '_OpenCL.cpp', '_Mkl.cpp')
BACKEND_GUARDS = {
    '_Ipp.cpp': 'ipp_dep.found()',
    '_Accelerate.cpp': 'accelerate_dep.found()',
    '_OpenCL.cpp': 'opencl_dep.found()',
    '_Mkl.cpp': 'mkl_dep.found()',
}

MODULES = ['utils', 'math', 'core', 'filter', 'io', 'cv', 'qt', 'geom', 'markers', 'physics']

def classify_file(name):
    """Returns (is_backend, guard_expr) for a .cpp file."""
    for suffix, guard in BACKEND_GUARDS.items():
        if name.endswith(suffix):
            return True, guard
    return False, None

def scan_module(mod):
    mod_dir = os.path.join(ROOT, 'icl', mod)
    sources = []
    headers = []
    backend_sources = {}  # guard -> [files]

    for f in sorted(os.listdir(mod_dir)):
        path = os.path.join(mod_dir, f)
        if not os.path.isfile(path):
            continue
        if f.endswith('.cpp'):
            is_backend, guard = classify_file(f)
            if is_backend:
                backend_sources.setdefault(guard, []).append(f)
            else:
                sources.append(f)
        elif f.endswith('.h') or f.endswith('.hpp'):
            headers.append(f)

    return sources, headers, backend_sources

def scan_targets(mod, subdir):
    """Scan demos/apps/examples directory."""
    target_dir = os.path.join(ROOT, 'icl', mod, subdir)
    if not os.path.isdir(target_dir):
        return []
    return sorted(f for f in os.listdir(target_dir) if f.endswith('.cpp'))

def format_files(var_name, file_list, indent=''):
    if not file_list:
        return f"{indent}{var_name} = []\n"
    lines = [f"{indent}{var_name} = files("]
    for f in file_list:
        lines.append(f"{indent}  '{f}',")
    lines.append(f"{indent})")
    return '\n'.join(lines) + '\n'

def main():
    for mod in MODULES:
        sources, headers, backends = scan_module(mod)
        print(f"# ===== icl/{mod} =====")
        print()

        var = f"icl_{mod}_sources"
        hvar = f"icl_{mod}_headers"
        print(format_files(var, sources))
        print(format_files(hvar, headers))

        for guard, files in sorted(backends.items()):
            print(f"if {guard}")
            print(f"  {var} += files(")
            for f in files:
                print(f"    '{f}',")
            print(f"  )")
            print(f"endif")
            print()

        for subdir in ['demos', 'apps', 'examples']:
            targets = scan_targets(mod, subdir)
            if targets:
                print(f"# {subdir}:")
                for t in targets:
                    print(f"#   {t}")
                print()

        print()

if __name__ == '__main__':
    main()
