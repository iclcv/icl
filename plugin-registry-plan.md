# Plugin Registry Unification Plan

Collapse ICL's 8+ bespoke plugin-registration mechanisms into one
`utils::PluginRegistry<...>` primitive with a semantic split between
**function-plugins** (payload is a callable) and **class-plugins**
(payload is a factory producing `unique_ptr<T>`).

## Current state — eight mechanisms

1. **`utils::PluginRegister<T>`** (`icl/utils/PluginRegister.h`) — template, singleton, last-wins, `T*(const Data&)` factory where `Data = map<string,string>`. Used by `PointCloudGrabber`, `PointCloudOutput` (geom module). Predates the ICLIO registries; no one ever wired the classics to it.
2. **`GrabberRegister`** (`icl/io/Grabber.{h,cpp}`) — singleton, throw-on-dup, file-scope static struct registration, `Grabber*(const string&)` factory + parallel maps for device-list / bus-reset / description.
3. **`CompressionRegister`** (`icl/io/CompressionRegister.{h,cpp}`) — singleton, throw-on-dup, `__attribute__((constructor))` registration, `unique_ptr<CompressionPlugin>()` factory. Codec plugins inherit `Configurable`.
4. **`FileWriterPluginRegister`** (`icl/io/FileWriter.{h,cpp}`) — singleton, first-wins + override-existing flag, attribute-constructor registration, `unique_ptr<FileWriterPlugin>()` factory, per-extension lifetime cache.
5. **`FileGrabberPluginRegister`** (`icl/io/FileGrabber.{h,cpp}`) — mirror of FileWriter.
6. **`GenericImageOutput`** (`icl/io/GenericImageOutput.cpp`) — NOT a registry: a hardcoded `#ifdef`-chain switch in `init()`. `"list"` type prints help.
7. **`BackendSelector<Context, Sig>`** (`icl/utils/BackendDispatching.h`) — per-Op prototype, enum-keyed, applicability predicate + priority resolution, clone-on-instance, forced-backend override, `R(Args...)` callable payload with optional stateful factory.
8. **`BackendDispatching<Context>`** — container of `BackendSelector`s, enum-indexed, with clone constructor. Outer layer above (7).

## Design decisions (locked)

### D1 — Scope: all 8, two of them via a degenerate path

- Classics (1–6) collapse onto the new primitive as direct replacements.
- `BackendSelector` (7) uses the same primitive internally.
- `BackendDispatching` (8) keeps its outer shell (enum selector indexing + clone + `forceAll`) — that's a container of registries, not a registry itself.

### D2 — Semantic split: function-plugin vs class-plugin

```cpp
namespace icl::utils {

struct NoContext {};

// One primitive
template <class Key, class Payload, class Context = NoContext>
class PluginRegistry;

// Alias A: function-plugin — payload IS the callable
template <class Signature>
using FunctionPluginRegistry =
    PluginRegistry<std::string, std::function<Signature>>;

// Alias B: class-plugin — payload is a factory producing unique_ptr<T>
template <class T, class... CtorArgs>
using ClassPluginRegistry =
    PluginRegistry<std::string, std::function<std::unique_ptr<T>(CtorArgs...)>>;

} // namespace icl::utils
```

No shared `PluginBase` type. Each domain's class hierarchy stays as-is
(`CompressionPlugin`, `Grabber`, etc.); the registry's template parameter
names that hierarchy.

### D3 — Entry fields inspired by `BackendSelector`

Each registration carries: key, payload, applicability predicate (default
always-true), description, priority (default 0). Classics default
applicability/priority to no-ops; BackendSelector uses them actively.

### D4 — Policies as ctor args, not template params

```cpp
enum class OnDuplicate { Throw, KeepFirst, Replace };
enum class InstanceCache { Fresh, CachedPerKey };

PluginRegistry(OnDuplicate = OnDuplicate::Throw,
               InstanceCache = InstanceCache::Fresh);
```

### D5 — One registration macro, `__attribute__((constructor, used))`

```cpp
#define ICL_REGISTER_PLUGIN(REGISTRY_EXPR, TAG, KEY, PAYLOAD)            \
  extern "C" __attribute__((constructor, used)) void                     \
  iclRegisterPlugin_##TAG() {                                            \
    (REGISTRY_EXPR).registerPlugin(KEY, PAYLOAD);                        \
  }
```

Drops the file-scope-static-struct idiom from GrabberRegister and
PluginRegister (Session 47 lessons: attribute-constructor is the only
macOS-portable approach that survives dead-stripping).

### D6 — Singleton access is external

`PluginRegistry<...>` is a plain object. Domain wrappers provide
`instance()` accessors (matches the `CompressionRegister::instance()`
style). `BackendSelector` uses it as a per-Op member — no singleton.

### D7 — GrabberRegister's extras stay on a thin wrapper

Per-backend description (moves into Entry), device-list function, and
bus-reset function are GrabberRegister-specific. Description moves into
the primitive's Entry; the other two stay as ancillary side maps on
`GrabberRegister` itself, which wraps a `ClassPluginRegistry<Grabber, const std::string&>`
plus those two small maps.

### D8 — Which mechanism uses which alias

| Mechanism | Alias | Notes |
|---|---|---|
| Existing `utils::PluginRegister<T>` | `ClassPluginRegistry<T, const Data&>` | Preserves `Data = map<string,string>` ctor-arg shape |
| `GrabberRegister` | `ClassPluginRegistry<Grabber, const std::string&>` | Plus side maps for bus-reset / device-list |
| `CompressionRegister` | `ClassPluginRegistry<CompressionPlugin>` | Codec plugins keep Configurable inheritance |
| `FileWriterPluginRegister` | **`FunctionPluginRegistry<void(const Image&, const std::string&)>`** | Single-method; class dies, free function replaces it |
| `FileGrabberPluginRegister` | **`FunctionPluginRegistry<Image(const std::string&)>`** | Mirror |
| `GenericImageOutput` backends | **`FunctionPluginRegistry<void(const Image&)>`** | Single-method; class dies, bound callable replaces it |
| `PointCloudGrabber` / `PointCloudOutput` | `ClassPluginRegistry<..., const Data&>` | Multi-method; stay as classes |
| `BackendSelector<Context, Sig>` | Direct use of primitive with `Key = Backend`, `Context ≠ NoContext` | Priority + applicability actively used |

Function-plugin conversions (FileWriter/FileGrabber/ImageOutput) delete
those plugin base classes entirely — they become naming conventions for
free functions.

---

## Execution phases

### Phase 0 — Primitive (~1.5h)

1. `icl/utils/PluginRegistry.h` — header-only template per D2/D3/D4
2. `icl/utils/PluginRegistryMacros.h` — `ICL_REGISTER_PLUGIN` per D5
3. `icl/utils/test/` — new directory; `test-plugin-registry.cpp` covering:
   - Both aliases round-trip
   - All three `OnDuplicate` policies
   - Cache-per-key + invalidation on re-register
   - Priority+applicability resolution
   - Forced override
   - Threading stress
4. Wire into `icl/utils/meson.build` (gtest discovery)
5. **Gate:** all new tests pass before touching any consumer

### Phase 1 — Refactor `BackendSelector` onto the primitive (~1.5h)

Validates the primitive's richer use case. BackendDispatching's outer
shell stays.

- `BackendSelector<Context, Sig>` becomes a thin façade over
  `PluginRegistry<Backend, shared_ptr<ImplBase>, Context>` with
  `OnDuplicate::Replace`.
- All filter-Op prototypes (Median, Convolution, Gabor, Bilateral, …)
  untouched at the source level — internal refactor only.
- Run full filter test suite; expect zero behavioural change.

### Phase 2 — Migrate `CompressionRegister` (~45m)

- `CompressionRegister` becomes a thin singleton wrapper over
  `ClassPluginRegistry<CompressionPlugin>` (throw-on-dup, fresh).
- `REGISTER_COMPRESSION_PLUGIN` becomes a one-line alias over
  `ICL_REGISTER_PLUGIN`.
- No changes to the 5 `CompressionPluginXxx.cpp` files.

### Phase 3 — Retire `utils::PluginRegister<T>` (~30m)

- Delete `icl/utils/PluginRegister.h`.
- Migrate `PointCloudGrabber` + `PointCloudOutput` registries to
  `ClassPluginRegistry<..., const Data&>` (3 `REGISTER_PLUGIN` sites:
  pcd, dcam, scene, null).
- Rename the `REGISTER_PLUGIN` macro sites to `ICL_REGISTER_PLUGIN`.

### Phase 4 — Function-plugin conversions (~2.5h)

#### 4a — `ImageOutput` → `FunctionPluginRegistry<void(const Image&)>`
- Delete `ImageOutput` base class; each backend becomes a free function
  capturing its state in a closure (e.g. `WSImageOutput`'s server socket).
- `GenericImageOutput::init()` reduces from ~140-line `#ifdef` switch to
  registry lookup + `"list"` auto-generated from `names()` + descriptions.
- Touches: `LibAVVideoWriter`, `ZmqImageOutput`, `WSImageOutput`,
  `V4L2LoopBackOutput`, `FileWriter` (as output), `NullOutput`.

#### 4b — `FileWriterPlugin` → `FunctionPluginRegistry<void(const Image&, const std::string&)>`
- Delete `FileWriterPlugin` base class; each backend becomes a free
  function. Per-extension cache migrates into the registry entry.
- Touches: PNG, JPG, BICL, CSV, PNM, ImageMagick, PDF (~7 files).

#### 4c — `FileGrabberPlugin` → `FunctionPluginRegistry<Image(const std::string&)>`
- Mirror of 4b. Touches symmetric set.

Each sub-phase is independently landable. Order 4a → 4b → 4c; each can
be its own commit.

### Phase 5 — Migrate `GrabberRegister` (~1h)

- Convert factory return type from `Grabber*` to `unique_ptr<Grabber>`.
- Wraps `ClassPluginRegistry<Grabber, const std::string&>` plus
  side maps for device-list / bus-reset.
- Description field flows through Entry now.
- Update ~10 grabber plugin .cpp files' factory lambdas (one-line each).

### Phase 6 — Demolish the shallow wrapper classes (~1.5h)

After Phases 2 and 3, `CompressionRegister` and `utils::PluginRegister<T>`
exist only as thin façades over the primitive. Delete them, adapt callers
directly onto `ClassPluginRegistry<...>`:

- **`CompressionRegister` → gone.** Consumers (`ImageCompressor.cpp`,
  `WSImageOutput.cpp`) switch to `ClassPluginRegistry<CompressionPlugin>&
  compressionRegistry()` (free function returning the singleton). The
  5 codec .cpp files swap `REGISTER_COMPRESSION_PLUGIN` for
  `ICL_REGISTER_PLUGIN(compressionRegistry(), jpeg, "jpeg", ...)`.
- **`utils::PluginRegister<T>` → gone.** The two geom consumers
  (`GenericPointCloudGrabber`, `GenericPointCloudOutput`) switch to
  `ClassPluginRegistry<PointCloudGrabber, const Data&>&`/`<...Output...>&`
  free-function singletons. The 4 registration sites swap
  `REGISTER_PLUGIN(TYPE, NAME, fn, desc, syntax)` for
  `ICL_REGISTER_PLUGIN(...)` — `creationSyntax` either folds into the
  description field or becomes a secondary free-function side-map (TBD
  when we get there).
- `getRegisteredInstanceDescription()`'s TextTable rendering migrates
  to a small free helper `utils::pluginListTable(registry, syntaxMap)`.

Deferred until after Phase 4 lands so the primitive's contract stabilizes
(Phase 4 may nudge the `PluginRegistry` API in minor ways — better to
settle once).

### Phase 7 (optional) — Extend priority usage

Replace FileWriter/FileGrabber's `overrideExisting` bool with priority:
- ImageMagick registers at `priority = -10`
- libpng / libjpeg register at default `priority = 0`
- libpng wins for `.png`; ImageMagick still handles formats libpng
  doesn't.

This is strictly nicer than the bool flag but not blocking.

### Phase 8 (landed 2026-04-20) — Final façade cleanup

After the post-Phase-7 discussion the remaining façades got treated in
three ways:

- **`FileWriterPluginRegister` façade → demolished.** Replaced by a free
  function `fileWriterRegistry()` returning `FunctionPluginRegistry<Sig>&`
  directly. `REGISTER_FILE_WRITER_PLUGIN` becomes a one-line alias over
  `ICL_REGISTER_PLUGIN`. ImageMagick's priority registration calls the
  registry's `registerPlugin` directly.
- **`FileGrabberPluginRegister` façade → demolished.** Symmetric to
  above; `fileGrabberRegistry()` free function.
- **`GrabberRegister` → renamed to `GrabberRegistry` (façade kept).**
  Keeps its side maps for device-list / bus-reset / per-backend
  descriptions — those orthogonal concerns don't fit the primitive's
  `Entry` shape. Rename acknowledges the new naming convention while
  admitting this façade has domain-specific value worth keeping.

Naming convention across the codebase is now:
- Free-function accessors for demolished façades: `compressionRegistry()`,
  `fileWriterRegistry()`, `fileGrabberRegistry()`, `pointCloudGrabberRegistry()`,
  `pointCloudOutputRegistry()`, `imageOutputRegistry()`.
- Remaining class-shaped façade: `GrabberRegistry` (was `GrabberRegister`).
- Primitive: `utils::PluginRegistry<...>` + aliases.

---

## Verification strategy

- Full build clean after each phase
- `ctest` — all 551+ tests pass (Phase 0 adds new unit tests)
- Smoke test after Phase 4: `icl-pipe -i create lena -o ws PORT` +
  `icl-pipe -i ws PORT -o file '/tmp/###.png'` round-trip
- Smoke test after Phase 5: `icl-camera-param-io -i list` shows all
  grabbers

## Estimated total effort

| Phase | Effort |
|---|---|
| 0 — Primitive + tests | ~1.5h |
| 1 — BackendSelector refactor | ~1.5h |
| 2 — CompressionRegister | ~45m |
| 3 — Retire PluginRegister + PointCloud migration | ~30m |
| 4 — Function-plugin conversions (3 sub-phases) | ~2.5h |
| 5 — GrabberRegister | ~1h |
| 6 — Demolish CompressionRegister + PluginRegister<T> façades | ~1.5h |
| 7 — Priority-based FileWriter conflicts | ~30m |
| 8 — Demolish FileWriter/FileGrabber façades, rename GrabberRegister → GrabberRegistry | ~45m |
| **Total (0–8)** | **~10h** |

## Net result

- ~600 lines of duplicated registry code collapse into one ~200-line template
- Three plugin base classes deleted (`ImageOutput`, `FileWriterPlugin`, `FileGrabberPlugin`)
- One macro (`ICL_REGISTER_PLUGIN`) replaces five domain-specific macros + one file-scope-static idiom
- Uniform introspection: `registry.names()`, `registry.entries()` across all registries
- `BackendSelector` and classic registries speak the same vocabulary
