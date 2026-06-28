#!/usr/bin/env bash
#
# Throw-away patcher: grant the sandbox extension the Metal compiler daemon needs
# to write its OpenCL module cache. RUN ON THE HOST (it writes ~/margin.mscc,
# read-only inside the sandbox). Idempotent. Relaunch mscc afterwards.
#
# Proven by scripts/sandbox-cl-smoke.sh: OpenCL kernel compilation inside mscc
# fails with
#   cl2Metal failed / unable to open output file '.../com.apple.metalfe/...opencl_c-*.pcm':
#   'Operation not permitted'
# The Metal compile runs in the com.apple.MTLCompilerService XPC daemon, which can
# only write the cache via a sandbox extension the (sandboxed) client must ISSUE.
# The real denial is:
#   Sandbox: <proc> deny(1) file-issue-extension target:.../com.apple.metalfe \
#            extension-class:com.apple.app-sandbox.read-write
# Allowing file-issue-extension for that class under /var/folders unblocks it.
set -euo pipefail

PROFILE="${1:-$HOME/margin.mscc/default-profile.darwin}"
ANCHOR='(allow file-ioctl)'   # stable line right after the /var/folders write block

[ -f "$PROFILE" ] || { echo "FATAL: profile not found: $PROFILE"; exit 1; }

if grep -q 'file-issue-extension' "$PROFILE"; then
  echo "Already patched ($PROFILE) — nothing to do."
  exit 0
fi
grep -qF "$ANCHOR" "$PROFILE" || {
  echo "FATAL: anchor '$ANCHOR' not found — profile layout changed."
  echo "Add this block manually (e.g. after the file-write* /var/folders rule):"
  echo '    (allow file-issue-extension'
  echo '        (require-all'
  echo '            (extension-class "com.apple.app-sandbox.read-write")'
  echo '            (subpath "/private/var/folders")))'
  exit 1
}

perl -0pi -e 's{(\Q(allow file-ioctl)\E\n)}{$1\n;;; Metal/OpenCL shader compile: the com.apple.MTLCompilerService XPC daemon\n;;; writes its module cache (.../com.apple.metalfe/) only via a sandbox extension\n;;; the (sandboxed) client must ISSUE. Without this, OpenCL kernel compilation\n;;; fails with EPERM writing opencl_c.pcm (see scripts/sandbox-cl-smoke.sh).\n(allow file-issue-extension\n    (require-all\n        (extension-class "com.apple.app-sandbox.read-write")\n        (subpath "/private/var/folders")))\n}' "$PROFILE"

echo "Patched: $PROFILE"
echo "Added (allow file-issue-extension ...) for the Metal module cache."
echo "Relaunch mscc for it to take effect."
