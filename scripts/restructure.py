#!/usr/bin/env python3
"""
One-shot script to restructure ICL directories:
  ICLFoo/src/ICLFoo/ → icl/foo/
  Flatten demos/apps/examples
"""

import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)

# Module mapping: old_prefix → new_dir
MODULES = {
    'ICLUtils':    'icl/utils',
    'ICLMath':     'icl/math',
    'ICLCore':     'icl/core',
    'ICLFilter':   'icl/filter',
    'ICLIO':       'icl/io',
    'ICLCV':       'icl/cv',
    'ICLQt':       'icl/qt',
    'ICLGeom':     'icl/geom',
    'ICLMarkers':  'icl/markers',
    'ICLPhysics':  'icl/physics',
}

# Include directive mapping (for sed)
INCLUDE_MAP = {
    'ICLUtils/':    'icl/utils/',
    'ICLMath/':     'icl/math/',
    'ICLCore/':     'icl/core/',
    'ICLFilter/':   'icl/filter/',
    'ICLIO/':       'icl/io/',
    'ICLCV/':       'icl/cv/',
    'ICLQt/':       'icl/qt/',
    'ICLGeom/':     'icl/geom/',
    'ICLMarkers/':  'icl/markers/',
    'ICLPhysics/':  'icl/physics/',
}

# Multi-file demos/apps: (old_dir, main_file_stem)
MULTI_FILE_TARGETS = {
    'ICLPhysics/demos/physics-maze':                       'physics-maze',
    'ICLPhysics/demos/physics-paper':                      'physics-paper',
    'ICLCV/apps/lens-undistortion-calibration':            'lens-undistortion-calibration',
    'ICLMarkers/apps/camera-calibration':                  'camera-calibration',
    'ICLMarkers/apps/camera-calibration-planar':           'camera-calibration-planar',
}

def run(cmd, check=True):
    """Run a shell command."""
    print(f"  $ {cmd}")
    result = subprocess.run(cmd, shell=True, check=check, capture_output=True, text=True)
    if result.returncode != 0 and not check:
        print(f"    (exit {result.returncode})")
    return result

def git_mv(src, dst):
    """git mv with directory creation."""
    dst_dir = os.path.dirname(dst)
    if dst_dir and not os.path.exists(dst_dir):
        os.makedirs(dst_dir, exist_ok=True)
    run(f'git mv "{src}" "{dst}"')

def step1_create_dirs():
    """Create the icl/ directory tree."""
    print("\n=== Step 1: Create directory tree ===")
    for new_dir in MODULES.values():
        for sub in ['', 'demos', 'apps', 'examples']:
            d = os.path.join(new_dir, sub) if sub else new_dir
            os.makedirs(d, exist_ok=True)
            print(f"  mkdir -p {d}")

def step2_move_sources():
    """Move source files: ICLFoo/src/ICLFoo/*.{h,cpp,hpp} → icl/foo/"""
    print("\n=== Step 2: Move source files ===")
    for old_mod, new_dir in MODULES.items():
        src_dir = f"{old_mod}/src/{old_mod}"
        if not os.path.isdir(src_dir):
            print(f"  SKIP {src_dir} (not found)")
            continue

        # Move all files (not directories) at top level
        for f in sorted(os.listdir(src_dir)):
            src_path = os.path.join(src_dir, f)
            if os.path.isfile(src_path):
                git_mv(src_path, os.path.join(new_dir, f))

        # Move subdirectories (OpenCL/, proto/)
        for f in sorted(os.listdir(src_dir)):
            src_path = os.path.join(src_dir, f)
            if os.path.isdir(src_path):
                git_mv(src_path, os.path.join(new_dir, f))

def step3_flatten_demos():
    """Flatten single-file demos; flatten multi-file with naming convention."""
    print("\n=== Step 3: Flatten demos ===")
    for old_mod, new_dir in MODULES.items():
        demos_dir = f"{old_mod}/demos"
        if not os.path.isdir(demos_dir):
            continue

        for entry in sorted(os.listdir(demos_dir)):
            entry_path = os.path.join(demos_dir, entry)
            if not os.path.isdir(entry_path) or entry == '.DS_Store':
                continue
            if entry_path.rstrip('/') == 'CMakeLists.txt':
                continue

            full_key = f"{old_mod}/demos/{entry}"

            if full_key in MULTI_FILE_TARGETS:
                # Multi-file: flatten with naming convention
                stem = MULTI_FILE_TARGETS[full_key]
                for f in sorted(os.listdir(entry_path)):
                    if f == 'CMakeLists.txt' or f == '.DS_Store':
                        continue
                    src = os.path.join(entry_path, f)
                    if not os.path.isfile(src):
                        continue
                    # Main file keeps its name, others get prefix
                    name, ext = os.path.splitext(f)
                    if name == stem:
                        dst = os.path.join(new_dir, 'demos', f)
                    else:
                        dst = os.path.join(new_dir, 'demos', f"{stem}-{f}")
                    git_mv(src, dst)
            else:
                # Single-file: just move the .cpp
                cpp_files = [f for f in os.listdir(entry_path)
                            if f.endswith('.cpp') and f != 'CMakeLists.txt']
                for f in cpp_files:
                    src = os.path.join(entry_path, f)
                    dst = os.path.join(new_dir, 'demos', f)
                    git_mv(src, dst)

def step4_flatten_apps():
    """Flatten single-file apps; flatten multi-file with naming convention."""
    print("\n=== Step 4: Flatten apps ===")
    for old_mod, new_dir in MODULES.items():
        apps_dir = f"{old_mod}/apps"
        if not os.path.isdir(apps_dir):
            continue

        for entry in sorted(os.listdir(apps_dir)):
            entry_path = os.path.join(apps_dir, entry)
            if not os.path.isdir(entry_path) or entry == '.DS_Store':
                continue

            full_key = f"{old_mod}/apps/{entry}"

            if full_key in MULTI_FILE_TARGETS:
                stem = MULTI_FILE_TARGETS[full_key]
                for f in sorted(os.listdir(entry_path)):
                    if f == 'CMakeLists.txt' or f == '.DS_Store':
                        continue
                    src = os.path.join(entry_path, f)
                    if not os.path.isfile(src):
                        continue
                    name, ext = os.path.splitext(f)
                    if name == stem:
                        dst = os.path.join(new_dir, 'apps', f)
                    else:
                        dst = os.path.join(new_dir, 'apps', f"{stem}-{f}")
                    git_mv(src, dst)
            else:
                # Single-file app
                for f in sorted(os.listdir(entry_path)):
                    if f == 'CMakeLists.txt' or f == '.DS_Store':
                        continue
                    src = os.path.join(entry_path, f)
                    if os.path.isfile(src):
                        dst = os.path.join(new_dir, 'apps', f)
                        git_mv(src, dst)

def step5_move_examples():
    """Move examples (some flat, some in subdirs)."""
    print("\n=== Step 5: Move examples ===")
    for old_mod, new_dir in MODULES.items():
        examples_dir = f"{old_mod}/examples"
        if not os.path.isdir(examples_dir):
            continue

        for entry in sorted(os.listdir(examples_dir)):
            entry_path = os.path.join(examples_dir, entry)
            if entry == 'CMakeLists.txt' or entry == '.DS_Store':
                continue

            if os.path.isfile(entry_path) and entry.endswith('.cpp'):
                # Flat example
                git_mv(entry_path, os.path.join(new_dir, 'examples', entry))
            elif os.path.isdir(entry_path):
                # Subdir example — flatten
                for f in sorted(os.listdir(entry_path)):
                    if f == 'CMakeLists.txt' or f == '.DS_Store':
                        continue
                    src = os.path.join(entry_path, f)
                    if os.path.isfile(src):
                        dst = os.path.join(new_dir, 'examples', f)
                        git_mv(src, dst)

def step6_update_includes():
    """Update all #include directives in the new icl/ tree, tests/, benchmarks/."""
    print("\n=== Step 6: Update #include directives ===")

    # Build sed expression
    sed_parts = []
    for old, new in INCLUDE_MAP.items():
        # Handle both <ICLFoo/...> and "ICLFoo/..."
        sed_parts.append(f's|#include <{old}|#include <{new}|g')
        sed_parts.append(f's|#include "{old}|#include "{new}|g')

    sed_expr = ' ; '.join(sed_parts)

    # Find all C++ source files in icl/, tests/, benchmarks/
    cmd = f'find icl/ tests/ benchmarks/ -type f \\( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \\) -exec sed -i "" \'{sed_expr}\' {{}} +'
    run(cmd)

def step7_update_multifile_includes():
    """Update internal includes in multi-file demos/apps after flattening."""
    print("\n=== Step 7: Update multi-file internal includes ===")

    # physics-maze: #include "MazeObject.h" → #include "physics-maze-MazeObject.h"
    # physics-maze: #include "HoleObject.h" → #include "physics-maze-HoleObject.h"
    updates = [
        ('icl/physics/demos/physics-maze.cpp', [
            ('MazeObject.h', 'physics-maze-MazeObject.h'),
            ('HoleObject.h', 'physics-maze-HoleObject.h'),
        ]),
        ('icl/physics/demos/physics-maze-MazeObject.cpp', [
            ('MazeObject.h', 'physics-maze-MazeObject.h'),
            ('HoleObject.h', 'physics-maze-HoleObject.h'),
        ]),
        ('icl/physics/demos/physics-maze-MazeObject.h', [
            ('HoleObject.h', 'physics-maze-HoleObject.h'),
        ]),

        # physics-paper
        ('icl/physics/demos/physics-paper.cpp', [
            ('DefaultGroundObject.h', 'physics-paper-DefaultGroundObject.h'),
            ('DefaultPhysicsScene.h', 'physics-paper-DefaultPhysicsScene.h'),
            ('InteractiveFoldLine.h', 'physics-paper-InteractiveFoldLine.h'),
            ('SceneMultiCamCapturer.h', 'physics-paper-SceneMultiCamCapturer.h'),
        ]),
        ('icl/physics/demos/physics-paper-SceneMultiCamCapturer.cpp', [
            ('SceneMultiCamCapturer.h', 'physics-paper-SceneMultiCamCapturer.h'),
        ]),

        # lens-undistortion-calibration
        ('icl/cv/apps/lens-undistortion-calibration.cpp', [
            ('UndistortionUtil.h', 'lens-undistortion-calibration-UndistortionUtil.h'),
        ]),
        ('icl/cv/apps/lens-undistortion-calibration-UndistortionUtil.cpp', [
            ('UndistortionUtil.h', 'lens-undistortion-calibration-UndistortionUtil.h'),
        ]),

        # camera-calibration
        ('icl/markers/apps/camera-calibration.cpp', [
            ('CameraCalibrationUtils.h', 'camera-calibration-CameraCalibrationUtils.h'),
        ]),
        ('icl/markers/apps/camera-calibration-CameraCalibrationUtils.cpp', [
            ('CameraCalibrationUtils.h', 'camera-calibration-CameraCalibrationUtils.h'),
        ]),

        # camera-calibration-planar
        ('icl/markers/apps/camera-calibration-planar.cpp', [
            ('GridIndicatorObject.h', 'camera-calibration-planar-GridIndicatorObject.h'),
            ('PlanarCalibrationTools.h', 'camera-calibration-planar-PlanarCalibrationTools.h'),
        ]),
        ('icl/markers/apps/camera-calibration-planar-GridIndicatorObject.cpp', [
            ('GridIndicatorObject.h', 'camera-calibration-planar-GridIndicatorObject.h'),
        ]),
        ('icl/markers/apps/camera-calibration-planar-PlanarCalibrationTools.cpp', [
            ('PlanarCalibrationTools.h', 'camera-calibration-planar-PlanarCalibrationTools.h'),
        ]),
    ]

    for filepath, replacements in updates:
        if not os.path.exists(filepath):
            print(f"  SKIP {filepath} (not found)")
            continue
        for old_inc, new_inc in replacements:
            sed_cmd = f'sed -i "" \'s|#include "{old_inc}"|#include "{new_inc}"|g\' "{filepath}"'
            run(sed_cmd, check=False)

def main():
    if '--dry-run' in sys.argv:
        print("DRY RUN - not executing")
        return

    step1_create_dirs()
    step2_move_sources()
    step3_flatten_demos()
    step4_flatten_apps()
    step5_move_examples()
    step6_update_includes()
    step7_update_multifile_includes()

    print("\n=== Done! ===")
    print("Next steps:")
    print("  1. Update CMakeLists.txt files")
    print("  2. Verify build: cmake -B build && cmake --build build -j16")
    print("  3. Commit")

if __name__ == '__main__':
    main()
