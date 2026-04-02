#!/bin/bash
# Build the ICL Docker image for Linux testing.
#
# Creates an Ubuntu 24.04 container with clang, cmake, rsync, and Intel MKL.
# The image is cached after the first build (~2GB due to MKL).
#
# Also creates two named volumes:
#   icl-src   — rsynced source (avoids macOS bind-mount overhead)
#   icl-build — persistent build dir (survives container restarts)
#
# Usage (from repo root):
#   ./scripts/docker/docker-build.sh
#
# After building:
#   ./scripts/docker/docker-shell.sh              # interactive shell
#   ./scripts/docker/docker-bench-fixedmatrix.sh   # run benchmarks
#   ./scripts/docker/docker-test.sh                # build + test ICL

set -e
cd "$(git rev-parse --show-toplevel)"

echo "Building ICL Docker image (icl-dev)..."
docker build -t icl-dev scripts/docker/
echo ""

echo "Creating persistent volumes..."
docker volume create icl-src 2>/dev/null && echo "  icl-src  (source)" || echo "  icl-src  (exists)"
docker volume create icl-build 2>/dev/null && echo "  icl-build (build artifacts)" || echo "  icl-build (exists)"
echo ""
echo "Done. Image: icl-dev"
echo "Next: ./scripts/docker/docker-shell.sh"
