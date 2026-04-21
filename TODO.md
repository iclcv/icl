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

## DataStore / Assign migration (Session 51 landed the flip; storage cleanup pending)

See `project_assign_migration.md` for full status.  Session 51
flipped `DataStore::Data::assign()` to `AssignRegistry::dispatch()`
and deleted the in-DataStore `AssignSpecial<>` machinery (~830
lines).  Identity enrollments + `Event → H` enrollments are in
place, so `HandleType h = gui["name"]` and `gui["x"].render()` both
work against the new registry.  What remains is the storage-side
cleanup — the step that finally retires the void*+RTTI-name
dispatch that only exists to bridge DataStore's legacy storage:

- [ ] **Replace `MultiTypeMap` base of `DataStore` with composed
  `AnyMap`.**  `DataStore::Data` internally holds a `std::any *`
  instead of a `DataArray *`; `assign` / `operator=(T)` / `as<T>()`
  pack and unpack `std::any` and call
  `AssignRegistry::dispatch(std::any&, std::any&)`.  `AnyMap` is
  already `std::map`-backed so stored references stay stable for
  the GUI-code pattern `m_poHandle = &allocValue<H>(...)`.

- [ ] **Delete the void*/name-based dispatch from `AssignRegistry`**
  once DataStore no longer calls it: drop the
  `dispatch(void*, type_index, void*, type_index)` and
  `dispatch(void*, string, void*, string)` overloads, the
  `m_nameToType` shadow table, and the `Fn::ptr` branch of the per-pair
  function pair.  Only the `dispatch(std::any&, std::any&)` path
  survives.

- [ ] **Delete `MultiTypeMap`** entirely (272 lines) — dead after
  DataStore stops inheriting it.  One `friend class` declaration in
  `GUIHandleBase.h` goes with it.

- [ ] **Retire the `Event` smuggling** in `DataStore::Data` —
  `render()`, `install()`, `link()`, `registerCallback()`,
  `enable()`, `disable()` currently work by building an `Event`
  struct and sending it through assign.  Replace with direct
  type-dispatch on the stored handle (now trivially doable once
  `*entry` is a `std::any`).  `Event` struct + the
  `HandleEventEnrollments.cpp` trait specializations go away with
  it.

- [ ] **`Slot` proxy** — the `gui["key"]` return type.  Implement
  templated `operator=(T)` and `operator T() const` that box/unbox
  via `std::any` and call `AssignRegistry::dispatch`.  Makes
  `int v = gui["key"]` work without users writing `.as<int>()`.
  Independent of the storage flip but natural to bundle.

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

## Dependency hygiene

- [ ] **Qt6 multimedia grabbers** — QVideoSink rewrite needed.  See `project_qt6_multimedia.md`.
- [ ] **LibAVVideoWriter** — rewrite for FFmpeg 6+/7+ API.  See `project_ffmpeg.md`.
- [ ] **ImageMagick 7 PixelPacket API** rewrite needed for ICLIO plugins.  See `project_imagemagick7.md`.
- [ ] **Apple Accelerate** as IPP replacement on macOS — partial; audit remaining IPP sites.  See `project_accelerate.md`.
- [ ] **OpenCL on macOS** — C++ bindings work deferred.  See `project_opencl_mac.md`.
- [ ] **NEON** as SSE replacement on ARM — investigate sse2neon coverage.  See `project_neon.md`.
- [ ] **Eigen** — brew's 5.0.1 has an Apple Clang bug; pin to 3.4.x.  See `project_eigen5.md`.

---

## C++17 source modernization (Sessions 27–28 residue)

- [ ] Any deferred source-code fixes from the C++17 migration.  See `project_cpp17.md`.

---

## How to maintain this file

- Each item is a one-line actionable "what".  Link to memory for "why" and context.
- Check items off when they land, with the commit hash in the message.
- Move fully-completed sections into the relevant Session summary in `CONTINUE.md`, then delete from here.
- New items go in the appropriate section; if no section fits, start a new one.
- Keep this file focused: if a theme becomes inactive for 6+ months, archive to memory.
