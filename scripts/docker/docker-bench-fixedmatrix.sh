#!/bin/bash
# Run FixedMatrix benchmarks inside Docker to compare Linux backends.
#
# Compiles benchmarks/bench-fixedmatrix.cpp in three modes:
#   1. Generic C++ (auto-vectorization disabled) — pure scalar baseline
#   2. With SSE2 (-msse2, auto-vectorization enabled) — what clang does at -O3
#   3. With MKL cblas — cblas_sgemm/dgemm for multiply, C++ for rest
#
# The benchmark file is self-contained (no ICL deps), so we compile it
# directly with clang++ rather than going through the CMake build.
#
# Source is rsynced into the icl-src volume (not bind-mounted).
# No persistent build volume needed — just compiles a single .cpp.
#
# Usage (from repo root):
#   ./scripts/docker/docker-build.sh   # first time only
#   ./scripts/docker/docker-bench-fixedmatrix.sh
#
# Output: timing tables for all three backends, side by side.

set -e
cd "$(git rev-parse --show-toplevel)"

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

echo "Running benchmarks..."
echo ""

docker run --rm \
  -v icl-src:/src:ro \
  -w /src \
  icl-dev bash -c '
set -e

BENCH=benchmarks/bench-fixedmatrix.cpp
MKL=${MKLROOT}

echo "==================================================================="
echo "FixedMatrix Linux Benchmarks"
echo "  $(clang++ --version | head -1)"
echo "  $(uname -m) on $(cat /proc/cpuinfo | grep "model name" | head -1 | cut -d: -f2 | xargs)"
echo "==================================================================="
echo ""

# ------------------------------------------------------------------
# 1. Generic C++ — disable auto-vectorization to get pure scalar code
# ------------------------------------------------------------------
echo "Compiling: Generic C++ (scalar, no auto-vectorize)..."
clang++ -std=c++17 -O3 -fno-tree-vectorize -fno-slp-vectorize \
  $BENCH -o /tmp/bench-scalar

# ------------------------------------------------------------------
# 2. SSE2 auto-vectorized — C++ loops, let clang auto-vectorize
# ------------------------------------------------------------------
echo "Compiling: C++ with auto-vectorization (SSE2)..."
clang++ -std=c++17 -O3 -msse2 -U__SSE2__ \
  $BENCH -o /tmp/bench-autovec

# ------------------------------------------------------------------
# 3. Manual SSE2 intrinsics — hand-written _mm_* code (like FixedMatrix.h)
# ------------------------------------------------------------------
echo "Compiling: Manual SSE2 intrinsics..."
clang++ -std=c++17 -O3 -msse2 \
  $BENCH -o /tmp/bench-sse2

# ------------------------------------------------------------------
# 3. MKL cblas — replace multiply with cblas_sgemm/dgemm
#    We write a small shim header that provides the asimd:: namespace
#    using MKL cblas functions, then include the benchmark.
# ------------------------------------------------------------------
echo "Compiling: MKL cblas for multiply..."
cat > /tmp/mkl_shim.h << '"'"'SHIMEOF'"'"'
#undef __APPLE__
#include <mkl_cblas.h>

// Forward-declare cpp:: functions (defined in the benchmark file)
namespace cpp {
  template<class T> T det4x4(const T*);
  template<class T> void inv4x4(const T*, T*);
}

namespace asimd {
  template<class T> void mult4x4(const T*a,const T*b,T*d);
  template<> inline void mult4x4<float>(const float*a,const float*b,float*d) {
    cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,4,4,4,1.0f,a,4,b,4,0.0f,d,4);
  }
  template<> inline void mult4x4<double>(const double*a,const double*b,double*d) {
    cblas_dgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,4,4,4,1.0,a,4,b,4,0.0,d,4);
  }
  template<class T> void mult4x4v(const T*m,const T*v,T*d);
  template<> inline void mult4x4v<float>(const float*m,const float*v,float*d) {
    cblas_sgemv(CblasRowMajor,CblasNoTrans,4,4,1.0f,m,4,v,1,0.0f,d,1);
  }
  template<> inline void mult4x4v<double>(const double*m,const double*v,double*d) {
    cblas_dgemv(CblasRowMajor,CblasNoTrans,4,4,1.0,m,4,v,1,0.0,d,1);
  }
  template<class T> T det4x4(const T*m) { return cpp::det4x4(m); }
  template<class T> void inv4x4(const T*s,T*d) { cpp::inv4x4(s,d); }
  template<class T> void add16(const T*a,const T*b,T*d) { for(int i=0;i<16;++i)d[i]=a[i]+b[i]; }
  template<class T> void smul16(T s,const T*a,T*d) { for(int i=0;i<16;++i)d[i]=s*a[i]; }
}
SHIMEOF

clang++ -std=c++17 -O3 \
  -DICL_BENCH_MKL=1 \
  -include /tmp/mkl_shim.h \
  -I${MKL}/include \
  $BENCH \
  -L${MKL}/lib/intel64 \
  -lmkl_intel_lp64 -lmkl_sequential -lmkl_core -lpthread -lm -ldl \
  -Wl,-rpath,${MKL}/lib/intel64 \
  -o /tmp/bench-mkl 2>&1 || echo "(MKL build failed)"

echo ""
echo "======================== 1. C++ Scalar (no vectorize) ==============="
/tmp/bench-scalar
echo ""
echo "======================== 2. C++ Auto-vectorized (SSE2) =============="
/tmp/bench-autovec
echo ""
echo "======================== 3. Manual SSE2 intrinsics =================="
/tmp/bench-sse2
echo ""
if [ -f /tmp/bench-mkl ]; then
  echo "======================== 4. MKL cblas ================================"
  /tmp/bench-mkl
fi
'
