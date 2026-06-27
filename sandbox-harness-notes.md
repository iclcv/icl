# Sandbox harnessing — current state & blockers

Goal: let Claude run ICL's **GL** and **Cycles (Metal)** code paths *headlessly* inside the
mscc sandbox, so visual/offscreen-render features can be built **and verified** here instead of
"build-checked only, real-display pass owed". This documents what works, what's blocked, and the
exact evidence — so that once the sandbox profile/code is available we can map each blocker to a
concrete rule change.

> **Update (Session 82).** The mscc profile/rule source is now in hand at `~/margin.mscc`
> (`default-profile.darwin` = the Seatbelt SBPL template; `backends/seatbelt.py` appends the
> per-run file rules; `sandbox.conf`/`sandbox.local` feed only **file** paths, never
> mach-lookup). Findings: **Blocker 1 is RESOLVED** by the current profile (verified here);
> **Blocker 2** needs mach-lookup additions to `default-profile.darwin` — exact rules below,
> but it can only be tested by editing that file and **relaunching** mscc (inside the sandbox
> `log` and nested `sandbox-exec` are both denied, so it can't be iterated from here).

## Environment facts

- Sandbox: **mscc** (Margin Sandboxed Claude Code). Project config = `sandbox.conf`
  (`allow *` / `allow .*` → whole project tree read-write) + `sandbox.local`
  (`allow /Users/celbrech/Qt` read-only) + `allow /Users/celbrech/.ccache`.
- `/tmp` and most of `$HOME` are sandbox-private and discarded on exit. There are also **many
  default macOS sandbox rules** in effect beyond `sandbox.conf` (the part we need to inspect).
- Session is an **Aqua** GUI session (`launchctl managername` → `Aqua`), so a WindowServer
  *exists*; the issue is the sandbox denying access to it, not its absence.
- Hardware: Apple M3 Max, Metal available (Cycles Metal device is enumerated and used).

### Darwin per-user dirs (`/var/folders/ll/<hash>/`)
| Dir | confstr | Writable? |
|---|---|---|
| `T/` | `DARWIN_USER_TEMP_DIR`  (`$TMPDIR`) | ✅ yes |
| `C/` | `DARWIN_USER_CACHE_DIR` | ❌ **listable but writes denied** (`Operation not permitted`) |
| scratchpad `/private/tmp/claude-501/.../scratchpad` | — | ✅ yes |

## What works today
- meson/ninja build, `icl-tests`, all **headless non-GL** code (e.g. `BVHSceneCapture` CPU raytrace).
- Qt apps init far enough under `QT_QPA_PLATFORM=offscreen` to test non-GL error paths.
- **Headless Cycles (Metal) — now works repeatedly** (`geom2-cycles-renderer-test-demo` → valid
  800×600 image every run). This was Blocker 1; see below.

## Blocker 1 — Metal / OpenCL compute-kernel cache (`C/com.apple.metalfe/`)  ← ✅ RESOLVED (Session 82)

**Root cause (historical):** Apple's OpenCL→Metal frontend (`cl2Metal` / `buildComputeProgram`)
and Cycles' Metal kernel compiler write compiled kernel modules (`*.pcm`) into the **Darwin
user cache dir** `…/C/com.apple.metalfe/`. The old symptom was:
```
UNSUPPORTED (log once): buildComputeProgram: cl2Metal failed
icl::utils::CLBuildException: error: unable to open output file
  '/var/folders/ll/<hash>/C/com.apple.metalfe/<hash>/opencl_c-<hash>.pcm': 'Operation not permitted'
```

**Why it's fixed:** the current `default-profile.darwin` grants `file-write*` on
`/var/folders` and `/private/var/folders` (added in mscc commit `a1e74ae`, 2026-06-19 — the
"deny writes outside $HOME" rework). The Darwin user cache dir (`DARWIN_USER_CACHE_DIR` =
`/var/folders/ll/<hash>/C/`) is under that subpath, so `com.apple.metalfe/*.pcm` writes now
succeed. (The earlier "write denied" evidence was against a *prior* profile revision.)

**Verified here (Session 82), inside the live sandbox:**
- Direct fs probe: `touch`/`mkdir` under `…/C/com.apple.metalfe/` → **WRITE OK**.
- `geom2-cycles-renderer-test-demo` run **twice back-to-back** → both saved a valid image
  (158 KB PNG). The historical "first run OK, every subsequent run fails" no longer reproduces.

This same cache path is what ICL's own `utils::CLProgram` / OpenCL kernel builds use, so the
fix covers them too (no ICL-OpenCL gtest exists to exercise it directly headlessly, but the
failing call — `cl2Metal` writing `opencl_c-*.pcm` — is byte-for-byte the path Cycles just
exercised successfully).

## Blocker 2 — Headless GL on macOS (WindowServer / Cocoa XPC)  ← harder, needed for GL render verification

Two independent sub-problems:

1. **`QT_QPA_PLATFORM=offscreen` has no GL backend at all** (Qt limitation, *not* the sandbox):
   ```
   This plugin does not support createPlatformOpenGLContext!
   QOpenGLWidget: Failed to create context
   ```
   So our offscreen `GLSceneCapture` / `Scene2::renderToImage` cannot run under the offscreen
   platform regardless of permissions.

2. **The `cocoa` platform (which *does* support GL) is denied its system services** by the
   sandbox. Running a GL app on the default platform prints:
   ```
   PasteBoard: Error creating pasteboard: com.apple.pasteboard.clipboard [-4960]
   Connection Invalid error for service com.apple.hiservices-xpcservice
   Error received in message reply handler: Connection invalid
   ```
   i.e. mach/XPC lookups to WindowServer-adjacent services are blocked; `QOpenGLWidget`'s ctx
   creation then crashes (per CLAUDE.md). These are **mach-lookup** denials, the classic macOS
   sandbox `(allow mach-lookup (global-name …))` surface.

### ✅ SOLVED for the HEADLESS path (Session 82) — only 2 mach services needed

The blocker above (the on-screen `QOpenGLWidget` cocoa crash) is a **dead end and the wrong
target**: an on-screen window drags in the entire macOS window-management/view-service surface
(`view-bridge`, `windowmanager.server`, `backboard.*`, `inputmethodkit.*`, `dock.*`,
`touchbarserver`, `iconservices`, …) — a huge, fragile set the sandbox shouldn't grant.

But ICL's actual offscreen render (`GLSceneCapture(ownContext=true)` = `QOffscreenSurface` +
`QOpenGLContext`, **no window**) needs almost none of that. Probed on the host (test.sh, which
can run `log show` + `sandbox-exec`, both denied inside the sandbox) with a raw-CGL probe and a
Qt-`QOffscreenSurface` probe:

| profile (file grants + …) | result |
|---|---|
| no mach additions | ❌ `CGLChoosePixelFormat err=10017 invalid CoreGraphics connection` |
| **+ `windowserver.active` + `cvmsServ`** | ✅ `GL_VERSION=4.1 Metal - 90.5`, `RENDERER=Apple M3 Max`, FBO pixel = `64,128,191,255` (rc 0) — **both** the raw-CGL and Qt-offscreen probes render |

So headless GL works with **exactly two** mach-lookup grants. The Qt probe also emitted the
old `pasteboard`/`hiservices-xpcservice` "Connection invalid" lines — but they are **non-fatal
warnings**: the context still created and the pixel still rendered. Everything else the probe
touched (`tccd.system`, `DiskArbitration`, `touchbarserver.mig`, `dock.server`, `analyticsd`,
`CoreServices.coreservicesd`, `CARenderServer`) stayed **denied with no effect on the render**.

**The fix — append to the `(allow mach-lookup …)` block in `~/margin.mscc/default-profile.darwin`:**
```scheme
    ;;; headless offscreen GL (QOffscreenSurface / GLSceneCapture / raw CGL)
    (global-name "com.apple.windowserver.active")   ; CG/GL renderer connection — required
    (global-name "com.apple.cvmsServ")              ; online GLSL→GPU shader compile — required
    ;;; optional: silence non-fatal Qt cocoa-init warnings (not needed to render)
    (global-name "com.apple.pasteboard.1")
    (global-name "com.apple.hiservices-xpcservice")
```
mach-lookup rules live *only* in `default-profile.darwin` (the `sandbox.conf`/`sandbox.local`/
`seatbelt.py` surfaces are file-paths only), so this is a one-time profile edit + mscc relaunch.
The probes + exact profiles are reproducible via `icl/test.sh` (host-side; `--qt` for the Qt
path, `--window` for the rejected on-screen comparison) and `icl/gl-offscreen-probe.c` /
`icl/qt-gl-offscreen-probe.cpp`.

**ICL-side requirement (not a profile issue):** the headless render must run on the **`cocoa`**
platform with a `QGuiApplication` — `QT_QPA_PLATFORM=offscreen` has *no* GL backend at all
(`This plugin does not support createPlatformOpenGLContext!`, a Qt limitation). So a headless
GL-capture entry point (e.g. driving `GLSceneCapture(ownContext=true)` without a GUI window)
is what exercises this; the on-screen `Canvas3D`/`QOpenGLWidget` apps still need a display.

## Status
1. **Blocker 1** (Metal/OpenCL cache write) — ✅ resolved by the current profile (`/var/folders`
   writable since `a1e74ae`); Cycles renders headlessly + repeatably. Verified inside sandbox.
2. **Blocker 2** (headless GL) — ✅ **DONE and verified in-sandbox** (Session 82 follow-up). The
   `windowserver.active` + `cvmsServ` grants are present in `default-profile.darwin` (applied via
   `patch.sh`). Proven two ways inside the live sandbox:
   - bare Qt `QOffscreenSurface`+`QOpenGLContext` probe (`scripts/sandbox-gl-probe-qt.cpp`) →
     `GL 4.1 Metal`, correct FBO pixel;
   - ICL end-to-end via the new `geom2-headless-gl-capture-demo` (cocoa `QGuiApplication` +
     `GLSceneCapture(ownContext=true)`, no window) → 640×480 shaded render, 2711 colors.
   Two real geom2 bugs surfaced + fixed in the process (GLEW not inited in the owned offscreen
   context → segfault; `Renderer::ensureShaderCompiled` left FBO 0 bound → `renderToImage`
   rendered to the incomplete default FBO). See next.md "Session 82 follow-up".

   Headless GL render verification is now a first-class capability here — geom2 GL apps no longer
   need to be "build-checked only".
