# ImageSource / ImageSink — io/ user-facing API rework (Plan B)

Status: **approved, not started.** Naming locked: `icl::io::ImageSource`
and `icl::io::ImageSink`.

## Motivation

The io/ acquisition + output API conflates three roles under one word
("Grabber") and is asymmetric between the read and write sides:

- `Grabber` is the abstract backend **contract**, *and* `GenericGrabber`
  is the user-facing **master**, *and* `FileGrabber`/`DCGrabber`/… are
  concrete **backends** — three different things, one suffix.
- "Grabber" is camera jargon; `ImageSource` / `ImageSink` are standard,
  symmetric, self-describing.
- The sink side is structurally weaker: there is **no** output base
  class. `GenericImageOutput` holds a `std::function<void(const Image&)>`,
  so it **cannot reach backend properties** (its own header says "access
  the backend directly"). `ImageSink` as a real Configurable object fixes
  this.
- Most backends are *already* private (`detail/`); only `FileGrabber` and
  `FileWriter` leak as public types. Finishing the job means apps only
  ever see two types: `ImageSource` and `ImageSink`, selected by spec
  string, with backend dispatch under the hood.

## Locked naming scheme

| Role | Today | New |
|---|---|---|
| User-facing source | `GenericGrabber` | `ImageSource` |
| User-facing sink | `GenericImageOutput` | `ImageSink` |
| Backend contract (→ `detail/`) | `Grabber` | `SourceBackend` |
| Backend contract (→ `detail/`) | *(none — callable)* | `SinkBackend` |
| Concrete source backends (`detail/`) | `FileGrabber`, `DCGrabber`, `WSGrabber`, `CreateGrabber`, … | `FileSource`, `DCSource`, `WSSource`, `CreateSource`, … |
| Concrete sink backends (`detail/`) | `FileWriter`, `WSImageOutput`, `LibAVVideoWriter`, null | `FileSink`, `WSSink`, `LibAVSink`, `NullSink` |
| Source registry / macro | `GrabberRegistry` / `REGISTER_GRABBER` | `sourceBackendRegistry()` / `REGISTER_SOURCE_BACKEND` |
| Sink registry / macro | `imageOutputRegistry()` / `REGISTER_IMAGE_OUTPUT` | `sinkBackendRegistry()` / `REGISTER_SINK_BACKEND` |
| Device descriptor | `GrabberDeviceDescription` | `DeviceDescription` |
| Backend escape hatch | `GenericGrabber::getGrabber()` | `ImageSource::backend()` / `ImageSink::backend()` → `utils::Configurable*` |

Verbs stay as-is for now: `ImageSource::grab()`, `ImageSink::send()`.
(Possible later harmonization to `read()`/`write()` — out of scope here.)

No long-term back-compat aliases: each stage does an atomic tree-wide
rename (perl), consistent with the project's delete-legacy style. A
short-lived `using GenericGrabber = ImageSource;` may bridge a single
stage if needed to keep the build green mid-rename, then removed.

## Current-state inventory (measured)

- **Source backends (22)**, all already in `detail/` except FileGrabber:
  `file` (FileGrabber, public), `create`, `demo`, `ps`, `optris`/`optrisv`,
  `xi`, `sr`, `onid`/`onic`/`onii`, `cvcam`, `cvvideo`, `pylon`, `ws`,
  `kinect2d/c/i`, `kinectd/c/i`, `dc`/`dc800`, `v4l`, plus `qtvideo`/`qtcam`
  (registered in `icl/qt/`).
- **Sink backends (4)**: `null_sink` + `file_sink` (in GenericImageOutput.cpp),
  `ws` (WSImageOutput), `video_libav` (LibAVVideoWriter).
- **Churn (files / refs, excl. 3rdparty):**
  `GenericGrabber` 89/168 · `FileWriter` 37/158 · `FileGrabber` 32/144 ·
  `GenericImageOutput` 22/57 · `GrabberRegistry` 17/54 ·
  `REGISTER_GRABBER` 20/36 · `REGISTER_IMAGE_OUTPUT` 4/7.
- `Grabber` base: subclasses `utils::Configurable`; surface =
  `acquireImage()` (pure virtual), `grab()`, the desired-params API
  (`useDesired`/`getDesired*`/`desired*Used`), `registerCallback`,
  `adaptGrabResult` (private), `getDeviceList`.
- `GenericGrabber` **composes** a `Grabber*` (does not inherit it),
  forwards desired-params + `grab()`, and leaks the backend via
  `getGrabber()`.

## Key decision — fate of public backend typed APIs

`FileGrabber` extras: `getFileCount()`, `getNextFileName()`, `next()`,
`prev()`, `bufferImages()`. `FileWriter`: the `FilenameGenerator`.

Resolution (so no public backend types survive):

- `getFileCount` → info property `file count` (read-only).
- frame navigation (`next`/`prev`, current index) → the existing
  `frame-index` property; drop `next`/`prev` or keep as thin sugar on
  `ImageSource` only if a real consumer needs them.
- `bufferImages` → a property / spec option (`-i file '...'@buffer=1`),
  already a ctor flag.
- `FilenameGenerator` → internal to `FileSink`; pattern is the spec string.
- Anything genuinely typed-only and irreplaceable → reachable through
  `ImageSource::backend()` / `ImageSink::backend()` returning
  `Configurable*` (discouraged; properties are the supported path).

## Staged execution (sink-first — weakest part, lowest churn)

### Stage 1 — Sink-side symmetrization (introduce `SinkBackend` + `ImageSink`) — DONE (`597458046`)
- [x] Add abstract `SinkBackend : utils::Configurable` in `io/detail/`
      with `virtual void send(const core::Image&) = 0`.
- [x] Backends derive `SinkBackend`: `WSImageOutput`, `LibAVVideoWriter`,
      plus new in-file `NullSink` + `FileSink` (FileSink wraps `FileWriter`
      so its jpeg/png/csv tunables surface as sink properties).
      **Concrete-class renames (`WSImageOutput`→`WSSink` etc.) deferred to
      the rename stage** — keeps Stage 1 focused on the structural change.
- [x] `sinkBackendRegistry()` returns `shared_ptr<SinkBackend>` (object),
      not a callable; `REGISTER_SINK_BACKEND` macro.
- [x] `GenericImageOutput` → `ImageSink`: holds a `shared_ptr<SinkBackend>`,
      `send()` delegates, **forwards the backend as a child Configurable**
      (fixes the property-access gap), adds `backend()`.
- [x] Migrated ~20 files; 877/877 green; runtime-verified file + ws
      property forwarding; regression test added.

### Stage 2 — Rename grabber → source (mechanical, atomic)
- [ ] `Grabber` → `SourceBackend`, move `Grabber.{h,cpp}` to
      `io/detail/` (it becomes the internal contract).
- [ ] `GenericGrabber` → `ImageSource` (`io/grabber/` → keep, or rename
      dir to `io/source/`? decide; lean: rename dir `io/grabber/`→`io/source/`).
- [ ] `GrabberRegistry`/`REGISTER_GRABBER` → `sourceBackendRegistry()`/
      `REGISTER_SOURCE_BACKEND`; `GrabberDeviceDescription` →
      `DeviceDescription`.
- [ ] Concrete backends drop suffix: `*Grabber` → `*Source` (22 backends,
      incl. the 4 orphans + the qt-registered `qtcam`/`qtvideo`).
- [ ] perl tree-wide rename across 89 files / 168 refs; tests green.

### Stage 3 — Hide remaining public backends
- [ ] `FileGrabber` → `FileSource` into `io/detail/`; apply the
      "Key decision" mapping (properties + spec options); remove the
      public header.
- [ ] `FileWriter` → `FileSink` into `io/detail/` (already covered if
      Stage 1 converted its output role; here retire the public header).
- [ ] Replace direct constructions in demos/tests with
      `ImageSource("file ...")` / `ImageSink("file ...")`.
- [ ] Add `ImageSource::backend()` / `ImageSink::backend()` escape hatch;
      retire `getGrabber()`.

### Stage 4 — Docs, umbrella, memory
- [ ] `IO.h` umbrella + install_headers groups updated; remove now-private
      headers from the public set.
- [ ] CLAUDE.md "Grabber Framework" / "Application Pattern" sections
      rewritten to ImageSource/ImageSink.
- [ ] Update memories referencing Grabber/GenericGrabber
      (`reference_websocket.md`, others) + add a convention memory.
- [ ] next.md session entry.

## Risks / notes

- **API-breaking**, ~150+ files across stages. Mitigate by keeping each
  stage independently green (build + `icl-tests -j 1`, 876+ baseline).
- The **4 orphan source backends** (Optris/PixelSense/SwissRanger/Xi) are
  not in the build — they get renamed in Stage 2 but stay
  compile-unverified (same caveat as the Session 64 migration).
- `qtcam`/`qtvideo` live in `icl/qt/` and are currently disabled
  (QtMultimedia missing-dep); rename them but expect no compile coverage.
- Spec-string syntax (`-i file ...`, `-o ws ...`) is the stable public
  contract and does **not** change — only the C++ type names do.
- Keep `grab()`/`send()` verb names this pass to bound churn.

## Out of scope

- `read()`/`write()` verb harmonization.
- libav FFmpeg 6/7 rewrite (`project_ffmpeg.md`), Qt multimedia revival.
- Any behavioural change to acquisition/output — this is a naming +
  surface-shape refactor only.
