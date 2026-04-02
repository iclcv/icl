# ICL Linux build/test environment
#
# Matches CI (Ubuntu 24.04, clang) plus optional MKL and IPP.
# Usage: see docker-build.sh and docker-bench-fixedmatrix.sh
#
# Build:  docker build -t icl-dev scripts/docker/
# Run:    docker run --rm -v $(pwd):/src icl-dev bash

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Core build tools + ICL required deps (libjpeg, libpng)
# rsync: for efficient source sync from host (avoids bind-mount overhead on macOS)
RUN apt-get update && apt-get install -y \
    build-essential clang cmake ccache git rsync \
    libjpeg-dev libpng-dev ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Optional: Intel MKL + IPP (free, from apt)
# oneAPI MKL provides cblas_sgemm/dgemm and LAPACK
# oneAPI IPP provides optimized image processing primitives (ippi*)
RUN apt-get update && apt-get install -y gnupg wget && \
    wget -qO- https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB \
      | gpg --dearmor -o /usr/share/keyrings/intel-oneapi-archive-keyring.gpg && \
    echo "deb [signed-by=/usr/share/keyrings/intel-oneapi-archive-keyring.gpg] https://apt.repos.intel.com/oneapi all main" \
      > /etc/apt/sources.list.d/intel-oneapi.list && \
    apt-get update && apt-get install -y intel-oneapi-mkl-devel intel-oneapi-ipp-devel && \
    rm -rf /var/lib/apt/lists/*

# MKL + IPP environment (needed for cmake to find them)
ENV MKLROOT=/opt/intel/oneapi/mkl/latest
ENV IPPROOT=/opt/intel/oneapi/ipp/latest
ENV CMAKE_PREFIX_PATH=${MKLROOT}:${IPPROOT}
ENV LD_LIBRARY_PATH=${MKLROOT}/lib/intel64:${IPPROOT}/lib/intel64:${LD_LIBRARY_PATH}

WORKDIR /src
