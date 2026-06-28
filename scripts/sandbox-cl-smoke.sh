#!/usr/bin/env bash
#
# Sandbox-harness OpenCL probe — RUN ON THE HOST (outside the mscc sandbox).
#
# Inside mscc, OpenCL kernel compilation fails:
#   cl2Metal failed / unable to open output file '.../com.apple.metalfe/...opencl_c-*.pcm':
#   'Operation not permitted'   (and could not build module 'opencl_c')
# even though /var/folders is write-granted and com.apple.MTLCompilerService is
# mach-allowed. This script reproduces the compile under the mscc profile and
# harvests EVERY sandbox denial (file AND mach-lookup) the probe triggers, so we
# can see exactly what the Metal frontend needs. `log show` can't run inside
# mscc, hence host-only.
#
# It runs the probe twice:
#   (1) base profile + file grants (mirror mscc runtime) — the failing baseline
#   (2) + a candidate grant block (CL_EXTRA) — to test a fix in one shot
#
# Does not modify ~/margin.mscc. Generated profile: scripts/sandbox-cl-last-profile.sb
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ICL_DIR="${ICL_DIR:-$(cd "$SCRIPT_DIR/.." && pwd)}"
MSCC_DIR="${MSCC_DIR:-$HOME/margin.mscc}"
BASE_PROFILE="$MSCC_DIR/default-profile.darwin"
OUT_PROFILE="$SCRIPT_DIR/sandbox-cl-last-profile.sb"

[ -f "$BASE_PROFILE" ] || { echo "FATAL: base profile not found: $BASE_PROFILE"; exit 1; }
command -v sandbox-exec >/dev/null || { echo "FATAL: sandbox-exec missing (need macOS host)"; exit 1; }

WORK="$(mktemp -d)"; trap 'rm -rf "$WORK"' EXIT

# Mirror what seatbelt.py injects at runtime: project-root rw + parent metadata.
emit_file_grants() {
  echo ""
  echo ";;; --- file grants (mirror mscc runtime: project root) ---"
  echo "(allow file-read* file-write* file-write-create file-read-metadata (subpath \"$WORK\"))"
  local p="$ICL_DIR"
  while [ "$p" != "/" ]; do p="$(dirname "$p")"; [ "$p" = "/" ] && break
    echo "(allow file-read-metadata (literal \"$p\"))"; done
  echo "(allow file-read* file-write* file-write-create file-read-metadata (subpath \"$ICL_DIR\"))"
}

mk_profile() { local out="$1"; shift; cp "$BASE_PROFILE" "$out"; emit_file_grants >> "$out"; printf '%s\n' "$@" >> "$out"; }

run_under() {  # $1=label  $2=profile  $3...=command
  local label="$1" prof="$2"; shift 2
  local start; start="$(date '+%Y-%m-%d %H:%M:%S')"
  echo "==================== $label ===================="
  ( sandbox-exec -D "HOME_DIR=$HOME" -f "$prof" "$@" ) >"$WORK/out.txt" 2>&1
  local rc=$?
  echo "--- probe output (rc=$rc) ---"; cat "$WORK/out.txt"
  echo "--- RAW Sandbox deny lines during this run ---"
  log show --start "$start" --style compact 2>/dev/null \
    | grep -iE "Sandbox|metalfe|cl2Metal|mtlcompiler|sandbox_apply|deny" \
    | grep -iE "deny|sandbox_apply|metalfe" | head -40
  echo "  (if nothing above, the EPERM is NOT an mscc Seatbelt denial)"
  echo
}

echo "host: $(sw_vers -productVersion)  session=$(launchctl managername 2>/dev/null)"
PROBE="$WORK/cl-probe"
echo "Building OpenCL probe..."
cc -x c "$SCRIPT_DIR/sandbox-cl-probe.c" -framework OpenCL -o "$PROBE" || { echo "FATAL: probe build failed"; exit 1; }

# Candidate fix: allow ISSUING the sandbox extension the MTLCompilerService daemon
# needs to write its module cache (the real blocker — file-issue-extension, seen as
# deny(1) file-issue-extension target:.../com.apple.metalfe).
CL_EXTRA=';;; --- candidate: Metal-compiler module-cache sandbox extension ---
(allow file-issue-extension
    (require-all
        (extension-class "com.apple.app-sandbox.read-write")
        (subpath "/private/var/folders")))'

mk_profile "$WORK/p_base.sb"
mk_profile "$WORK/p_extra.sb" "$CL_EXTRA"
mk_profile "$WORK/p_allow.sb" ";;; --- DECISIVE: allow everything (override deny default) ---" "(allow default)"

run_under "BASELINE (mscc profile + file grants) — expect FAIL"  "$WORK/p_base.sb"  "$PROBE"
run_under "DECISIVE: mscc profile + (allow default)"             "$WORK/p_allow.sb" "$PROBE"
run_under "+ candidate Metal-compile mach grants only"           "$WORK/p_extra.sb" "$PROBE"

cp "$WORK/p_extra.sb" "$OUT_PROFILE" 2>/dev/null && echo "candidate profile -> $OUT_PROFILE"
echo
echo "INTERPRETATION:"
echo " * DECISIVE run PASSES ('OK: kernel built') -> it IS an mscc Seatbelt rule;"
echo "   the deny lines / candidate run tell us which grant to add to the profile."
echo " * DECISIVE run STILL FAILS even with (allow default) -> NOT mscc's profile"
echo "   (Metal frontend self-sandboxes; macOS forbids nesting). Workaround: run any"
echo "   OpenCL program on the HOST once to rebuild the opencl_c.pcm cache; inside"
echo "   mscc it is then read-only and OpenCL compiles fine."
