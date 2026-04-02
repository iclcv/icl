#!/bin/bash
# Open an interactive shell in the ICL Docker container.
#
# Volumes:
#   - Source is rsynced to /src inside the container (not bind-mounted)
#     to avoid macOS filesystem overhead and .DS_Store/sandbox issues.
#     The rsync excludes build artifacts and git objects.
#   - A named volume (icl-build) persists the build directory across runs
#     so incremental builds work — no full rebuild after tiny changes.
#
# Usage (from repo root):
#   ./scripts/docker/docker-shell.sh
#
# Inside the container:
#   cd /build
#   cmake /src -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
#   cmake --build . -j$(nproc)
#   ctest

set -e
cd "$(git rev-parse --show-toplevel)"

# Create persistent build volume (survives container restarts)
docker volume create icl-build 2>/dev/null || true

# Rsync source into a temporary container, then start interactive shell
# The rsync container shares /src with the shell container via a volume.
echo "Syncing source to container..."
docker run --rm \
  -v "$(pwd):/host:ro" \
  -v icl-src:/src \
  icl-dev rsync -a --delete \
    --exclude='build/' --exclude='build-docker/' \
    --exclude='.git/objects' --exclude='.DS_Store' \
    --exclude='*.o' --exclude='*.dylib' \
    /host/ /src/

echo "Starting shell (source at /src, build dir at /build)..."
docker run --rm -it \
  -v icl-src:/src \
  -v icl-build:/build \
  -w /src \
  icl-dev bash
