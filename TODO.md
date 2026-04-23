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

## ICLWidget OSD

- [ ] **Scale-range (zoom/fit) button behaves strangely.**  Surfaced
  with `icl-region-inspector -i create cameraman` 2026-04-21 — the
  scale-range OSD button in the image-viewer panel responds
  oddly.  Symptom not yet characterized in detail; worth reproducing
  and capturing before digging.  Possibly connected to signal /
  state caching around `bciUpdateAuto` / `channelUpdateAuto` after
  the `.out()`-retirement handle-pointer-to-std::function<bool()>
  conversion in `icl/qt/Widget.cpp`, but could be independent.

## Qt GUI component plumbing

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

## Next: Configurable internal storage → typed `std::any`

- [ ] **Move `Configurable::Property::value` from `std::string` to `std::any`.**  Session 52 retired `utils::Any` and flipped Configurable's *API surface* to `AutoParse<std::string>`, but the *internal storage* is still a serialized string.  Next step: store property values as typed `std::any` (matching the declared property type), with `str(v)` / `parse<T>(s)` only at config-file I/O boundaries.  Affects:
  - `Configurable.h`: `Property::value` (`std::string` → `std::any`), `Property::payload` (already `std::any` — may fold together).
  - `Configurable.cpp`: `addProperty` constructs the `std::any` from the initial value using the declared type; `setPropertyValue(AutoParse<string>)` parses incoming strings into the declared type; `getPropertyValue()` returns `AutoParse<std::any>` so callers keep `T x = c.getPropertyValue(name)` ergonomics.
  - `ConfigFile`: serialize with declared type on save, parse on load.  No on-disk format change.
  - `qt::Prop`: already reads both string (`getPropertyValue`) and any (`getPropertyPayload`) — reconcile into a single typed read.
  - Existing plugin `setPropertyValue` overrides (`UnaryOp`, `CornerDetectorCSS`, etc.) that manually `parse<float>(value)` simplify — they can take the typed value directly.
  - Reading: 33 subclasses, ~524 `addProperty` call sites, ~557 `set/getPropertyValue` reads.  Most call sites are unaffected because `AutoParse<std::any>` extraction is drop-in for `AutoParse<std::string>` extraction (same `T x = ap` spelling).
  Open question: what to declare as the type for `"menu"` / `"flag"` properties (strings vs bools)?  Probably a map from property-type-string to `std::type_info` at `addProperty` time.

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

## Filter module

- [ ] **Backend split for legacy Ops** — `X.cpp / X_Cpp.cpp / X_Ipp.cpp / X_SSE.cpp / X_OpenCL.cpp` layout, pair with `filter/detail/` placement.  See `project_filter_dispatch_arch.md` and `project_module_subdirs.md`.
- [ ] **IPP cross-check** — verify non-IPP filter impls against IPP in a Linux container.  See `project_ipp_crosscheck.md`.
- [ ] **`BackendProxy::backends(Backend b)`** proxy for terser multi-registration.  See `project_backends_proxy.md`.

---

## IO / compression / transport

From Session 48 deferrals:

- [ ] **Capability-flag codec classification** (lossy vs lossless; supported depths) → enables `auto` codec mode via the PluginRegistry's applicability machinery.
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

- [ ] **Replace XML with YAML for `ConfigFile`.**  Current implementation (`icl/utils/ConfigFile.{h,cpp}` + `icl/utils/detail/pugi/`) serializes property trees as XML via a bundled pugixml.  YAML reads better for humans, nests naturally for ICL's dotted-key layout, and would let us drop pugi from the dependency surface entirely.  Pick a YAML lib (yaml-cpp is the obvious candidate; consider rapidyaml for zero-alloc reads), keep the ConfigFile public API stable (load/save/register_type still string-backed), migrate on-disk format.  Open questions: migration path for existing `.xml` configs (auto-detect + one-way convert? separate tool?), behavior for mixed-format trees during transition.  See `project_yaml_config.md` once drafted.

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

- [ ] **`core/Image.h` should include `core/Img.h`.**  `Image::as<T>()` (inline template) does `static_cast<Img<T>*>(ImgBase*)`; Clang 21 rejects this unless `Img<T>`'s derivation from `ImgBase` is visible at the cast site.  Today `Image.h` only forward-declares `Img<T>` / `ImgBase` — it compiles only because every current consumer happens to include `Img.h` through some other path.  The TU compiling `test-prop-constraints.cpp` surfaced this when it reached `core/Image.h` through `core/prop/Constraints.h` without pulling in `Img.h`.  Fix: add `#include <icl/core/Img.h>` at the bottom of `Image.h`.  Incrementally cheaper than asking every new consumer to remember to add the include manually.

---

## C++17 source modernization (Sessions 27–28 residue)

- [ ] Any deferred source-code fixes from the C++17 migration.  See `project_cpp17.md`.
- [ ] **Drop explicit template args on `std::lock_guard` / `std::scoped_lock` / `std::unique_lock`.**  CTAD has been available for these since C++17, and the toolchain bump to Apple Clang 21 (Session 53) removed any remaining compiler gap.  ~281 sites across ICL currently write `std::lock_guard<std::mutex> g(m)` or `std::lock_guard<std::recursive_mutex> g(m)` — CTAD lets these drop to `std::lock_guard g(m)`.  Consider flipping `lock_guard` → `scoped_lock` at the same time (scoped_lock is the C++17+ default, handles multi-mutex, zero cost for the single-mutex case).  Mechanical with `perl -pi -e`; verify with a single clean rebuild.

---

## How to maintain this file

- Each item is a one-line actionable "what".  Link to memory for "why" and context.
- Check items off when they land, with the commit hash in the message.
- Move fully-completed sections into the relevant Session summary in `CONTINUE.md`, then delete from here.
- New items go in the appropriate section; if no section fits, start a new one.
- Keep this file focused: if a theme becomes inactive for 6+ months, archive to memory.
