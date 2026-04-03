#!/bin/bash
# Build and test ICL inside Docker (Linux).
#
# Uses named volumes:
#   icl-src   — rsynced source from host
#   icl-build — persistent build dir (incremental builds work)
#
# Only changed files are recompiled thanks to the persistent build volume.
# First run takes several minutes (full build); subsequent runs are fast.
#
# Usage (from repo root):
#   ./scripts/docker/docker-build.sh   # first time only
#   ./scripts/docker/docker-test.sh    # build + run tests
#   ./scripts/docker/docker-test.sh --clean  # clean build

set -e
cd "$(git rev-parse --show-toplevel)"

# Handle --clean flag
if [ "$1" = "--clean" ]; then
  echo "Cleaning build volume..."
  docker volume rm icl-build 2>/dev/null || true
  docker volume create icl-build
fi

# Sync source
echo "Syncing source..."
docker run --rm \
  -v "$(pwd):/host:ro" \
  -v icl-src:/src \
  icl-dev rsync -a --delete \
    --exclude='build/' --exclude='build-docker/' \
    --exclude='.git/objects' --exclude='.DS_Store' \
    --exclude='*.o' --exclude='*.dylib' \
    /host/ /src/

echo "Building and testing..."
docker run --rm \
  --cpus=$(docker info --format '{{.NCPU}}' 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || nproc) \
  -v icl-src:/src \
  -v icl-build:/build \
  icl-dev bash -c '
set -e
cd /build
cmake /src \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=ON \
  -DBUILD_WITH_IPP=ON \
  -DBUILD_WITH_MKL=ON \
  -DBUILD_WITH_IMAGEMAGICK=OFF \
  -DBUILD_WITH_LIBAV=OFF \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++
cmake --build . -j$(nproc)
ctest --output-on-failure
'
