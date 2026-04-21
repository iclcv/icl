#!/bin/sh
# Symlink all built executables into build/bin/ for convenience
builddir="$1"
bindir="$builddir/bin"
mkdir -p "$bindir"
# Clean stale symlinks
find "$bindir" -type l -delete 2>/dev/null
# Link all executables (skip libraries, objects, pch, meson internals)
find "$builddir" -maxdepth 5 -type f -perm +111 \
  -not -name "*.dylib" -not -name "*.so" -not -name "*.o" -not -name "*.pch" \
  -not -path "*/meson-*" -not -path "*/bin/*" \
  -exec ln -sf {} "$bindir/" \;
count=$(ls "$bindir" | wc -l | tr -d ' ')
echo "Symlinked $count executables into $bindir/"
