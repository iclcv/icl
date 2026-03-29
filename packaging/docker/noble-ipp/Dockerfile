FROM --platform=linux/amd64 ubuntu:24.04

LABEL description="Ubuntu 24.04 (Noble) ICL build with Intel IPP for cross-validation"

# System dependencies (minimal — no GUI libs needed for testing)
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential git cmake clang ca-certificates \
    libjpeg-dev libpng-dev libmagick++-dev libpugixml-dev \
    gnupg2 wget \
    && rm -rf /var/lib/apt/lists/*

# Intel oneAPI IPP via APT
RUN wget -qO- https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB \
    | gpg --dearmor -o /usr/share/keyrings/oneapi-archive-keyring.gpg && \
    echo "deb [signed-by=/usr/share/keyrings/oneapi-archive-keyring.gpg] https://apt.repos.intel.com/oneapi all main" \
    > /etc/apt/sources.list.d/oneAPI.list && \
    apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends intel-oneapi-ipp-devel && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

ENV IPPROOT=/opt/intel/oneapi/ipp/latest
ENV LD_LIBRARY_PATH=${IPPROOT}/lib:${LD_LIBRARY_PATH}
ENV CMAKE_PREFIX_PATH=${IPPROOT}:${CMAKE_PREFIX_PATH}

WORKDIR /build
