# ICL — Continuation Guide

## Next Step

Session 55 closed the dynamic child-configurable arc and swept the
residual polish items from Session 54's "Next Step" list.  Highlights:

- `Configurable::onChildSetChanged` observer API + qt::Prop rebuild
  (landed).  `ConfigurableGUIWidget::buildFromConf()` extracted from
  the ctor; rebuild coalesces via `QMetaObject::invokeMethod(Qt::QueuedConnection)`.
  Two real-world consumer demos land alongside: `icl-compressor-playground-demo`
  (ImageCompressor codec swap) and `icl-grabber-backend-swap-demo`
  (GenericGrabber::init re-entry).
- `Configurable::Handle::as<T>()` facade-read fix — reads now forward
  via `getPropertyValue`, matching the write-path protocol.  Latent
  bug; every `Prop(...)` on a Configurable with children was hitting it.
- `ImageCompressor` compress/uncompress/installPlugin now serialized
  by a recursive_mutex.  Pre-existing race surfaced by the new demo
  (GUI-thread `mode` flip vs. worker `compress`).
- `configurable-gui-demo` thread finally stops cleanly on exit
  (`~B() { stop(); }` + `while(running())`).  Pre-existing exit crash.
- Float-setter / int-property audit (5 Ops), `core/Image.h` pulls
  `Img.h` transitively, CTAD sweep for `lock_guard` / `scoped_lock` /
  `unique_lock` (~278 sites, 71 files).  All mechanical.

Concrete work items remaining (in suggested order):

- **`removeChildSetCallback` API.**  Subscribers can't unregister
  today — if a ConfigurableGUIWidget dies while its Configurable
  survives, the captured `this` is dangling and the next
  add/removeChildConfigurable segfaults.  Same shape as the
  long-standing `removedCallback` gap on the property-change
  channel.  Either ID-based registration on both, or a token-
  returning pattern.

- **Compression-codec capability flags.**  Surfaced by the
  compressor demo: `1611` only handles single-channel icl16s and
  throws on RGB input.  Today the demo try/catches and reports
  in the ratio label; proper fix is to put a capability matrix
  (format / depth / channels) on `CompressionPlugin` and gray out
  incompatible choices in the UI.  Dovetails with the Session 48
  "auto codec" deferral.

- **ICLWidget OSD scale-range button misbehaves** (2026-04-21
  report).  User-visible; needs a proper repro session.

- **Designated-init GUI component syntax** — `gui << Slider{.min=0,
  .max=255}`.  Bigger arc; pairs with retiring `GUIComponent::toString()`'s
  string round-trip.

- **Fun: icl-edit image editor demo.**  Needs new filter Ops
  (`BrillianceOp`, `VibranceOp`, `ClarityOp`, `CurvesOp`, interactive
  crop/rotate).  Good exercise of the push-callback refresh +
  filter chaining + ConfigFile round-trip.

---

## Current State (Session 55 — dynamic child-configurable + polish)

### Session 55 Summary

15 commits.  Two logical arcs: (1) clear the Session 54 "Next Step"
polish list (float-setters, Image.h include, CTAD sweep); (2) land
the dynamic child-configurable refresh and two real-world consumer
demos.

#### Polish items (mechanical)

- `177624b10` **core: `Image.h` pulls in `Img.h` transitively.**
  `Image::as<T>()`'s inline `static_cast<Img<T>*>` needs the
  derivation visible under Clang 21; every consumer now gets it
  automatically.
- `268e36225` **filter+geom: align Op property constraint value_types
  with setter args.**  Five Ops (ThresholdOp, CannyOp,
  UnaryArithmeticalOp, UnaryCompareOp, RansacBasedPoseEstimator).
  Range<int> widened to Range<float> where the setter was float/icl32f;
  icl64f setters cast explicitly to float at the write site (only
  Range<int>/Range<float> adapters are registered); Ransac's
  integer-count setter was narrowed from float to int instead.
- `11b5eeb7c` **tree-wide: CTAD sweep for lock_guard / scoped_lock /
  unique_lock.**  ~278 sites, 71 files.  `lock_guard<std::mutex>` →
  `scoped_lock` (and similarly for `recursive_mutex`); `scoped_lock`
  is the C++17+ default and handles multi-mutex for free.
- `289db2d2e` **utils: drop stale dead code in AssignRegistry.h /
  ConfigFile.h.**  #if-0 blocks + stale transitional comments.

#### Dynamic child-configurable refresh

- `8532471f7` **Handle::as<T>() forwards through getPropertyValue.**
  Latent facade-read bug: facades carry a constraint but no
  `typed_value` (owning storage lives on the child).  Writes already
  forwarded; reads now match.
- `be71b2b54` **Configurable::onChildSetChanged** — observer API.
  Fires from the end of `addChildConfigurable` / `removeChildConfigurable`.
- `51b2ec0a6` **qt: ConfigurableGUIWidget rebuilds on child-set
  change.**  Constructor body extracted into `buildFromConf()`;
  `clearWidgets()` tears the Qt child tree down; `rebuild()` =
  clear + rebuild; `enqueueRebuild()` coalesces via
  `QMetaObject::invokeMethod(Qt::QueuedConnection)` + a
  rebuildScheduled flag (same pattern as the property-change push
  channel from Session 54).
- `2491c3aa4` **qt: dynamic-child-props demo.**  Host with
  add/remove Command buttons — simplest visual proof that the
  mechanism works.
- `23b637b2d` **configurable-gui demo stops thread cleanly on exit.**
  Pre-existing `while(true)` + no `stop()` led to "recursive_mutex
  lock failed" or "Property not supported" on exit depending on
  destruction-order roulette.  Two-line fix.

#### Real-world consumers

- `87b70f544` **io: ImageCompressor serializes
  compress/uncompress/installPlugin.**  Latent race: GUI-thread
  `mode` flip destroys the old plugin while a worker thread is
  partway through `compress()`; worker reads the new plugin's
  `name()` after encoding with the old plugin → envelope lies
  about the payload → `uncompress` aborts with codec-specific
  errors ("Not a JPEG file: starts with 0xff 0xd9").  recursive_mutex
  in the pimpl; pair with the onChildSetChanged demo work since
  the demo is what surfaced it.
- `614d4fbdd` **qt: compressor-playground demo.**  Grabber → compress
  → uncompress side-by-side.  Flipping `mode` swaps the codec child,
  the widget rebuilds, codec-specific knobs (`quality`, `level`)
  appear/disappear.  Catches incompatible combos (e.g. `1611` on
  RGB) and surfaces them in the ratio label.
- `403f2a897` **qt: grabber-backend-swap demo.**  Four buttons drive
  `GenericGrabber::init()` with different (type, id) pairs;
  Prop on the grabber rebuilds as the backend child swaps.
  Documents the init() params shape gotcha (`"type=id"`, not
  bare id) in the `swapBackend` helper.

#### Docs + follow-ups

- `8fedcd7f5` / `deb8035df` — TODO entries for the polish checkoffs
  + the three follow-ups (`removeChildSetCallback`, ImageCompressor
  demo, GenericGrabber demo — the latter two now checked off in
  this session).

#### Verification

Full test suite: 659/659 passing at every commit under
`meson compile -j16` (CCACHE_DISABLE=1 in this sandbox).

---

## Current State (Session 53 — Configurable typed-storage migration + proxy)

### Session 53 Summary

23 commits across a single multi-step arc.  Closed out the typed-
std::any migration for Configurable property storage, queued at
end of Session 52.  End state: every ICL property has a structured
`prop::*` constraint + typed value; `Property::value` (std::string)
retired; `prop("x").value = v` preserved as an ergonomic syntax via
a write-through proxy that routes through a new typed setter.

#### 1. `prop::` constraint framework (commits 12ea853e0 .. 4d27d8453)

New `icl/utils/prop/Constraints.h` introduces first-class C++ types
that replace the legacy stringly-typed `addProperty(name, type,
info, value)` grammar:

    prop::Range<T>{.min, .max, .step, .ui}   // Slider / Spinbox
    prop::Menu<T>{choices...}                // categorical pick-one
    prop::Flag{}                             // bool
    prop::Command{}                          // button, no value
    prop::Info{}                             // read-only string
    prop::Text{.maxLength}                   // free-form string
    prop::menuFromCsv(csv)                   // runtime-parsed Menu<string>

Plus `core::prop::Color` (RGB triplet) and `core::prop::ImageView`
(volatile image readback) in `icl/core/prop/` for constraints that
need core-module types.

Each constraint has a `ConstraintAdapter` (toString / fromString /
typeId / infoString function table) enrolled at static-init time in
a `PluginRegistry<std::type_index, ConstraintAdapter>`.  Adapters
synthesize the legacy type + info + value strings for backward
compat — ConfigFile save and qt::Prop's string-matching dispatcher
keep working through the migration.  Apple Clang 21 (Xcode CLT 26.4)
aggregate CTAD + designated-init supports
`prop::Range{.min=0.f, .max=500.f}` without spelling the template
argument.

#### 2. Typed addProperty<C> + migration (commits 69767fa82 .. 434a0f0a3)

Configurable gains a templated `addProperty<C>` overload:

    addProperty("gain",    prop::Range{.min=0.f, .max=500.f},  250.f);
    addProperty("mode",    prop::Menu{"fast","slow","auto"},   "fast");
    addProperty("enabled", prop::Flag{},                       true);
    addProperty("save",    prop::Command{});
    addProperty("bg",      core::prop::Color{},                core::Color(0,0,0));

`Property::constraint` (std::any) + `Property::typed_value` (std::any
of declared C++ type) added alongside existing `value` (std::string)
for transitional coexistence.

Step-6 bulk migration: ~500 `addProperty` call sites across
filter/io/cv/qt/geom/geom2/markers/physics migrated from the legacy
string form to typed overloads via `scripts/migrate-addProperty.py`
— 80-90% automated, manual cleanup for concat'd info strings,
menu-with-named-constant, and a few dynamic-registration sites.

#### 3. Storage + API flip (commit 8c40415c7)

`Configurable::getPropertyValue` return type flipped from
`AutoParse<std::string>` to `AutoParse<std::any>`.  Callers writing
`T x = c.getPropertyValue(name)` continue working via the cascade;
string-specific sites use `.str()` / `.as<std::string>()`.

#### 4. `payload` folded into typed_value (commits f3bb77ae6 .. 0f8c54c05)

`Property::payload` (std::any, used only for "image") retires.
`typed_value` absorbs its role.  qt::Prop's `VolatileImageUpdater`
reads through `getPropertyValue` (typed fast path) before falling
back to `getPropertyPayload` for unmigrated callers.  GaborOp's
"kernel preview" migrates to `core::prop::ImageView`; Scene/Scene2
background color migrate to `core::prop::Color`.

#### 5. Regression fix (commit 83d7155de)

Direct `prop("x").value = v` writes bypassed `setPropertyValue` →
stale `typed_value` → broken sliders (LocalThresholdOp et al).
Migrated 13 such sites to `setPropertyValue`.  Also caught an
infinite recursion in `UnaryOp::setClipToROI`: its override of
`setPropertyValue` re-dispatches to `setClipToROI`; changing
`setClipToROI` to call `setPropertyValue` looped.  Fixed by
explicit `Configurable::setPropertyValue(...)` (non-virtual
dispatch).

#### 6. Step 9 — retire Property::value, add proxy (commits f789b934e .. 533ec1cd7)

Six commits:

  1. **Unify legacy addProperty dispatch** — the string-taking
     `addProperty(name, "type", "info", value)` overload internally
     builds a typed constraint via `buildConstraintFromLegacy` and
     populates `constraint` + `typed_value` just like the typed
     overload.  After this, every property has both fields.
  2. **Property::as<T>()** — typed read helper off typed_value
     through `AutoParse<any>`'s cascade.
  3. **Bulk-migrate ~155 callback reads** — `parse<T>(prop.value)`
     → `prop.as<T>()`, `prop.value == "x"` → `prop.as<std::string>() == "x"`.
     Perl-scripted across filter/io/cv/geom/markers/physics.
  4. **Retire `Property::value` field** — `typed_value` becomes
     sole storage.  getPropertyValue always wraps typed_value;
     setPropertyValue parses into typed_value via constraint adapter.
  5. **`setPropertyValueTyped(name, std::any)`** — new public
     setter that writes typed_value directly.  No stringify.
  6. **`PropertyHandle` + `PropertyValueRef` proxy** —
     `Configurable::prop(name)` returns a short-lived handle with a
     `PropertyValueRef value` proxy (operator= → setPropertyValueTyped)
     + reference members for name/type/info/constraint/typed_value/
     volatileness/etc.  Ergonomic end state:

         prop("gain").value = 0.5f;        // typed, no string
         float g = prop("gain").value;     // typed read, no parse
         if (prop("enabled").value == "on") { ... }
         std::any_cast<const prop::Range<float>&>(prop("x").constraint);

  Internal storage accessor renamed to `prop_storage(name)` — used
  by Configurable.cpp setters/getters and the inline
  `addProperty<C>` template that pokes Property fields directly.

#### 7. Verification

- `tests/icl-tests -j 1`: 658/658 pass at every commit of this
  session (626 pre-session + 32 new `utils.prop.*` + `core.prop.*`
  cases).
- Full tree builds clean at every commit under `meson compile -j16`
  (CCACHE_DISABLE=1 in this sandbox — ~/.ccache isn't in the allow
  list; unrelated to the session).

#### 8. Toolchain bump

Apple Clang 15 → 21 (Xcode CLT 26.4).  Enabled C++20 aggregate CTAD
with designated initializers (P2082R1) — core ergonomic win for the
`prop::Range{.min=..., .max=...}` call-site style.  No code change
required; `sudo xcode-select -s /Library/Developer/CommandLineTools`
picked up the new toolchain.

#### 9. Documentation artifacts

- `scripts/migrate-addProperty.py` — reusable tool for migrating
  legacy addProperty call sites in future Configurable subclasses.
  Handles flag / info / command / string / menu / range{,:slider,
  :spinbox} / float / int / value-list; `:`-as-separator oddity;
  `str()` wrapper stripping for numeric value args.
- TODO entries added: step-9 follow-ups (proxy-or-retire for
  Property::value — closed by this session), designated-init GUI
  component syntax, lock_guard/scoped_lock CTAD cleanup,
  `core/Image.h` include-Img.h fix.

#### 10. What's deferred

- **Step 5** — qt::Prop variant-visit dispatch on constraint type.
  See "Next Step" above.
- **Step 7** — retire legacy string-taking `addProperty` overload.
  Blocked on dynamic-registration sites (PylonCameraOptions etc.)
  that supply type/info as runtime strings; need a typed
  equivalent or keep the overload as the dynamic-registration
  entry point.
- **Step 8** — synthesize `getPropertyType`/`getPropertyInfo` from
  constraint on demand; drop `Property::type` and `Property::info`
  fields.  Pattern matches step-9's `Property::value` retirement.
- **Step 10** — this file.  Done.

---

## Previous State (Session 52 — `utils::Any` retired; AutoParse<Backend> landed; plugin APIs split into overloads)

### Session 52 Summary

7 commits along a single coherent arc.  Closed out the long-
standing `utils::Any` vs `std::any` confusion by introducing a
properly-named, properly-scoped conversion proxy
(`utils::AutoParse<Backend>`), migrating every consumer onto it,
and deleting `utils::Any.h` outright.  Net: ICL's own source no
longer contains a general-purpose container called "Any" — the
name collision with `std::any` is gone, and every former role of
`utils::Any` is served by an appropriately-specific tool.

#### 1. `utils::AutoParse<Backend>` — new conversion proxy (commit 56ad2eebc)

New `icl/utils/AutoParse.h`, a lightweight by-value conversion
proxy meant for index-style accessors like `ProgArg::operator[]`
and `DataStore::operator[]`.  Two backends:

- **`AutoParse<std::string>`** — publicly inherits `std::string`
  (same trick `utils::Any` used; safe here because AutoParse is
  disciplined-proxy-only, never stored).  Normal string operations
  (concat with `+`, comparison, stream-insertion, passing to
  `const std::string&` APIs) work without per-operator overloads.
  Templated `operator T()` parses into any stream-extractable T
  via `parse<T>`, SFINAE-excluding char-like types reachable
  through the base to avoid ambiguity.  Templated constructor
  accepts any streamable T (via `str(t)`) so
  `setPropertyValue("gain", 500)` keeps working.

- **`AutoParse<std::any>`** — cascade on extraction: exact
  `std::any_cast<T>` → numeric widening for arithmetic T → parse
  the stored value if it is a `std::string` → stringify the stored
  value if T is `std::string` and the stored value is a numeric.
  Used internally by `DataStore::Slot::operator T()` as a fast-path
  before falling through to the existing AssignRegistry cross-type
  dispatch — adds numeric-widening paths that previously required
  a string round-trip.

Discipline rule documented in the header: *AutoParse is a
conversion proxy, not a storage type.*  Never stored, never a
function-parameter type, never in a container — consume
immediately as `T x = ap` or `ap.as<T>()`.

Initial adopters: `ProgArg::operator[](int)` (return type flip
from `utils::Any` to `AutoParse<std::string>`) and
`DataStore::Slot::operator T()` (delegation to
`AutoParse<std::any>` on the fast path).

19 unit tests in `tests/test-auto-parse.cpp` covering both
backends.

#### 2. Configurable migration (commit 27a0cc5ea)

The biggest former consumer.  Flipped
`addProperty` / `setPropertyValue` / `getPropertyValue` / ditto on
`ConfigurableProxy` + all subclass overrides (`UnaryOp`,
`CornerDetectorCSS`, `MultiCamFiducialDetector::property_callback`)
from `utils::Any` to `AutoParse<std::string>`.  Internal property
storage untouched (still `std::string`), config-file
serialization untouched — this was an API-surface rename.

Semantics preserved exactly: both types inherit `std::string`,
both accept arbitrary T in the constructor via `str(t)`, both
parse on extraction via `parse<T>`.  Call sites unchanged.

#### 3. `ParamMap` moved out of `Any.h` (commit d23d60298)

`utils::ParamMap` — the `std::map<string, Any, std::less<>>` used
across plugin APIs — moved to its own header
`icl/utils/ParamMap.h` and retyped as
`std::map<string, AutoParse<std::string>, std::less<>>`.
`Any.h` kept the include transitively for continuity.

#### 4. Plugin-code `Any()` sentinel cleanup (commit 6a8077d4c)

7 residual `Any()` / `Any(value)` wrappers in
`FileGrabber.cpp`, `DCDeviceFeatures.cpp`, `V4L2Grabber.cpp`
replaced with plain `""` / the value itself — the wrapper was
only there to pacify the old `const Any&` param type.

#### 5. `FiducialDetector` overloads (commit c92621de7)

The cleanest-design part of the arc.  Deleted the single
`const Any &which` parameter throughout the FiducialDetector
family and replaced with three purpose-built overloads reflecting
what callers actually pass:

```
loadMarkers(int)                 → single numeric ID (BCH / ICL1)
loadMarkers(const vector<int> &) → typed list (no string parse)
loadMarkers(const string &)      → range "[a,b]", list "{a,b,c}",
                                    or file path/glob (ART, Amoeba)
```

`unloadMarkers`, `createMarker`, and the constructor's
`markersToLoad` parameter got the same treatment.  Int and
vector-int overloads delegate internally to the string form so
plugins still see a single string spec — plugin impls changed only
by virtue of signature (`const std::string &` instead of
`const Any &`), and the implicit Any→int conversions in three
plugin create-marker bodies became explicit `parse<int>(...)`.

18 files touched; `create-marker-grid-svg.cpp` was the only app
call site needing an update (`Any()` dropped from the ctor call).

#### 6. Delete `utils::Any.h` (commit 4e4c55f19)

Final retirement.  Remaining consumers:

- **`ImageRegion::{set,get}MetaData`** + `ImageRegionData::meta`
  and **`DefineRectanglesMouseHandler`'s meta bag**: genuine
  type-erased metadata stores, migrated to `std::any`.  Only the
  visualization path needed a tiny guard — `w.text(meta, ...)`
  became `if (auto *s = std::any_cast<std::string>(&meta)) w.text(*s, ...)`,
  which preserves the previous implicit "Any→string via is-a"
  behaviour.

- **`VisualizationDescription::Part::content`** (stringly-typed
  payload for draw primitives): migrated to
  `AutoParse<std::string>`.  Callers passed `std::vector<float>` /
  `<int>` for polygon/point lists; old Any had a binary packing
  specialization.  Replaced with text CSV via new
  `str(vector<T>)` + `parse<vector<T>>` specializations in
  `StringUtils.h` for float / int / double.  Cheaper to reason
  about than the binary packing; configs are cold-path anyway.

- **`Any::ptr<T>` pointer encoding** (the only non-string-round-trip
  use of Any): replaced by two small free functions
  `qt::encode_pointer<T>` / `decode_pointer<T>` in
  `qt/GUIComponents.h`.  Used only by the Prop GUI component to
  smuggle a `Configurable*` through the stringly-typed GUI
  parameter channel; will become moot when the GUIComponent
  stringification rework lands.

- **`ConfigFile.cpp`**: dropped `REGISTER_CONFIG_FILE_TYPE(Any)` —
  plain `std::string` already covers that role.

Headers that transitively pulled in `Any.h` rewired to include
`ParamMap.h` (FiducialDetector, FiducialDetectorPlugin,
Marker*Detector).  `Configurable.h` drops the Any include
outright.

Net delete: `icl/utils/Any.h` (211 LOC) + `examples/any.cpp`
(43 LOC benchmark of the binary packing).

#### 7. YAML config TODO (commit da12ae6f0)

User flagged "I don't like XML anymore".  Added a TODO entry
under a new `Configuration format` section in `TODO.md` + a
`project_yaml_config.md` memory.  Migrate `ConfigFile` on-disk
format from pugixml to YAML (yaml-cpp likely, rapidyaml if cold
path turns out to matter); keep the public API stable; auto-detect
existing `.xml` files on load during transition.  Leaves the
pugi dependency deletable afterward.

#### 8. Verification

- `tests/icl-tests -j 1`: 626/626 pass at every commit (607 from
  Session 51 + 19 new `utils.autoparse.*` cases).
- Full tree builds clean at every commit (`meson compile -j16`).
- No runtime verification of the FiducialDetector overloads (Qt
  GUI ctor still crashes in this sandboxed-macOS environment per
  the Session 51 note); API-surface change only, confirmed via
  build + unit tests.

#### 9. What's deferred

- **AutoParse template flip**.  The decision to keep `AutoParse`
  as `template<class Backend>` with explicit specializations for
  `std::string` and `std::any` is held open for future extension.
  No current consumer needs a third backend.
- **Typed `std::any` internal storage for Configurable properties**.
  Discussed during scoping; deferred.  Would require changes to
  config-file format semantics (today all round-trips through
  strings).  Separate future modernization pass.
- **GUIComponent stringification rework** (TODO item still open):
  will eliminate the `@pointer@:` encoding, making
  `qt::encode_pointer` / `decode_pointer` deletable.

---

## Previous State (Session 51 — thread-safe handle reads, `.out()` retired, DataStore on AnyMap, Event smuggling gone)

### Session 51 Summary

20 commits along a single coherent arc.  Closed out everything on
the Session 50 "DataStore / Assign" punch list plus the larger
`.out()` retirement that the atomic-cache work unblocked.  Net:
the `void* + RTTI-name` dispatch that had been the main source of
hacky type erasure in the GUI path is gone, the DataStore is
std::any-backed end-to-end, and every `Data::*` verb
(`render` / `install` / `link` / `registerCallback` / `enable` /
`disable` / `removeCallbacks`) is a one-line macro expansion over
a direct `std::any_cast<H>` type-cascade.

#### 1. Thread-safe handle reads (6 commits)

Problem: every value-carrying handle's `getValue()` reached into
the wrapped Qt widget from the application thread (e.g.
`SliderHandle::getValue()` called `QSlider::value()`).  Qt doesn't
document widgets as thread-safe for reads outside the GUI thread.

Pattern landed in two forms:

- **Slider / FSliderHandle** — the pre-existing
  `ThreadedUpdatableSlider` widget subclass gained
  `std::atomic<int> m_atomicValue`, seeded in both ctors and
  published from `collectValueChanged` (the GUI-thread slot wired
  to `valueChanged(int)` in the widget ctor — always connected
  before any external `connect` call).  Handle `getValue()` reads
  via `atomicValue()`.

- **Remaining 9 handles** (Spinner / Int / Float / CheckBox /
  Button / ButtonGroup / Combo / String / Color) — Option H
  chosen over Option W (no new widget subclasses): each handle
  owns a `std::shared_ptr<std::atomic<T>>` (or a mutex-guarded
  struct for string / color / combo index+text).  The primary
  ctor installs a `QObject::connect(widget, signal, widget,
  lambda)` with the widget as the connection's context object;
  the lambda captures the `shared_ptr` by value so the cache
  outlives every handle copy, and the connection dies with the
  widget.

- **Signal-ordering bug** discovered during testing: GUI.cpp
  connects `widget→ioSlot` before `allocValue<Handle>()` installs
  the cache-update lambda, so user callbacks (dispatched from
  ioSlot) ran before the cache updated — visible in
  `icl-filter-playground` as a "one selection behind" Combo lag.
  Fixed by reordering four widget ctors (Combo, Spinner, CheckBox,
  ButtonGroup) so the handle is allocated first.

- **`ButtonHandle::m_triggered`** upgraded from `shared_ptr<bool>`
  to `shared_ptr<std::atomic<bool>>` with `exchange(false)` read-
  and-clear semantics.

#### 2. `.out()` retirement (4 commits, 38 files migrated)

After atomic caches, `.out("name")` — which allocated a raw `int`
/ `bool` / `float` / `std::string` in the DataStore and updated
it via a Qt signal slot — no longer served any purpose; reading
via `gui["handle-name"]` dispatched through the registry and hit
the thread-safe cache.

- New `scripts/migrate-out-to-handle.py` mechanically rewrote
  consumer sites per the rule table (dead handle → drop; both
  named → keep reader's name; single `.out()` → promote to
  `.handle()`).  287 edits across 38 files; 1 ambiguous case
  (`video-player.cpp` reading both names) fixed manually.  3
  sites with dynamic-string keys (`"enable-obj-"+str(c)`) fixed by
  hand — the regex was intentionally conservative.

- User-API surface deleted: the `.out()` method and
  `GUIComponentWithOutput` subclass.  Every
  `GUIComponentWithOutput` inheritor (Button, CheckBox, Slider,
  FSlider, Int, Float, Spinner, ColorSelect, ButtonGroup)
  collapsed onto `GUIComponent` directly; the `Options::out` field
  and `@out=X` serialization removed.

- Producer-side removal: 11 `allocValue<primitive>(def.output(0),
  ...)` branches in `GUI.cpp` gutted, each with its cache field
  (`m_piValue` / `m_pfValue` / `m_stateRef` / `m_psOutput` /
  `m_uiIdx` / `m_psCurrentText`) and the processIO slot that used
  to mirror widget state into that primitive.  ProcessIOs became
  empty or reduced to the remaining side effect (LCD display
  update for sliders).

- Handle API cleanup: `CheckBoxHandle::m_stateRef` and
  `StringHandle::m_str` dropped from handle ctors (no longer need
  the external pointer); `CheckBoxHandle::check/uncheck` no longer
  write to the stateRef, they just call setCheckState and let the
  signal update the atomic cache.

- `ColorSelectGUIWidget` fix: was leaking a `Color4D` (allocated
  at the top of the ctor, then m_color reassigned to point into
  DataStore).  Replaced with inline `Color4D m_color;` member.

- Fallout cleanup: `gui.get<primitive>("key")` sites (27 across
  11 apps/demos) rewritten as `T v = gui["key"]` — snapshot reads
  via the handle rather than live references to a DataStore
  primitive that no longer exists.  `pipe.cpp`'s
  `bool *ppEnabled` indirection replaced with
  `std::function<bool()>`; `Widget.cpp`'s bciUpdateAuto /
  channelUpdateAuto got the same treatment.

#### 3. DataStore/Assign finalization (7 commits)

Picking up from Session 50's infrastructure (AnyMap, Assign trait,
AssignRegistry with compile-time + std::any runtime dispatch):

- **Flip the dispatch.**  `DataStore::Data::assign()` routes to
  `AssignRegistry::dispatch()`.  The ~830-line in-DataStore
  `AssignSpecial<>` / `create_assign_map` / `INST_NUM_TYPES` /
  `ADD_T_TO_T` / `register_assignment_rule` machinery deleted in
  one pass.  Identity enrollments (`enroll_identity<H>()`) added
  to every migrated handle's constructor + a consolidated
  `HandleIdentityEnrollments.cpp` for the remaining ones (Border,
  Box, Disp, FPS, Plot, Splitter, State).  `Event → H` rules
  re-enrolled under AssignRegistry via a concept-driven
  `Assign<H, Event>` specialization (later retired — see #4).

- **AnyMap → std::map.**  Swap the internal container from
  `std::unordered_map` to `std::map` for pointer-stability, so
  GUI code like `m_poHandle = &allocValue<H>(...)` keeps working.

- **Compose AnyMap inside DataStore.**  DataStore stops
  inheriting `MultiTypeMap`; instead composes
  `shared_ptr<AnyMap>` + `shared_ptr<recursive_mutex>` (preserving
  the shared-backing semantics GUI copy-assign relies on).
  `Data::m_entry` changes from `DataArray*` to `std::any*`;
  `operator=(T)` / `as<T>()` pack and unpack via
  `std::in_place_type<T>` (required to disambiguate
  std::any's templated ctor for types like `utils::Any` that
  publicly inherit `std::string`).  `AssignRegistry`'s transitional
  `void*` / name-table paths all deleted — one dispatch form
  (`dispatch(std::any&, std::any&)`) remains.  `MultiTypeMap.{h,cpp}`
  deleted (~312 lines) along with its `friend class` in
  `GUIHandleBase.h`.  The vestigial `GUI::release<T>` wrapper
  goes with it.

- **GUI::registerCallback/removeCallbacks fix.**  These used to
  `reinterpret_cast` through `getValue<GUIHandleBase>(key, false)`
  — safe only under `MultiTypeMap`'s void* storage, UB under
  `std::any`.  Rewritten to route through the `Slot` proxy's own
  `registerCallback` / `removeCallbacks` methods.

- **Retire the `Event` smuggling.**  `Data::render() /
  install() / link() / registerCallback() / enable() / disable() /
  removeCallbacks()` used to build an `Event` struct and push it
  through AssignRegistry to a per-handle `Assign<H, Event>`
  specialization.  Now that the store is `std::any`-backed,
  dispatch goes directly: new `qt/HandleVerbDispatch.cpp` owns a
  single `visitHandle()` type-cascade over a
  `std::tuple<AllHandles...>`, with per-verb capability concepts
  (`HasRender`, `HasInstallMouse`, etc.) gating the call via
  `if constexpr`.  Each `Slot` method collapses to a one-line
  `ICL_SLOT_VERB(Cap, action)` macro expansion — verb name comes
  from `__func__` so the log label can't drift from the method
  name.  `Event` struct + `Assign<H, Event>` specialization +
  `HandleEventEnrollments.cpp` deleted.

- **Kill void* on the Data API.**  `Data::install(void*)` →
  `Data::install(MouseHandler*)`; `Data::link(void*)` →
  `Data::link(GLCallback*)`.  `GLCallback` pulled out of its
  nested `ICLDrawWidget3D::GLCallback` scope into a top-level
  `icl::qt::GLCallback` class (new 40-line `qt/GLCallback.h`), with
  a backward-compat `using GLCallback = icl::qt::GLCallback;`
  alias inside the widget class preserving all 89 existing
  spellings.  One knock-on: `Scene2::getGLCallback` returned
  `shared_ptr<Scene2::GLCallback>` where the inner type was
  forward-declared — now returns `shared_ptr<qt::GLCallback>`
  (base) with the impl class moved to an anonymous namespace as
  `SceneGLCallback`.  One file-local name clash fixed in
  `camera-calibration-planar.cpp` (`struct GLCallback` renamed to
  `MultiViewGLCallback`).

- **Rename `DataStore::Data` → `DataStore::Slot`.**  Historical
  "Data" name was an artifact of `MultiTypeMap`'s "data arrays";
  `Slot` is the modern description of a one-entry view over the
  keyed store.  Purely cosmetic — same template API as before,
  `int v = gui["k"]` already worked without `.as<int>()`.

#### 4. Verification & test status

- `tests/icl-tests -j 1` — 607/607 pass at every commit.
- Full build of all 240 targets clean at every commit.
- GUI runtime surfaced a few fallouts handled as they appeared
  (`Any = FSliderHandle` assign rule missing; signal-ordering
  lag; `any_cast<GUIHandleBase&>` exact-match failure).  All
  captured in the history.

#### 5. Bugs surfaced; not fixed (deferred to TODO.md)

- **AffineOp: translation component ignored** (Filter section).
  Rotate and scale work; translate-x / translate-y sliders in
  `icl-filter-playground` don't shift the output.
- **Scale-range OSD button in ICLWidget behaves strangely** —
  symptom uncharacterized.
- **Qt6 `QOpenGLWidget` ctor crashes in sandboxed/macOS-26
  terminals** — Qt / Cocoa issue, not ICL.  Blocks runtime GUI
  verification in this environment.

#### 6. What's deferred

TODO.md's "DataStore / Assign migration" section is effectively
closed.  Remaining items are the surfaced bugs above + speculative
ones (core-type identity enrollments — not needed unless a
non-handle ends up in DataStore).  The GUIComponent
stringification rework the user flagged is in a new
"Qt GUI component plumbing" section, unrelated to this arc.

---

## Previous State (Session 50 — AnyMap + Assign<Dst,Src> infrastructure; 15 qt handles migrated)

### Session 50 Summary

One extended design conversation that landed a fresh, orthogonal
replacement for the `MultiTypeMap` / `DataStore::Assign` machinery and
then started mechanical per-handle migration onto it.  Key properties
of the new design: compile-time and runtime dispatch paths that share
one source of truth; no implicit conversion operators on handles
(they're a known overload-resolution foot-gun); explicit `as<T>()`
extraction for readback; `<Dst, Src>` template-arg order matching
`dst = src` and `std::is_assignable`.  Legacy `DataStore` untouched
throughout — both dispatch tables coexist.

#### 1. Infrastructure — `icl/utils/` (commit 9fdd01a6b)

Three new public headers, all header-only or tiny:

- **`AnyMap.h`** — thin typed wrapper around
  `std::unordered_map<std::string, std::any>`.  `set<T>`, `get<T>`
  (throws on miss/mismatch), `tryGet<T>` (nullptr), `contains` /
  `containsAs<T>`, `typeOf`, `erase`, `clear`, iteration.  Replaces
  `MultiTypeMap`'s `void* + RTTI-string + static T _NULL + len-bit-hack`
  machinery with plain `std::any`.  Not wired into DataStore yet;
  orthogonal for now.

- **`Assign.h`** — `Assign<Dst, Src>` trait collapsed to one template
  with two `apply()` overloads on disjoint constraints:

  ```cpp
  template<typename Dst, typename Src>
  concept DirectlyAssignable = std::is_assignable_v<Dst&, Src>;

  template<typename Dst, typename Src>
  concept ExtractableAs = requires(Dst &d, Src &s) {
    d = s.template as<Dst>();
  };

  template<typename Dst, typename Src>
  struct Assign : std::bool_constant<
      DirectlyAssignable<Dst, Src> || ExtractableAs<Dst, Src>>
  {
    static void apply(Dst &dst, Src &src)
      requires DirectlyAssignable<Dst, Src> { dst = src; }
    static void apply(Dst &dst, Src &src)
      requires (!DirectlyAssignable<Dst, Src> && ExtractableAs<Dst, Src>)
      { dst = src.template as<Dst>(); }
  };
  ```

  No per-pair specializations.  Direct path wins when both are viable.
  Class-side convention: `operator=(Src)` for incoming, `as<T>()`
  member template (constrained with `requires`) for outgoing.

- **`AssignRegistry.{h,cpp}`** — runtime type-erased dispatcher.
  **Fully static** public API (no `.instance().` noise at call sites;
  the singleton is private-internal).  Map stores bare function
  pointers via `+[](std::any&, std::any&){…}`, no `std::function`
  overhead.  API:

  ```cpp
  AssignRegistry::enroll<Dst, Src>();               // static_asserts
  AssignRegistry::enroll_symmetric<A, Bs...>();     // A↔B pairs
  AssignRegistry::enroll_receiver<Dst, Srcs...>();  // Dst = each Src
  AssignRegistry::enroll_provider<Src, Dsts...>();  // each Dst = Src
  AssignRegistry::dispatch(any &dst, any &src);
  AssignRegistry::has(type_index, type_index);
  AssignRegistry::size();
  ```

  `enroll` static-asserts `is_assignable_v<Dst, Src>` so runtime
  registration and compile-time trait stay in sync by construction.

#### 2. Per-handle migrations — `icl/qt/`

**Idiom** each migrated handle's `.cpp` picks up:

```cpp
namespace {
  using icl::utils::AssignRegistry;
  using icl::qt::XxxHandle;
  __attribute__((constructor))
  static void icl_register_xxx_handle_assignments() {
    AssignRegistry::enroll_symmetric<XxxHandle, int, float, double, std::string>();
  }
}
```

— one-line enroll call per handle.  Symmetric/receiver/provider
variants handle the asymmetric cases.  **15 handles** landed across
6 commits:

| Commit | Handles | Shape |
|---|---|---|
| 30452ca07 | SliderHandle | symmetric value (pilot) |
| 063b4b8fa | IntHandle, FSliderHandle, SpinnerHandle, FloatHandle | symmetric value |
| 83c79a4b0 | *(infra: fully static API + enroll_symmetric)* | — |
| 7e0364dc6 | CheckBoxHandle, StringHandle, ButtonHandle, ButtonGroupHandle, TabHandle | mixed (+ `enroll_receiver`/`enroll_provider`) |
| cac200f0f | ComboHandle, LabelHandle | symmetric + receiver |
| 398c01f19 | ImageHandle, DrawHandle, DrawHandle3D, ColorHandle | receiver (images) + symmetric (Color/Color4D) |

Notable per-handle details:

- **ComboHandle**: dropped the legacy `operator int() const` and
  `operator std::string() const` implicit conversion operators — they
  were the exact kind of landmine we're avoiding.  One caller
  (`filter/apps/local-thresh.cpp`) had to be updated from
  `(int)comboHandle` to `comboHandle.getSelectedIndex()`.  Zero other
  callers in tree depended on the implicit operators.
- **StringHandle**: templated `operator=(T)` for arithmetic writes via
  `utils::str`; templated `as<T>()` for arithmetic reads via
  `utils::parse`.
- **DrawHandle3D**: was missing `operator=(const ImgBase&)` — legacy
  FROM_IMG used `setImage(&src)`.  Added it.
- **Image handles** (ImageHandle, DrawHandle, DrawHandle3D): each
  enrolls 18 image-type pairs (5 `Img<T>` by value, 12 pointer
  variants, `core::Image`) via a single `enroll_receiver<H, …>()`
  call.  All Img<T> paths fall through to
  `operator=(const ImgBase&)` via derived-to-base conversion.
- **Smuggled-command pairs retired**: `Event → handle` (for
  render/install/link) and `Range → SliderHandle` (setRange) do **not**
  map through the new trait.  Those idioms become direct method calls
  (`handle.render()`, `slider.setRange(a, b)`) in the new model.
- **Color↔Color4D cross-conversions** deliberately not ported into
  qt — they belong on the types themselves in core, not leaked into
  qt registration.

#### 3. Verification

- Clean builds after every commit (full target set + tests + demos).
- `tests/icl-tests -j 1` → **607/607 pass** throughout the migration.
- New test coverage: 16 `utils.assign.*` tests (compile-time trait,
  direct path, extract path, direct-wins priority, runtime dispatch
  for both paths, parity, enroll_symmetric variadic expansion,
  idempotent re-enrollment) + 24 `utils.anymap.*` tests.

#### 4. What's deferred

See `project_assign_migration.md` for the full punch list.  Highlights:

- **Identity enrollments** (`enroll<H, H>()`) for every migrated handle.
  Old DataStore had `ADD_T_TO_T` for each pair; new system doesn't yet.
  Needed before flipping DataStore — a consumer writing
  `gui["a"] = gui["b"]` with same-type handles needs this entry.
- **Core-type identity** (Rect, Size, Point, Image, std::string, Any)
  — same concern for non-handle types.  Needs a home outside qt.
- **Event-only handles** (PlotHandle, FPSHandle, BoxHandle, DispHandle,
  StateHandle, SplitterHandle, MultiDrawHandle) — had only `Event → H`
  + identity in old DataStore; not yet migrated.  Decide whether to
  skip entirely or add identity-only enrollment.
- **`Slot` proxy** — the `gui["key"]` return type.  Needs
  implicit-template `operator=(T)` and `operator T() const` that route
  through `AssignRegistry::dispatch`.  This is what makes
  `int v = gui["key"]` work without users writing `.as<int>()`.
- **`DataStore::Data::assign()` flip** — replace the legacy
  `AssignSpecial<>` map lookup with `AssignRegistry::dispatch()`;
  retire `DataStore::Assign`, `create_assign_map()`,
  `register_assignment_rule()`.
- **`MultiTypeMap` replacement** — swap DataStore's inherited
  `MultiTypeMap` for composed `AnyMap`.  Bigger API break.

---

## Previous State (Session 49 — math detail/ reorg + pugi privatization + DynamicGUI retirement)

### Session 49 Summary

Three landings, one theme: **strict `detail/` invariant**. Under the strict
rule (locked this session per user directive), anything under a module's
`detail/` subdirectory must never be reachable by an installed public
header. Previously `utils/detail/pugi/` was installed-anyway because
`utils/XML.h` pulled in pugi types — that was the anti-pattern. This
session fixes it, along with a related math reorg and a dead-code cull.

See `feedback_detail_strict_rule.md` for the rule itself.

#### 1. `icl/math/` detail/ reorg

Backend-dispatch facade files moved under per-family subdirs:

```
icl/math/detail/
  blas/     BlasOps.{h,cpp} + BlasOps_{Accelerate,Cpp,Mkl}.cpp
  fft/      FFTOps.{h,cpp}  + FFTOps_{Accelerate,Cpp,Mkl}.cpp
  lapack/   LapackOps.{h,cpp} + LapackOps_{Accelerate,Cpp,Eigen,Mkl}.cpp
  mathops/  MathOps.{h,cpp} + MathOps_Cpp.cpp
```

**19 files moved.** All 4 facade headers dropped from the `math_headers`
install list — they're genuinely private (zero external consumers; the
22 `#include`s of them are all from `.cpp`s inside `icl/math/` itself).
Facade types (`utils::BackendDispatching<>` subclasses named `BlasOps`,
`LapackOps`, `FFTOps`, `MathOps`) never appeared in any other public
header, so no API break. Top-level `icl/math/` shrinks 74 → 55 files.

Meson updates: per-subdir source paths + removal from `math_headers`.
Internal `#include`s rewritten: `<icl/math/BlasOps.h>` →
`<icl/math/detail/blas/BlasOps.h>`, etc. 22 include sites, all inside
`icl/math/`.

#### 2. Pugi privatization

The trigger: `utils/meson.build` had this anti-pattern block —

```
# Vendored pugixml lives under detail/pugi/ — it's only reached via
# icl/utils/XML.h (which aliases pugi:: types as icl::utils::XML*).
# The header must still be installed so XML.h is self-contained.
install_headers(files('detail/pugi/PugiXML.h', 'detail/pugi/pugiconfig.hpp'), …)
```

"detail/ but installed because a public header needs it" is exactly the
state the strict rule forbids. Fixed by making pugi actually private:

**`utils/XML.h` — deleted.** Contained 21 `using` aliases (`XMLDocument =
pugi::xml_document`, etc). Audit showed **zero** ICL public headers
consumed any of those aliases, and only 4 in-tree .cpps used them —
trivially switched to `pugi::` directly. Dropped from `utils_headers`,
removed from `Utils.h` umbrella include, deleted.

**`utils/ConfigFile.h` → pugi-free via PIMPL.** Four pugi references in
the public header, all now gone:

| Old | New |
|---|---|
| `ConfigFile(pugi::xml_document *handle)` ctor | **Deleted.** 2 external callers (ImageUndistortion, Camera) switched to existing `ConfigFile(std::istream&)` ctor. |
| `const pugi::xml_document *getHandle()` inline | **Deleted.** Zero callers. |
| `static void add_to_doc(pugi::xml_document&, …)` static private | Moved to anonymous namespace free function in `ConfigFile.cpp`. |
| `mutable std::shared_ptr<pugi::xml_document> m_doc` member | **PIMPL**: `struct Impl { pugi::xml_document doc; }; mutable std::shared_ptr<Impl> m_impl;` |
| `namespace pugi { class xml_document; }` forward decl | **Deleted.** |

The PIMPL member is `m_impl` (renamed from `m_doc` per user preference
— name reflects it's the opaque pimpl, not just the document).

**`utils/ConfigFile.cpp`** — swapped the 21 XML-alias references for
direct `pugi::xml_document` / `pugi::xml_node` / `pugi::xml_attribute` /
`pugi::xpath_query` / `pugi::xpath_node_set` / `pugi::xpath_node`.
`is_text_node`, `add_to_doc`, `get_id_path` moved into an anonymous
namespace. `#include <icl/utils/XML.h>` replaced by direct
`#include <icl/utils/detail/pugi/PugiXML.h>` (privately — the header is
no longer installed, but `.cpp`s can reach into `detail/` freely during
in-tree builds).

**`filter/ImageUndistortion.cpp`, `geom/Camera.cpp`** — dropped the
`new XMLDocument + doc->load(is)` pattern; now `ConfigFile(is)` directly.
Cleaner anyway; the pattern existed only because the `ConfigFile(pugi::…*)`
ctor used to take ownership of a raw pugi doc, which is no more.

**`io/detail/grabbers/OptrisGrabber.cpp`** — uses pugi directly now:
`pugi::xml_document`, `pugi::xpath_node`. Include path updated to
`<icl/utils/detail/pugi/PugiXML.h>` (private — this .cpp is itself under
`io/detail/`, so it's in-tree-only anyway).

**`geom/Primitive3DFilter.cpp`** — was already pugi-direct; no change.

**`utils/meson.build`** — dropped `install_headers(files('detail/pugi/PugiXML.h', 'detail/pugi/pugiconfig.hpp'), …)`. Pugi is now genuinely private: compiled into libicl-utils, not shipped in the install tree.

#### 3. DynamicGUI retirement

`icl/qt/DynamicGUI.{h,cpp}` surfaced during the pugi work — three private
methods had `pugi::xml_node` in their signatures (already forward-declared,
so the header compiled without pugi, but still leaked pugi names).

Audit: **zero in-tree consumers.** The only `#include <icl/qt/DynamicGUI.h>`
in the entire tree is `DynamicGUI.cpp` itself. No apps, demos, examples,
tests, other modules. Framework-DSL-for-out-of-tree-users or dead code —
the user confirmed it was dead ("cannot remember needing this at any
time"). Retired:

- `icl/qt/DynamicGUI.h` — deleted
- `icl/qt/DynamicGUI.cpp` — deleted
- `icl/qt/meson.build` — entries removed

(A brief intermediate state had DynamicGUI's pugi refs moved to anonymous-
namespace helpers in the .cpp — rendered moot by the subsequent deletion.
Not worth preserving in git history.)

#### Verification

- Full build clean (124 targets linked post-retirement).
- `tests/icl-tests -j 1` → **567/567 pass**, unchanged.
- `grep 'pugi::' icl/**/*.h` → zero hits outside vendored `detail/pugi/PugiXML.h` itself. Rule holds strictly.
- `grep '#include.*detail/' icl/**/*.h` → zero installed-header hits into `detail/`. Audit clean.

#### Memory writes this session

- `feedback_detail_strict_rule.md` — new. Captures the strict invariant
  "`detail/` ⟺ not installed; no installed header may reach into detail/".
- `project_module_subdirs.md` — updated: math now DONE; utils now fully
  DONE (pugi genuinely private).
- `project_utils_subdirs.md` — updated: XML.h deletion noted; pugi's
  not-installed status noted; cross-link to `feedback_detail_strict_rule.md`.
- `project_test_parallel_flakiness.md` — new, surfaced during verification.
  `tests/icl-tests` default-parallel loses 5-10 tests; `-j 1` passes all
  567; shared static/global somewhere in `Quick2` path.

#### What's deferred

- **The other module reorgs** (`icl/core/`, `icl/filter/`). `filter/`
  pairs with `project_filter_dispatch_arch.md` (splitting legacy Ops
  into `X.cpp / X_Cpp.cpp / X_Ipp.cpp / X_SSE.cpp`). Core is smaller.
- **Parallel-test flakiness root cause** — likely `QuickContext` pool
  (see `project_memorypool.md`) or a filter Op scratch buffer. Debug
  with `TEST_FOREACH` around a flaky test to reproduce reliably.
- All Session 48 deferrals still open: browser viewer for WSGrabber,
  `wss://`, additional codecs (`webp`, `jxl`, `lz4`), `Configurable`
  events on child-set change, capability-flag codec classification.

---

## Previous State (Session 48 — plugin-registry unification + ZMQ retirement)

### Session 48 Summary

Nine landings over one session, consolidated into two git commits
(`ce7b47aa7` + `4d464b558`). Every plugin-registration mechanism in the
codebase now runs on a single primitive (`utils::PluginRegistry<Key,
Payload, Context>`); the remaining façades are either thin
free-function accessors or one class (`GrabberRegistry`) that carries
real domain-specific side maps. Three plugin base classes gone, two
more façades gone, one network transport retired.

See `plugin-registry-plan.md` at repo root for the full phase-by-phase
intent; what's below is what actually shipped.

#### 1. The primitive (`utils::PluginRegistry<Key, Payload, Context>`)

New header `icl/utils/PluginRegistry.h` (~220 lines). Keyed-entry
registry with five orthogonal capabilities the various ICL registries
used to reimplement inconsistently:

- **Payload-agnostic**: Payload is a template parameter. Callers pick
  between `std::function<Sig>` (callable-plugin) or
  `std::function<std::unique_ptr<T>(Args...)>` (class-plugin).
- **Priority**: each entry has a `priority` int. `KeepHighestPriority`
  policy resolves conflicts deterministically across TU static-init
  orderings (replaces ImageMagick's previous dead `overrideExisting`
  bool — libpng at prio 0 now beats ImageMagick at prio -10 for `.png`
  by construction, not by linker luck).
- **Applicability predicate**: optional `ApplicabilityFn<Context>`.
  Used by `BackendSelector` for context-aware backend picking (depth,
  size, IPP-applicability, …). Classics leave it empty.
- **Forced-key override**: `setForced(key)` / `clearForced()` —
  testing affordance used by `BackendDispatching::forceAll` to
  cross-validate filter backends.
- **OnDuplicate policies**: `Throw` / `KeepFirst` / `Replace` /
  `KeepHighestPriority`.

Public API: `registerPlugin`, `get`, `getOrThrow`, `resolve`,
`resolveOrThrow`, `has`, `keys`, `entries`, `setForced`,
`unregisterPlugin`, `clear`.

Two canonical aliases:

```cpp
template <class Sig>
using FunctionPluginRegistry =
    PluginRegistry<std::string, std::function<Sig>>;

template <class T, class... CtorArgs>
using ClassPluginRegistry =
    PluginRegistry<std::string, std::function<std::unique_ptr<T>(CtorArgs...)>>;
```

One registration macro `ICL_REGISTER_PLUGIN(registry_expr, tag, ...)`
using `__attribute__((constructor, used))` — the only macOS-portable
mechanism that survives dead-stripping (established in Session 47 for
compression plugins; now applied universally).

18 unit tests in `tests/test-plugin-registry.cpp` covering both
aliases, all four policies, priority+applicability resolution, forcing,
exact-match vs predicate-based lookup, threading stress (8 threads ×
200 concurrent ops).

#### 2. `BackendSelector` refactored onto the primitive

The inner `impls` vector + manual sort + manual `setImpl` gone.
`BackendSelector<Context, Sig>` now holds a `PluginRegistry<Backend,
shared_ptr<ImplBase>, Context>` with `OnDuplicate::Replace`. Priority
= `static_cast<int>(backend)` (matches pre-refactor "higher enum value
= preferred" ordering).

`ImplBase` slimmed: description + applicability hoisted to the
registry's `Entry`; `cloneFn` stays for stateful backends.
`BackendSelectorBase::forcedBackend` field → virtual `force/unforce/
forcedBackend()` methods delegating to the registry's
`setForced/clearForced/forcedKey`.

`BackendDispatching<Context>` outer shell untouched (heterogeneous
multi-Sig container + enum-indexed selector array + clone ctor +
`forceAll`/`unforceAll` + `allBackendCombinations` + `BackendProxy`
fluent API — all stay, they address a distinct "outer" dimension that
the primitive doesn't).

Zero behavioural change; all filter tests pass. `BackendDispatching.h`
shrunk 378 → 327 lines.

#### 3. Classic registries migrated

| Old mechanism | Outcome |
|---|---|
| `utils::PluginRegister<T>` | **Deleted.** Replaced by domain-specific `pointCloudGrabberRegistry()` + `pointCloudOutputRegistry()` free-function accessors. `REGISTER_PLUGIN(TYPE, NAME, ...)` macro → domain-specific `REGISTER_POINT_CLOUD_GRABBER` / `REGISTER_POINT_CLOUD_OUTPUT`. TextTable rendering (`getRegisteredInstanceDescription`) moved to `pointCloudGrabberInfoTable()` / `pointCloudOutputInfoTable()` free functions. `creationSyntax` lives in a free-function-accessed side map. |
| `CompressionRegister` | **Demolished.** Replaced by free-function `compressionRegistry()` returning `ClassPluginRegistry<CompressionPlugin>&`. `REGISTER_COMPRESSION_PLUGIN` macro is now a one-line alias over `ICL_REGISTER_PLUGIN`. 5 codec .cpps + WSImageOutput.cpp swapped include to `<icl/io/CompressionRegistry.h>`. ImageCompressor's 3 call sites rewritten to `compressionRegistry().getOrThrow(mode).payload()` + `.keys()` + sort. |
| `FileWriterPluginRegister` | **Demolished.** Replaced by `fileWriterRegistry()` free function. `REGISTER_FILE_WRITER_PLUGIN` is a one-line alias. ImageMagick's priority-based loop calls `fileWriterRegistry().registerPlugin(...)` directly. |
| `FileGrabberPluginRegister` | **Demolished.** Symmetric. Plus: `HeaderInfo` struct hoisted from nested `FileGrabberPlugin::HeaderInfo` to namespace-scope `icl::io::HeaderInfo` (consumers: `JPEGDecoder.cpp`, `FileGrabberPluginPNM.cpp`, `FileGrabberPluginCSV.cpp`). |
| `GrabberRegister` | **Renamed to `GrabberRegistry`** — stays a class because it carries three orthogonal side maps (device-list per backend, bus-reset function per backend, per-backend description strings) that don't fit the primitive's `Entry`. Factory map now uses `PluginRegistry<string, CreateFn>` internally; side maps stay as plain `std::map`s on the class. `REGISTER_GRABBER` and `REGISTER_GRABBER_BUS_RESET_FUNCTION` macros upgraded to `__attribute__((constructor, used))` (drops the file-scope-static-struct idiom). Zero changes to the 19 grabber backend .cpp files. |
| `GenericImageOutput` (was hardcoded `#ifdef` switch) | **Flipped to registry-based dispatch.** `imageOutputRegistry()` accessor; each backend (`WSImageOutput`, `LibAVVideoWriter`, `OpenCVVideoWriter`, `V4L2LoopBackOutput`, plus built-in `null`/`file`) registers a `params → sender-callable` factory via `REGISTER_IMAGE_OUTPUT`. `init()` shrunk from ~140 lines of `#ifdef` chain to ~20 lines of registry lookup. `-o list` affordance auto-populated from registry entries. |

#### 4. Function-plugin conversions (3 base classes retired)

- **`ImageOutput` base class deleted.** 7 backends drop inheritance.
  WSImageOutput keeps `utils::Configurable` (its compression settings
  + port/clients/bytes properties are still exposed); its ImageOutput
  ancestry is just replaced by the class being directly-instantiable
  and registering itself with `imageOutputRegistry()`. `GenericImageOutput`
  itself no longer inherits anything; its `impl` is now
  `std::function<void(const Image&)>` instead of `shared_ptr<ImageOutput>`.

- **`FileWriterPlugin` base class deleted.** 6 plugins (PNG, JPEG, CSV,
  PNM, BICL, ImageMagick) drop inheritance. Each plugin's registration
  lambda uses a **per-macro-type function-local static** instance for
  state, avoiding the dyld-init-time construction that caused the
  Session 47 BICL/CompressionRegister ordering bug. BICL's multiple
  variants (rle1/4/6/8, jicl) each get their own distinct lambda type
  → their own static instance with distinct ctor args.

- **`FileGrabberPlugin` base class deleted.** Symmetric conversion.
  `HeaderInfo` struct hoisted to namespace scope (see above).

#### 5. ZMQ backend retired (~260 LOC)

Deleted `ZmqGrabber.{h,cpp}`, `ZmqImageOutput.{h,cpp}`, the
`libzmq`/`cppzmq` dep detection in root `meson.build`, the `zmq`
option in `meson.options`, the `libzmq3-dev` apt install and
`BUILD_WITH_ZMQ=ON` CMake flag in CI, and all doc references.

Why retire: `ws` (Qt6 WebSockets, added Session 46) covers the same
pub/sub-over-network use case with auto-reconnect resilience, browser
compatibility, and lower build-config weight. ZMQ's additional value
(cross-language interop) has no active consumer in the ICL ecosystem.
Strong circumstantial evidence of disuse: `ZmqImageOutput::send` had
been missing its class qualifier (`void send(...)` instead of `void
ZmqImageOutput::send(...)`) — a latent undefined-virtual that would've
been a runtime crash or linker error for anyone actually using it.
Parallels the SharedMemory retirement in Session 47.

Pre-existing bugfix discovered in passing: the same missing-qualifier
bug also affected `V4L2LoopBackOutput::send`. Fixed.

#### 6. Priority-based conflict resolution (replaces dead `overrideExisting`)

FileWriter / FileGrabber registries now use
`OnDuplicate::KeepHighestPriority`. Register signature:
`registerExtension(ext, factory, int priority = 0)`. ImageMagick
registers all its extensions at `priority = -10` (fallback); libpng /
libjpeg register at default priority 0 and win for .png/.jpg/.jpeg
deterministically. Extensions ImageMagick uniquely handles (tiff,
gif, bmp, svg, …) resolve to ImageMagick unopposed.

The dead `overrideExisting = true` flag — nobody actually passed it —
is gone.

#### 7. `V4L2LoopBackOutput::send` + `ZmqImageOutput::send` bugfix

See §5 above. Latent undefined-virtuals caused by missing
`ClassName::` qualifier on the method definitions. ZMQ version fixed
then deleted; V4L2 version kept (the V4L2 backend is still present).

#### Final layout

| Entity | Kind |
|---|---|
| `utils::PluginRegistry<Key, Payload, Context>` | primitive template |
| `utils::FunctionPluginRegistry<Sig>` | alias (callable payload) |
| `utils::ClassPluginRegistry<T, CtorArgs...>` | alias (factory-producing-unique_ptr payload) |
| `ICL_REGISTER_PLUGIN(registry_expr, tag, ...)` | macro (attribute-constructor) |
| `compressionRegistry()` | free-fn accessor |
| `fileWriterRegistry()` | free-fn accessor |
| `fileGrabberRegistry()` | free-fn accessor |
| `imageOutputRegistry()` | free-fn accessor |
| `pointCloudGrabberRegistry()` / `pointCloudOutputRegistry()` | free-fn accessors (with side-map + TextTable helpers) |
| `GrabberRegistry` (class) | façade with side maps for device-list / bus-reset / descriptions |
| `BackendSelector<Context, Sig>` (class) | Sig-specialization over the primitive, for filter backend dispatch |
| `BackendDispatching<Context>` (class) | outer container over N BackendSelectors, for filter Op prototypes |

Every `REGISTER_*` macro in the codebase now expands to
`__attribute__((constructor, used))`. Every plugin-registration
mechanism is backed by one primitive.

#### Verification

- Full build clean throughout (per phase)
- 551 → 567 tests (18 new `utils.plugin-registry.*` + 1 `get_or_throw`
  + 1 `policy.keep_highest_priority`). Zero regressions.
- End-to-end smoke: `icl-pipe -i create lena -o ws PORT` + `-i ws PORT
  -o file '/tmp/###.png'` round-trip. PNG write/read. BICL write.
  `-o list` + `-i list` affordances auto-populated from registries.

#### Memory writes this session

- `project_plugin_registry_unification.md` — rewritten with the locked
  design decisions (function-plugin vs class-plugin split, `Register`
  → `Registry` rename intent, façade-demolition map, open questions
  for future cleanup).
- `project_module_subdirs.md` — new. TODO: consider `detail/` subdirs
  per module (top-level = public API only; implementation-only files
  like per-backend grabbers and per-extension plugins go under
  `module/detail/<group>/`; second-order groupings like
  `detail/pylon/`, `detail/video/`, `detail/network/`,
  `detail/file-plugins/`, `detail/compression-plugins/`). Separate
  session — best done after the plugin-registry work stabilizes
  (done now).
- `feedback_sed_sandbox.md` — updated. Use `perl -pi -e '...'` (not
  `sed -i`) for bulk in-place substitutions in Agent sessions.

#### What's deferred (post-Session-48)

- **Directory reorganization** — `module/detail/<group>/` structure
  per `project_module_subdirs.md`. Biggest cleanup still pending;
  touches the entire source tree.
- **Capability-flag codec classification** (lossy/lossless +
  supported depths) — enables `auto` codec mode via the primitive's
  `applicability` machinery. Currently deferred.
- **Additional codecs**: `webp`, `jxl`, `lz4`, `deflate`/`zlib`.
- **`Configurable` events on child-set change** (`qt::Prop`
  auto-rebuild when `ImageCompressor` swaps codec) — surfaced in
  Session 47, unblocked by this session but not done.
- **Browser viewer for WSGrabber** (JS-side envelope parser).
- **`wss://` TLS** for WS transport.
- **WSGrabber server mode** (push-source workflow).
- **Path-based multi-stream on one WS server** (`/cam0`, `/cam1`).

---

## Previous State (Session 47 — SharedMemory retired + ImageCompressor → plugin framework + FileWriter/Grabber factory-ized)

### Session 47 Summary

Three landings, related by a common theme (uniform plugin-registration
across ICLIO).

#### 1. Retired the SharedMemory backend (1274 LOC)

- Deleted `SharedMemoryGrabber.{h,cpp}`, `SharedMemoryPublisher.{h,cpp}`,
  `SharedMemorySegment.{h,cpp}`. Removed from meson, GenericGrabber.h,
  GenericImageOutput.{h,cpp}, IO.h, V4L2LoopBackOutput.h (dead include),
  ZmqGrabber.h (stale comment).
- Migrated 4 consumers off `sm`: `SceneMultiCamCapturer.cpp` and
  `physics-paper-SceneMultiCamCapturer.cpp` now use `ws=PORT`;
  `Widget.cpp`'s auto-cap Combo now offers `ws` instead of `sm` (and
  finally consumes the `FILE/VIDEO/XCFP/SM`-style "active" markers it
  was preparing all along — that was dead code).
- WSImageOutput/Grabber doxygen note that they replace SM.
- Why: WS loopback covers the same loose-coupling-between-processes use
  case with auto-reconnect resilience and cross-host as a free bonus,
  for ~100 µs more latency (invisible at typical 30 fps workloads).

#### 2. ImageCompressor → plugin framework

`ImageCompressor` is now a thin **facade** over a process-wide
`CompressionRegister`. Built-in codecs (`raw`, `rlen`, `jpeg`, `1611`)
are now plugin classes that self-register at static init via
`REGISTER_COMPRESSION_PLUGIN`; new codecs drop in as a single .cpp.
**`zstd`** added as a proof-of-extensibility (optional dep on libzstd —
`brew install zstd` on macOS, gated by `ICL_HAVE_ZSTD`).

Wire format **broken vs the pre-Session-47 `Header::Params` POD** (per
explicit user OK — "no backwards compat required"). New envelope is a
46-byte fixed prefix + variable-length codec-name / codec-params /
image-meta / payload. Codec name has no length cap (so codecs longer
than 4 chars work, e.g. `deflate`).

Each plugin inherits `Configurable` so per-codec tunables auto-surface.
`ImageCompressor` itself inherits `Configurable` and exposes a `mode`
property (menu populated from the registry) plus the **active plugin
as a child Configurable** with empty prefix — its `quality`/`level`/etc.
appear as siblings of `mode` and switch when the codec changes. When
`ImageCompressor` is itself added as a child of (e.g.) `WSImageOutput`
under prefix `compression`, the user sees `compression.mode` plus
`compression.quality` / `compression.level` / etc.

**Implemented `Configurable::removeChildConfigurable`** along the way
(previously a stub that threw `"is not yet implemented"`) so codec
swaps actually work.

`ImageOutput`'s historic `protected ImageCompressor` inheritance dropped
— the legacy `setCompression`/`getCompression` re-exports were the only
users; consumers that need compression now own an `ImageCompressor`
explicitly and expose it as a child Configurable (clean diamond-free
inheritance hierarchy).

#### 3. FileWriter + FileGrabber → factory-based plugin map

The static initialization order trap: `FileWriter::s_mapPlugins[".bicl"]
= new FileWriterPluginBICL` ran during dyld init, eagerly constructing
an `ImageCompressor`, which queried the (still-empty) `CompressionRegister`
and threw — aborting the rest of dyld's init pass and leaving the
process with a permanently empty registry.

Fix: make both file-format plugin maps factory-based, mirroring
`REGISTER_GRABBER` and `REGISTER_COMPRESSION_PLUGIN`:

- `icl/io/FileWriter.h`: new `FileWriterPluginRegister` singleton +
  `REGISTER_FILE_WRITER_PLUGIN(tag, ext, factory)` macro using
  `__attribute__((constructor, used))`.
- `icl/io/FileGrabber.h`: symmetric `FileGrabberPluginRegister` +
  `REGISTER_FILE_GRABBER_PLUGIN`.
- Each `FileWriterPluginXxx.cpp` / `FileGrabberPluginXxx.cpp` adds its
  registrations at the bottom (one line per extension; ImageMagick uses
  a custom `__attribute__((constructor))` for its long extension list).
- The old `FileWriterPluginMapInitializer` static class and the
  function-local-static `find_plugin` map are gone.
- Plugin instances are built lazily on first lookup and cached for the
  lifetime of the process (semantically identical to the previous
  shared-instance model, but no static-init-time construction).

With both file-format plugin systems factory-based, **no `ImageCompressor`
is ever constructed before main**, so its constructor can again be eager
(install the active plugin immediately). Consumers don't need to know
about any laziness — `ImageCompressor` "just works".

**Static-init lesson learned (worth recording):**
`__attribute__((constructor))` on a free function is the only macOS-portable
way to guarantee dyld-time invocation. Anonymous-namespace static-storage
objects with non-trivial constructors *can* be dead-stripped at the .o
level even though their `__GLOBAL__sub_I_*` symbol survives in `nm`.
LLVM/Clang/GoogleTest/Boost-Test all use the constructor-attribute idiom
for this reason.

#### Verification

- Full build clean.
- 551/551 tests pass sequentially (was 547; +4 new compression-framework
  tests: `CompressionRegister.builtins_registered`,
  `ImageCompressor.raw.roundtrip`, `ImageCompressor.auto_detect_codec`,
  `ImageCompressor.zstd.roundtrip`).
- End-to-end smoke: `icl-pipe -i create lena -o ws 9999 -no-gui` +
  `icl-pipe -i ws 9999 -o file '/tmp/v4_###.png' -no-gui` produces
  24 PNGs/2s. WSGrabber introspection (`icl-camera-param-io -i ws PORT
  -l`) shows the full property tree.

#### Memory writes this session

- `reference_websocket.md` — extended with the SM-replacement note (was
  written in Session 46, this session validates and extends).
- `project_dynamic_child_configurables.md` — TODO: `qt::Prop` doesn't
  auto-rebuild when child Configurables are added/removed at runtime
  (surfaced when ImageCompressor swaps codec plugins).
- `project_plugin_registry_unification.md` — TODO: collapse the now-4
  plugin-registration patterns (GrabberRegister, CompressionRegister,
  FileWriterPluginRegister, FileGrabberPluginRegister, plus the filter
  backend dispatch idiom) into one generic `utils::PluginRegistry<T>`
  template + one macro family. Estimated ~4-6 hours.

#### What's deferred

- **Generic plugin registry in ICLUtils** (per `project_plugin_registry_unification.md`)
- **Capability-flag-based codec classification** (lossy vs. lossless +
  supported depths) — surfaces in the v2 `auto` codec mode + future
  `auto-but-non-lossy` mode hint
- **`auto` codec mode** that picks per-frame based on entropy/sparsity
- **Additional codecs**: `webp`, `jxl`, `lz4`, `deflate`/`zlib`
- **Browser viewer** for WSGrabber (would need a JS-side envelope parser)
- **`Configurable` events on child set change** so `qt::Prop` can
  rebuild when codec swaps

---

## Previous State (Session 46 — WebSocket image I/O via Qt6 WebSockets)

### Session 46 Summary

Added a WebSocket-based image transfer pair to ICLIO for loose coupling
between ICL processes (and potentially browser viewers later).

#### Files added

- `icl/io/WSImageOutput.{h,cpp}` — server side (`QWebSocketServer`),
  broadcasts every `send()`-ed image to all connected clients. PIMPL,
  Configurable.
- `icl/io/WSGrabber.{h,cpp}` — client side (`QWebSocket`) with
  auto-reconnect state machine (Disconnected → Connecting → Connected),
  exponential backoff (250 ms → 5 s capped), bounded frame queue
  (drop-oldest), block-with-timeout + replay-last semantics in
  `acquireImage()`.
- `tests/test-quick-io.cpp` — three new tests:
  `WS.loopback.roundtrip`, `WS.multi_client.broadcast`,
  `WS.client_survives_server_restart`.
- Memory: `reference_websocket.md`.

#### Wiring

- meson detects `Qt6 WebSockets` (`brew install qtwebsockets` on macOS),
  defines `ICL_HAVE_QT_WEBSOCKETS`, gates the new sources in
  `icl/io/meson.build`. Build-clean either way.
- `WSGrabber` registers itself with the GenericGrabber plugin map via
  `REGISTER_GRABBER(ws, ...)` — `-i ws ws://host:port` works through any
  GenericGrabber-driven app.
- `WSImageOutput` plugged into `GenericImageOutput.cpp`'s switch — `-o
  ws PORT` (or `-o ws BIND:PORT`) works through `icl-pipe` etc.

#### URL form

```
-o ws PORT                # server: bind 0.0.0.0:PORT, broadcast (LAN-open)
-o ws BIND:PORT           # server: bind a specific interface
-i ws ws://host:port      # client: connect & receive (auto-reconnect)
```

Server bind defaults to `0.0.0.0` per the loose-coupling-between-machines
use case. Server-mode WSGrabber (`-i ws server:PORT`, push-source) is
deferred to v2.

#### Wire format

`ImageCompressor::Header` + compressed payload (one binary WS frame per
image). Default mode = `none` (raw bytes, lossless). The `compression`
property on WSImageOutput exposes ImageCompressor's full menu (`none`,
`rlen`, `jpeg`, `1611`).

#### Resilience

The WSGrabber's reconnect state machine on a private QThread papers over
server outages: on disconnect, schedules a `QTimer::singleShot` retry
with exponential backoff. `acquireImage()` blocks for up to `block
timeout ms` (default 1000), then either replays the last successfully
decoded frame (default true) or returns null. The application loop never
observes a dead state.

#### Threading + bootstrap

QWebSocket(Server) needs an event loop. Each WS class owns its own
QThread that runs the local event loop — no main-thread `exec()`
required. But `QObject` machinery requires *some* `QCoreApplication` to
exist in the process. ICL apps using `ICLApp::exec()` already create a
`QApplication`. Headless callers (`icl-pipe -no-gui`, library users
embedding ICL without Qt main loops) are covered by a static
`ensureQCoreApplication()` lazy bootstrap inside both WS classes' ctors.
Tests get a `QCoreApplication` from `tests/icl-tests.cpp main()` via a
new `#ifdef ICL_HAVE_QT` block.

#### Verification

- Full build clean (118 binaries, all module libs, including
  `WSImageOutput.cpp.o` and `WSGrabber.cpp.o`).
- 546/546 tests pass sequentially (was 543; +3 WS tests).
- End-to-end smoke: `icl-pipe -i create lena -o ws 19090 -no-gui`
  publisher + `icl-pipe -i ws ws://127.0.0.1:19090 -o file
  '/tmp/wspipe_###.png' -no-gui` receiver wrote 24 PNG frames in 2 s.
- Reconnect path proven by `WS.client_survives_server_restart`: the same
  grabber receives frames from a brand-new server brought up on the
  same port post-disconnect.

#### Doxygen / discoverability

- `GenericGrabber.h` backend-list comment now includes `ws` and `zmq`.
- `GenericImageOutput.h` backend-list comment now includes `ws`.

#### What's deferred (per `reference_websocket.md`)

- `wss://` (TLS)
- WSGrabber server mode (push-source workflow)
- Path-based multi-stream on one server (`/cam0`, `/cam1`)
- Generic `CompressionPlugin` interface + `auto` codec mode (drops
  zstd/webp/jxl/lz4/etc. into the same envelope without touching
  `ImageCompressor.cpp`)
- Promote `ImageOutput` base to `Configurable`
- Browser viewer

---

## Previous State (Session 45 — ICLIO demos retired, apps modernized)

### Session 45 Summary

Audited ICLIO module (12 apps + 3 demos) following the playbook from
Session 43 (Filter audit). Net: 3 demos + 2 apps retired, 1 demo
converted to a test, 4 latent bugs fixed, dead code stripped from
2 apps, two apps modernized, framework callback type upgraded to
`std::function`.

#### A. Demos folder is now empty

All three IO demos retired:
- **`png_write_test`** → already covered by `Quick2.IO.save.load.png`
- **`undistortion`** → subsumed by the `@udist=file.xml` qualifier on
  every GenericGrabber-driven app (`GenericGrabber.cpp:273` calls
  `enableUndistortion(propVal)` directly during init)
- **`depth_img_endcoding_test`** → converted into two real tests in
  `tests/test-quick-io.cpp`:
  - `ImageCompressor.1611.lossless_in_range` (lossless 11-bit pack
    with quality="1")
  - `ImageCompressor.1611.clamps_above_11bit` (overflow → mask 2047,
    not modulo wrap)
  - Discovered while writing the tests: `"1611"` quality `"0"` is the
    *lossy* depth-mapping variant (`pack16to11_2`/`unpack11to16_2`,
    applies the Kinect Z formula); quality `"1"` is the lossless
    bit-pack. The original demo used quality `"0"` so its visual
    "roundtrip" was actually lossy.

#### B. Apps retired (2)

- **`icl-k2`** — bypassed ICL's own `Kinect2Grabber` to talk to
  libfreenect2 directly (~110 lines, with pointless GLFW init). Use
  `icl-pipe -i kinect2 0` instead.
- **`icl-dcdeviceinfo`** — trivial `DCGrabber::getDCDeviceList()`
  wrapper. Subsumed by `-i dc 0@info` on any GenericGrabber-driven app.

Remaining IO apps (8 + 2 dc1394-only): camera-param-io, convert,
create, jpg2cpp, multi-viewer, pipe, reset-bus, video-player +
dcclearisochannels, reset-dc-bus.

#### C. Real bugs fixed (4)

1. **`icl-convert`** — every typed flag (`-size`, `-format`, `-scale`,
   `-depth`) crashed with `parse<T>: type is not stream-extractable`.
   Root cause: `parse<X>(pa("-y"))` doesn't compile cleanly for ProgArg
   inputs; should be `pa("-y").as<X>()`. Same file already used the
   correct form in one place.
2. **`icl-convert`** `-scalemode` was silently ignored — inner
   `std::string sm = pa("-scalemode").as<std::string>();` shadowed the
   outer `scalemode sm`, then `sm = interpolateNN;` assigned an enum
   into the *string* variable (compiles via implicit char conversion!),
   never updating the actual scalemode used. Fixed by renaming the
   local string `s`.
3. **`icl-camera-param-io`** had two duplicate `pa_explain` entries:
   `-p` was described twice (first wrong, "grabber type"; second
   correct, "parameter file"), and `-g` was described twice (second
   actually for `-go`/`-grab-once`). Help output dropped the first
   in each pair. Fixed: each flag described once with the right
   meaning.
4. **`icl-jpg2cpp`** with a path-containing input filename emitted
   invalid C++ identifiers (`aauc_Data_/tmp/lena[NROWS][NCOLS]`).
   Documented caveat, but trivially fixed with
   `std::filesystem::path::stem()`.

Plus the `icl-dcclearisochannels` source had unqualified `vector<>`
(would not compile even with libdc); fixed.

#### D. Dead code stripped

- **`icl-pipe`**: `pthread.h` include + retired
  `EXPLICITLY_INSTANTIATE_PTHREAD_AT_FORK` comment, commented-out
  `setIgnoreDesiredParams` block, ~14-line commented-out
  `-reinterpret-input-format` block + matching `-dist` arg, "interactive
  clip mode is not yet implemented" branch + corresponding help text.
- **`icl-video-player`**: dead globals `disableNextUpdate`,
  `mouseInWindow`. Old name `ICLApplication` → `ICLApp`.

#### E. Apps modernized (3)

1. **`icl-pipe`** — the `-pp <name>` filter ladder used to be a hard-coded
   string menu (`gauss/gauss5/median/median5`); now also accepts any
   UnaryOp class registered with `REGISTER_CONFIGURABLE` (CannyOp, FFTOp,
   BilateralFilterOp, GaborOp, ConvolutionOp, MedianOp, ... — all 29
   from Session 43). The fallback uses `Configurable::create_configurable`
   and a `dynamic_cast<UnaryOp*>`; raw `UnaryOp*` static became
   `unique_ptr`. Help text updated; bad names get a friendly error
   instead of an uncaught exception.
2. **`icl-convert`** — one-shot CLI no longer uses static buffers for
   intermediate ImgBase pointers. Each stage (convert / flip / rotate /
   crop) now produces an `Image` (shared_ptr-backed) which auto-releases
   on reassignment. Added missing `return 0;` and second `return -1;`
   on the bad-crop-rect error path.
3. **`icl-multi-viewer`** — collapsed `template<int N> void run()` plus
   8 explicit `if(nInputs > N) app.addThread(run<N>);` blocks (lines
   213-220) into a single `void run(int n)` registered via
   `app.addThread([i]{ run(i); })`. Required upgrading the framework
   callback type (see F).

#### F. Framework: ICLApplication callback type → std::function

`icl::qt::ICLApplication::callback` was `void(*)(void)` (raw function
pointer, 2006-style). Upgraded to `std::function<void()>` so capturing
lambdas work. Function pointers still convert implicitly so every
existing `app.addThread(my_run_fn)` call site is unchanged. Internal
`ExecThread` ctor also updated to take callback by value + move. Net
cost: one indirection per thread tick (invisible at typical 30-fps
loops). Net win: lambdas become first-class throughout the framework.

#### G. Verification

- Full build clean (118 binaries linked, down from 121 — 3 demos +
  2 apps retired, 1 net new test pair added).
- 543/543 tests pass sequentially. Parallel-run flakes (4 in Quick2
  Math/Filter/Compose) reproduce on master, unrelated.
- Smoke-tested the modernized apps: `icl-pipe -pp gauss5` (legacy
  mnemonic), `icl-pipe -pp CannyOp` (registry fallback), `icl-pipe -pp
  NotAnOp` (friendly error), `icl-convert -size … -scalemode NN`,
  `icl-convert -rotate 30`, `icl-convert -flip both`, `icl-convert
  -c x y w h`, `icl-multi-viewer` in both sync and async modes.

#### H. What's next

Filter playbook step matched on IO. Possible next targets within IO:
- ImageMagick 7 PixelPacket → Quantum (per `project_imagemagick7.md`)
- LibAVVideoWriter for FFmpeg 6/7 (per `project_ffmpeg.md`)
- Qt6 multimedia grabbers (per `project_qt6_multimedia.md`)
- Or move on to **ICLCV** (next module in the dependency chain).

---

## Previous State (Session 44 — io::Grabber mutex pattern + entry into ICLIO audit)

### Session 44 Summary

Ported the `UnaryOp::m_applyMutex` pattern (Session 43, see
`project_configurable_op_threadsafety.md`) into `io::Grabber` as a
preventative against the analogous race (GUI/control thread firing
`processPropertyChange` mid-`acquireImage()`). Same shape, same rationale —
a property mutation that rebuilds backend state (size, format, depth,
exposure, gain, mode menus, etc.) must not interleave with the grab
thread's read of that state.

#### Grabber base changes
- `protected: mutable std::recursive_mutex m_grabMutex;` added.
- `void registerCallback(const Configurable::Callback &cb);` overload added,
  wrapping `cb` with a `scoped_lock(m_grabMutex)` before dispatch. Mirror
  of `UnaryOp::registerCallback`. `using Configurable::registerCallback;`
  re-exposes the base overload list.
- `Grabber::grab(ImgBase**)` acquires `m_grabMutex` at the top — single
  funnel for every backend's `acquireImage()` + `adaptGrabResult` + warp
  undistortion. No subclass changes required for reader-side coverage.

#### Subclass migrations (writer-side)
All 16 Grabber subclass ctors swapped from
`Configurable::registerCallback([this](Property &p){processPropertyChange(p);})`
→ unqualified `registerCallback(...)` so the wrap takes effect:
- always-built: CreateGrabber, FileGrabber, DemoGrabber, SharedMemoryGrabber,
  GenericGrabber, OpenCVCamGrabber, OpenCVVideoGrabber
- platform-conditional (mechanical rename, can't local-build): DCGrabber,
  V4L2Grabber, KinectGrabber, Kinect2Grabber, OpenNIGrabber,
  SwissRangerGrabber, XiGrabber

OptrisGrabber and PylonGrabber don't register property callbacks at the
Grabber level so they didn't need migration. Helpers that are Configurables
but not Grabbers (DCDeviceFeatures, OpenNIUtils generator-options,
PylonCameraOptions) intentionally still call `Configurable::registerCallback`
— they're outside the Grabber inheritance chain and have their own locking
strategies (often a per-class `m_propertyMutex` already).

#### Verification
- Full build clean (240/240 targets) on macOS with the always-built
  backends.
- Test suite: 541/541 pass when run sequentially (`-j 1`). Parallel runs
  (`-j 16`, the harness default) show 4 pre-existing flakes
  (Quick2.Math.scalar.add, Quick2.Math.chained, Quick2.Filter.filter.chain,
  Quick2.Compose.vconcat.triple) — confirmed identical behaviour against
  the unmodified tree, unrelated to this change.

#### What's next — ICLIO audit (after Filter)
Module dependency order is `…→Filter→IO→CV→Qt→…`, so IO is the natural
next module to audit. Suggested entry points:
- Run each grabber backend's demo/app under `icl-camviewer -i <backend>`
  to confirm the property-callback path is exercised post-migration.
- Look at `icl-pipe`, `icl-stream-server`, `icl-create`, `icl-image-viewer`,
  `icl-rtsp-streamer`, `icl-recorder`, `icl-video-recorder` etc. — same
  Configurable-driven UI pattern as the filter audit suggests these may
  benefit from `qt::Prop(&grabber)` to auto-render device controls
  (would need a brief look at how CamCfgWidget already does it).
- ImageOutput counterpart: spot-checked — ImageOutput is a minimal
  `send(Image)` interface, NOT a Configurable, no property callbacks. No
  race surface analogous to Grabber. Skip.

---

## Previous State (Session 43 — ICLFilter Configurable migration + filter-playground)

### Session 43 Summary

Completed Phase 1 of the ICLFilter migration plan (see
`project_filter_playground.md` memory): 29 UnaryOps ported from hand-rolled
setters/getters to `utils::Configurable` properties, a unified
`icl-filter-playground` app built that auto-generates the UI from any Op's
properties, and 9 redundant per-op demos + the `UnaryOp::fromString`
string-registry deleted (~970 lines net). Several latent framework bugs
found and fixed along the way. Four Configurable-level extensions landed
so the migration could avoid per-Op boilerplate.

Final op count by family:

- **Arithmetic/logic/compare:** UnaryArithmeticalOp, UnaryCompareOp, UnaryLogicalOp
- **Thresholding:** ThresholdOp, LocalThresholdOp
- **Neighborhood:** ConvolutionOp, MedianOp, MorphologicalOp, WienerOp
- **Affine:** AffineOp + RotateOp/ScaleOp/TranslateOp (inherit AffineOp with
  irrelevant knobs hidden via `deactivateProperty` regex filters) +
  MirrorOp (BaseAffineOp direct)
- **Derivative/gradient:** CannyOp, GradientOp (new — supersedes the
  non-UnaryOp `GradientImage` class which was retired)
- **Color/LUT:** LUTOp, PseudoColorOp, DitheringOp
- **Bank/feature:** GaborOp (with live kernel-preview image property),
  BilateralFilterOp, MotionSensitiveTemporalSmoothing (MSTS)
- **Transform/rescale:** FFTOp, WarpOp, FixedConvertOp, IntegralImgOp,
  ChamferOp, WeightChannelsOp, WeightedSumOp

Skipped: ProximityOp (BinaryOp + apply currently unimplemented),
ImageRectification (needs 4-point quadrangle input, not a straightforward
UnaryOp).

#### A. Configurable framework extensions

1. **"image" property type + type-erased `Property::payload`** —
   `utils::Configurable::Property` gained a `std::any payload` field and
   two new virtuals (`setPropertyPayload` / `getPropertyPayload`). The Qt
   `Prop` widget now renders an embedded `Display` for `type == "image"`,
   polled on the property's volatileness timer by a new
   `VolatileImageUpdater`. `core::Image` is ref-counted via shared_ptr so
   the handoff is cheap. First consumer: GaborOp's kernel preview —
   visible in the playground without any bespoke GUI plumbing. Design
   captured and then resolved in `project_configurable_image_type.md`.

2. **UnaryOp-level apply/callback mutex** — `UnaryOp` now owns a
   protected `mutable std::recursive_mutex m_applyMutex` and overrides
   `registerCallback` to auto-wrap every callback with a lock on that
   mutex. Subclasses only need one line (`std::scoped_lock lock(m_applyMutex);`
   at the top of their `apply()`). Hit after 3 open-coded consumers
   (filter-swap UAF, GaborOp vector race, WienerOp mid-apply mask race).
   Design captured and resolved in `project_configurable_op_threadsafety.md`.

3. **GUIComponents `String` now escapes commas** — initText with literal
   commas (e.g. CSV defaults like `"0.299,0.587,0.114"` for
   WeightChannelsOp) was tripping the GUI definition parser's
   comma-split. Now backslash-escapes commas and backslashes before
   concatenation; the parser's StrTok (configured with `\\` as escape
   char) unescapes on split. Latent bug, affected anyone with commas in
   `String` defaults.

4. **Collapsed setter boilerplate** — all migration sites went from
   `prop("X").value = str(v); call_callbacks("X", this);` to
   `setPropertyValue("X", v);` (existing Configurable API — just wasn't
   being used by the pre-existing LocalThresholdOp pattern that everyone
   was copying).

#### B. filter-playground (`icl/filter/apps/filter-playground.cpp`)

A single unified app exposing every migrated Op through
introspection-driven UI. Users pick a filter from a combo; the properties
panel is rebuilt via `Prop(&currentOp)` using the CamCfgWidget-style
BoxHandle-swap pattern. The playground subsumes 9 retired single-op
demos (canny-op, convolution-op, dither-op, fft, temporal-smoothing,
warp-op, bilateral-filter-op, gabor-op, filter-array).

Polish landed this session:

- **Source controls** — size / depth / format combos feeding `useDesired`
  on the grabber, plus a source-ROI mode combo with 7 presets
  (none/UL/UR/LL/LR/center/interactive).
- **Interactive ROI** — left-click-drag on the source canvas defines a
  rubber-banded (transparent blue) rect; release commits it (red
  outline); right-click resets.
- **Dynamic Prop panel** — full filter swap on combo change, serialized
  against the exec thread's `apply()` via `opMutex`.
- **Auto-range result display** — `ImageHandle::setRangeMode(rmAuto)` on
  the result so 16s/32f filter outputs render over their actual dynamic
  range (gradients, FFT magnitude, etc. stop looking mostly-black).
- **Timing + status** — apply-time label, fps counter, status label
  (shows "ok" or the exception text).

#### C. Framework bug fixes exposed by the playground

1. **`CannyOp::followEdge` stack overflow** — recursive 8-connected flood
   fill blew the thread stack when low threshold ≈ 0 made most pixels
   weak edges. Rewrote as iterative with a heap-backed work stack;
   identical semantics. Latent for years, trivially reached once the
   playground exposed the default `lowT=0`.
2. **`ConvolutionOp` black-output-on-depth-flip** — when reverting
   float→int, `m_kernel.toInt(true)` truncates normalized gauss/sobel
   kernels to all-zero. Fix: if the kernel has a known `fixedType`,
   rebuild from the lookup table instead of truncating. Custom float
   kernels still warn+truncate as before.
3. **`GaborOp`** — three bugs: (a) float-Gabor-kernel applied to int src
   degraded to toInt zeros + Img32f append type mismatch (now converts
   non-float src once via an internal `m_src32fBuffer`); (b) default ctor
   produced empty kernel bank → black first frame (now seeds from
   property defaults); (c) property-driven updateKernels raced the exec
   thread's vector iteration (initially fixed with a per-op mutex, then
   folded into the framework-level `UnaryOp::m_applyMutex`).
4. **`WienerOp` had no C++ fallback** — implemented one: classic
   adaptive Wiener (output = μ + max(0, σ²−noise)/max(σ², noise) ·
   (src − μ)) over 8u/16s/32f via an integral-image optimization
   (allocates once per apply, reused across channels, gives O(W·H)
   independent of mask size). Playground default seeded at 5×5 /
   noise=100 so the filter produces visible output at load time
   (`noise=0` is a mathematical no-op — textbook Wiener with "no noise
   expected" returns src unchanged). Mid-apply mask race (same pattern
   as GaborOp) fixed via the UnaryOp mutex. 3 new tests.
5. **GradientImage retired** — fully replaced by GradientOp
   (x/y/intensity/angle mode menu + normalize flag, ConvolutionOp-backed).

#### D. Phase 2 deferred items

- ImageRectification → UnaryOp (needs a 4-point quadrangle GUI picker;
  playground would grow a point-picker mode)
- `project_configurable_op_threadsafety.md` pattern potentially applies
  to `io::Grabber`/similar Configurables outside ICLFilter; revisit if
  those hit the same races

#### E. Memory writes this session

- `project_filter_playground.md` — the overall plan (already approved)
- `project_configurable_image_type.md` — resolved (first consumer GaborOp)
- `project_configurable_op_threadsafety.md` — resolved (UnaryOp mutex)
- `feedback_img_channel_access.md` — prefer `ImgChannel<T> c = img[0];
  c(x,y)` over `img(x,y,c)` in pixel loops (cached pointer vs. vector
  re-lookup)

---

## Previous State (Session 42 — rectify-image fix, AffineOp backends, Quick2 pool)

### Session 42 Summary

Finished the filter module audit (9/10 demos runtime-verified, warp-op deferred).
Major framework work driven by a broken `icl-rectify-image`:
- Hartley-normalized homography estimator (fixes ~128 px fitting errors)
- AffineOp Accelerate backend fixes (rotation direction, ROI respect, centering)
- Quick2 pool accounting workaround (trackedBytes per PooledBuffer)
- DataStore typed `install(MouseHandler*)` overload for MI-safe pointer offsets

Also: Common2.h now transitively exposes FPSLimiter. Two pre-existing
gotchas flagged (parse<T>(pa()) trap, vImage matrix convention).

#### A. rectify-image fix trail

Root cause was a chain of four independent issues, each found by attempting
to reproduce the reported "can't drag corners, bad output" failure mode:

1. **Mouse install broken** — `gui["draw"].install(mouse)` didn't compile
   (non-copyable AffineOp). Changed to `install(&mouse)`. Added a typed
   `DataStore::Data::install(MouseHandler*)` overload
   (icl/qt/DataStore.h:137-139) that performs the derived→base conversion at
   the call site, so the stored `void*` actually points to the MouseHandler
   subobject — matters for multiply-inherited handlers like
   DefineQuadrangleMouseHandler. Forward-declared `MouseHandler` in DataStore.h.

2. **Handles unclickable in small window** — DefineQuadrangleMouseHandler's
   click-tolerance was 8 *image pixels*. On a FullHD camera shown in a
   compact window the handles became sub-pixel-sized. Rewrote process() at
   icl/qt/DefineQuadrangleMouseHandler.cpp:105-166 to use *widget-pixel*
   distance via `e.getWidgetPos()` + widget-image scale, `max(projected,
   10px)` threshold. Rendering unchanged (still in image coords).

3. **Homography residuals of ~128 px** — unnormalized DLT is numerically
   unstable. Added Hartley normalization in
   icl/math/Homography2D.cpp:17-63 (translate centroid to origin, scale to
   mean-distance √2, fit there, un-normalize with `H = Ty^-1 · H̃ · Tx`).
   Both algorithms previously failed on rotated quads; now < 1 px residual.
   **Deleted the `Simple` algorithm** — nobody in the codebase passed it,
   it was structurally wrong for any perspective mapping. Also removed
   `advanedAlgorithm` param from ImageRectification::apply. 3 new tests
   in tests/test-math.cpp:1174-1221.

4. **Opaque error messages** — ImageRectification's "at least one edge…
   outside the source image rectangle" throw now names the offending
   corner, its mapped (x,y) value, the src rect, and the source quad
   corner for debugging (icl/filter/ImageRectification.cpp:106-115).

#### B. AffineOp backend rework (macOS/Accelerate)

Working on affine-op-demo revealed three distinct bugs in the Accelerate
LIN path. All in icl/filter/AffineOp_Accelerate.cpp.

1. **Rotation direction flipped vs NN** — vImage's matrix convention is
   empirically not what the docs imply. Guesswork (invert + row-vector,
   invert + column-vector) all broke centering. Working fix: decompose
   the forward matrix as an isotropic similarity (scale + angle + user
   translation), negate the angle, recompute the bbox-centering
   translation, feed through the original (known-centered) struct layout.
   Documented in the project_affineop_vimage.md memory.

2. **Lost user translation** — the decomposition was rebuilding the
   translation purely from ROI bbox, throwing away `op.translate(1, 0)`
   calls and breaking Filter.AffineOp.translate test. Fixed by separating
   user-translation from bbox-centering adjustment
   (AffineOp_Accelerate.cpp:71-75).

3. **ROI not respected** — vImage_Buffer described the full image, so
   clipToROI silently bled pixels outside the ROI. Fixed by pointing
   srcBuf at ROI origin (`s.getData(c) + roi.y*W + roi.x`), using ROI
   dims, and adjusting the translation by `A·(roi.x, roi.y) + t`
   (column-vector form). Initial sign error of `t − (roi.x, roi.y)` only
   worked for identity A — corrected to proper formula.

Separate cleanup: **AffineOp::apply** (icl/filter/AffineOp.cpp:91-95)
now consults `getClipToROI()` to choose the bbox region
(`src.getROI()` vs `src.getImageRect()`). `m_adaptResultImage` stays
orthogonal (used by TranslateOp + tests, unchanged).

#### C. Quick2 / QuickContext pool accounting

The pool's `currentUsage` (a `size_t`) was underflowing to ~2^44 during
icl-tests. Diagnosed with a new `setTracing(bool)` emitting per-event
alloc/resize/evict/unpooled traces to stderr, plus an `ERROR_LOG` +
clamp-to-0 guard that caught the underflow. Root cause: pool tracked
sizes via live `Image::memoryUsage()` queries at mutation time, which
desyncs if callers externally resize a handed-out buffer.

Workaround (not the real fix): **PooledBuffer{Image, size_t trackedBytes}**
pairs (icl/qt/QuickContext.cpp:62-64). Decrements use `trackedBytes`, not
live queries. Underflow vanishes.

Also added **`setThrowOnCapExceeded(bool)`** for strict-mode test assertions
— keeps stderr clean in icl-tests' `Quick2.Context.memoryCap` test.

**Real fix deferred**: replace the Image-holding pool with a generic
`utils::MemoryPool` of raw byte chunks + `shared_ptr` custom-deleter
release (captured in project_memorypool.md memory). Green-field refactor,
not worth interleaving with other work.

#### D. Filter demo audit (runtime verification)

All 10 filter demos from Session 41 had been *header-swap Done*. Session 42
ran each one end-to-end:

- ✅ affine-op (modernized), bilateral-filter-op, canny-op, convolution-op,
  dither-op, fft, gabor-op (modernized + parse<T>(pa()) fix),
  pseudo-color, temporal-smoothing
- ⏸ warp-op — needs camera-distortion pipeline to produce warp tables

Modernization touched affine-op.cpp (Img8u global → Image, static AffineOps
with explicit reset(), FPSLimiter, two orthogonal checkboxes + QuickDraw
ROI overlay) and gabor-op.cpp (std::vector<float>{x} over vec1 helper,
useDesired<T>(pa()) over parse<T>(pa())).

Minor cleanups: **canny-op** and **temporal-smoothing** had a useless
`update()` indirection (run() just called update()). Folded into run().

#### E. Common2.h + pre-existing gotchas

- **Common2.h** now `#include`s FPSLimiter.h transitively, matching the
  old Common.h convention. Stripped the redundant explicit include from
  25 demo/app files.
- **parse<T>(pa("…"))** throws at runtime — ProgArg's templated
  `operator T()` matches `operator std::string_view()` which recurses
  into `parse<std::string_view>` (not stream-extractable). Use
  `pa(...).as<T>()` or `useDesired<T>(pa(...))` instead. Present in
  ~20 files, worth a sweep later. Documented inline in gabor-op.cpp
  and in this guide.
- **vImage matrix convention** stays opaque; the Accelerate backend
  works for isotropic similarity transforms (ICL's overwhelmingly common
  case). Documented in project_affineop_vimage.md memory.

---

## Session 41 (Filter module audit + framework bug fixes)

### Session 41 Summary

Filter module audit complete. Three critical framework bugs found and fixed.
Color-segmentation app ported to geom2. All compiler warnings eliminated.

#### A. Filter demos/apps audit (all 10 demos + 4 apps checked)

- **Modernized** 8 files: affine-op, bilateral-filter-op, convolution-op,
  gabor-op, temporal-smoothing, filter-array, local-thresh, rectify-image
  (Image-based apply, smart pointers, removed raw ImgBase* buffers)
- **dither-op, fft, pseudo-color, canny-op** — already clean, no changes
- **warp-op** — fixed gui key bug (`gui["lin"]` → `gui["interpolation"]`)
- **bilateral-filter-op** — major cleanup: removed unused geom includes,
  eliminated template dispatch, simplified to Image pipeline
- **color-segmentation** — ported from geom::Scene to geom2::Scene2
  (GroupNode + CuboidNode children, MeshNode for axes, TextNode for labels,
  3 separate mouse handlers instead of widget-dispatch pattern)
- **local-thresh** — split into GUI app + headless `local-thresh-batch` CLI;
  fixed ROI clamping to image bounds, skip filter when ROI too small for mask
- **rectify-image** — fixed. Added typed `DataStore::Data::install(MouseHandler*)`
  overload that forwards to the `void*` version after implicit derived-to-base
  conversion — ensures the stored void* points to the MouseHandler subobject for
  multiply-inherited handlers (DefineQuadrangleMouseHandler etc.). Call site
  now passes `&mouse` (was `install(mouse)` which didn't compile).

#### B. clipped_cast UB fix (ClippedCast.h) — CRITICAL

`static_cast<icl8u>(-FLT_MAX)` is undefined behavior. On ARM/Apple Clang
with optimizations, this caused `clipped_cast<icl8u,icl32f>` to always
return `-FLT_MAX`, breaking ALL SSE-path color conversions framework-wide.
Rewritten with `if constexpr` dispatch: int→float just casts, float→int
clamps, int→int uses wide intermediary. Added 5 tests.

#### C. sse_for pointer underflow fix (SSEUtils.h)

`dstEnd - (step - 1)` wraps when image dim < step (16 pixels), causing
the SSE loop to process garbage. Fixed all 64 occurrences with safe guard:
`(dstEnd - dst0 >= step) ? dstEnd - (step - 1) : dst0`.

#### D. ColorSegmentationOp::lutEntry fix

- Loop start `a-rA` goes negative when `a=0`, step 256 skips all valid bins.
  Fixed with `std::max(0, ...)` clamping + precomputed end values.
- Missing `return` on the `fmt == m_segFormat` early path (fell through to
  double-conversion).
- `pow(2, shift)` → `1 << shift`.

#### E. Compiler warnings eliminated

- Added `override` to 30 methods in PointCloudObject.h, PCLPointCloudObject.h,
  PointCloudObjectBase.h
- Fixed `class`/`struct` ViewRay mismatch in Scene2.h, BVH.h, RayCastOctree.h
- Removed unused variables in SceneSynchronizer.cpp
- Debug build now produces zero warnings from ICL code

#### F. cc() color conversion tests

Added 3 tests: 1x1 image conversion, RGB→YUV→RGB roundtrip, large image
spot-check. Verified SSE path matches scalar path within ±1.

#### G. Build system

- Disabled ccache (was causing spurious rebuilds + stale .ninja_deps)
- Fixed corrupted .ninja_deps by deleting and rebuilding

### Session 40 Summary

Module-by-module audit of all demos/apps. Completed utils, core, math.
Major framework improvements along the way.

#### A. DataStore improvements

- `operator string_view()` — thread-local buffer for `parse<T>(gui["key"])`
- `operator T()` fallback — stream-extractable types try string→parse<T>
  when direct assignment fails (e.g. `cc(image, gui["fmt"])`)

#### B. PseudoColorConverter → PseudoColorOp

- Moved from core to filter as proper UnaryOp. `pseudo()` Quick2 function.
- Updated all 7 consumers (kinect demos, OptrisGrabber, etc.)

#### C. AbstractCanvas removed (unused outside its own demo)

#### D. Quick2 additions

- `create()` accepts `optional<depth>` for direct depth conversion
- `pseudo(image, stops, maxValue)` in QuickFilter
- `roi(Image&, Rect)` and `copyroi(Image&, Rect)` overloads
- AbstractPlotWidget default background → white

#### E. Bug fixes

- **DrawWidget::customPaintEvent** null PaintEngine crash on macOS Core Profile
- **QuadTree nn()**: rewrote with double-precision distance math; fixed AABB
  degeneration bug (integer halfSize/2 reaches 0 after ~9 levels); fixed
  queryAll() inner loop bound + missing root points
- **Renderer::invalidateCache()** — deferred GL cleanup to render thread
  (was calling glDelete* from run thread → crash)

#### F. Scene2 enhancements

- Inherits Configurable: background color, wireframe, enable lighting,
  point size, info properties. OSD button on Canvas3D.
- Thread safety: lock()/unlock() with recursive mutex, render() auto-locks
- **BVH::raycastToImage()** — CPU raycast to Img8u + Img32f depth buffer
- **RayCastOctree** (new, geom2) — Octree with rayCast/rayCastSort methods
- **DemoScene2::setupNatureScene()** — green ground, rocks, trees
- **raycast-octree demo** — BVH→pointcloud→octree, mouse probe highlights

#### G. Module audit results

- **utils**: done — configurable-info checked (no changes needed)
- **core**: done — colorspace modernized, pseudo-color moved to filter,
  canvas removed
- **math**: done — all 6 demos checked:
  - k-means: done (header swap)
  - llm-1D: cleaned up (constexpr, brace init)
  - llm-2D: fully ported from ImgQ to Image + planarToInterleaved
  - octree: removed → replaced by geom2/raycast-octree demo
  - quad-tree: cleaned up, QuadTree NN bug fixed
  - polynomial-regression: split into 3 demos (1D plot, 2D surface
    with Scene2, image approximation), all ported to Quick2/geom2

#### H. PCL integration

- Meson: manual include path extraction for Homebrew
  (meson strips Cellar paths as "system")
- PCL includes propagated through icl_geom_dep

### What's next (after Session 42)

**Module audit** — continue through remaining modules:
- [x] filter (demos: 9/10; apps: 4+1 batch) — warp-op deferred
- [ ] io (demos: 3; apps: 12)
- [ ] cv (demos: 12; apps: 6) — `lens-undistortion-calibration` also unlocks warp-op
- [ ] qt (demos: 8; apps: 7; examples: 2)
- [ ] geom (demos: 23; apps: 16)
- [ ] geom2 (demos: 6)
- [ ] markers (demos: 2; apps: 8)
- [ ] physics (demos: 8)

**Deferred framework cleanups** (all non-blocking, documented in memory):
- `utils::MemoryPool` refactor → QuickContext migrates to raw-byte chunks
  with shared_ptr custom-deleter release. Current `(Image, trackedBytes)`
  workaround fixes the observed underflow without restructuring anything.
  See `project_memorypool.md`.
- vImage matrix convention investigation — needs empirical pixel-diff
  testing against C++ fallback. Current backend's "decompose & negate"
  approach works for isotropic similarity transforms. See
  `project_affineop_vimage.md`.
- Sweep `parse<T>(pa("..."))` usages (~20 files) and convert to
  `pa(...).as<T>()` / `useDesired<T>(pa(...))`. They all throw at runtime
  but most haven't been exercised yet.
- CPP LIN backend in `AffineOp_Cpp.cpp` samples from outside ROI when
  bilinear neighborhood is within full-image bounds — falls back to
  ROI-respecting NN only at the edges. Pre-existing, inconsistent with
  NN behavior. Not the same path as the Accelerate backend so doesn't
  affect macOS users.

For each: compile, run, check if still useful/valid, modernize API usage,
remove if obsolete. See `porting-progress.md` for per-file status.

**Quick2 Phase 2 remaining** (ImgQ pixel access rewrites):
- signature-extraction demo — deleted (was the only demo left on ImgQ)
- 3 library files: Scene.cpp, FiducialDetectorPluginICL1.cpp, DrawWidget.h
- ~16 umbrella headers — trivial swap once library code is done
- Final: delete Quick.h/Quick.cpp, retire Common.h

**Scene2 open items**:
- Wire remaining Configurable properties to Renderer: "enable lighting"
  (needs Renderer toggle), "point size" (default override for all point clouds)
- Consider adding more properties: shadows toggle, SSR toggle, debug viz mode,
  exposure, ambient level

**Quick2 open items**:
- MorphologicalOp opening/closing crash via Quick2 `filter()` — pre-existing
- Pool byte-accounting desync — *workaround landed* Session 42:
  `(Image, trackedBytes)` per buffer + ERROR_LOG underflow clamp +
  `setTracing()` for diagnosis + `setThrowOnCapExceeded()` for tests.
  Real fix (raw-byte MemoryPool) still deferred — see `project_memorypool.md`.
- `ImgROI2`: store target ROI separately instead of modifying image (deferred)
- `.out("name")` on `Int`/`Float`/`String` GUI components only updates on
  Enter/returnPressed, not on every keystroke. The `.handle("name")` path
  reads the live widget value via `getValue()`. Consider removing `.out()`
  entirely or making it sync on every change.
- QuadTree: SF template parameter is dead weight with double math — remove

---

## Session 38 (Quick2 framework)

### Session 38 Summary (12 commits)

Complete implementation of Quick2 — the Image-based replacement for
Quick.h's float-only ImgQ API. Quick.h stays untouched for incremental
migration (Phase 2).

#### A. Quick2 Architecture (8 sub-files + multi-includer)

- **QuickContext**: memory-capped buffer pool (default 256 MB) with eviction,
  thread-local activation via `QuickScope` RAII, drawing state (color/fill/font)
  with push/pop stack, grabber cache, `applyOp(UnaryOp&/&&)` and
  `applyOp(BinaryOp&/&&)` for one-liner pool-backed op application
- **QuickCreate**: zeros, ones, load, create, grab — all return `Image` at
  native depth (no forced float conversion)
- **QuickFilter**: 18 functions (filter, blur, cc, rgb/hls/lab/gray, scale,
  channel, levels, thresh, copy, copyroi, norm, rotate, flipx, flipy) via
  `applyOp` + `poolCopy`/`poolConvert`
- **QuickMath**: arithmetic (+,-,*,/), math (exp,ln,sqr,sqrt,abs), logical
  (||,&&), bitwise (binOR/XOR/AND) — all via `BinaryArithmeticalOp`/
  `UnaryArithmeticalOp` + `applyOp`
- **QuickCompose**: concatenation (,/%/|) with depth promotion, `ImgROI2`
  with `shallowCopy()` for side-effect-free ROI operations
- **QuickDraw**: `DrawTarget<T,NC>` template with compile-time channel count,
  cached raw channel pointers, channel-outer loops for cache-friendly bulk ops.
  `withDrawTarget(image, lambda)` dispatches 5 depths × 3 NC = 15 variants
- **QuickIO**: save, show, print
- **Quick2.h**: multi-includer entry point

#### B. Framework additions

- `Image::memoryUsage()` — pool tracking
- `Image::shallowCopy()` — new ImgBase, shared pixels, independent metadata
- `Image::isExclusivelyOwned()` — `shared_ptr::use_count() == 1`, for pool
  safety. Documents two levels of sharing (ImgBase handle vs channel data)
- `UnaryOp::getDestinationParams()` / `BinaryOp::getDestinationParams()` —
  virtual, returns `pair<depth, ImgParams>`. `NeighborhoodOp` overrides to
  subtract mask margin. `prepare()` refactored to non-virtual, delegates to
  `getDestinationParams()`
- `LineSampler::forEach(a, b, callback)` — zero-allocation Bresenham with
  fully-inlined callback (engine moved to header `detail` namespace)

#### C. Critical bug fix — pool aliasing

`Image` copies share the same `ImgBase` via `shared_ptr`. The pool's
`isIndependent()` check tested channel-level sharing (SmartPtr use counts),
but both the pool entry and the returned Image share the same ImgBase object
— so channel SmartPtrs have use_count=1 even when the buffer is held externally.
Fix: `isExclusivelyOwned()` checks `shared_ptr::use_count() == 1`.

#### D. Test suite

528 tests in single `icl-tests` binary (384 existing + 144 Quick2).
7 test files: context, create, filter, math, compose, draw, io.
3 multithreaded stress tests (10 parallel workers each): arithmetic+drawing,
concurrent image drawing, pool isolation with 4MB cap.

#### E. Misc

- `CLDeviceContext` startup message changed from `std::cout` to `DEBUG_LOG`
- TODO: `BackendDispatching::addStateful` eagerly calls factory at static init
- Removed stale `Testing/` CTest artifact directory
- Plan document: `iclquick-plan.md`
- MorphologicalOp opening/closing crash via Quick2 `filter()` — pre-existing bug
- Consider `localThresh()` convenience function
- Pool memory accounting may drift over time (currentUsage counter) — add
  periodic reconciliation or compute from scratch

---

## Session 37 — Shadow mapping + soft shadows + renderer fixes

### Session 37 Summary (4 commits)

Ported shadow mapping from geom GLRenderer to geom2 Renderer, added
per-light soft shadows with Poisson disk PCF, fixed several rendering issues.

#### A. Shadow mapping pipeline

Shadow depth pass ported from geom `GLRenderer` (4× 2048² FBOs,
`sampler2DShadow` with `GL_COMPARE_REF_TO_TEXTURE` for hardware PCF).
Light VP matrix auto-computed from LightNode world position (lookAt toward
origin + 90° perspective). Per-light shadow slot mapping in PBR shader via
`uLightShadowSlot[8]` → `uShadowMatrix[4]` → `sampleShadow()`.

#### B. Per-light soft shadows

`LightNode::setSoftShadowRadius(float texels)` — 0 = hard (default),
\>0 = 16-sample Poisson disk PCF. Radius stored per shadow slot as
`uShadowSoftness[slot]` (converted from texels to UV space). Hard shadows
take the fast single-sample path with zero overhead.

DemoScene2: key light gets soft shadows (radius 3), top light hard shadows.
Overlay viewer has interactive shadow softness slider (0-20).

#### C. Rendering fixes

- **Scene2::render()**: letterbox viewport to preserve camera aspect ratio
  (was stretching when widget AR ≠ camera AR)
- **Billboard text Y-flip**: negate local Y column of billboard matrix
  (ICL projection uses Y-down image convention)
- **Billboard text unlit**: added `uUnlit` flag to PBR shader — when set,
  outputs baseColorMap directly without lighting (keeps alpha for transparency)
- **TextNodes excluded from shadow depth pass** (no box shadows from labels)
- **Light color auto-detection**: `collectLights()` handles both 0-1 and
  0-255 GeomColor ranges (LightNode defaults are 0-1, old demos use 0-255)
- **Renderer destructor**: properly cleans up GL resources (programs, FBOs, textures)

### Session 36 Summary (4 commits)

Complete SSR rewrite from broken world-space/texture-space approaches to
working view-space ray march. Added reflectivity uniform, improved demo
scene, fixed Cycles smooth shading bug.

#### A. SSR rewrite — view-space ray march

**Three failed approaches** before finding the working one:
1. World-space stepping + projected depth comparison — banding, wrong depths
2. Screen-space DDA + linear depth interpolation — depth is non-linear for
   rays toward camera (floor reflecting objects above), fundamentally broken
3. Texture-space linear march (ported from 3rdparty/SSR) — self-intersection
   from previous-frame depth mismatch, tight threshold issues

**Working approach**: view-space ray march with per-step projection:
- Transform fragment to previous frame's view space via `uPrevView`
- Step along reflection ray in view space (linear Z — no non-linear artifacts)
- At each step: project to screen via `uPrevProjection`, read depth buffer,
  reconstruct view-space Z via `uPrevInvProjection`, compare in linear space
- 8-iteration binary search refinement on sign change
- **4x supersampled**: traces 4 rays with stratified jitter offsets (0, 0.25,
  0.5, 0.75 of one step), confidence-weighted average eliminates Moiré ring
  artifacts on curved surfaces
- 256 steps, ray distance 4x camera depth, screen-edge + roughness fade
- SSR skipped for non-reflective surfaces (`reflectivity < 0.01 && !metallic`)

**New uniforms**: `uPrevView`, `uPrevProjection`, `uPrevInvProjection`
(uploaded alongside existing `uPrevVP`). Inverse projection computed per
frame via `Mat::inv()`.

**Depth buffer**: upgraded from `GL_DEPTH_COMPONENT24` to
`GL_DEPTH_COMPONENT32F` for better precision.

#### B. Reflectivity uniform

`uReflectivity` wired from `Material::reflectivity` to PBR shader. Controls
reflection strength: `reflFactor = max(envFresnel, vec3(uReflectivity))`.
At reflectivity=0, only Fresnel contributes; at 1.0, full mirror. Previously
the reflectivity field was ignored by the GL renderer.

#### C. Demo scene overhaul

**SSR test scene** (default when no `-scene` files given):
- RGB wireframe cube: 104 CuboidNode voxels forming 12 thick edges, corner
  colors from RGB cube (x,y,z → R,G,B), `smoothShading=false` for crisp edges
- Red sphere (90% reflective, roughness 0.15) — mirror-like SSR test
- Gold metallic sphere (metallic 0.9, roughness 0.35, reflectivity 0.3)
- Brown/black checkerboard ground (2x bigger, 50% reflective)
- Checkerboard back wall
- `-no-checkerboard` flag: flat black ground, no wall
- Camera near/far set tight to scene (20/6400, ratio 320:1) for depth precision

#### D. Cycles smooth shading fix

`SceneSynchronizer.cpp`: smooth shading now based on `Material::smoothShading`
only, not on presence of normals. Previously any geometry with normals
(including flat-faced cuboids) got smooth shading → rounded/glassy appearance.

### What's next

**SSR polish**:
- Step aliasing still visible at extreme close-up (could increase to 512 steps)
- Temporal accumulation: ping-pong feedback should naturally denoise over frames
- Consider Hi-Z acceleration for performance (hierarchical depth mip chain)

**Quick2 framework** (Phase 1 complete, Phase 2 next):
- Quick2.h fully implemented: QuickContext (memory-capped pool, thread-local
  activation, applyOp), QuickCreate, QuickFilter, QuickMath, QuickCompose,
  QuickDraw (DrawTarget<T,NC> optimized), QuickIO. 144 tests passing.
- Phase 2: migrate consumers from Quick.h → Quick2.h one file at a time
  (demos first, then library code). See `iclquick-plan.md` for full list.
- Key architectural additions: Image::memoryUsage(), Image::shallowCopy(),
  Image::isExclusivelyOwned(), UnaryOp/BinaryOp::getDestinationParams(),
  LineSampler::forEach(), QuickContext::applyOp(UnaryOp&&/BinaryOp&&).
- Pool bug found+fixed: isIndependent() vs isExclusivelyOwned() — two levels
  of Image sharing (ImgBase handle vs channel pixel data).
- MorphologicalOp opening/closing crash via Quick2 filter() — pre-existing bug.

**BackendDispatching TODO**:
- `addStateful` eagerly calls factory() at static init (registration time),
  triggering CLDeviceContext/CLProgram construction before main(). For OpenCL
  backends this causes heavyweight GPU driver calls during library loading.
  Fix: defer first factory() call to first resolve/apply. See TODO in
  `icl/utils/BackendDispatching.h:addStateful`.
- CLDeviceContext should be a per-thread singleton instead of created per-
  CLProgram. Currently ~30 CLDeviceContext instances created during test runs.

**Signature extraction demo** (`icl/cv/demos/signature-extraction.cpp`):
- Uses RotateOp, LocalThresholdOp, RegionDetector, blur, Quick.h functions
- Currently crashes — needs debugging (likely RegionDetector on filtered
  binary image, or ImgQ channel mismatch in the `scaled | blurred` concat)
- GUI: rotation, local threshold (mask size + global offset), min region
  size filter, alpha blur, scale-down factor, save PNG with transparency
- Uses `executeInGUIThread()` for save dialog (blocking, from worker thread)

**Shadow maps** (DONE — Session 37):
- Full pipeline: depth FBOs, shadow depth pass, PBR integration, soft PCF
- See Session 37 summary above for details
- TODO: shadow camera direction/FOV controls on LightNode,
  directional light ortho projection, PCSS (variable penumbra),
  debug visualization mode for shadow maps

**geom2 API cleanup**:
- Scene2 getters: return references or shared_ptrs instead of raw pointers
- `getGLCallback()`: return raw ptr instead of shared_ptr (avoid `.get()`)
- CoordinateFrameSceneObject: add PIMPL

**Other work**:
- CI update — meson in GitHub Actions
- ImageMagick 7 / FFmpeg 7+ rewrites
- ConvolutionOp IPP mixed-depth

---

## Previous State (Session 34 — Matrix migration + Cycles geom2 integration)

### Session 34 Summary (8 commits)

Two major areas: completed the matrix (row,col) convention migration across the
entire codebase, and wired Cycles renderer into geom2 with demos.

#### A. Matrix (row,col) migration — COMPLETE

**Full migration**: `operator()(col,row)` → `operator()(row,col)` across 67
files, ~1200 call sites. Three-phase approach:
1. Remove `operator()`, add `index_yx(row,col)` to both `FixedMatrix` and
   `DynMatrixBase`. Automated via `scripts/fix-matrix-indexing.py` (column-based)
   and new `scripts/fix-matrix-indexing2.py` (regex inside-out, handles nested
   calls, deref patterns, chained calls). Iterated build→fix→rebuild until clean.
2. Migrate `at(col,row)` → `at(row,col)` (5 files, ~60 sites).
3. Rename `index_yx` back to `operator()` — clean `M(row, col)` syntax restored.

**False positives caught**: `pow.index_yx`, `atan2.index_yx`, `std::swap.index_yx`,
`.mult.index_yx`, `setSamplingResolution.index_yx`, `Point32f.index_yx`,
`ICL_TEST_EQ.index_yx` — all fixed. Two semantic bugs caught in geom module
review (Camera.cpp principal point, Posit.cpp atan2 arg order).

**384/384 tests pass** after migration.

#### B. Cycles renderer wired into geom2

**Meson build wiring** (`icl/geom2/meson.build`): `CyclesRenderer.cpp` and
`SceneSynchronizer.cpp` now compiled when `cycles_found`. Reuses include paths,
compile flags, and link libraries from geom module's meson config.

**SceneSynchronizer fixes** for current Cycles 4.x API:
- `unique_ptr<ShaderGraph>` for `set_graph()`
- Removed `graph->add()` (nodes auto-added via `create_node`)
- `PointLight` instead of generic `Light`
- Camera FOV uses `getSamplingResolutionY()` + `compute_auto_viewplane()`
- Light color normalized from 0-255 to 0-1 before applying physical intensity
- Gradient sky background added (zenith/horizon/ground blend)
- Analytic sphere path disabled (known offset bug), always tessellates

---

## Previous State (Session 32 — Material refactoring + geom2 scene graph)

### Session 32 Summary (23 commits)

Two major areas of work:

## Previous State (Session 33 — PointCloud, BVH, textures, text, mouse interaction)

Session 33 added PointCloud, PointCloudNode, BVH raytracer, texture/text
rendering, mouse interaction to geom2. Started matrix indexing migration
(added `index_yx`, migrated math module). 5 commits.

### Session 32 Summary (23 commits)

#### A. Material & deprecated warning cleanup (geom module, 7 commits)

**Material restructured** with lazy sub-structs:
- `TextureMaps` behind `shared_ptr` (null for untextured objects, saves ~80 bytes)
- `TransmissionParams` behind `shared_ptr` (null for opaque objects)
- New fields: `lineColor`, `pointColor`, `pointSize`, `lineWidth`
- Non-copyable with explicit `deepCopy()` returning `shared_ptr<Material>`
- New factory: `fromColors(faceColor, wireColor)` for mixed face+wire colors

**SceneObject copy semantics fixed:**
- `operator=` deleted, `copy()` renamed to `deepCopy()`
- Protected copy ctor preserved for subclass deepCopy()
- Old `operator=` was buggy (missing material, reflectivity, emission)
- All PointCloud subclasses updated: `copy()` → `deepCopy()` with override

**GLRenderer line/point rendering:**
- New unlit shader (GLSL 410) for GL_LINES + GL_POINTS
- Separate VAOs in GLGeometryCache for lines and points
- Lines and points now visible in GLRenderer (were silently dropped)

**All 77 deprecated setColor/setShininess calls migrated** across 26 files.

**Bugs fixed:**
- Analytic sphere overlay offset: DemoScene auto-scale modified vertices
  but not `m_sphereCenter`/`m_sphereRadius`. Fixed by updating sphere params
  after transforms + adding `setSphereParams()` setter.

#### B. geom2 — Clean scene graph module from scratch (16 commits)

**Why:** SceneObject was a 940-line monolith (geometry + scene graph + materials +
transforms + rendering + physics base). Every incremental refactor tangled in
backward compatibility. geom2 is a greenfield design in `icl/geom2/` namespace
`icl::geom2`, independent of ICLGeom.

**Node hierarchy (clean separation of concerns):**
```
Node (abstract)              ← transform + visibility + name + locking. PIMPL'd.
├── GroupNode                ← has children. No geometry. Pure container.
├── GeometryNode (abstract)  ← read-only geometry + material (renderer interface)
│   ├── MeshNode             ← mutable geometry leaf (addVertex, getVertices()&)
│   ├── SphereNode           ← parametric sphere (no mutable vertices)
│   ├── CuboidNode           ← parametric box
│   ├── CylinderNode         ← parametric cylinder
│   └── ConeNode             ← parametric cone
├── LightNode                ← point/directional/spot, position from transform
└── CoordinateFrameNode      ← GroupNode with 3 CuboidNode axes (R=X, G=Y, B=Z)
```

**Key design rules:**
- GroupNode has children. Leaf nodes (MeshNode, SphereNode, LightNode) do not.
- GeometryNode provides read-only const access for renderers.
- MeshNode adds mutable access (for physics/dynamic geometry).
- Parametric shapes inherit GeometryNode (NOT MeshNode) — no mutable vertex access.
- All nodes PIMPL'd with `unique_ptr<Data>`, Rule of 5 (copy+move).
- No raw pointer ownership. `shared_ptr` everywhere.
- No display lists. No legacy GL. Core profile only.
- `MeshNode::ingest(MeshData{...})` for zero-copy bulk geometry loading.

**Components implemented:**
- `Node.h/.cpp` — abstract base (~120 lines)
- `GroupNode.h/.cpp` — children container (~80 lines)
- `GeometryNode.h/.cpp` — read-only geometry + material (~230 lines)
- `MeshNode.h/.cpp` — mutable geometry + `ingest()` (~120 lines)
- `Primitive.h` — Line/Triangle/Quad as plain structs (~50 lines)
- `SphereNode`, `CuboidNode`, `CylinderNode`, `ConeNode` — parametric shapes
- `CoordinateFrameNode` — ported from geom's CoordinateFrameSceneObject
- `LightNode` — point/directional/spot with color+intensity
- `Renderer.h/.cpp` — GL 4.1 Core, PBR shader + unlit shader, multi-light (~530 lines)
- `Scene2.h/.cpp` — scene manager with cameras, shared_ptr ownership, GL callback
- `Loader.h/.cpp` — .obj + .glb/.gltf file loading via `ingest()`
- `Raytracer.h`, `CyclesRenderer.h`, `SceneSynchronizer.h/.cpp` — Cycles scaffolding
  (headers ready, .cpp adapted but not compiled — needs Cycles build wiring)
- `PORTING.md` — geom → geom2 migration guide
- `demos/geom2-hello.cpp` — working demo with all shape types + lighting

**Total: 24 files, ~4800 lines, zero warnings.**

**Material is shared** between geom and geom2 (`icl::geom::Material` used by both).

### What's next for geom2

Remaining items from PORTING.md:
- **PointCloudNode** — replaces PointCloudObjectBase (important for CV visualization)
- **Mouse interaction / picking** — hit testing against the scene graph
- **Text primitives / billboard text** — needed for labels and debug overlays
- **Texture primitives** — textured quads
- **Cycles build wiring** — connect CyclesRenderer.cpp + SceneSynchronizer.cpp to Cycles
  libraries in geom2/meson.build (headers and adapted .cpp are ready)
- **ComplexCoordinateFrameNode** — cones + cylinders + text labels
- **More demos** — file loading demo, interactive mouse demo

### Build

```bash
meson setup build --buildtype=debug --wipe
CCACHE_DISABLE=1 meson compile -C build -j16
bin/geom2-hello-demo   # first geom2 visual demo
```

---

## Previous State (Session 31 — Meson build system, directory restructure)

### Session 31 Summary

**Build system migrated from CMake to Meson+Ninja:**

The entire project structure was overhauled in one session. 9 commits.

**Directory restructure:**
- `ICLGeom/src/ICLGeom/Scene.h` → `icl/geom/Scene.h` (matches `icl::geom` namespace)
- All 10 modules renamed: `ICLUtils→icl/utils`, `ICLMath→icl/math`, etc.
- `src/` wrapper dropped — headers and sources coexist directly
- Include paths: `#include <icl/geom/Scene.h>` (was `<ICLGeom/Scene.h>`)
- Library names: `libicl-geom.dylib` (was `libICLGeom.dylib`)

**Demos/apps flattened:**
- 120+ single-file subdirectories eliminated (each had its own CMakeLists.txt)
- Now: `icl/filter/demos/canny-op.cpp` (was `ICLFilter/demos/canny-op/canny-op.cpp`)
- Multi-file targets use naming convention: `camera-calibration-CalibrationGrid.cpp`

**Meson build system:**
- `meson.build` + `meson.options` replace ~170 CMakeLists.txt + 29 cmake modules
- 10 module `meson.build` + 10 target `meson.build` files
- All dependencies detected: Qt6, OpenCV, Eigen3, OpenCL, ImageMagick, FFmpeg,
  Bullet, Accelerate, Cycles (full integration with 21 compile defs, 20+ include
  dirs, 14 static + 18 shared libs)
- Config headers generated without `.in` templates (Meson `configure_file()`)
- Qt MOC via `qt6.compile_moc()` — trimmed to 11 headers with actual Q_OBJECT
- PCH headers in `icl/_pch/` (required: macOS case-insensitive FS shadows
  system `<time.h>` with our `Time.h` — fixed via `implicit_include_directories: false`)
- Post-build hook symlinks all executables into `build/bin/`

**Build result:** 122 targets (10 libs + 68 demos + 35 apps + 8 examples + 1 test)
all compile and link. Tests pass.

**Dead code removed:**
- `VideoGrabber.cpp/.h` — xine dependency no longer exists
- Old CMake system: 518 files deleted (45K lines)
- Old `ICL*` module directories removed
- `ICLExperimental/` removed (Cycles demos moved to `icl/geom/demos/`)

**Scenes and assets:** moved to `icl/geom/scenes/` (9 glb/obj files),
doc images moved into `icl/*/doc/`.

**Key Meson commands:**
```bash
meson setup build                              # configure
meson setup build -Ddemos=true -Dapps=true     # with demos+apps
meson compile -C build -j16                    # build
meson test -C build                            # run tests
meson configure build -Dtests=true             # reconfigure option
```

**Known disabled targets (pre-existing code issues, not build system):**
- 3 OpenCV legacy C API files (`OpenCVCamCalib`, `LensUndistortionCalibrator`,
  `OpenSurfLib`) — need rewrite for OpenCV 4+
- `TemplateTracker` — depends on IPP-only `ProximityOp`
- `corner-detection-css` demo — uses removed `DebugInformation` API
- `heart-rate-detector` demo — needs `ICL_OPENCV_INSTALL_PATH` define
- `octree` demo — needs PCL
- `point-cloud-primitive-filter` app — needs dead RSB dependency

### Next Steps

**Immediate:**
- **CI update** — `.github/workflows/ci.yaml` needs `pip install meson ninja`
  and `meson setup/compile/test` instead of cmake
- **CLAUDE.md update** — build instructions reference cmake, need meson equivalents
- **Docker scripts** — `scripts/docker/` needs meson adaptation

**Build system polish:**
- OpenCL kernel header generation (`scripts/cl2header.py` + `custom_target` in
  `icl/filter/meson.build`) — currently no `.cl` kernels are being compiled
- Fix disabled targets: rewrite OpenCV C API files for OpenCV 4+
- Add `subprojects/*.wrap` for Windows builds (zlib, libpng, libjpeg, gtest)

**Other work (unchanged from Session 30):**
- **ConvolutionOp IPP: mixed-depth support**
- **Benchmark IPP backends**
- **ImageMagick 7** — rewrite for Quantum/Pixels API
- **FFmpeg 7+** — rewrite LibAVVideoWriter.cpp for modern API

## Previous State (Session 30 — Bilinear fix, LAPACK helpers, IPP backend implementations)

### Session 30 Summary

**C++ bilinear scaling bug fixed (`Img_Cpp.cpp`):**
- Root cause: formula used `(src-1)/dst` instead of `(src-1)/(dst-1)` for the
  bilinear mapping. For identity scale (8→8), this gave `fSX = 7/8 = 0.875`
  instead of `1.0`, producing wrong pixel values (e.g., 8.75 instead of 10).
- Also fixed edge clamping: the old code clamped the base index `xll`, which
  gave the wrong pixel at the last row/column. Now clamps the +1 neighbor
  index (`x1 = min(x0+1, maxX)`) while keeping fractional weights correct.
- Test `Img.scaledCopy_identity` now passes on all platforms.

**LAPACK transpose helpers migrated to Accelerate and MKL backends:**
- Replaced 28 manual transpose loops in `LapackOps_Accelerate.cpp` and
  `LapackOps_Mkl.cpp` with `lapack_row_to_col` / `lapack_col_to_row` calls.
- Added vector-returning overload `auto AT = lapack_row_to_col(A, M, N, lda)`
  that allocates and returns the transposed buffer. Eliminates manual sizing.
- GELSD backward transpose kept manual (BT stride `mx` ≠ row count `N`).
- Net -78 lines across both backend files.

**5 IPP backends implemented (Docker-verified, 384/384 tests pass with IPP+MKL):**

| File | IPP Functions | Depths | Notes |
|------|--------------|--------|-------|
| UnaryArithmeticalOp_Ipp.cpp | ippiAddC/SubC/MulC/DivC + ippiSqr/Sqrt/Ln/Exp/Abs | 8u/16s/32f | withVal + noVal ops |
| LUTOp_Ipp.cpp | ippiReduceBits (modern signature with buffer param) | 8u | noise=0, ippDitherNone |
| MedianOp_Ipp.cpp | ippiFilterMedianBorder | 8u/16s | Fixed + generic sizes |
| ConvolutionOp_Ipp.cpp | ippiFilterBorder (spec-based) | 8u/32f | Odd kernels only; mixed-depth/even delegates to C++ |
| AffineOp_Ipp.cpp | ippiWarpAffineNearest/Linear (spec-based) | 8u/32f | NN + bilinear interpolation |

**MorphologicalOp_Ipp.cpp remains a stub:** IPP 2022.3 removed the general
morphology border API (ippiMorphologyBorderGetSize/Init, ippiDilateBorder,
ippiErodeBorder). Only `ippiDilate3x3_64f_C1R` / `ippiErode3x3_64f_C1R`
remain. The C++ backend handles all morphology; Accelerate provides
vImageDilate/Erode for 8u/32f on macOS.

**ConvolutionOp IPP limitations:** IPP's `ippiFilterBorder` uses center-anchored
convolution, which doesn't match ICL's configurable anchor for even-sized kernels.
Also, mixed-depth cases (8u→16s for Sobel) require `ippiFilterBorder_8u16s_C1R`
which isn't implemented yet. The IPP backend checks for odd kernel + same depth
and delegates all other cases to the C++ backend via explicit `get(Backend::Cpp)`.

**Docker Dockerfile updated:** pinned `intel-oneapi-mkl-devel-2022.2.1` to match
IPP 2022.3 era (avoids potential symbol conflicts between MKL 2025 and IPP 2022).
Confirmed: IPP+MKL work correctly together — earlier "MKL crash" was stale build
artifacts from bisection testing, not a real incompatibility.

**Build: 384/384 tests pass on macOS and Docker Linux (IPP+MKL).**

### Next Steps

**Immediate:**
- **ConvolutionOp IPP: mixed-depth support** — implement `ippiFilterBorder_8u16s_C1R`
  for 8u→16s (Sobel, Laplace) and even-kernel anchor alignment
- **Benchmark IPP backends** — measure speedup vs C++ for the 5 new IPP ops

**Experimental — Raytracing:**
- Real-time raytracer in `ICLExperimental/Raytracing/` — see
  [continue-raytracing.md](ICLExperimental/Raytracing/continue-raytracing.md)
  for full details. CPU backend working (BVH + OpenMP + reflections). Next: Metal RT
  backend for hardware-accelerated raytracing on Apple Silicon.

**Other work:**
- **ImageMagick 7** — rewrite FileGrabberPluginImageMagick.cpp and
  FileWriterPluginImageMagick.cpp for Quantum/Pixels API
- **FFmpeg 7+** — rewrite LibAVVideoWriter.cpp for modern API
- **OpenCL on macOS** — bundle Khronos cl2.hpp header for C++ bindings
- **Linux benchmarks on real x86** — Docker Rosetta benchmarks are directionally
  useful but not reliable for absolute numbers

## Previous State (Session 29 — IPP/MKL Docker verification, LAPACK bug fixes)

### Session 29 Summary

**Docker IPP+MKL build infrastructure:**
- Added `intel-oneapi-ipp-devel` to Dockerfile alongside MKL
- Enabled `-DBUILD_WITH_IPP=ON -DBUILD_WITH_MKL=ON` in docker-test.sh
- Fixed CMake IPP detection: removed dead `iomp5` (Intel OpenMP) and `ippm`
  (deprecated matrix lib) from required libs. IPP doesn't need Intel OpenMP;
  system OpenMP works fine.
- MKL: switched from `mkl_intel_thread`+`iomp5` to `mkl_sequential` (avoids
  Intel OpenMP dependency; ICL's own OpenMP handles parallelism)
- Added oneAPI search paths for MKL
- Fixed `--cpus` in docker-test.sh to use Docker's available CPU count
- Fixed PugiXML.cpp: `#include "pugixml.hpp"` → `"PugiXML.h"` (pugixml.hpp
  was never in git; PugiXML.h IS the header, renamed to ICL convention)

**All 6 real _Ipp.cpp files compile against oneAPI IPP — zero API changes needed.**
APIs verified available in modern oneAPI: ippiMirror, ippiSet, ippiLUTPalette,
ippiMax/Min/Mean, ippiMulC/AddC, ippiCopyReplicateBorder, ippiCopy P↔C,
ippiThreshold, ippiCompareC, ippiAndC/OrC/XorC/Not, ippiRemap, ippiFilterWiener.

**Bug found: cpp_getri (C++ LU inverse) produced completely wrong results.**
- The in-place U-inversion algorithm used `A[i][i]` which had already been
  overwritten to `1/U[i][i]` in a prior iteration. Errors were 20-500x, not
  precision issues.
- Hidden because on macOS the Accelerate backend (higher priority) was always
  selected — the C++ fallback was never exercised.
- Discovered by running tests in Docker where only the C++ backend is available.
- Fix: rewrote to solve `A*x=e_j` column-by-column using the LU factorization
  directly (forward-substitute with L, then back-substitute with U).

**Bug found: MKL geqrf/orgqr/getrf/getri row-major handling was wrong.**
- MKL backend used a dimension-swap trick (passing swapped M,N to LAPACK) to
  handle row-major → column-major. This works for symmetric operations (SVD,
  eigenvalue) but NOT for QR: QR(A^T) ≠ QR(A).
- Caused Docker test `math.dyn.qr_reconstruct` to fail with errors of 17+
  and subsequent heap corruption from wrong Q/R propagating.
- Fix: all four functions now use explicit transposition (matching the
  Accelerate backend pattern). `gelsd` already had this correct.

**Transpose helpers added to LapackOps:**
- `lapack_row_to_col(A, M, N, lda, AT)` and `lapack_col_to_row(AT, M, N, A, lda)`
  centralize the row-major ↔ column-major pattern used by all LAPACK backends.
- Defined in LapackOps.cpp, declared in LapackOps.h, instantiated for float/double.
- Backend callers to be migrated to use helpers in a future pass.

**C++ QR fallback (cpp_geqrf, cpp_orgqr) verified correct.** Standalone testing
with the exact Wikipedia 3×3 matrix from the test suite, plus 4×3, 5×5, and
double precision — all produce correct Q*R=A reconstruction within float epsilon.
Also verified cpp_gelsd (SVD least-squares solve) is correct.

**Pre-existing bug: C++ bilinear scaling produces wrong results for identity scale.**
- Test `Img.scaledCopy_identity` fails in Docker (383/384 pass without MKL).
- Scales 8×6 → 8×6 with `interpolateLIN`, gets `8.75` for pixel value `10`.
- The value `8.75 = 0.875 * 10` suggests `fSX = 7/8 = 0.875` (wrong) instead
  of `fSX = 7/7 = 1.0` (correct for identity scale with the `(src-1)/(dst-1)` formula).
- Investigation in `Img_Cpp.cpp` shows the bilinear code at line 394 computes
  `fSX = (srcSize.width - 1) / (dstSize.width - 1) = 7/7 = 1.0` — which is correct.
- The `scaledCopy` call path passes `getSize() → srcSize` and `poDst->getSize() →
  dstSize`, both `(8,6)` — confirmed correct.
- Cannot reproduce on macOS because Accelerate backend (vImageScale, Lanczos)
  takes priority. Need Docker debug session with print statements in
  `cpp_scaledCopyChannel` to see actual argument values at runtime.
- Hypotheses: (1) some other code path is being called that computes fSX
  differently, (2) the ImgOps dispatch wraps sizes differently, (3) ROI
  handling mutates the sizes before the backend sees them.
- TODO: Add `fprintf(stderr, ...)` debug prints to `cpp_scaledCopyChannel` in
  a Docker shell session to trace actual argument values.

**Docker test results (IPP only, no MKL): 383/384 pass.**
Remaining failure is the bilinear scaling bug above. With MKL: QR and
inverse tests now pass after the fixes, but MKL+Rosetta causes heap
corruption in some LAPACK paths (genuinely Rosetta — the MKL code is
now correct per our analysis). Need real x86 Linux for full MKL verification.

**Build: 100% clean (zero warnings), tests: 384/384 pass on macOS.**

### Next Steps

**Immediate:**
- **Fix C++ bilinear scaling bug** — Docker debug session needed
- **Migrate LAPACK backends to transpose helpers** — replace manual loops
  in Accelerate, MKL, Eigen backends with `lapack_row_to_col`/`lapack_col_to_row`

**IPP stubs (6 files, incremental performance):**
- AffineOp_Ipp.cpp — rewrite with modern `ippiWarpAffineNearest`/`Linear` + spec init
- ConvolutionOp_Ipp.cpp — rewrite with `ippiFilterBorder_*` (spec-based API)
- MorphologicalOp_Ipp.cpp — rewrite with `ippiMorphInit_*` + spec-based API
- MedianOp_Ipp.cpp — rewrite with `ippiFilterMedianBorder_*`
- LUTOp_Ipp.cpp — update `ippiReduceBits` signature (added noise param)
- UnaryArithmeticalOp_Ipp.cpp — write registrations for `ippiAddC/MulC/SubC/DivC`

**Other work:**
- **ImageMagick 7** — rewrite FileGrabberPluginImageMagick.cpp and
  FileWriterPluginImageMagick.cpp for Quantum/Pixels API
- **FFmpeg 7+** — rewrite LibAVVideoWriter.cpp for modern API
- **OpenCL on macOS** — bundle Khronos cl2.hpp header for C++ bindings
- **Linux benchmarks on real x86** — Docker Rosetta benchmarks are directionally
  useful but not reliable for absolute numbers or MKL verification

## Previous State (Session 28 — C++17 Phases 2-6: complete modernization)

### Session 28 Summary

**C++17 Phases 2-6 — mechanical and structural modernization across 183 files:**

Completed the entire C++17 modernization plan (phases 2-6) in a single session.
All changes are zero-semantic-impact except where noted (string_view API, transparent
comparators). Build clean (zero warnings), tests: 384/384 pass throughout.

**Phase 2 — scoped_lock, structured bindings, if-init, [[maybe_unused]] (118 files):**

- `std::scoped_lock` (75 files, 281 replacements): all `std::lock_guard` replaced.
  GradientImage.cpp and DC.cpp manual lock()/unlock() converted to RAII scoped_lock.
- Structured bindings (10 files, ~20 conversions): map iteration loops converted to
  `auto [key, value]` syntax. Pair unpacking in GenericGrabber (`split_at_first`).
  Files: ConfigFile, UnaryOp, GUI, Widget, PointCloudObjectBase,
  PhysicsPaper3ContextMenu, ManipulatablePaper, CCFunctions, GenericGrabber,
  ProcessMonitor.
- If-with-initializer (25 files, ~40 conversions): `find()`+`if` patterns converted to
  `if(auto it = find(); it != end())`. Files: Configurable, ConfigFile, Size,
  UnaryOp, Grabber, FileGrabber, GenericGrabber, Scene, MarkerGridDetector,
  PointCloudObjectBase, Benchmark, SignalHandler, and more.
- `[[maybe_unused]]` (33 files, ~52 conversions): `(void)var` casts replaced with
  `[[maybe_unused]]` on declarations/parameters. Includes 14 ICLFilter Op files
  (static init pattern), function parameters, catch variables, conditional vars.

**Phase 3 — std::filesystem (2 files):**

- `File.cpp`: `break_apart()` rewritten with `fs::path` methods (parent_path, stem,
  extension). Fixes pre-existing bug where `.gz` double-extension handling truncated
  the last character of basename (`substr(0,p-1)` → correct via `fs::path::stem`).
  `file_exists()` → `fs::exists()`, `file_is_dir()` → `fs::is_directory()`,
  `erase()` → `fs::remove()`. Removed `<sys/stat.h>` and `DIR_SEPERATOR`.
- `FileList.cpp`: eliminated all three platform-specific glob implementations
  (`wordexp` Linux, `glob` macOS, `FindFirstFile` Windows). Replaced with
  `fs::directory_iterator` + `std::regex_match`. Added tilde expansion via `$HOME`,
  deterministic sort, no-wildcard fast path. Removed `<wordexp.h>`, `<glob.h>`,
  `<windows.h>`, `<cstring>`.

**Phase 4 — std::from_chars / std::to_chars (7 files):**

- Integer parsing: `atoi()` → `std::from_chars` in IntHandle, GUIDefinition,
  FileGrabberPluginCSV, TestImages, JPEGDecoder.
- Float parsing: `atof()` / `istringstream` → `std::strtof` / `std::strtod` in
  FloatHandle, GUIDefinition, FileGrabberPluginCSV, StringUtils (parse_icl32f/64f).
  Apple Clang libc++ lacks `from_chars` for floats; `strtof`/`strtod` are
  locale-independent and avoid istringstream allocation overhead.
- Integer formatting: `snprintf` → `std::to_chars` in StringUtils (i2str, time2str).
  Buffer sizes use `std::numeric_limits<T>::digits10 + 3` for portability.

**Phase 5 — if constexpr cleanup (3 files):**

- `SimdCompat.h`: added non-Apple scalar fallbacks for `add`/`sub`/`smul` in
  `simd_compat` namespace (compiler auto-vectorizes at -O3).
- `FixedMatrix.h`: removed all 9 `#ifdef ICL_HAVE_APPLE_SIMD` blocks in element-wise
  operators (*, /, +, -, negate). Each replaced with a direct `simd_compat::` call
  that works on all platforms — Apple SIMD for 4x4/2x2, scalar fallback otherwise.
- `BackendDispatching.h`: 6 `enable_if` SFINAE constraints replaced with
  `static_assert` (cleaner error messages, `_v` trait aliases).

**Phase 6 — std::string_view + transparent map comparators (52 files):**

- `StringUtils.h/cpp`: `tok`, `toLower`, `toUpper`, `parse<T>`, `startsWith`,
  `endsWith`, `match`, `skipWhitespaces`, `analyseHashes`, `to8u/16s/32s/32f/64f`
  all take `std::string_view` instead of `const std::string&`. Deleted dangerous
  `parse<const char*>` specialization. Functions needing null-terminated strings
  (`strtof`, `regcomp`) construct `std::string` internally.
- `StrTok.h/cpp`: constructor takes `string_view`, internal tokenization updated.
- Transparent map comparators (`std::less<>`) added to all `std::map<std::string,...>`
  declarations across 48 files, enabling heterogeneous lookup with `string_view` keys
  without allocating temporary `std::string`. Modules: ICLUtils (Configurable,
  ConfigFile, ParamList, PluginRegister, MultiTypeMap, Benchmark, IppInterface),
  ICLCore (Color), ICLFilter (UnaryOp), ICLIO (Grabber, FileWriter, GenericGrabber,
  DCDeviceFeatures, SharedMemorySegment, OpenNIUtils, V4L2Grabber), ICLGeom
  (PointCloudObjectBase, PointCloudSerializer, Primitive3DFilter, PCDFileGrabber,
  Scene, DepthCameraPointCloudGrabber), ICLQt (Widget, GUI, Quick, MultiDrawHandle,
  DefineRectanglesMouseHandler, IconFactory), ICLPhysics (ManipulatablePaper,
  PhysicsPaper3ContextMenu).

**Build: 100% clean (zero warnings), tests: 384/384 pass.**

### C++17 Modernization — Final Status

| Phase | What | Files | Status |
|---|---|---|---|
| 1 | Nested namespaces, std::clamp, [[nodiscard]] | 821 | Done (session 27) |
| 2 | scoped_lock, structured bindings, if-init, [[maybe_unused]] | 118 | Done (session 28) |
| 3 | std::filesystem (File.cpp, FileList.cpp) | 2 | Done (session 28) |
| 4 | std::from_chars / std::to_chars | 7 | Done (session 28) |
| 5 | if constexpr (SimdCompat, BackendDispatching) | 3 | Done (session 28) |
| 6 | std::string_view + transparent map comparators | 52 | Done (session 28) |
| 7 | std::optional | ~10 | Deferred — do opportunistically |

Phase 7 (std::optional) is low-priority. The guide recommends doing it when touching
the relevant APIs for other reasons (MarkerGridDetector::getPos, ImgBase::getMax/Min,
FilenameGenerator error returns).

### Next Steps

**Remaining non-C++17 work:**
- **ImageMagick 7** — rewrite FileGrabberPluginImageMagick.cpp and
  FileWriterPluginImageMagick.cpp for Quantum/Pixels API
- **FFmpeg 7+** — rewrite LibAVVideoWriter.cpp for modern API
- **OpenCL on macOS** — bundle Khronos cl2.hpp header for C++ bindings
- **Re-enable IPP backends** on Linux — update to modern oneAPI APIs
- **Linux benchmarks on real x86** — Docker Rosetta benchmarks are directionally
  useful but not reliable for absolute numbers. Run on CI or cloud x86 VM.

## Previous State (Session 27 — C++17 Phase 1: namespaces, std::clamp, [[nodiscard]])

### Session 27 Summary

**C++17 Phase 1 — mechanical modernization across 821+ files:**

Three categories of changes, all purely mechanical with zero semantic impact:

**1. Nested namespace fusion (821 files):**
- Fused `namespace icl { namespace X {` → `namespace icl::X {` across the entire
  codebase using `scripts/fuse-namespaces.py`. Script handles the standard case
  (outer `namespace icl {` on one line, inner `namespace X {` on the next).
- 11 files with non-standard patterns fixed manually:
  - Sibling namespaces (`namespace icl { namespace qt {...} namespace geom {...} }`)
    in Scene.h, ShaderUtil.h, SceneLight.h, PhysicsWorld.h, PhysicsUtils.h, Quick.h,
    ConvexHull.cpp, DCDeviceFeatures.cpp — split into separate `namespace icl::X` blocks.
  - Multi-block files (SignalHandler.cpp — two `#ifdef` blocks with separate namespaces).
  - Forward-declaration blocks (Grabber.h, JPEGHandle.h).
- Closing braces updated: `} // namespace X` → `} // namespace icl::X`, outer `}` removed.
- Content dedented by 2 spaces (`scripts/dedent-namespaces.py`) to restore standard
  2-space indent for namespace content (664 files, purely whitespace).

**2. std::clamp (3 files):**
- `ICLUtils/ClippedCast.h` — `clip()` body replaced with `std::clamp(tX, tMin, tMax)`.
  All ~60 call sites across the codebase get std::clamp behavior automatically.
- `ICLFilter/LocalThresholdOp.cpp` — `myclip()` replaced with `std::clamp` + cast.
- `ICLPhysics/demos/physics-maze/physics-maze.cpp` — `std::max(std::min(...))` →
  `std::clamp(val*5, -1., 1.)`.

**3. [[nodiscard]] (3 files):**
- `ICLCore/Image.h` — deepCopy, convert, scaledCopy, selectChannel, selectChannels,
  mirrored (all return new Image values that should never be silently discarded).
- `ICLFilter/UnaryOp.h` — single-arg apply() and operator()().
- `ICLUtils/BackendDispatching.h` — registeredBackends, bestBackendFor,
  applicableBackendsFor (pure query methods).
- `tests/test-filter.cpp` — 19 call sites suppressed with `(void)` where apply() is
  called for side effects (feeding frames into MotionSensitiveTemporalSmoothing).

**New utility scripts:**
- `scripts/fuse-namespaces.py` — fuses two-level `namespace icl { namespace X {` into
  `namespace icl::X {`. Uses last-zero-indent-brace strategy for closing brace removal.
  Skips one-line forward declarations. Does NOT handle sibling-namespace files (those
  need manual fixup).
- `scripts/dedent-namespaces.py` — removes 2 leading spaces from all lines inside
  fused namespace blocks. Skips blank lines and preprocessor directives.
- `scripts/fix-orphan-braces.py` — earlier iteration, superseded by the fuse script
  improvements (can be deleted).

**Build: 100% clean (zero warnings), tests: 384/384 pass.**

### C++17 Source Modernization — Phased Plan

Full codebase scan completed (session 27). Already well-modernized in some areas:
`SimdCompat.h` (if constexpr), `EnumDispatch.h` (fold expressions), `VisitorsN.h`
(index_sequence + pack expansion), `StringUtils.h` (void_t + if constexpr).

#### Phase 1 — COMPLETED (session 27)

std::clamp, nested namespaces (`namespace icl::X`), `[[nodiscard]]`.
See session 27 summary above for details.

#### Phases 2-6 — COMPLETED (session 28)

See session 28 summary above for details.

#### Phase 7 — std::optional (deferred)

**Sentinel return values → optional:**
- `MarkerGridDetector::getPos()` returns `Point(-1,-1)` for "not found"
- `PCLPointCloudObject` methods returning `-1` for unsupported features
- `FilenameGenerator.cpp:93,109` returns `""` for error

**Optional output parameters → optional return:**
- `ImgBase::getMax(int ch, Point *coords=0)` / `getMin()` / `getMinMax()`
- `Primitive.cpp:87` — `compute_normal(..., bool *ok=0)`

Best done opportunistically or when touching these APIs for other reasons.

## Previous State (Session 26 — FixedMatrix compile-time SIMD acceleration)

### Session 26 Summary

**Compile-time SIMD acceleration for FixedMatrix:**

FixedMatrix (`ICLMath/FixedMatrix.h`) handles fixed-size matrices (2x2, 3x3, 4x4)
used in hot paths (camera projection, scene graph transforms, rotation composition).
Previously only 4x4 float multiply had SIMD (SSE2/sse2neon). All other sizes and
double precision fell back to generic `std::inner_product` with strided column
iterators — no vectorization. The runtime BlasOps dispatch mechanism was too costly
for these tiny matrices.

**Solution: compile-time `#ifdef` selection with zero dispatch overhead.**

**Apple SIMD (`<simd/simd.h>`) on macOS:**
- All functions are inline `SIMD_CFUNC` — zero call overhead, native NEON
- Row-major/column-major compatibility solved with zero overhead:
  - 4x4/2x2: `memcpy` reinterpret (same byte size), swap multiply args
  - Matrix-vector: `simd_mul(v, A_cm)` = `A_rm * v`
  - det/inv: transpose-invariant, reinterpret works directly
- Covers: mult, inv for 4x4/2x2 float/double; det for 4x4/2x2; matvec for 4x4
- Element-wise ops (add, sub, scalar mul, negate, div) for 4x4/2x2
- inv checks determinant and throws SingularMatrixException (matching C++ semantics)
- Replaces existing SSE2/sse2neon specializations on macOS

**SSE2/sse2neon (non-macOS fallback):**
- Existing specializations for 4x4 float multiply + matvec
- On Linux x86, clang -O3 auto-vectorizes remaining C++ loops to SSE/AVX

**Benchmark results (Apple M-series, -O3, batch=128 independent ops):**

| Operation | float SIMD | float C++ | Speedup | double Speedup |
|---|---|---|---|---|
| 4x4 multiply | 1.8 ns | 8.6 ns | **4.8x** | **3.2x** |
| 4x4 * vec4 | 0.8 ns | 1.4 ns | **1.6x** | 1.0x |
| 4x4 inverse | 5.4 ns | 20.1 ns | **3.7x** | **2.1x** |
| 4x4 det | 2.0 ns | 2.2 ns | ~1x | ~1x |
| Full pipeline | 2.6 ns | 6.5 ns | **2.5x** | **1.7x** |

**Key findings from benchmarking:**
- 3x3 Apple SIMD is **10x slower** than C++ due to `simd_float3` padding overhead
  (48 vs 36 bytes for 3x3). Excluded — uses C++ closed-form instead.
- cblas/MKL is **25x slower** than C++ for 4x4 (~100ns call overhead for a 4ns op).
  Removed from FixedMatrix — only useful for DynMatrix via BlasOps.
- Element-wise ops (add, smul, det) show ~1x because clang -O3 auto-vectorizes
  the C++ loops to equivalent NEON/SSE code.
- `memcpy` for load/store compiles to identical assembly as `reinterpret_cast`
  at -O2. Kept for correctness (alignment not guaranteed by FixedArray).

**New files:**
- `ICLMath/src/ICLMath/SimdCompat.h` — Apple SIMD load/store helpers + element-wise
  ops (add, sub, smul) for 4x4/2x2 float/double
- `benchmarks/bench-fixedmatrix.cpp` — standalone benchmark (no ICL deps), supports
  Apple SIMD, SSE2 intrinsics, MKL cblas, and plain C++ backends
- `scripts/docker/` — Docker infrastructure for Linux testing:
  `docker-build.sh`, `docker-shell.sh`, `docker-test.sh`, `docker-bench-fixedmatrix.sh`
  Uses rsync + named volumes for fast incremental builds.

**Modified files:**
- `ICLMath/src/ICLMath/FixedMatrix.h`:
  - `#include <ICLMath/SimdCompat.h>`, `<type_traits>`, `<initializer_list>`
  - Apple SIMD specializations: mult (4), matvec (2), inv (4), det (4),
    element-wise ops in operator+/-/*/negate bodies
  - SSE2 block restructured: `#elif defined(ICL_HAVE_SSE2)` (non-macOS fallback)
  - New `std::initializer_list<T>` constructor
  - 3x3 inv/det always use C++ closed-form (even on macOS)
- `tests/test-math.cpp`: 12 new cross-validation tests

**ICP.cpp inner loop heap allocations eliminated:**
- Transform loop did `new DynMatrix`+`delete` per point per iteration — replaced
  with a single stack-allocated temp buffer reused across all points.
- `error()` function simplified to avoid DynMatrix temporaries (direct element access).

**Codebase scan for BlasOps/SIMD wiring — completed:**
Systematic scan of all modules found that remaining hand-written loops (VectorTracker
`eucl_dist`, MathFunctions `euclidian`, KMeans `dist`) are NOT worth wiring through
BlasOps: they operate on small runtime-sized vectors (2-10 elements) where BlasOps
dispatch overhead (~2ns) exceeds the computation itself. The compiler auto-vectorizes
these short loops at -O3. BlasOps is already wired where it matters (DynMatrix),
and SimdCompat/Apple SIMD is already wired where it matters (FixedMatrix 4x4/2x2).

**Image scaling dispatched via ImgOps with Accelerate backend:**
- `scaledCopyChannelROI` (the single worker for all image scaling) added to ImgOps
  as `Op::scaledCopy` with C++ and Accelerate backends.
- C++ implementation moved from Img.cpp to Img_Cpp.cpp (NN, bilinear, region-average).
  Also replaced 8x `new[]`/`delete[]` in RA mode with `std::vector`.
- New `Img_Accelerate.cpp`: vImageScale for icl8u (Planar8), icl16s (Planar16S),
  icl32f (PlanarF). Uses Lanczos resampling (superior to bilinear/RA).
  NN mode uses inline integer indexing. icl32s/icl64f fall back to C++.
- CMakeLists.txt: `_Accelerate.cpp` exclusion added for ICLCore.
- Benchmark results (640x480 single channel, -O3):

  | Operation | Accelerate | C++ | Speedup |
  |---|---|---|---|
  | NN downscale 8u | 46 us | 134 us | **2.9x** |
  | LIN downscale 8u | 47 us | 146 us | **3.1x** |
  | RA downscale 8u | 49 us | 180 us | **3.7x** |
  | LIN downscale 32f | 140 us | 167 us | **1.2x** |
  | LIN upscale 8u | 52 us | 583 us | **11.2x** |

**Tests: 384/384 pass (5 new scaling tests).** Build clean on macOS.

### C++17 Source Modernization — Phased Plan

Full codebase scan completed (session 27). Already well-modernized in some areas:
`SimdCompat.h` (if constexpr), `EnumDispatch.h` (fold expressions), `VisitorsN.h`
(index_sequence + pack expansion), `StringUtils.h` (void_t + if constexpr).

#### Phase 1 — COMPLETED (session 27)

std::clamp, nested namespaces (`namespace icl::X`), `[[nodiscard]]`.
See session 27 summary above for details.

#### Phase 2 — Structured bindings, if-init, scoped_lock (~40 files, low risk)

**Structured bindings** (~40+ locations with `.first`/`.second`):
- `ICLIO/GenericGrabber.cpp:247-280` — `split_at_first()` result unpacking
- `ICLMarkers/FiducialDetectorPluginICL1.cpp:89-90` — pair unpacking
- `ICLPhysics/PhysicsPaper3.cpp:105-116,1656,1720` — map lookups, coords
- All map iterations using `it->first`/`it->second`

**If-with-initializer** (~25+ map-find-then-check patterns):
- `ICLMarkers/MarkerGridDetector.cpp:42-44` — `auto it = find(); if(it != end())`
- `ICLCore/Color.cpp:43-45` — color map lookup
- `ICLIO/FileGrabber.cpp:154-156` — plugin map lookup
- `ICLIO/DCDeviceFeatures.cpp:302-304` — feature map lookup
- `ICLGeom/PointCloudObjectBase.cpp:340-343` — metadata lookup
- `ICLPhysics/PhysicsPaper3ContextMenu.cpp:35,45` — action map
- `ICLQt/MultiDrawHandle.cpp:113-115` — name→index map
- `ICLUtils/SignalHandler.cpp:151,165,180` — handler maps

**std::scoped_lock** (replaces lock_guard, fixes manual lock/unlock):
- `ICLGeom/PointCloudCreator.cpp:125,294` — `lock_guard<recursive_mutex>` → `scoped_lock`
- `ICLQt/AbstractPlotWidget.cpp:110` — same
- `ICLFilter/GradientImage.cpp:29,42` — **manual lock()/unlock()** → RAII (bug risk)

**[[maybe_unused]]** (replaces `(void)variable` casts):
- `ICLGeom/Scene.cpp`, `ICLQt/GLImg.cpp`, `ICLGeom/PointCloudObjectBase.cpp`,
  `ICLGeom/CoplanarPointPoseEstimator.cpp`, `ICLIO/SharedMemoryGrabber.cpp`, etc.

#### Phase 3 — std::filesystem (~15 files, medium risk)

Biggest single win. Eliminates platform `#ifdef` blocks in FileList.cpp entirely.

**Core targets:**
- `ICLUtils/File.cpp:34-62` — `break_apart()` does manual `rfind('/')`,
  `rfind('.')`, `substr()` for dir/basename/suffix/filename. Replace with
  `path::parent_path()`, `stem()`, `extension()`. Special `.gz` double-extension
  handling needs care.
- `ICLUtils/File.cpp:110-119` — `file_exists()` and `file_is_dir()` use
  `struct stat`. Direct replacement with `std::filesystem::exists()`/`is_directory()`.
- `ICLUtils/File.cpp:590` — `File::erase()` uses C `remove()` →
  `std::filesystem::remove()`.
- `ICLIO/FileList.cpp:40-126` — Three platform-specific directory listing
  impls (`wordexp` Linux, `glob` macOS, `FindFirstFile` Windows). Replace with
  `std::filesystem::directory_iterator`. Note: glob pattern matching still needed
  on top of directory_iterator (std::filesystem doesn't do glob natively).
- `ICLIO/V4L2Grabber.cpp:91-95,147-150` — string concatenation with `"/"`
  for device paths → `operator/`; `stat()` for device existence → `fs::exists()`.
- `ICLQt/Widget.cpp:703-705` — `QDir("/").mkpath()` →
  `std::filesystem::create_directories()`.
- `ICLIO/V4L2Grabber.cpp:434-435` — hardcoded `/tmp/` →
  `std::filesystem::temp_directory_path()`.

#### Phase 4 — std::from_chars / std::to_chars (~10 files, low risk)

Faster, locale-independent number parsing/formatting:
- `ICLQt/IntHandle.cpp:17` — `atoi()`
- `ICLQt/FloatHandle.cpp:17` — `atof()`
- `ICLQt/GUIDefinition.cpp` — multiple `atoi`/`atof`
- `ICLIO/FileGrabberPluginCSV.cpp:26,110,118` — CSV float parsing with `atof`
- `ICLIO/TestImages.cpp:130,424-427` — XPM parsing with `atoi`
- `ICLIO/JPEGDecoder.cpp:140,142` — timestamp/ROI parsing
- `ICLUtils/StringUtils.cpp:22-42` — `snprintf` for number→string

#### Phase 5 — if constexpr cleanup (~5 files, medium risk)

- `ICLUtils/BackendDispatching.h:268-327` — 6 `enable_if` SFINAE overloads for
  enum/integral key types → single template with `if constexpr(is_enum_v<K>)`
- `ICLMath/FixedMatrix.h:300-336` — 4 `#ifdef ICL_HAVE_APPLE_SIMD` blocks in
  operator bodies → `if constexpr` with a constexpr flag
- Depth switch statements (~240 instances) — many could leverage the existing
  `dispatchEnum` fold-expression helper from EnumDispatch.h more broadly

#### Phase 6 — std::string_view (20+ files, higher risk — API change)

Changes function signatures. Start with leaf utilities (no virtual overrides):

**Leaf utilities (safe to change first):**
- `StringUtils.h` — `tok()`, `toLower()`, `toUpper()`, `parse<T>()`,
  `startsWith()`, `endsWith()`, `match()`
- `File.h` — `read_file()`, `read_lines()`, `write_file()`, constructors

**Map-lookup APIs (key parameter only reads):**
- `Configurable.h` — `prop()`, `addProperty()`, `getPropertyType/Info/Tooltip()`,
  `supportsProperty()`, `deactivateProperty()`
- `MultiTypeMap.h` — `allocValue()`, `getValue()`, `contains()`, `getType()`
- `ConfigFile.h` — `load()`, `save()`, `set()`, `check_type()`

**Caveat:** Functions that call `.c_str()` internally (fopen, etc.) need the string
materialized — string_view doesn't guarantee null termination.

#### Phase 7 — std::optional (10+ files, highest risk — API change)

**Sentinel return values → optional:**
- `MarkerGridDetector::getPos()` returns `Point(-1,-1)` for "not found"
- `PCLPointCloudObject` methods returning `-1` for unsupported features
- `FilenameGenerator.cpp:93,109` returns `""` for error

**Optional output parameters → optional return:**
- `ImgBase::getMax(int ch, Point *coords=0)` / `getMin()` / `getMinMax()`
- `Primitive.cpp:87` — `compute_normal(..., bool *ok=0)`

Best done opportunistically or when touching these APIs for other reasons.

## Previous State (Session 25 — NeighborhoodOp fix, dead IPP cleanup, Accelerate mapping)

### Session 25 Summary

**NeighborhoodOp even-mask IPP workaround removed:**
- `computeROI()` unconditionally shrunk output ROI by 1px for even-sized masks —
  this was an IPP-specific workaround (`#ifdef ICL_HAVE_IPP`) that had been
  applied unconditionally in the rewritten "NEW Code" path. The C++ backend
  has no anchor bug, so the shrink was incorrect (lost a valid row/column).
- Removed the workaround and cleaned up dead commented-out old code block.
- Added 3 even-kernel tests: 4x4 identity (verifies 7x7 output from 10x10),
  2x2 sum (verifies 7x7 from 8x8), 4x4 cross-validate across backends.

**Dead CONFIGURE_GTEST macro removed:**
- Removed the unused `CONFIGURE_GTEST` function from ICLHelperMacros.cmake
  (~67 lines). No module called it — tests migrated to centralized `tests/`
  directory in session 19. The per-module test stubs no longer exist.

**Dead `#if 0` IPP blocks removed (~1,000 lines):**
- 5 _Ipp.cpp files cleaned to TODO stubs with modern API + Accelerate notes:
  AffineOp_Ipp.cpp (57→16 lines), ConvolutionOp_Ipp.cpp (261→17),
  LUTOp_Ipp.cpp (35→14), MedianOp_Ipp.cpp (110→16), MorphologicalOp_Ipp.cpp (229→19)
- CannyOp.cpp: removed 22-line `#if 0` IPP block, kept C++ fallback
- IntegralImgOp.cpp: removed 40-line `ICL_HAVE_IPP_DEACTIVATED` block
  (C++ loop-unrolled version was already faster)
- CoreFunctions.cpp: removed 448-line SSE block (moved to PixelOps.cpp)
  and 45-line dead IPP histogram block

**NeighborhoodOp.h cleanup:**
- Removed stale `TODO:: check!!` and `TODO: later private` comments
- Fixed typos in class documentation (shrinked→shrunk, adaptROI→computeROI)

**Accelerate-IPP mapping reference created:**
- `claude.insights/accelerate-ipp-mapping.md` documents which IPP operations
  have Apple Accelerate (vImage) equivalents, with function names and limitations
- Good coverage: affine warp, convolution, histogram, basic morphology
- Gaps: no median filter, no Canny, no integral image in vImage

**Accelerate filter backends — ConvolutionOp, MorphologicalOp, AffineOp:**
- `ConvolutionOp_Accelerate.cpp` using `vImageConvolve_PlanarF` for icl32f
  - Handles even-sized kernels by padding to odd dimensions (vImage requires odd)
- `MorphologicalOp_Accelerate.cpp` using `vImageDilate/Erode_Planar8/PlanarF`
  for icl8u and icl32f (all 11 optypes: basic, 3x3, borderReplicate, composites)
  - Planar8: uses ICL's binary mask directly (unsigned char, nonzero=include)
  - PlanarF: converts binary mask to float kernel (0.0=include, -INF=exclude)
    because vImage PlanarF morphology uses additive structuring elements
  - Composite ops (open/close/tophat/blackhat/gradient) create sub-ops that
    dispatch through Accelerate for inner dilate/erode
- `AffineOp_Accelerate.cpp` using `vImageAffineWarp_Planar8/PlanarF` for
  icl8u and icl32f with bilinear interpolation; NN falls back to C++ inverse map
  - Maps ICL's 2x3 forward matrix to `vImage_AffineTransform` struct
  - Background fill with 0 for out-of-bounds pixels
- All registered as `Backend::Accelerate` (priority 6, above C++ fallback)
- Accelerate header included before ICL to avoid macOS Point/Size name conflicts
- ICLFilter CMakeLists.txt: `_Accelerate.cpp` excluded when `NOT ACCELERATE_FOUND`;
  Accelerate framework linked via 3RDPARTY_LIBS
- All cross-validated against C++ backend across all test cases

**Benchmark results (640x480 images, 50 iterations):**

| Benchmark | Accelerate | C++ | Speedup |
|---|---|---|---|
| convolution gauss3x3 32f | 235 us | 314 us | 1.3x |
| convolution gauss5x5 32f | 339 us | 9904 us | **29x** |
| morphology dilate3x3 8u | 243 us | 6477 us | **27x** |

**DynMatrixBase<bool> for non-float/double users:**
- Replaced `DynMatrix<bool>` → `DynMatrixBase<bool>` across 18 files (85 occurrences)
- DynMatrix<T> method bodies are out-of-line, only instantiated for float/double;
  DynMatrix<bool> only "worked" because callers used inherited DynMatrixBase methods
- DynMatrixBase<bool> is the correct type: fully header-only, all operations available
- Files: GraphCutter.h/.cpp, 12 ICLGeom segmentation files, PhysicsWorld.cpp

**SmartArray<T> replaced with std::shared_ptr<T[]>:**
- C++17 `shared_ptr<T[]>` provides native array semantics (operator[], delete[])
- Replaced all 38 SmartArray usage sites across 10 files
- Owning: `shared_ptr<T[]>(ptr)`. Non-owning: `shared_ptr<T[]>(ptr, [](T*){})`
- PlotWidget::Buffer changed from inheritance to composition (can't inherit shared_ptr)
- `SmartArray.h` deleted — zero remaining consumers
- Key files: Img.h/cpp (channel storage), Array2D.h, SOM.h, Scene.h, PlotWidget/LowLevelPlotWidget

**Debug-mode warnings fixed (zero remaining):**
- AffineOp.cpp: member initializer order
- BinaryArithmeticalOp_Simd.cpp: unused lambda capture
- DynVector.cpp: struct/class mismatch in explicit instantiation

**Accelerate vDSP arithmetic backends (UnaryArithmeticalOp + BinaryArithmeticalOp):**
- `UnaryArithmeticalOp_Accelerate.cpp`: vDSP_vsadd/vsmul (add/sub/mul/div),
  vDSP_vsq (sqr), vvsqrtf/vvlogf/vvexpf (sqrt/ln/exp), vDSP_vabs — for icl32f
- `BinaryArithmeticalOp_Accelerate.cpp`: vDSP_vadd/vsub/vmul/vdiv + vabs — for icl32f
- Scalar multiply benchmark: **5.7x speedup** over C++

**Full BLAS Layer (Level 1/2/3) in BlasOps:**
- Extended BlasOps with complete BLAS coverage via backend dispatch:
  - Level 3: gemm (existing)
  - Level 2: gemv (matrix-vector multiply) — new
  - Level 1: vadd/vsub/vmul/vdiv/vsadd/vsmul (existing), dot/nrm2/asum/axpy/scal (new)
- C++ fallbacks in BlasOps_Cpp.cpp, Accelerate backends (cblas + vDSP) in
  BlasOps_Accelerate.cpp — both float and double
- Cached inline dispatch: `BlasOps<T>::dot(a, b, n)` — function-local static
  resolves backend on first call, ~1-2ns overhead thereafter
- DynMatrix element-wise ops (operator+/-/*, elementwise_mult/div) wired through
  BlasOps dispatch instead of std::transform

**Accelerate-IPP mapping updated:**
- Deep-dive of all Accelerate APIs (vImage, vDSP, vForce, BNNS, simd)
- Tier 1-3 opportunities ranked by impact
- See `claude.insights/accelerate-ipp-mapping.md`

**Tests: 367/367 pass.** Build clean, zero warnings on macOS.

### Next Steps

- **Wire BlasOps through codebase** — grep for hand-written dot products, norm
  calculations, axpy-style accumulations, and matrix-vector multiplies across
  ICLMath, ICLGeom, ICLCV, ICLFilter; replace with `BlasOps<T>::dot/nrm2/axpy/gemv`
- **Image scaling Accelerate backend** — `vImageScale_Planar8/PlanarF` for
  `scaledCopyChannelROI()` in Img.cpp (needs backend dispatch added first)
- **ImageMagick 7** — rewrite FileGrabberPluginImageMagick.cpp and
  FileWriterPluginImageMagick.cpp for Quantum/Pixels API
- **FFmpeg 7+** — rewrite LibAVVideoWriter.cpp for modern API
- **OpenCL on macOS** — bundle Khronos cl2.hpp header for C++ bindings
- **C++17 source pass** — std::filesystem, std::string_view, structured bindings
- **Re-enable IPP backends** on Linux — update to modern oneAPI APIs

## Previous State (Session 24 — LapackOps expansion, API cleanup, DynMatrixBase split)

### Session 24 Summary

**New LapackOps (3 new ops, 2 gap-fills):**
- `geqrf` + `orgqr` (QR factorization via Householder reflectors) — all 4 backends
- `gelsd` (SVD least-squares solve) — all 4 backends
- `getrf`/`getri` added to MKL and Eigen backends (were missing)

**Accelerate backend row-major fix:**
- getrf, getri, geqrf, orgqr, gelsd now transpose explicitly to/from column-major
  before calling LAPACK. Ensures packed output (L/U, Householder reflectors)
  matches C++ backend convention. gesdd/syev keep dimension-swap trick.

**Consumer wiring:**
- `decompose_QR()` → geqrf + orgqr (was hand-written Gram-Schmidt)
- `decompose_LU()` → getrf + unpack (was hand-written partial pivoting)
- `solve()` → gelsd directly (was string-based method dispatch with "lu"/"svd"/"qr"/"inv")
- `pinv()` → always reduced SVD + BLAS gemm (was bool useSVD with QR fallback)
- `mult()` → BLAS gemm for float/double (was hand-written inner_product loop)
- `matrix_mult_t()` → gemm transpose flags for float/double (no temp copies)

**API simplification:**
- Removed `big_matrix_pinv()` — merged into `pinv()`
- Removed `big_matrix_mult_t()` — merged into `matrix_mult_t()`
- Removed `pinv(bool useSVD)` parameter — always SVD
- Removed `solve(b, string method)` parameter — always gelsd
- Removed `solve_upper_triangular()` / `solve_lower_triangular()` — dead code
- Removed `PolynomialRegression::apply()` useSVD parameter
- Removed dead `#if 0` block in DynMatrixUtils.h (old svd_cpp_64f)
- Removed dead EigenICLConverter (zero callers)

**DynMatrixBase<T> refactor — compilation time reduction:**
- New `DynMatrixBase<T>` (header-only, ~290 lines): storage, element access,
  flat iterators, properties, operator<</>>
- `DynMatrix<T>` inherits DynMatrixBase, adds col/row iterators, DynMatrixColumn,
  all arithmetic and linalg — declarations only in header (~200 lines)
- All method bodies in `DynMatrix.cpp` (~700 lines), whole-class instantiation
  for float/double: `template class ICLMath_API DynMatrix<float/double>;`
- `DynVector.h` declarations only, `DynVector.cpp` with whole-class instantiation
- DynMatrix.h includes only `<iterator>` beyond ICL headers (was `<numeric>`,
  `<functional>`, `<vector>`, `<cmath>`, `<algorithm>`)
- Concatenation operators (operator,/%) moved out of line

**License header cleanup:**
- Replaced 29-line decorative headers with 3-line SPDX across 1038 files (~27,600 lines removed)
- Format: `SPDX-License-Identifier: LGPL-3.0-or-later` / `ICL - Image Component Library (URL)` / `Copyright (original authors)`
- New top-level LICENSE file with project info and EXC 277 acknowledgment

**Tests: 364/364 pass (15 new).** Build clean, zero warnings on macOS.

### LapackOps Summary (8 operations, 4 backends)

| Op | Signature | C++ | Accelerate | MKL | Eigen |
|---|---|---|---|---|---|
| gesdd | SVD divide-and-conquer | ✓ | ✓ | ✓ | ✓ |
| syev | Symmetric eigenvalue | ✓ | ✓ | ✓ | ✓ |
| getrf | LU factorization | ✓ | ✓ | ✓ | ✓ |
| getri | LU inverse | ✓ | ✓ | ✓ | ✓ |
| geqrf | QR factorization | ✓ | ✓ | ✓ | ✓ |
| orgqr | Form Q from reflectors | ✓ | ✓ | ✓ | ✓ |
| gelsd | SVD least-squares solve | ✓ | ✓ | ✓ | ✓ |
| gemm | Matrix multiply (BlasOps) | ✓ | ✓ | ✓ | — |

### DynMatrix File Layout

```
DynMatrixBase.h   — storage, element access, streaming (header-only, any type)
DynMatrix.h       — col/row iterators, arithmetic/linalg declarations (~200 lines)
DynMatrix.cpp     — all method bodies + whole-class instantiation float/double
DynVector.h       — DynColVector/DynRowVector declarations
DynVector.cpp     — all method bodies + whole-class instantiation float/double
```

## Previous State (Session 23 — Full Backend Dispatch Architecture)

### Session 23 Summary

**FFTOps_Cpp.cpp segfault fixed:**
- Root cause: bug in `fft2D_cpp` — `buf(src.rows(),src.cols())` was element access
  (operator()) on a null pointer, not resize. Fix: one-liner → `buf.setBounds(...)`.
- FFTOps_Cpp.cpp keeps non-owning wrappers for src/dst (zero-copy, no overhead).

**FFTUtils.cpp fully wired to FFTOps — 21 MKL blocks eliminated:**
- Generic `fft2D`/`ifft2D` templates dispatch through FFTOps with type conversion.
- FFTDispatching.h/.cpp deleted. All explicit MKL specializations removed.

**SVD deduplicated — svd_dyn dispatches through LapackOps:**
- Single generic `svd_dyn<T>` template, no float/double specializations.
- Handles LAPACK Vt → ICL V convention via post-dispatch transpose.

**GLImg.cpp — last active IPP block removed.**

**Dead code cleanup — ~1,600 lines removed total:**
- `#if 0` blocks in FFTUtils.cpp, DynMatrixUtils.cpp, DynMatrix.cpp deleted.
- PThreadFix.h stripped to empty header.
- `jacobi_iterate_vtk` + `find_eigenvectors` removed from DynMatrix.cpp (moved to
  LapackOps_Cpp.cpp as `cpp_syev`).
- `get_minor_matrix` removed (inv() no longer uses cofactor expansion).

**Apple Accelerate backend:**
- CMake auto-detects on macOS via `find_library(Accelerate)`.
- `BlasOps_Accelerate.cpp`: cblas_sgemm/dgemm.
- `FFTOps_Accelerate.cpp`: vDSP_DFT row-column decomposition (arbitrary sizes).
- `LapackOps_Accelerate.cpp`: sgesdd/dgesdd, ssyev/dsyev, sgetrf/dgetrf, sgetri/dgetri.

**LapackOps dispatcher — clean BLAS/LAPACK separation:**
- `LapackOps.h`: `enum LapackOp { gesdd, syev, getrf, getri }`.
- `LapackOps.cpp`: constructor, instance(), toString().
- gesdd moved from BlasOps to LapackOps (all backends updated, all consumers rewired).
- BlasOps now contains only GEMM.

**LapackOps backends:**
- `LapackOps_Cpp.cpp`: gesdd (Golub-Kahan), syev (Jacobi), getrf (LU), getri (LU inverse).
- `LapackOps_Accelerate.cpp`: gesdd, syev, getrf, getri via Apple LAPACK.
- `LapackOps_Mkl.cpp`: gesdd, syev via MKL LAPACK.
- `LapackOps_Eigen.cpp`: gesdd (JacobiSVD), syev (SelfAdjointEigenSolver) via Eigen3.

**Backend enum expanded:**
- Added `Backend::Eigen = 3` between OpenBlas and Ipp.
- Priority order: Cpp(0) < Simd(1) < OpenBlas(2) < Eigen(3) < Ipp(4) < FFTW(5)
  < Accelerate(6) < Mkl(7) < OpenCL(8).

**Consumer wiring:**
- `svd_dyn()` → `LapackOps<T>::gesdd`
- `big_matrix_pinv()` → `LapackOps<T>::gesdd` + `BlasOps<T>::gemm`
- `DynMatrix::eigen()` → `LapackOps<T>::syev` (resolveOrThrow, C++ fallback always available)
- `DynMatrix::inv()` → `LapackOps<T>::getrf` + `getri` (O(n³) LU, replaces O(n!) cofactor)
- `DynMatrix::det()` for n>4 → `LapackOps<T>::getrf` (product of U diagonal, replaces O(n!) cofactor)

**C++ fallback convention enforced:**
- All C++ backend registrations now in `_Cpp.cpp` files.
- MathOps C++ backends moved from DynMatrixUtils.cpp → `MathOps_Cpp.cpp`.
- ImgOps channelMean moved from CoreFunctions.cpp → `Img_Cpp.cpp`.
- Only exception: ImgBorder.cpp (depends on file-local `_copy_border`, documented).

**Design decisions:**
- BlasOps for GEMM only; element-wise ops stay inlined (bandwidth-bound).
- FixedMatrix (4x4): keep inlined, IPP ippm dropped, compiler auto-vectorization sufficient.
- det() keeps special cases for 1x1–4x4 (zero overhead); n>4 uses LU.

**Tests: 349/349 pass.** Build clean on macOS.

### Next Steps

- **Add more LapackOps** — geqrf (QR factorization) to accelerate decompose_QR(), solve()
- **Investigate legacy test stubs** — Per-module test executables compile to 0 tests
- **NeighborhoodOp.cpp** — Two `#ifdef ICL_HAVE_IPP` workaround blocks (anchor bug)

## Previous State (Session 22 — BlasOps/FFTOps + BackendDispatching Refactor + BLAS Consumer Wiring)

### Session 22 Summary (continued)

**New BLAS/FFT abstraction layer + BackendDispatching refactor.**

**BackendDispatching refactored: array → sorted vector:**
- `std::array<ImplPtr, NUM_BACKENDS>` replaced with sorted `std::vector<Entry>`
- Backends sorted by priority descending (highest first); only registered backends occupy space
- `NUM_BACKENDS` constant removed entirely — enum can grow freely
- New `resolve()` / `resolveOrThrow()` no-arg overloads for dummy-context singletons
- New `Entry` struct: `{ Backend backend; shared_ptr<ImplBase> impl; }`
- `setImpl()` private helper maintains sorted order on insertion

**Backend enum expanded (values encode priority):**
```
Cpp=0, Simd=1, OpenBlas=2, Ipp=3, FFTW=4, Accelerate=5, Mkl=6, OpenCL=7
```

**BlasOps\<T\> — BLAS/LAPACK abstraction (new):**
- `BlasOps.h` — singleton template, `enum class BlasOp { gemm, gesdd }`
- `BlasOps.cpp` — constructor (addSelector), instance(), toString()
- `BlasOps_Cpp.cpp` — C++ fallback: naive GEMM + SVD via existing `svd_dyn`
- `BlasOps_Mkl.cpp` — MKL backend: `cblas_sgemm/dgemm` + `sgesdd/dgesdd`
- Raw pointer interface (no DynMatrix dependency in signatures)
- Consumer code will use `resolveOrThrow()` — C++ fallback always available

**FFTOps\<T\> — FFT abstraction (new, replaces FFTDispatching):**
- `FFTOps.h` — singleton template, `enum class FFTOp { r2c, c2c, inv_c2c }`
- `FFTOps.cpp` — constructor, instance(), toString()
- `FFTOps_Cpp.cpp` — C++ fallback via existing `fft2D_cpp` / `ifft2D_cpp`
- `FFTOps_Mkl.cpp` — MKL DFTI backend (forward/inverse, real/complex)
- Raw pointer interface; DynMatrix wrapping stays in consumers
- `FFTOps<float>` + `FFTOps<double>` — separate singletons per precision

**FFTDispatching transitional rename:**
- `FFTOp` enum → `LegacyFFTOp` to avoid conflict with FFTOps
- FFTDispatching struct kept temporarily until FFTUtils.cpp is rewired
- Will be deleted once FFTUtils.cpp uses FFTOps directly

**Convention: C++ fallbacks in `_Cpp.cpp` files:**
- All backend implementations (including C++ fallback) go in `_<Backend>.cpp` files
- The `.cpp` file (e.g., `BlasOps.cpp`) only contains constructor, instance(), toString()
- Pattern: `BlasOps_Cpp.cpp`, `BlasOps_Mkl.cpp`, `FFTOps_Cpp.cpp`, `FFTOps_Mkl.cpp`

**CMake: new exclusion patterns in ICLMath/CMakeLists.txt:**
- `_FFTW.cpp` excluded when `!FFTW_FOUND`
- `_Accelerate.cpp` excluded when `!ACCELERATE_FOUND`
- `_OpenBlas.cpp` excluded when `!OPENBLAS_FOUND`

**BLAS consumers wired (DynMatrix + DynMatrixUtils):**
- `DynMatrix.cpp`: `big_matrix_pinv` now uses `BlasOps<T>::gesdd` + `gemm` via
  `resolveOrThrow()`. No fallback logic — C++ backend always available.
- `DynMatrixUtils.cpp`: `big_matrix_mult_t` now uses `BlasOps<T>::gemm` via
  `resolveOrThrow()`. MKL explicit specializations removed.
- All `#ifdef ICL_HAVE_MKL` removed from DynMatrix.h, DynMatrix.cpp, DynMatrixUtils.cpp (6 blocks).
- `BlasOps_Cpp.cpp` GEMM optimized: compile-time dispatch of transA/transB/alpha/beta
  via template specialization + `if constexpr` (eliminates inner-loop conditionals).
  Epsilon-based `isZero()`/`isOne()` for float-safe alpha/beta comparison.
- `BlasOps_Cpp.cpp` SVD: full Golub-Kahan bidiagonalization (`svd_bidiag`) moved here
  from DynMatrixUtils.cpp. C++ GESDD backend always works (no more stub returning -1).

**FFT consumer wiring attempted but reverted (segfault):**
- Rewrote `fft2D<T1,T2>` and `ifft2D<T1,T2>` generic templates to use FFTOps dispatch.
  All 19 explicit specializations with `#ifdef ICL_HAVE_MKL` eliminated (replaced by
  template instantiations). The generic template handles type conversion + dispatch.
- **Segfault in multi-threaded tests.** Root cause: `FFTOps_Cpp.cpp` wraps raw pointers
  in non-owning `DynMatrix(cols, rows, ptr, false)` and passes to `fft2D_cpp`. The FFT
  row-column decomposition writes to `dst` with transposed dimensions — the non-owning
  wrapper's buffer may be too small or have wrong stride. Fix: use owned DynMatrix
  intermediates in the C++ backend, copy result back to raw pointer at the end.
- **Changes reverted** to keep branch green. FFTUtils.cpp still has 21 MKL blocks.

**Remaining `ICL_HAVE_MKL` references:**
- FFTUtils.cpp: 21 blocks (2 wrapper blocks + 19 specialization blocks, all still active)
- Camera.cpp: 1 (dead comment)
- PThreadFix.h: 1 (dead, inside `#if 0`)
- ICLConfig.h.in / doxyfile.in: build system definitions (must stay)

**Tests: 349/349 pass.** Build clean on macOS.

### Next Steps

- **Fix FFTOps_Cpp.cpp segfault** — Use owned DynMatrix in C++ FFT backend instead of
  non-owning wrappers. The issue is `fft2D_cpp`'s row-column decomposition writes `dst`
  with transposed buffer layout. Either: (a) allocate an owned DynMatrix, call fft2D_cpp,
  copy result to raw pointer; or (b) rewrite the C++ FFT backend to work directly on
  raw pointers without going through fft2D_cpp.
- **Wire FFTUtils.cpp to FFTOps** — The generic template rewrite is ready (tested
  single-threaded), just needs the C++ backend fix above. Once fixed, 21 MKL blocks
  disappear and FFTDispatching.h/.cpp can be deleted.
- **Deduplicate SVD** — `svd_internal` now lives in both `BlasOps_Cpp.cpp` and
  `DynMatrixUtils.cpp`. Wire `svd_dyn()` to dispatch through `BlasOps<T>::gesdd`,
  then remove the old copy from DynMatrixUtils.cpp.
- **Camera.cpp** — Remove dead MKL comment (trivial)
- **GLImg.cpp** — Last active `#ifdef ICL_HAVE_IPP` block (min/max, could use ImgOps)
- **Add Accelerate backend** — `BlasOps_Accelerate.cpp` + `FFTOps_Accelerate.cpp`
  for macOS (cblas via Accelerate framework, vDSP FFT)
- **Consider LapackOps** — Separate dispatch for LAPACK operations (eigenvalue
  decomposition, Cholesky, LU, etc.) beyond just GESDD
- **FixedMatrix BLAS** — For small matrices (commonly 4x4), BlasOps dispatch overhead
  is too high. Keep inlined implementations. IPP's `ippm` module (small matrix ops) was
  dropped from modern IPP. Best choice for small matrices: compiler auto-vectorization
  of the inlined code, or hand-written SIMD for the hot 4x4 case.

## Previous State (Session 21 — A2 + B + C/IPP Complete)

### Session 21 Summary

**A2 complete, B complete, C/IPP complete for all modules.** All active
`#ifdef ICL_HAVE_IPP` operational code across ICLUtils/ICLCore/ICLMath/
ICLFilter/ICLIO has been migrated to dispatch or removed with TODOs.

**A2 — all three phases:**

**Phase 1 — Global string registry removed:**
- Deleted `detail::RegistryEntry`, `detail::globalRegistry()`, `detail::addToRegistry()`
- Deleted `registerBackend()`, `registerStatefulBackend()` static methods
- Deleted `loadFromRegistry()` private method
- Deleted `BackendDispatching.cpp` (only contained the registry impl)
- Removed `m_selectorByName` map and `m_prefix` string member
- Removed `initDispatching()` — all 15 filters, ImgOps, FFTDispatching updated
- Removed string-keyed `addSelector(string)`, `getSelector(string)`, `selectorByName(string)`
- Enum-keyed `addSelector(K)` is now standalone (no delegation to string version)
- Added `selector(K)` returning `BackendSelectorBase*` (replaces `selectorByName` for tests)
- Fixed `ThresholdOp_Simd.cpp` — switched from global registry to prototype direct registration

**Phase 2 — Stateful backend cloning (factory pattern):**
- `ImplBase` now has `std::function<shared_ptr<ImplBase>()> cloneFn` (null for stateless)
- `BackendSelector::clone()`: calls `cloneFn()` for stateful backends, shares `shared_ptr` for stateless
- `BackendSelector::addStateful(b, factory, applicability, desc)` — factory called per clone
- `BackendDispatching::addStatefulBackend<Sig>(key, b, factory, app, desc)` — convenience
- Migrated 4 stateful backends:
  - `WienerOp_Ipp.cpp` — IPP scratch buffer now per-instance
  - `WarpOp_OpenCL.cpp` — CLWarpState (GPU buffers/kernels) now per-instance
  - `BilateralFilterOp_OpenCL.cpp` — CLBilateralState now per-instance
  - `MorphologicalOp_Ipp.cpp` — MorphIppState (IPP state objects) now per-instance

**Phase 3 — AffineOp test bug fixed:**
- `AffineOp_Cpp.cpp`: bilinear interpolation bounds check now correctly validates
  the full 2x2 neighborhood (`x2 < width-1, y2 < height-1`)
- Edge pixels outside the safe bilinear zone fall back to nearest-neighbor
- **349/349 tests pass both single-threaded AND multi-threaded** (SIGTRAP eliminated)

**FFTDispatching migrated to enum pattern:**
- Added `enum class FFTOp : int { fwd32f, inv32f, fwd32fc }` with `toString()`
- Selector setup moved to `FFTDispatching()` constructor
- `FFTUtils.cpp` now uses `getSelector<Sig>(FFTOp::xxx)` (enum-keyed O(1))

**Test changes:**
- All `selectorByName("name")` calls replaced with `selector(FilterOp::Op::name)`

**B: ICLCore IPP blocks — all active blocks migrated:**
- `CCFunctions.cpp`: `planarToInterleaved`/`interleavedToPlanar` IPP specializations
  extracted to `Img_Ipp.cpp` as ImgOps selectors. Generic template now dispatches
  through ImgOps for same-type cases (S==D), falling back to `_Generic` if no backend.
  The `#ifdef` block + conditional instantiations removed. Two new `ImgOps::Op` values added.
- `BayerConverter.h/.cpp`: Removed dead IPP code. `nnInterpolationIpp()` was defined but
  never called (the `apply()` method always calls `nnInterpolation()` instead).
  `m_IppBayerPattern` member and constructor switch statements removed.
- Only `Types.h` retains compile-time `ICL_HAVE_IPP` guards (enum definitions, stays).

**BackendProxy for terser registration:**
- Added `BackendProxy` struct + `backends(Backend b)` method on `BackendDispatching`
- All ~65 `addBackend`/`addStatefulBackend` call sites across _Cpp/_Ipp/_Simd/_OpenCL files
  migrated to use the proxy: `auto cpp = proto.backends(Backend::Cpp); cpp.add<Sig>(...);`
- Removed `addBackend`/`addStatefulBackend` as public API — proxy calls getSelector().add() directly

**C/ICLMath — MathOps dispatch framework + all IPP removed:**
- Created `MathOps<T>` template singletons (`MathOps<float>`, `MathOps<double>`)
  using `BackendDispatching<int>` (dummy context, no applicability checks)
- `enum class MathOp` with 11 selectors: mean, var, meanvar, min, max, minmax,
  unaryInplace, unaryCopy, unaryConstInplace, unaryConstCopy, binaryCopy
- Sub-operation enums: `UnaryMathFunc` (12 ops), `UnaryConstFunc` (5 ops), `BinaryMathFunc` (6 ops)
- `DynMatrixUtils.cpp`: entire `#ifdef ICL_HAVE_IPP` block (lines 80-548) replaced with
  MathOps dispatch + C++ backend registration
- `DynMatrix.h`: removed IPP specializations (sqrDistanceTo, distanceTo, elementwise_div,
  mult-by-scalar, norm), added TODO
- `MathFunctions.h`: removed IPP mean specializations, added TODO
- `FixedMatrix.h`: removed unused `#include <ipp.h>`
- `CMakeLists.txt`: added `_Ipp.cpp`/`_Mkl.cpp` exclusion patterns

**C/ICLIO — all IPP removed:**
- `DC.cpp`: removed `ippiRGBToGray_8u_C3C1R`, always use weighted-sum loop
- `ColorFormatDecoder.cpp`: removed `ippiYUVToRGB_8u_C3R` + `ippiCbYCr422ToRGB_8u_C2C3R`
- `PylonColorConverter.h/.cpp`: removed `Yuv422ToRgb8Icl` and `Yuv422YUYVToRgb8Icl`
  IPP-only classes, always use PylonColorToRgb fallback
- TODOs added at every removal site

### Remaining `ICL_HAVE_IPP` References

Only these remain in the codebase:

| Category | Files | Status |
|---|---|---|
| Type definitions | BasicTypes.h, Size.h, Point.h, Rect.h, Types.h | Compile-time aliases, must stay |
| `#if 0` dead code | 5 *_Ipp.cpp files, FFTUtils (2), CannyOp, ImageRectification, CoreFunctions.cpp (histogramEven), DynMatrixUtils.cpp (ippm matrix ops) | Disabled, contains code for future re-enablement |
| IPP bug workaround | NeighborhoodOp.cpp (2 blocks) | Minor, stays |
| Qt GPU upload | GLImg.cpp (1 block) | ICLQt, deferred |
| Comments | CCFunctions.cpp (2 `/* */` blocks), DynMatrix.cpp (`#if 1 // was:`), DynMatrixUtils.cpp (SVD comment), IntegralImgOp.cpp (`ICL_HAVE_IPP_DEACTIVATED_...`) | Dead |
| CMake / config | CMakeLists.txt, ICLConfig.h.in, doxyfile.in | Build system definitions |
| Dead (nested `#if 0`) | PThreadFix.h | Inside outer `#if 0`, unreachable |

### Future IPP Optimization Opportunities (TODOs in code)

| Location | IPP Function | What It Accelerates |
|---|---|---|
| BayerConverter.cpp | `ippiCFAToRGB_8u_C1C3R` | Bayer → RGB nearest-neighbor |
| DynMatrix.h | `ippsNormDiff_L2`, `ippsDiv`, `ippsMulC`, `ippsNorm_L1/L2` | Matrix distance, element-wise div, scalar mult, norms |
| MathFunctions.h | `ippsMean_32f/64f` | Scalar mean over float/double arrays |
| DynMatrixUtils.cpp | All math ops already dispatched | Re-enable via `DynMatrixUtils_Ipp.cpp` |
| DC.cpp | `ippiRGBToGray_8u_C3C1R` | RGB → grayscale conversion |
| ColorFormatDecoder.cpp | `ippiYUVToRGB_8u_C3R`, `ippiCbYCr422ToRGB_8u_C2C3R` | YUV → RGB color conversion |
| PylonColorConverter.cpp | `ippiCbYCr422ToRGB_8u_C2C3R`, `ippiYCbCr422ToRGB_8u_C2C3R` | Pylon YUV422 → RGB |
| ConvolutionOp_Ipp.cpp | `ippiFilterSobelBorder_*`, `ippiFilterGaussBorder_*` | 34 Sobel/Gauss/Laplace specializations |
| MorphologicalOp_Ipp.cpp | `ippiDilate/Erode_*_C1R_L` + spec | Modern morphology with border |
| AffineOp_Ipp.cpp | `ippiWarpAffineNearest/Linear_*` + spec | Affine warp |
| MedianOp_Ipp.cpp | `ippiFilterMedianBorder_*_C1R` | Median filter with border |
| LUTOp_Ipp.cpp | `ippiReduceBits` (new signature) | Bit reduction |
| CannyOp.cpp | Modern `ippiCanny` with border spec | Canny edge detection |

### Next Steps

- **Wire consumers to BlasOps/FFTOps** — Replace 27 `#ifdef ICL_HAVE_MKL` blocks
  in DynMatrix.cpp, DynMatrixUtils.cpp, FFTUtils.cpp with BlasOps/FFTOps dispatch.
  Remove MKL includes, delete FFTDispatching.h/.cpp and LegacyFFTOp.
- **Add Accelerate backend** — `BlasOps_Accelerate.cpp` + `FFTOps_Accelerate.cpp`
  for macOS (cblas via Accelerate framework, vDSP FFT)
- **Re-enable disabled IPP backends** — update to modern oneAPI APIs (see table above)
- **GLImg.cpp** — 1 remaining `ICL_HAVE_IPP` block in ICLQt
- **NeighborhoodOp.cpp** — 2 IPP bug workaround blocks (minor)
- **Expand benchmarks on Linux** — IPP vs C++ vs SIMD vs MKL comparison

### Key Files (Updated)

```
ICLUtils/src/ICLUtils/BackendDispatching.h     — framework (sorted vector storage)
ICLMath/src/ICLMath/BlasOps.h                  — BLAS/LAPACK dispatch singleton
ICLMath/src/ICLMath/BlasOps.cpp                — constructor, instance, toString
ICLMath/src/ICLMath/BlasOps_Cpp.cpp            — C++ fallback (naive GEMM + svd_dyn SVD)
ICLMath/src/ICLMath/BlasOps_Mkl.cpp            — MKL backend (cblas_xgemm + xgesdd)
ICLMath/src/ICLMath/FFTOps.h                   — FFT dispatch singleton
ICLMath/src/ICLMath/FFTOps.cpp                 — constructor, instance, toString
ICLMath/src/ICLMath/FFTOps_Cpp.cpp             — C++ fallback (row-column FFT)
ICLMath/src/ICLMath/FFTOps_Mkl.cpp             — MKL DFTI backend
ICLMath/src/ICLMath/FFTDispatching.h           — TRANSITIONAL (LegacyFFTOp, delete after wiring)
ICLMath/src/ICLMath/FFTDispatching.cpp         — TRANSITIONAL (delete after wiring)
```

## Previous State (Session 20 — All Filters Migrated to Prototype+Clone)

### Session 20 Summary

**All 15 filters now use the prototype+clone pattern.** The global string
registry is no longer needed for filter dispatch.

Migrated the remaining 14 filters (ThresholdOp was already done in session 19):

| Filter | Selectors | Backend Files | Notes |
|---|---|---|---|
| WienerOp | apply | _Cpp, _Ipp | Now always built (removed IPP-only exclusion from CMakeLists) |
| LUTOp | reduceBits | _Cpp, _Ipp | Two constructors, both clone from prototype |
| AffineOp | apply | _Cpp, _Ipp | C++ uses FixedMatrix for inverse transform |
| ConvolutionOp | apply | _Cpp, _Ipp | Large dispatch chain moved to _Cpp.cpp |
| MorphologicalOp | apply | _Cpp, _Ipp | Buffer accessors added for _Cpp.cpp access |
| BilateralFilterOp | apply | _Cpp, _OpenCL | C++ bilateral filter (all depths) |
| BinaryLogicalOp | apply | _Cpp, _Simd | dispatchEnum pattern preserved |
| BinaryArithmeticalOp | apply | _Cpp, _Simd | dispatchEnum pattern preserved |
| BinaryCompareOp | compare, compareEqTol | _Cpp, _Simd | 2 selectors |
| WarpOp | warp | _Cpp, _Ipp, _OpenCL | 3 backend files |
| MedianOp | fixed, generic | _Cpp, _Ipp, _Simd | Sorting networks + Huang median in _Cpp |
| UnaryArithmeticalOp | withVal, noVal | _Cpp, _Ipp, _Simd | 2 selectors |
| UnaryLogicalOp | withVal, noVal | _Cpp, _Ipp, _Simd | 2 selectors |
| UnaryCompareOp | compare, compareEqTol | _Cpp, _Ipp, _Simd | 2 selectors |

**Pattern for each migrated filter:**
1. **Header** — `enum class Op : int { ... }`, `static prototype()`,
   `toString(Op)` declaration with `ICLFilter_API`
2. **_Cpp.cpp** — C++ backend implementations, registers via
   `prototype().addBackend<Sig>(Op::x, Backend::Cpp, fn, desc)`
3. **_Ipp/_Simd/_OpenCL.cpp** — same registration pattern into prototype
4. **.cpp** — constructor is `ImageBackendDispatching(prototype())`,
   `apply()` uses `getSelector<Sig>(Op::x)` (enum-indexed, O(1))
5. `toString(Op)` free function for ADL (used by `addSelector(K)`)

**CMake change:** WienerOp is no longer excluded when `!IPP_FOUND`. Its C++
backend throws an exception (IPP required), but the class is always available.

**MorphologicalOp special handling:** The C++ backend accesses private
composite-operation buffers. Added public buffer accessors
(`openingBuffer()`, `gradientBuffer1()`, `gradientBuffer2()`) so the free
function in `_Cpp.cpp` can use them via the `MorphologicalOp&` parameter.

**14 new _Cpp.cpp files created:**
```
ICLFilter/src/ICLFilter/WienerOp_Cpp.cpp
ICLFilter/src/ICLFilter/LUTOp_Cpp.cpp
ICLFilter/src/ICLFilter/AffineOp_Cpp.cpp
ICLFilter/src/ICLFilter/ConvolutionOp_Cpp.cpp
ICLFilter/src/ICLFilter/MorphologicalOp_Cpp.cpp
ICLFilter/src/ICLFilter/BilateralFilterOp_Cpp.cpp
ICLFilter/src/ICLFilter/BinaryLogicalOp_Cpp.cpp
ICLFilter/src/ICLFilter/BinaryArithmeticalOp_Cpp.cpp
ICLFilter/src/ICLFilter/BinaryCompareOp_Cpp.cpp
ICLFilter/src/ICLFilter/WarpOp_Cpp.cpp
ICLFilter/src/ICLFilter/MedianOp_Cpp.cpp
ICLFilter/src/ICLFilter/UnaryArithmeticalOp_Cpp.cpp
ICLFilter/src/ICLFilter/UnaryLogicalOp_Cpp.cpp
ICLFilter/src/ICLFilter/UnaryCompareOp_Cpp.cpp
```

**Test results:** 349/349 pass (single-threaded). Multi-threaded test crash
is a **pre-existing bug** in the AffineOp test: heap-buffer-overflow in
`Img<icl8u>::subPixelLIN()` when bilinear-interpolating a 2×2 image
(ASAN confirmed at `AffineOp_Cpp.cpp:45`, `test-filter.cpp:745`).

### Known Issue: Stateful Backend Sharing

With the prototype+clone pattern, `ImplBase` objects are shared across all
instances via `shared_ptr`. This is correct for stateless backends (free
functions), but **stateful backends share mutable state** across instances:

| Backend | Shared State | Impact |
|---|---|---|
| WienerOp_Ipp.cpp | IPP scratch buffer (`vector<icl8u>`) | Concurrent calls corrupt buffer |
| WarpOp_OpenCL.cpp | `CLWarpState` (GPU buffers/kernels) | Concurrent calls corrupt GPU state |
| BilateralFilterOp_OpenCL.cpp | `CLBilateralState` | Same |
| MorphologicalOp_Ipp.cpp | `MorphIppState` (IPP state objects) | Same |

**Fix needed:** Add a factory/creator pattern to `ImplBase` so that stateful
backends can create fresh state per clone. The `BackendSelector::clone()`
should call `impl->cloneImpl()` instead of copying the `shared_ptr`:

```
Option A: virtual cloneImpl() on ImplBase
  - Stateless impls: return shared_from_this() (share as before)
  - Stateful impls: call factory, return new Impl with fresh state

Option B: Store optional factory lambda on ImplBase
  - add() takes optional Factory parameter
  - clone() calls factory if present, else shares shared_ptr
  - Backends register: proto.addStatefulBackend<Sig>(key, backend,
      factory, applicability, desc)
  - The factory lambda creates fresh state each time
```

**Important:** If the state contains `Image` or `Img<T>` members, the factory
must `deepCopy()` them — ICL images use shallow copy by default.

## Previous State (Session 19 — Full ImgOps Dispatch Migration)

### Session 19 Summary

**Img.cpp + Img.h now have zero `#ifdef ICL_HAVE_IPP` blocks.**

Migrated all 7 remaining IPP-guarded operations in Img.cpp/Img.h to the
ImgOps BackendDispatch framework (Img_Cpp.cpp / Img_Ipp.cpp):

| Operation | IPP Functions | Depths |
|---|---|---|
| clearChannelROI | `ippiSet_*_C1R` | 8u, 16s, 32s, 32f |
| lut | `ippiLUTPalette_8u_C1R` | 8u |
| getMax | `ippiMax/MaxIndx_*_C1R` | 8u, 16s, 32f |
| getMin | `ippiMin/MinIndx_*_C1R` | 8u, 16s, 32f |
| getMinMax | `ippiMinMax/MinMaxIndx_*_C1R` | 8u, 32f |
| normalize | `ippiMulC/AddC_32f_C1IR` | 32f |
| flippedCopyChannelROI | `ippiMirror_*_C1R` | 8u, 32f |

**Key design details:**
- **Type-erased dispatch signatures** — operations that return typed values
  (getMax, getMin) use `icl64f` return through dispatch, cast back at call site
- **const methods** — `getMax`/`getMin`/`getMinMax`/`lut` use `const_cast` to
  pass `const ImgBase*` through `ImgBase*` dispatch context (safe: resolve
  only checks depth, backends don't modify source)
- **clearChannelROI** — dispatch at `Img<T>::clear()` level, not in the header
  template itself (avoids circular include: Img.h→ImgOps.h→Image.h→Img.h).
  Direct callers of `clearChannelROI<T>()` get the C++ path.
- **Mirror helpers moved to Img_Cpp.cpp** — `getPointerOffset`,
  `getMirrorPointerOffsets`, `getMirrorPointers`, plus the per-channel
  `Img<T>::mirror(axis, int, Point, Size)` definition (with explicit
  instantiations). These were only used by mirror and flippedCopy.
- **scaledCopyChannelROI** `#if 0` dead code cleaned up — just uses
  `ICL_INSTANTIATE_ALL_DEPTHS` now (IPP APIs deprecated, TODO for future)
- **`getStartIndex`/`getEndIndex`** are protected on `Img<T>` — backends
  inline the logic: `startC = ch < 0 ? 0 : ch`

**New dispatch signatures in ImgOps:**
```
ClearChannelROISig  = void(ImgBase&, int ch, icl64f val, const Point& offs, const Size& size)
LutSig             = void(ImgBase& src, const void* lut, ImgBase& dst, int bits)
GetMaxSig          = icl64f(ImgBase&, int ch, Point* coords)
GetMinSig          = icl64f(ImgBase&, int ch, Point* coords)
GetMinMaxSig       = void(ImgBase&, int ch, icl64f* minVal, icl64f* maxVal, Point* minCoords, Point* maxCoords)
NormalizeSig       = void(ImgBase&, int ch, icl64f srcMin, icl64f srcMax, icl64f dstMin, icl64f dstMax)
FlippedCopySig     = void(axis, ImgBase& src, int srcC, const Point& srcOffs, const Size& srcSize,
                          ImgBase& dst, int dstC, const Point& dstOffs, const Size& dstSize)
```

**BackendDispatching rewrite (also this session):**
- `map<Backend,ImplPtr>` → `array<shared_ptr<ImplBase>, 4>` (fixed array, shared for cloning)
- Removed `backendPriority[]` — reverse iteration over enum values
- `BackendSelector` un-nested from `BackendDispatching` (standalone template)
- `resolve`/`resolveOrThrow`/`get` now const
- Added `virtual clone()` + clone constructor for per-instance dispatch
- Enum-keyed `addSelector(K)` with index assertion + ADL `toString(K)`
- Enum-keyed `getSelector<Sig>(K)` — O(1) vector index
- ImgOps backends register directly into singleton (no global registry)
- ThresholdOp migrated as proof of concept for prototype+clone pattern:
  all backends in `_Cpp.cpp` / `_Ipp.cpp`, constructor just clones prototype
- Removed dead code (`callWith`, `qualifiedName`, `backendPriority[]`)
- CoreFunctions `channel_mean` + ImgBorder `replicateBorder` migrated to ImgOps

**CMake note:** `FILE(GLOB)` is evaluated at configure time. After adding new
`_Cpp.cpp` / `_Ipp.cpp` files, re-run `cmake ..` to pick them up.

### Previous Session Summary (Session 18)

**Docker IPP build — now green:**
- Fixed `ContourDetector.cpp` missing `#include <cstring>`
- Fixed `CornerDetectorCSS.cpp` removed `ippsConv_32f` (deprecated, use C++ fallback)
- Fixed `CV.cpp` removed `ippiCrossCorrValid_Norm` / `ippiSqrDistanceValid_Norm` (deprecated)
- Fixed `TemplateTracker.h` missing `#include <ICLUtils/Point32f.h>`
- Fixed `DataSegment.h` missing `#include <cstring>`
- Fixed `ICLMarkers/CMakeLists.txt` spurious Qt PCH headers
- Fixed `FiducialDetectorPluginART.cpp` dead `Quick.h` include
- Fixed `FiducialDetectorPluginICL1.cpp` guarded `Quick.h`, added proper includes
- Fixed `ProximityOp.cpp` — provided stub implementations (was entirely inside `#if 0`)
- All modules compile and tests pass on Linux/IPP (Docker) and macOS

**Incremental Docker builds:**
- `build-and-test.sh` now uses `rsync` (preserves timestamps) instead of `cp -a`
- Use named Docker volume for persistent build cache
- Dockerfile adds `rsync` package

**ICLFilter IPP migration — complete:**
- Extracted `WarpOp` inline IPP (`ippiRemap_8u/32f_C1R`) to `WarpOp_Ipp.cpp` as `Backend::Ipp`
- Removed `#ifdef ICL_HAVE_IPP` from NeighborhoodOp.cpp (anchor workaround now always-on)
- Removed `#ifdef ICL_HAVE_IPP` from LocalThresholdOp.cpp (C++ path is both faster and higher quality)
- Removed `#ifdef ICL_HAVE_IPP` from UnaryOp.cpp (Canny creator — works without IPP now)
- Removed redundant `#ifdef ICL_HAVE_IPP` guards from all `_Ipp.cpp` files (CMake already excludes them)
- ICLFilter now has zero active `#ifdef ICL_HAVE_IPP` in non-backend files

**ImgOps singleton + ImgBaseBackendDispatching (new this session):**

Established the pattern for migrating `Img<T>` utility functions to dispatched backends.
First operation migrated: **mirror**.

Key design decisions:
- **ImgBaseBackendDispatching** (`BackendDispatching<ImgBase*>`) — new dispatch context for
  `Img<T>` methods, so they can dispatch via `this` without constructing an `Image` wrapper
- **ImgOps singleton** — inherits from `ImgBaseBackendDispatching`, owns `BackendSelector`s
  for Img utility operations (mirror, and later: min/max, lut, normalize, etc.)
- **ALL implementations in separate files** — `Img_Cpp.cpp` has the C++ fallback,
  `Img_Ipp.cpp` has the IPP backend. The `Img<T>` method itself is dispatch-only.
  This ensures dispatch is always used regardless of call path (`Image::mirror()` or
  `Img<T>::mirror()` directly).
- **`applicableToBase<Ts...>()`** — applicability helper for `ImgBase*` context (checks depth)
- **`resolveOrThrow()`** — safe dispatch that throws `std::logic_error` with selector name
  instead of returning nullptr

**Mirror migration details:**

```
Call chain:
  Image::mirror(axis)
    → ImgBase::mirror(axis, bool)  [virtual]
      → Img<T>::mirror(axis, bool)  [dispatch-only, calls resolveOrThrow]
        → ImgOps::instance().getSelector<MirrorSig>("mirror").resolveOrThrow(this)
          → Backend::Ipp: Img_Ipp.cpp — ippiMirror_8u/16u/32s_C1IR (4 depths)
          → Backend::Cpp: Img_Cpp.cpp — calls Img<T>::mirror(axis, ch, offset, size)

The per-channel Img<T>::mirror(axis, int, Point, Size) is the raw C++ swap
implementation. It never dispatches — backends call it directly.
```

Files created:
- `ICLCore/src/ICLCore/ImgOps.h` — singleton header, dispatch signatures
- `ICLCore/src/ICLCore/ImgOps.cpp` — singleton impl, creates selectors
- `ICLCore/src/ICLCore/Img_Cpp.cpp` — C++ backend (mirror)
- `ICLCore/src/ICLCore/Img_Ipp.cpp` — IPP backend (mirror)
- `ICLCore/src/ICLCore/ImageBackendDispatching.h` — added `ImgBaseBackendDispatching` + `applicableToBase<>`

Changes:
- `ICLCore/CMakeLists.txt` — added `_Ipp.cpp` exclusion pattern (same as ICLFilter)
- `ICLCore/src/ICLCore/Img.cpp` — `Img<T>::mirror(axis, bool)` is now dispatch-only
- `ICLCore/src/ICLCore/Img.h` — made per-channel mirror/normalize public (backends need access)
- `ICLUtils/src/ICLUtils/BackendDispatching.h` — added `resolveOrThrow()`, `#include <stdexcept>`

### Important Rules (Learned This Session)

1. **Never delete IPP/MKL code** — extract to `_Ipp.cpp`/`_Mkl.cpp` backend files.
   IPP specializations have real performance value (BLAS, image ops, etc.).

2. **No `#ifdef ICL_HAVE_IPP` in `_Ipp.cpp` files** — CMake excludes them via
   `list(FILTER SOURCES EXCLUDE REGEX "_Ipp\\.cpp$")` when `!IPP_FOUND`.

3. **All implementations in backend files** — both `_Cpp.cpp` AND `_Ipp.cpp`.
   The main code is dispatch-only. This ensures dispatch works regardless of call path.

4. **MKL follows the same pattern** — `_Mkl.cpp` files, `Backend::Mkl` enum (to be added).

### Previous Session Summary (Session 17)

**BackendDispatching refactoring:**
- Nested `BackendSelectorBase`, `BackendSelector<Sig>`, `ApplicabilityFn` inside `BackendDispatching<Context>`
- `ImageBackendDispatching` is now just `using = BackendDispatching<Image>` (removed `Dispatching` alias)
- All 15 filter headers updated from `core::Dispatching` to `core::ImageBackendDispatching`
- `dispatchEnum` applied to BinaryOp SIMD backends to eliminate inner-loop branching

**Cross-validation tests (20 new, 349 total):**
- Added `crossValidateBackends()` template helper (forces C++ ref, iterates all backend combos)
- All 15 BackendDispatch filters now have cross-validation tests
- Tests cover per-depth validation for all applicable depths

**Benchmarks (25 filter benchmarks):**
- All benchmarks use 1000x1000 (1M pixels) baseline
- Backend parameter: `-p backend=cpp/simd/ipp/auto` for direct comparison

### IPP APIs — What's Active vs Disabled

**ACTIVE (compiles with modern oneAPI IPP 2022+):**

| Backend File | IPP Functions | Filter |
|---|---|---|
| ThresholdOp_Ipp.cpp | `ippiThreshold_LTVal/GTVal_*` | ThresholdOp |
| UnaryCompareOp_Ipp.cpp | `ippiCompareC_*` | UnaryCompareOp |
| UnaryLogicalOp_Ipp.cpp | `ippiAndC/OrC/XorC_*` | UnaryLogicalOp |
| WienerOp_Ipp.cpp | `ippiFilterWiener_*` | WienerOp |
| WarpOp_Ipp.cpp | `ippiRemap_*` | WarpOp |
| Img_Ipp.cpp | `ippiMirror_*` | Img::mirror (new) |
| Img.cpp (inline) | `ippiLUTPalette_*`, `ippiMin/Max*`, `ippiMulC/AddC_*` | Img utilities (TODO) |
| CoreFunctions.cpp (inline) | `ippiMean_*` | channel mean (TODO) |
| DynMatrixUtils.cpp (inline) | `ippsMean_*`, `ippsStdDev_*`, `ippsMeanStdDev_*` | matrix stats (TODO) |
| DynMatrix.h (inline) | `ippsDiv_*`, `ippsMulC_*`, `ippsNorm_*`, `ippsNormDiff_*` | matrix ops (TODO) |
| MathFunctions.h (inline) | `ippsMean_*` | math mean (TODO) |

**DISABLED (deprecated/removed APIs — TODO re-add via BackendDispatch):**

| Location | Deprecated API | Modern Replacement | Priority |
|---|---|---|---|
| `ConvolutionOp_Ipp.cpp` | `ippiFilterSobelHoriz/Vert/Laplace/Gauss_*` | `ippiFilterSobelBorder_*`, `ippiFilterGaussBorder_*` | HIGH — 34 specializations |
| `MorphologicalOp_Ipp.cpp` | `ippiMorphologyInitAlloc_*`, `ippiDilate/Erode_*_C1R` | `ippiDilate/Erode_*_C1R_L` + spec buffers | HIGH |
| `AffineOp_Ipp.cpp` | `ippiWarpAffine_*_C1R` | `ippiWarpAffineNearest/Linear_*` + spec | MEDIUM |
| `MedianOp_Ipp.cpp` | `ippiFilterMedian_*_C1R` | `ippiFilterMedianBorder_*_C1R` | MEDIUM |
| `LUTOp_Ipp.cpp` | `ippiReduceBits_8u_C1R` | Modern `ippiReduceBits` (added noise param) | LOW |
| `CannyOp.cpp` (inline) | `ippiCanny_32f8u/16s8u_C1R` | Modern `ippiCanny` with border spec | MEDIUM |
| `ProximityOp.cpp` | `ippiSqrDistance/CrossCorr Full/Same/Valid_Norm_*` | `ippiSqrDistanceNorm_*` | LOW |
| `Img.cpp` (inline) | `ippiResizeSqrPixel_*` | `ippiResizeLinear/Nearest_*` | MEDIUM |
| `CoreFunctions.cpp` (inline) | `ippiHistogramEven_*` | `ippiHistogram_*` (new API) | LOW |
| `FFTUtils.cpp` (inline) | `ippiFFTInitAlloc_*` | `ippiFFTInit_*` + manual buffers | MEDIUM (or use MKL) |
| `DynMatrix.h/.cpp` (inline) | `ippmMul_mm/Invert/Det/Eigen_*` | MKL BLAS/LAPACK | MEDIUM (ippm module dropped entirely) |
| `DynMatrixUtils.cpp` (inline) | `ippmAdd/Sub/Mul_mm/tm/tt_*` | MKL BLAS | MEDIUM |

### Backend Dispatch Framework

```
BackendDispatching<Context>           — ICLUtils (header-only, no .cpp)
  BackendSelectorBase<Context>        — abstract per-selector base
  BackendSelector<Context, Sig>       — typed dispatch table
    .add(b, f, applicability, desc)   — register stateless backend
    .addStateful(b, factory, app, d)  — register stateful backend (factory per clone)
    .resolve(ctx) → ImplBase*         — returns nullptr if no match
    .resolveOrThrow(ctx) → ImplBase*  — throws logic_error if no match
    .clone()                          — stateful: calls cloneFn(); stateless: shares shared_ptr
  ApplicabilityFn<Context>            — std::function<bool(const Context&)>
  ImplBase::cloneFn                   — optional factory for stateful backends

API on BackendDispatching<Context>:
  addSelector<Sig>(K key)             — enum-keyed only (no string overloads)
  getSelector<Sig>(K key)             — O(1) vector index
  selector(K key)                     — returns BackendSelectorBase* (introspection/tests)
  addBackend<Sig>(K, b, f, app, desc) — convenience for getSelector().add()
  addStatefulBackend<Sig>(K, b, factory, app, desc) — convenience for getSelector().addStateful()

Two context types:
  ImageBackendDispatching             — BackendDispatching<Image>
  ImgBaseBackendDispatching           — BackendDispatching<ImgBase*>

ImgOps singleton                      — ICLCore (enum class Op, 10 selectors)
FFTDispatching singleton              — ICLMath (enum class FFTOp, 3 selectors)

Filter prototype+clone pattern        — all 15 ICLFilter ops
  Static prototype() holds selectors + ImplBase objects
  Constructor clones: ImageBackendDispatching(prototype())
  Stateful backends get fresh state per instance via factory cloneFn
  _Cpp.cpp / _Ipp.cpp / _Simd.cpp / _OpenCL.cpp register into prototype()

Backend enum: Cpp, Simd, Ipp, OpenCL  — ICLUtils
Priority: OpenCL > Ipp > Simd > Cpp

CMake: _Ipp.cpp excluded when !IPP_FOUND, _OpenCL.cpp when !OPENCL_FOUND
       _Cpp.cpp always built
```

### Remaining Inline `#ifdef ICL_HAVE_IPP` Blocks to Migrate

**ICLCore — Img.cpp and Img.h are DONE (zero `#ifdef ICL_HAVE_IPP`).**

Remaining ICLCore files:
- `CoreFunctions.cpp` — channel_mean specializations (4 depths)
- `ImgBorder.cpp` — border replication (8u, 32f)
- `CCFunctions.cpp` — planarToInterleaved/interleavedToPlanar macros
- `BayerConverter.h/.cpp` — Bayer pattern conversion
- `Types.h` — conditional enum definitions (compile-time, may stay)

**ICLMath — needs own dispatch singleton (similar pattern):**
- `DynMatrix.h` — `ippsNormDiff_L2_*`, `ippsDiv_*`, `ippsMulC_*`, `ippsNorm_*`
- `MathFunctions.h` — `ippsMean_*`
- `DynMatrixUtils.cpp` — mean/stddev/meanstddev (3 blocks), unary math functions (large block)

**ICLIO — needs own dispatch or extend ImgOps:**
- `DC.cpp` — `ippiRGBToGray_8u_C3C1R`
- `ColorFormatDecoder.cpp` — `ippiYUVToRGB_8u_C3R`
- `PylonColorConverter.cpp/.h` — YUV conversion classes

### Docker Build Commands

```bash
# First run (full build with persistent volume):
docker build --platform linux/amd64 -t icl-ipp packaging/docker/noble-ipp
docker run --platform linux/amd64 --rm -e JOBS=16 -e BUILD_DIR=/build-cache \
  -v $(pwd):/src:ro -v icl-ipp-build:/build-cache \
  icl-ipp bash /src/packaging/docker/noble-ipp/build-and-test.sh

# Subsequent runs (incremental — only recompiles changed files):
# Same command — volume "icl-ipp-build" persists CMake state + object files
```

### Key Files

```
ICLUtils/src/ICLUtils/BackendDispatching.h     — framework template (header-only, no .cpp)
ICLUtils/src/ICLUtils/EnumDispatch.h           — dispatchEnum utility
ICLCore/src/ICLCore/ImageBackendDispatching.h  — Image + ImgBase* typedefs
ICLCore/src/ICLCore/ImgOps.h                   — singleton header, dispatch signatures
ICLCore/src/ICLCore/ImgOps.cpp                 — singleton impl, creates selectors
ICLCore/src/ICLCore/Img_Cpp.cpp                — C++ backends (8 ops + mirror helpers)
ICLCore/src/ICLCore/Img_Ipp.cpp                — IPP backends (8 ops)
ICLMath/src/ICLMath/FFTDispatching.h           — FFTOp enum, FFTDispatching singleton
ICLMath/src/ICLMath/FFTDispatching.cpp         — FFT C++ backends
ICLFilter/src/ICLFilter/*_Cpp.cpp              — 15 C++ backend files (one per filter)
ICLFilter/src/ICLFilter/*_Ipp.cpp              — IPP backends (excluded when !IPP_FOUND)
ICLFilter/src/ICLFilter/*_Simd.cpp             — SIMD backends (always built)
ICLFilter/src/ICLFilter/*_OpenCL.cpp           — OpenCL backends (excluded when !OPENCL_FOUND)
tests/test-filter.cpp                          — 349 tests
benchmarks/bench-filter.cpp                    — 25 filter benchmarks
packaging/docker/noble-ipp/                    — Docker IPP build
.github/workflows/ci.yaml                     — CI with IPP job
```

### Next Steps

#### A. ~~Migrate all 14 remaining filters to prototype+clone pattern~~ **DONE** (Session 20)

All 15 filters now use prototype+clone. See Session 20 summary above.

#### A2. ~~Remove global string registry + add stateful backend cloning~~ **DONE** (Session 21)

See Session 21 summary above. All three phases complete.

#### B. ~~Remaining ICLCore IPP blocks~~ **DONE** (Session 21)

- ~~CoreFunctions.cpp — channel_mean~~ **DONE** (Session 19)
- ~~ImgBorder.cpp — border replication~~ **DONE** (Session 19)
- ~~CCFunctions.cpp — planarToInterleaved/interleavedToPlanar~~ **DONE** (Session 21, added to ImgOps)
- ~~BayerConverter.h/.cpp~~ **DONE** (Session 21, removed dead IPP code — `nnInterpolationIpp` was never called)
- Types.h — enum value definitions (compile-time, stays as-is)

#### C. Other modules

- ~~**ICLMath IPP**~~ **DONE** (Session 21) — MathOps<T> singletons, all `ICL_HAVE_IPP` removed
  from headers + DynMatrixUtils.cpp. CMake `_Ipp.cpp`/`_Mkl.cpp` exclusion added.
- **ICLMath MKL** — 27+ `#ifdef ICL_HAVE_MKL` blocks remain (DynMatrix, DynMatrixUtils,
  FFTUtils). Need `_Mkl.cpp` files + `Backend::Mkl` enum value. Deferred.
- ~~**ICLIO**~~ **DONE** (Session 21) — DC.cpp, ColorFormatDecoder.cpp, PylonColorConverter
  IPP guards removed, C++ fallbacks always used, TODOs added
- **Update disabled IPP backends** to modern oneAPI APIs
- **Expand benchmarks on Linux** — IPP vs C++ vs SIMD comparison
