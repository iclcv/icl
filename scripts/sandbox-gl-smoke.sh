#!/usr/bin/env bash
#
# Sandbox-harness GL probe — RUN ON THE HOST (outside the mscc sandbox).
#
# Inside mscc, `log show` and nested `sandbox-exec` are denied, so the GL/sandbox
# interaction can't be tested there. This script does it from the host.
#
# DEFAULT MODE — headless offscreen GL (what we actually want):
#   Builds a tiny pure-CGL probe (no Qt, no window) that creates an OFFSCREEN GL
#   context + renders one FBO pixel — the minimal stand-in for ICL's
#   GLSceneCapture(ownContext=true). Runs it under:
#     (1) file-grants only            -> baseline, expect GL context failure
#     (2) file-grants + minimal GL    -> + windowserver.active + cvmsServ
#   and harvests sandbox mach-lookup denials from the unified log for each.
#   If (2) prints "OK pixel=64,128,191,255", headless GL needs only those 2 services.
#
# QT MODE (scripts/sandbox-gl-smoke.sh --qt):
#   Same, but the probe is Qt's QOffscreenSurface + QOpenGLContext on the cocoa
#   platform — the exact path GLSceneCapture(ownContext=true) uses.
#
# WINDOW MODE (scripts/sandbox-gl-smoke.sh --window):
#   Runs the on-screen icl-viewer (QOpenGLWidget) under a broad GL profile. This
#   drags in the whole window-server/view-bridge surface (fragile, pops a real
#   window) — kept only for comparison; NOT the recommended headless path.
#
# Does not modify ~/margin.mscc. Augmented profiles are kept under a temp dir and
# the last one is copied to scripts/sandbox-gl-last-profile.sb for inspection.
#
# To APPLY the fix this proves, run: scripts/sandbox-gl-patch.sh

set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ICL_DIR="${ICL_DIR:-$(cd "$SCRIPT_DIR/.." && pwd)}"
MSCC_DIR="${MSCC_DIR:-$HOME/margin.mscc}"
BASE_PROFILE="$MSCC_DIR/default-profile.darwin"
OUT_PROFILE="$SCRIPT_DIR/sandbox-gl-last-profile.sb"   # generated; gitignored

MODE="headless"
[ "${1:-}" = "--window" ] && MODE="window"
[ "${1:-}" = "--qt" ] && MODE="qt"
QT_DIR="${QT_DIR:-$(ls -d "$HOME"/Qt/*/macos 2>/dev/null | sort -V | tail -1)}"

[ -f "$BASE_PROFILE" ] || { echo "FATAL: base profile not found: $BASE_PROFILE"; exit 1; }
command -v sandbox-exec >/dev/null || { echo "FATAL: sandbox-exec missing (need macOS host)"; exit 1; }

WORK="$(mktemp -d)"; trap 'rm -rf "$WORK"' EXIT

# --- file grants: reproduce what seatbelt.py injects at runtime (project root rw +
#     sandbox.local external paths + parent-dir metadata to traverse $HOME). Without
#     these the bare profile denies all of $HOME and nothing under it can be read/exec'd.
emit_file_grants() {
  echo ""
  echo ";;; --- file grants (mirror mscc runtime: project root + sandbox.local) ---"
  echo "(allow file-read* file-write* file-write-create file-read-metadata (subpath \"$WORK\"))"
  local p="$ICL_DIR"
  while [ "$p" != "/" ]; do p="$(dirname "$p")"; [ "$p" = "/" ] && break
    echo "(allow file-read-metadata (literal \"$p\"))"; done
  echo "(allow file-read* file-write* file-write-create file-read-metadata (subpath \"$ICL_DIR\"))"
  if [ -f "$ICL_DIR/sandbox.local" ]; then
    while read -r kw path _; do
      [ "$kw" = "allow" ] || continue
      path="${path/#\~/$HOME}"; [ -e "$path" ] || continue
      local q="$path"
      while [ "$q" != "/" ]; do q="$(dirname "$q")"; [ "$q" = "/" ] && break
        echo "(allow file-read-metadata (literal \"$q\"))"; done
      echo "(allow file-read* file-read-metadata (subpath \"$path\"))"
    done < "$ICL_DIR/sandbox.local"
  fi
}

mk_profile() {  # $1=outfile  $2..=extra SBPL lines (a mach-lookup block, etc.)
  local out="$1"; shift
  cp "$BASE_PROFILE" "$out"; emit_file_grants >> "$out"
  printf '%s\n' "$@" >> "$out"
}

run_under() {  # $1=label  $2=profile  $3...=command
  local label="$1" prof="$2"; shift 2
  local proc; proc="$(basename "$1")"
  local start; start="$(date '+%Y-%m-%d %H:%M:%S')"
  echo "==================== $label ===================="
  ( sandbox-exec -D "HOME_DIR=$HOME" -D "TMP_DIR=${TMPDIR:-/tmp}" \
        -D "CACHE_DIR=${XDG_CACHE_HOME:-$HOME/.cache}" -f "$prof" "$@" ) >"$WORK/out.txt" 2>&1 &
  local pid=$!; sleep "${RUN_SECS:-5}"; kill "$pid" 2>/dev/null; wait "$pid" 2>/dev/null
  local rc=$?
  echo "--- output (rc=$rc) ---"
  head -20 "$WORK/out.txt"; [ -s "$WORK/out.txt" ] || echo "  (no output)"
  echo "--- sandbox mach-lookup denials for $proc during this run ---"
  log show --start "$start" --style compact 2>/dev/null \
    | grep -iE "Sandbox.*deny.*mach-lookup" | grep -i "$proc" \
    | grep -oiE "mach-lookup com\.apple\.[a-z0-9._-]+" | sort | uniq -c | sort -rn | head -40
  echo
}

GL_MIN='(allow mach-lookup (global-name "com.apple.windowserver.active") (global-name "com.apple.cvmsServ"))'

echo "host: $(sw_vers -productVersion 2>/dev/null)  session=$(launchctl managername 2>/dev/null)  mode=$MODE"
echo

if [ "$MODE" = "headless" ]; then
  PROBE="$WORK/gl-offscreen-probe"
  echo "Building headless GL probe..."
  if ! cc -x c "$SCRIPT_DIR/sandbox-gl-probe.c" -framework OpenGL -o "$PROBE" 2>"$WORK/cc.txt"; then
    echo "FATAL: probe build failed:"; cat "$WORK/cc.txt"; exit 1
  fi
  mk_profile "$WORK/p_file.sb"
  mk_profile "$WORK/p_gl.sb" "" ";;; --- minimal headless-GL mach grants ---" "$GL_MIN"
  run_under "BASELINE  (file grants only)"            "$WORK/p_file.sb" "$PROBE"
  run_under "GL-MINIMAL (+ windowserver.active,cvmsServ)" "$WORK/p_gl.sb" "$PROBE"
  cp "$WORK/p_gl.sb" "$OUT_PROFILE" 2>/dev/null && \
    echo "minimal-GL profile copied to: $OUT_PROFILE"
  echo
  echo "If GL-MINIMAL prints 'OK pixel=64,128,191,255' => headless GL needs ONLY"
  echo "windowserver.active + cvmsServ. Add that block to default-profile.darwin."
elif [ "$MODE" = "qt" ]; then
  [ -d "$QT_DIR" ] || { echo "FATAL: Qt dir not found (set QT_DIR=...): $QT_DIR"; exit 1; }
  PROBE="$WORK/qt-gl-offscreen-probe"
  echo "Building Qt offscreen GL probe (Qt: $QT_DIR)..."
  if ! clang++ -std=c++17 -include arm_acle.h "$SCRIPT_DIR/sandbox-gl-probe-qt.cpp" \
        -F"$QT_DIR/lib" -framework QtCore -framework QtGui -framework OpenGL \
        -Wl,-rpath,"$QT_DIR/lib" -o "$PROBE" 2>"$WORK/cc.txt"; then
    echo "FATAL: probe build failed:"; cat "$WORK/cc.txt"; exit 1
  fi
  # headless GL min + the Qt-cocoa-init services (pasteboard/hiservices were non-fatal
  # warnings for icl-viewer, but grant them so QGuiApplication init is clean).
  GL_QT='(allow mach-lookup
      (global-name "com.apple.windowserver.active") (global-name "com.apple.cvmsServ")
      (global-name "com.apple.pasteboard.1") (global-name "com.apple.hiservices-xpcservice"))'
  mk_profile "$WORK/p_glmin.sb" "" ";;; min GL" "$GL_MIN"
  mk_profile "$WORK/p_qt.sb"    "" ";;; GL + Qt cocoa init" "$GL_QT"
  run_under "QT GL-MINIMAL (windowserver+cvmsServ only)"        "$WORK/p_glmin.sb" "$PROBE" -platform cocoa
  run_under "QT + cocoa-init (+pasteboard.1,hiservices-xpc)"    "$WORK/p_qt.sb"    "$PROBE" -platform cocoa
  cp "$WORK/p_qt.sb" "$OUT_PROFILE" 2>/dev/null && \
    echo "Qt-GL profile copied to: $OUT_PROFILE"
  echo
  echo "If either prints 'OK pixel=...', ICL's GLSceneCapture offscreen path works headless"
  echo "under the sandbox with that mach set."
else
  BIN="$ICL_DIR/builddir/bin/icl-viewer"
  GL_BROAD='(allow mach-lookup
      (global-name "com.apple.windowserver.active") (global-name "com.apple.cvmsServ")
      (global-name "com.apple.pasteboard.1") (global-name "com.apple.hiservices-xpcservice")
      (global-name "com.apple.CARenderServer") (global-name "com.apple.windowmanager.server")
      (global-name "com.apple.window_proxies") (global-name "com.apple.dock.server")
      (global-name "com.apple.iconservices") (global-name "com.apple.iconservices.store")
      (global-name "com.apple.coreservices.appleevents") (global-name "com.apple.CoreServices.coreservicesd")
      (global-name "com.apple.diskarbitrationd") (global-name "com.apple.DiskArbitration.diskarbitrationd")
      (global-name "com.apple.tccd.system") (global-name "com.apple.analyticsd")
      (global-name-prefix "com.apple.tsm.") (global-name-prefix "com.apple.inputmethodkit.")
      (global-name-prefix "com.apple.backboard.") (global-name-prefix "com.apple.view-bridge"))'
  mk_profile "$WORK/p_win.sb" "" ";;; --- broad on-screen GUI/GL mach grants ---" "$GL_BROAD"
  RUN_SECS=6 run_under "WINDOW (icl-viewer, broad GUI profile)" "$WORK/p_win.sb" "$BIN" -i create cameraman
  cp "$WORK/p_win.sb" "$OUT_PROFILE" 2>/dev/null
  echo "Note: on-screen GL needs the whole window/view-bridge surface; prefer headless mode."
fi
