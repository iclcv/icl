# ICL — TODO

Live backlog of concrete actionable items.  Entries link to memory files
for depth where applicable.  See `CONTINUE.md` for the chronological
session log, and `module-audit-checklist.md` for the audit protocol.

---

## Ongoing — module header audits

Applied per `module-audit-checklist.md`.

- [x] `icl/utils/` — audit run Session 49+.  Small/medium cleanups landed.  Long-term items migrated into this TODO (below).
- [ ] `icl/core/` — pending.  Probably few `detail/` candidates (mostly foundational types).
- [ ] `icl/math/` — facade reorg done; remaining public headers haven't had a full audit pass.
- [ ] `icl/filter/` — pair with backend-split proposal (see `project_filter_dispatch_arch.md`).
- [ ] `icl/io/` — subdirs landed; remaining public headers worth an audit pass.
- [ ] `icl/cv/`, `icl/geom/`, `icl/geom2/`, `icl/qt/`, `icl/markers/`, `icl/physics/` — never audited.

---

## Configurable — onChildSetChanged follow-ups

- [x] **`removeChildSetCallback` API.**  Landed.  Introduced
  `Configurable::CallbackToken` (opaque `std::uint64_t`, `0` = invalid).
  `registerCallback` / `onChildSetChanged` now return a token;
  `removeCallback(CallbackToken)` / `removeChildSetCallback(CallbackToken)`
  unregister.  Storage flipped to `vector<pair<CallbackToken, Callback>>`;
  dead `removedCallback` stub deleted.  `UnaryOp::registerCallback` and
  `Grabber::registerCallback` overloads widened to the same token-returning
  shape (their `using Configurable::registerCallback;` clauses removed — they
  were redundant after the signature match).  `ConfigurableGUIWidget` now
  captures both tokens and unregisters them in a new destructor so a
  Configurable outliving the widget can't fire into a dangling `this`.  5
  new tests in `tests/test-utils.cpp` (fire, distinct-nonzero tokens,
  unregister, stale/zero no-op, child-set add+remove round-trip).  871/871.

- [x] **Wire ImageCompressor codec swap to the new rebuild channel.**
  Landed as `icl-compressor-playground-demo` (commit `614d4fbdd`).
  Flipping `mode` swaps the codec child, Prop rebuilds, codec-specific
  tunables appear/disappear.  Surfaced a pre-existing race on
  `m_data->plugin` — a GUI-thread `installPlugin()` vs. a worker-thread
  `compress()`; fixed with a recursive_mutex in the pimpl
  (commit `87b70f544`).

- [x] **Same for GenericGrabber backend swap.**  Landed as
  `icl-grabber-backend-swap-demo` (commit `403f2a897`).  Four buttons
  drive `GenericGrabber::init()` with different `(type, id)` pairs;
  Prop on the grabber rebuilds as the backend child swaps.  Minor
  gotcha: `init(type, params)` expects params in `"type=id"` parse
  shape, not the bare id — documented in the demo's `swapBackend`
  helper.

- [x] **Capability flags for compression codecs.**  Landed
  Session 60 (commit `d99753476`).  `CompressionPlugin::Capabilities`
  declares `(depths, [minChannels, maxChannels], formats)` whitelists
  per codec (empty = no constraint); `ImageCompressor::compress()`
  pre-validates and emits a uniform codec-named error.  Codec
  declarations: raw / zstd accept anything; rlen requires depth8u;
  jpeg requires depth8u + 1/3 channels; 1611 requires depth16s + 1
  channel.  Two new tests (`capabilities.1611_rejects_rgb`,
  `capabilities.raw_accepts_any`).  Plugin-internal throws kept as
  defense-in-depth.

- [ ] **qt::Prop UI integration of capability flags.**  Data layer
  is ready (`plugin->capabilities()`); only the `mode` menu greying-
  out for incompatible codecs given the current source image
  remains.  Closes the Session 48 "auto codec" deferral when paired.

---

## ICLWidget OSD

- [x] **Scale-range (zoom/fit) button behaves strangely.**  Landed
  Session 60 (commit `3a37f9184`).  Three layered fixes in
  `icl/qt/Widget.cpp`: (1) **root cause** — `rebufferImageInternal`
  pushed manual BCI sliders unconditionally, so `setRangeMode` only
  took effect on the *next* `setImage` call rather than the current
  frame; now mirrors the rm-aware branch from `setImage`.
  (2) **`OSDGLButton::stateFn`** — new live-state callback so the
  icon derives from the actual `m_data->rm` each paint, not from a
  cached `toggled` flag.  (3) **`setRangeModeNormalOrScaled`
  round-trips through `rmOff`** via `Data::rmLastNonOff`,
  preserving the user's manual-BCI ("custom"/`rmOn`) configuration.
  Also dropped the `create_menu` + double-`showHideMenu` hack and
  added a `bciAuto` snapshot in `create_menu` to fix a separate
  `KeyNotFoundException: bci-update-mode` on `setMenuEmbedded` rebuild.

- [ ] **OSDGLButton drift audit on the other toggle buttons.**  Now
  that `stateFn` exists, the same caching-vs-truth drift as the
  scale-range button likely affects the other OSD toggles
  (interpolation NN/LIN, embedded-zoom, fullscreen,
  detach/attach).  Wire each one's `stateFn` to its underlying
  state owner so all OSD toggles always reflect truth.  Quick
  polish (~30 min).

## Qt / GL stability

- [x] **`GLImageRenderer` post-free SIGBUS in deferred Metal-OpenGL
  upload.**  Landed Session 60 (commit `a2f446392`).  The RGBA
  staging buffer in `uploadTexture()` was a stack-local
  `std::vector<icl8u>` whose data pointer was passed to
  `glTexSubImage2D`.  macOS's OpenGL-on-Metal layer is deferred
  (texture uploads queue onto Metal command buffers consumed on
  `com.Metal.CompletionQueueDispatch`), so the GPU read freed
  memory after the function returned — observed as `SIGBUS` at
  page-aligned `0x200000000`.  Fixed with a persistent member
  buffer + `glFlush()` after upload.

- [ ] **Verify Metal-OpenGL completion crash is gone outside the
  sandbox.**  User reported the same `0x200000000` post-free
  signature on `com.Metal.CompletionQueueDispatch` even after the
  GLImageRenderer + rlen-rewrite fixes landed.  Likely either (a)
  another upstream source of heap corruption we haven't found, or
  (b) a Qt6 QRhi-on-Metal stability issue we can't fix from inside
  ICL.  Needs a re-test against the new build before either path
  is decidable.

## Qt GUI component plumbing

- [x] **Designated-init GUI component syntax.**  Landed Session 59 —
  all 33 components available under `qt::ui::` with a mixed
  positional+designated-init shape:
  ```cpp
  gui << ui::Slider(0, 255, 42, {.vertical=true, .step=2,
                                 .handle="gain", .label="Gain"});
  ```
  Primary "obvious" data args stay positional; everything else moves
  into a trailing `XxxOpts{}` pack via C++20 designated init.  See
  `ui-plan.md` for the design rationale (per-component Opts, shared
  metadata duplicated across Opts, `if constexpr(requires{...})` in
  `applyCommon`, containers-via-inheritance).  Legacy
  `Slider(0,255,42).handle(...)` stream-insertion builder unchanged —
  both routes converge on the same widget factory.  Commits
  `103e9316f` (spike), `fbafa28a9` (Phase 2: 12 leaves), `b3ccf13d8`
  (Phase 3: 10 leaves), `8891274e2` (Phase 4+5: 7 containers + 3
  finalizers), plus `9ffa376c7` for `ui-plan.md`.  `ui-syntax-demo`
  exercises every component interactively.

- [ ] **Rework `GUIComponent` internal representation.**  Today every
  component is serialized to a `type(params)[@handle=X@label=L@...]`
  string in `GUIComponent::toString()`, parsed back by
  `GUIDefinition` inside `GUI::create()`, then dispatched to a
  `GUIWidget` subclass.  The string round-trip is baroque (comma
  escaping issues, default-out-N autogeneration, parse errors that
  bubble up to the user as opaque exceptions) and exists mainly
  because the original GUI API accepted raw string definitions.
  Replace with direct typed component → widget dispatch: each
  component emits a typed descriptor struct; `GUI::create()`
  dispatches without going through a string.  The string parser
  can remain as a legacy entry point for callers that still pass
  raw definitions.

---

## Configurable typed-storage migration — LANDED Sessions 53–54

Session 53 (~23 commits) + Session 54 follow-ups (6 commits) on the same arc:

- Typed `prop::` constraint family (`Range<T>`, `Menu<T>`, `Flag`, `Command`, `Info`, `Text`, `Generic`, `core::prop::Color`, `core::prop::ImageView`).
- Typed `addProperty<C>` overload; all ~500 in-tree call sites migrated.
- `Property::value` string field retired; `typed_value` is sole storage.
- `Property::type` / `Property::info` string fields retired; synthesized on demand from `constraint` via adapter (Session 54 step 8).
- `setPropertyValueTyped(name, std::any)` — direct typed writes.
- `PropertyHandle` / `PropertyValueRef` proxy; `prop("x").value = v` routes through `setPropertyValueTyped`.
- `Property::as<T>()` for typed reads off `typed_value`.
- `getPropertyValue` returns `AutoParse<std::any>` — zero-parse fast path for typed-matching reads.
- `getPropertyConstraint(name)` — public getter for the structured constraint.
- qt::Prop widget dispatch keyed on `constraint.type()` via `std::any_cast` chain (Session 54 step 5), not legacy type strings.

See `CONTINUE.md` Session 53 Summary for the full arc.

### Step 9 follow-ups

- [x] **Step 5 — qt::Prop constraint-driven dispatch.**  Landed — four commits (`7a460df84` `ee2296433` `c9145c3bb` `6a424250b`).  All three dispatch sites in `icl/qt/GUI.cpp` now `std::any_cast` on `Property::constraint`; `getPropertyType` / `getPropertyInfo` no longer consumed by qt::Prop.
- [~] **Step 7 — retire legacy string-taking `addProperty` overload.**  Skipped intentionally (2026-04-22).  The overload serves as the dynamic-registration entry point for PylonCameraOptions / OpenNI / dc that pass hardware-introspected type+info strings; renaming has low value and removing would force a typed API those callers can't easily construct.
- [x] **Step 8 — drop `Property::type` / `Property::info` fields; synthesize on demand.**  Landed — two commits (`c90055b37` `7312e5385`).  Added `prop::Generic` catch-all constraint (echoes legacy type/info strings verbatim) so `buildConstraintFromLegacy` always returns a populated constraint; retired the two stored fields; getters synthesize via `prop::lookupAdapter(constraint.type())`.

---

## DataStore / Assign migration (storage cleanup landed; Event smuggling pending)

Session 51 closed every storage-side item on this section:
flipped `DataStore::Data::assign()` to `AssignRegistry::dispatch()`
(~830 lines of `AssignSpecial<>` machinery deleted); added identity
+ `Event → H` enrollments; replaced `MultiTypeMap` with composed
`AnyMap` inside DataStore; deleted the `void* + RTTI-name`
dispatch paths from `AssignRegistry` (now std::any-based
end-to-end); and deleted `MultiTypeMap` entirely (272 lines + its
`friend class` in `GUIHandleBase.h`).  `HandleType h = gui["name"]`
and `gui["x"].render()` both work against the new registry.

What's still open:

(Event smuggling retirement + Slot proxy — landed; the Data class
was renamed `Slot`, the `Event` struct / `Assign<H, Event>` trait
/ `HandleEventEnrollments.cpp` are deleted.  `Slot::render()` and
the other verbs do a direct type-cascade over the stored
`std::any` in `qt/HandleVerbDispatch.cpp`.  `Slot` already carries
templated `operator=(T)` / `operator T() const`, so
`int v = gui["k"]` works without `.as<int>()`.)

- [ ] **Core-type identity** enrollments (Rect, Size, Point, Image,
  std::string, utils::Any) if any end up stored in DataStore.  Not
  currently needed — after `.out()` retirement DataStore only holds
  handles.  Add if/when a non-handle type gets an allocValue entry.

- [ ] **Smuggled-command fallout audit** — `= Event(` / `= Range` call
  sites outside `DataStore::Data`'s own methods.  Earlier session
  note from Session 50 expected these to exist; quick grep didn't
  find any in-tree, but worth a second pass as part of the Event
  retirement above.

## Utils — long-term migrations (from Session 49+ audit)

These duplicate / predate modern stdlib features.  Migrating them is a
user-API break and touches many callers.

- [ ] **`File.h` → `std::filesystem` + `std::ifstream`.**  Predates C++17.  35 consumers.  Plausibly migrate piecemeal — `File` stays as a thin facade during transition.
- [ ] **`Time.h` / `Timer.h` → `std::chrono`-based.**  41 consumers of `Time`.  Pervasive.  Likely keep the ICL `Time` name as a type alias over `std::chrono::nanoseconds` (or similar) — callers don't need to change arithmetic, just serialization.
- [ ] **`Lockable.h` → `std::mutex` composition.**  13 classes inherit publicly for `lock()/unlock()/getMutex()`.  Migration: each class holds a `mutable std::mutex` and exposes `getMutex()`; callers write `std::scoped_lock lk(obj.getMutex())`.
- [ ] **`UncopiedInstance<T>` → rename** to `DefaultConstructOnCopy<T>` or similar.  Style-only; 2 consumers (`Configurable`, `ViewBasedTemplateMatcher`).
- [ ] **`Any.h`** — name collision with `std::any` is unfortunate.  No practical rename since it's pervasive.  Document the semantic distinction; don't touch.
- [ ] **`Timer`** — `int m_iTimerMode` → enum; `startTimer()` → `start()`; raw-long returns → `std::chrono::nanoseconds`.

## Utils — small follow-ups

- [ ] **`ProcessMonitor`** — `getInstance()` returns raw ptr; should return reference.  Also a **PRIVATIZE** candidate (only `qt/GUI.cpp` uses it; no installed header pulls it in).
- [ ] **`CompatMacros.h`** — extract the GL includes (`GL/glew.h`, `<OpenGL/gl.h>`, …) into a dedicated `GLIncludes.h` only included where GL is needed.  CompatMacros shouldn't pull GL into the 381 TUs that include it.
- [ ] **`FixedArray`** — long-term: replace with `std::array<T,N>` + free accessors for `x`/`y`/`z`/`w`.  The union-based layout is technically UB (but widely-relied-on).

---

## Image / core migration residue

See `project_image_migration.md`.

- [ ] **`applyParallel`** in the `Image` class — scheduled but not implemented.
- [ ] **`bpp` removal** from `Image`/`ImgBase` interface — deprecated but not removed.
- [ ] Filter migration to `Image` — most Ops done (29 as of Session 43); remaining UnaryOp subclasses need the Prop-driven Configurable migration.

---

## Op API — float setter on int-typed property

- [x] **Tighten setter arg types to match constraint value_type.**  Landed
  (Session 55 follow-up).  Per-Op resolution:
  - `ThresholdOp` — widened props to `Range<float>` (internal storage is float).
  - `CannyOp` — widened thresholds to `Range<float>` (internal storage is `icl32f`).
  - `UnaryArithmeticalOp::setValue(icl64f)` — widened prop to `Range<float>`;
    setter casts the `icl64f` arg to float at the write site to match the
    stored type (only `Range<int>` / `Range<float>` adapters are registered).
  - `UnaryCompareOp::setValue / setTolerance(icl64f)` — same treatment as
    UnaryArithmeticalOp.
  - `RansacBasedPoseEstimator::setMinPointsForGoodModel(float)` — narrowed to
    `setMinPointsForGoodModel(int)` (the value is semantically an integer
    count); ctor param narrowed to match.

## Filter module

- [ ] **Backend split for legacy Ops** — `X.cpp / X_Cpp.cpp / X_Ipp.cpp / X_SSE.cpp / X_OpenCL.cpp` layout, pair with `filter/detail/` placement.  See `project_filter_dispatch_arch.md` and `project_module_subdirs.md`.
- [ ] **IPP cross-check** — verify non-IPP filter impls against IPP in a Linux container.  See `project_ipp_crosscheck.md`.
- [ ] **`BackendProxy::backends(Backend b)`** proxy for terser multi-registration.  See `project_backends_proxy.md`.
- [x] **LocalThresholdOp collapses multi-channel input.**  Landed `aa196d084`.
  Root cause: `apply_local_threshold_six` hoisted `src.begin(0)` / `ii.begin(0)`
  outside the per-channel loop, so every iteration recomputed thresholds from
  channel 0 of both src and the integral image and wrote into channel 0 of dst
  (channels 1+2 stayed at ensureCompatible's zero init → red-black display
  for a 3-channel input via formatMatrix).  Moved `begin(c)` into the loop;
  also fixed the "invert output" post-pass that only inverted channel 0.
  Added `Filter.LocalThreshold.regionMean_multichannel` regression test.

---

## Fun — image-editor demo ("icl-edit"?)

- [ ] **Build a mini photo-editor app on top of ICL.**  Fun target that
  doubles as a stress test for the Op framework + qt::Prop +
  GenericGrabber (file) + the callback-push GUI channel that just
  landed.  Needs some new filters first:
  - **`BrillianceOp`** — Apple-Photos-style tone lift: shadow raise +
    midtone contrast S-curve + highlight rolloff.  Drives luminance
    (Y in YUV or V in HSV), blends back into RGB.  LUT-driven for
    speed.  Expose `amount` (range), `shadow-lift`, `highlight-keep`.
  - **`VibranceOp`** — saturation boost that spares already-saturated
    pixels and protects skin tones.  Operates on HSV S with a
    protection mask keyed off H band.  Expose `amount`, `skin-protect`.
  - **`ClarityOp`** / `StructureOp` — local mid-tone contrast via
    unsharp-mask on luminance with an edge-aware kernel
    (bilateral-weighted).  Expose `amount`, `radius`, `threshold`.
  - **Curves** — user-tunable tone curve over RGB or per-channel.
    Probably a new GUI component (draggable control points) — pairs
    with the "designated-init GUI components" TODO.
  - **Crop / rotate / straighten** — interactive geometry ops on the
    canvas.  Probably uses `AffineOp` + a `CropOp` (maybe just
    `setROI` + `clip-to-ROI`).
  - **Histogram display** — reuse `qt::HistogrammWidget`.

  App structure: `Display` as canvas, a vertical stack of `Prop(op)`
  for each filter in a pipeline, drag-to-reorder, preview-debounced
  render.  Save/load pipeline as a ConfigFile (XML for now, YAML
  when `project_yaml_config.md` lands).  Exportable recipes.

  Out-of-scope but related: nondestructive editing (layer stack),
  RAW loading (needs libraw integration).  Keep v1 to "load JPEG,
  tweak, save JPEG."

  Good validation of: filter chaining ergonomics, the push-callback
  GUI refresh at interactive rates, per-property clamping on
  sliders with gamma-style non-linear scales, ConfigFile round-trip.

---

## IO / compression / transport

From Session 48 deferrals:

- [x] **Capability-flag codec classification** (lossy vs lossless;
  supported depths) → enables `auto` codec mode via the
  PluginRegistry's applicability machinery.  Landed Session 60
  (commit `d99753476`) — see "Capability flags for compression
  codecs" under the onChildSetChanged section.  `auto` codec mode
  is the obvious follow-up that uses this surface.
- [ ] **Additional codecs**: `webp`, `jxl`, `lz4`, `deflate`/`zlib`.
- [ ] **`Configurable` events on child-set change** — so `qt::Prop` auto-rebuilds when `ImageCompressor` swaps codec.  See `project_dynamic_child_configurables.md`.
- [ ] **Browser viewer for WSGrabber** (JS-side envelope parser).  See `reference_websocket.md`.
- [ ] **`wss://` (TLS)** for WS transport.
- [ ] **WSGrabber server mode** (push-source workflow).
- [ ] **Path-based multi-stream** on one WS server (`/cam0`, `/cam1`).

---

## Geometry / rendering (geom2, Cycles)

- [ ] **Port old `raytracing-physics-demo` to CyclesRenderer.**  See `project_physics_demo.md`.
- [ ] **Analytic sphere positional offset bug in Cycles** — disabled for now.  See `project_analytic_sphere.md`.
- [ ] **`CoordinateFrameSceneObject` → PIMPL.**  See `project_coordframe_pimpl.md`.
- [ ] **`Scene2` getters** — return refs / `shared_ptr`, not raw pointers.  See `project_scene2_getters.md`.
- [ ] **`Scene2::getGLCallback`** — return raw ptr, not `shared_ptr`.  See `project_scene2_glcallback.md`.
- [ ] **Generic `utils::MemoryPool`** with custom-deleter release, replacing QuickContext's Image-tracking pool.  See `project_memorypool.md`.

---

## Tests & build

- [ ] **Parallel test flakiness.**  `tests/icl-tests` under default `-j` drops 5–10 tests; `-j 1` passes 567/567.  Shared static/global somewhere in `Quick2.{Create,Filter,Math,Compose}` paths (ROI bleed across tests).  See `project_test_parallel_flakiness.md`.
- [ ] **Meson → CMake removal** — Meson migration landed Session 31; CMake files may still linger in corners.  See `project_meson.md`.

---

## Configuration format

- [x] **Replace XML with YAML for `ConfigFile`.**  Session 56: landed in two phases.
  - Phase 1 — in-house `icl::utils::yaml` parser / emitter under `icl/utils/Yaml.{h,cpp}` + `icl/utils/detail/yaml/`.  Zero-copy for parse (views into source buffer), arena-backed `Mapping` for programmatic insertion.  Benchmarks beat yaml-cpp by 20-40× and tie rapidyaml on config-shaped inputs.  112 tests + 117 corpus cases from yaml-test-suite / JSONTestSuite.  See `project_yaml_config.md` for subset details.
  - Phase 2 — `ConfigFile` migrated to YAML backend.  Wire format switched to nested YAML (typeless, caller's `get<T>` is authoritative), `register_type` / `Maps` / RTTI machinery deleted (408-line net deletion).  Restrictions demoted to in-memory-only.  824/824 tests green.
- [x] **Phase 3 — pugi retirement.**  LANDED.  Replaced by
  Session 57: in-house `icl::utils::xml` (parser + DOM + emitter
  + XPath subset) replaced pugi end-to-end.  Primitive3DFilter and
  OptrisGrabber migrated; `icl/utils/detail/pugi/` deleted
  (~15,218 LOC gone).  866/866 tests green.  See
  `xml-config-plan.md` and `project_xml_config.md` memory.  Open
  follow-up: deferred SIMD perf pass for the parser's per-byte
  character-class scans (~4× gap vs pugi on raw parse; tied on
  XPath/traverse).

---

## Dependency hygiene

- [ ] **Qt6 multimedia grabbers** — QVideoSink rewrite needed.  See `project_qt6_multimedia.md`.
- [ ] **LibAVVideoWriter** — rewrite for FFmpeg 6+/7+ API.  See `project_ffmpeg.md`.
- [ ] **ImageMagick 7 PixelPacket API** rewrite needed for ICLIO plugins.  See `project_imagemagick7.md`.
- [ ] **Apple Accelerate** as IPP replacement on macOS — partial; audit remaining IPP sites.  See `project_accelerate.md`.
- [ ] **OpenCL on macOS** — C++ bindings work deferred.  See `project_opencl_mac.md`.
- [ ] **NEON** as SSE replacement on ARM — investigate sse2neon coverage.  See `project_neon.md`.
- [ ] **Eigen** — brew's 5.0.1 has an Apple Clang bug; pin to 3.4.x.  See `project_eigen5.md`.

---

## Core — latent Image.h include dependency

- [x] **`core/Image.h` includes `core/Img.h`.**  Landed.  `Image::as<T>()`
  needs `Img<T>`'s derivation from `ImgBase` visible at the `static_cast`
  site under Clang 21; `Image.h` now pulls `Img.h` at the bottom so every
  consumer of `Image.h` gets it transitively, no per-TU opt-in needed.

---

## C++17 source modernization (Sessions 27–28 residue)

- [ ] Any deferred source-code fixes from the C++17 migration.  See `project_cpp17.md`.
- [x] **Drop explicit template args on `std::lock_guard` / `std::scoped_lock` / `std::unique_lock`.**  Landed.  Sweep flipped `lock_guard<std::mutex>` /
  `lock_guard<std::recursive_mutex>` to CTAD-deduced `scoped_lock`,
  `scoped_lock<std::mutex>` to plain `scoped_lock`, and
  `unique_lock<std::mutex>` to CTAD `unique_lock` (kept as `unique_lock`
  — distinct semantics vs. `scoped_lock`).  ~278 sites across 73 files.
  Remaining `std::lock_guard<Scene2>` occurrences are custom-lockable uses
  (not `std::mutex`) and stay explicit.

---

## How to maintain this file

- Each item is a one-line actionable "what".  Link to memory for "why" and context.
- Check items off when they land, with the commit hash in the message.
- Move fully-completed sections into the relevant Session summary in `CONTINUE.md`, then delete from here.
- New items go in the appropriate section; if no section fits, start a new one.
- Keep this file focused: if a theme becomes inactive for 6+ months, archive to memory.
