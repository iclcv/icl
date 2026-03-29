#!/bin/bash
set -e

# Source oneAPI environment (sets IPP paths)
source /opt/intel/oneapi/setvars.sh --force 2>/dev/null || true

# Copy source to writable location (CMake generates files in-tree)
echo "=== Copying source ==="
cp -a /src /icl-src

echo "=== Configuring ICL with IPP ==="
cmake -S /icl-src -B /build \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_WITH_IPP=ON \
  -DBUILD_TESTS=ON \
  -DBUILD_WITH_QT=OFF \
  -DBUILD_WITH_OPENGL=OFF \
  -DBUILD_WITH_OPENCV=OFF \
  -DBUILD_WITH_EIGEN3=OFF \
  -DBUILD_WITH_LIBAV=OFF \
  -DCMAKE_PREFIX_PATH="/opt/intel/oneapi/ipp/latest"

JOBS=${JOBS:-$(nproc)}
echo "=== Building ($JOBS threads) ==="
cmake --build /build -j$JOBS

echo "=== Running tests ==="
cd /build && ctest --output-on-failure
