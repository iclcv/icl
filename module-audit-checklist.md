# Module Header Audit Checklist

Reusable audit protocol for ICL modules.  Applied first to `icl/utils/`
(Session 49+), to be rerun for `icl/core/`, `icl/filter/`, `icl/math/`
public-API residue, and so on.

For each header file in a module, walk the checks below and record a
verdict per the "Decision matrix" at the bottom.

---

## Per-header checks

### 1. Usage footprint

- **In-tree include count.** `grep -rn "#include.*<module>/Foo\.h" icl/ tests/ benchmarks/` minus self-include.
- **Which modules consume it?** Layering sanity check: utils → nothing; math → utils; core → utils+math; filter → core; etc.
- **Tests / benchmarks / apps / demos / examples?** Hit counts for each category.
- **External signatures?** Does it appear in any *other installed public header*'s signatures? If yes, even with zero in-tree .cpp use, it's load-bearing.

### 2. Purpose — is it still required?

- One-line description: what does it provide?
- Does modern C++ stdlib cover this?
  - `chrono` vs hand-rolled `Time` / `Timer`
  - `std::filesystem` vs custom `File`
  - `<random>` vs hand-rolled RNG
  - `std::format` / `std::println` vs custom printing
  - `std::variant` / `std::any` vs hand-rolled `Any`
  - `std::scoped_lock` + `std::mutex` vs `Lockable`
  - `{fmt}` / `spdlog` / `std::print` vs custom logging
- Does a third-party header-only lib cover it better (e.g., CLI11 for ProgArg)?
- Is the functionality still exercised by any active code path?

### 3. If unused — still useful as framework API?

ICL is a framework; out-of-tree users may reasonably want APIs nothing
in-tree uses.  But "might be useful someday" is not the bar.

- Is the API named/shaped in a way an external user would discover?
- Is it referenced in user-facing docs (README, manual/, doxygen pages)?
- Is it mentioned in the thesis chapter, papers, or ROS integration?
- If none of the above → it's dead code.  Retire it.

### 4. Does it look like it works?

Quick smoke-read of the file:

- Obvious bugs, unused members, dangling TODOs referencing removed features?
- Commented-out blocks that predate the last refactor (smell)?
- Dead branches / `#if 0` / old `#ifdef HAVE_FOO` for deps ICL no longer supports?
- Broken doxygen (`\ingroup` pointing nowhere, stale example code)?
- Stale coauthor tags / copyright dates out of sync?

### 5. C++ idiom compliance

- **Reinventing stdlib?** See §2. Concrete example: a custom `non_copyable` base is usually a pre-C++11 smell — `= delete` is the right answer today.
- **Pre-C++11 patterns?** `boost::noncopyable`-style CRTP for what's now `= delete`; hand-rolled `scoped_ptr` for `unique_ptr`; hand-rolled RAII where `std::scoped_lock` / `std::unique_lock` / `std::lock_guard` suffices.
- **Naked `#define`** constants or function-like macros that could be `constexpr` / `inline`.
- **Raw pointer ownership** (`T *` returned meaning "caller owns") — should be `unique_ptr<T>` in modern code.
- **`typedef`** rather than `using`.
- **`throw()`** rather than `noexcept`.
- **Output parameters** `void foo(T *out)` where `T foo()` returning by value is clearer (NRVO).
- **Friend-abuse** where a small public accessor would do.
- **Header-visible PIMPL** (full impl struct definition in the header, only the pointer is the "PIMPL") — defeats the purpose.
- **Identifier style**: ICL prefers camelCase for variables and methods (per `feedback_camelcase.md`).  snake_case in new APIs is a smell.
- **Zero-arg explicit ctors** `explicit Foo()` — `explicit` does nothing on 0-arg ctors pre-C++11 usage; in C++20 for list-init it does, but the common case is just noise.

### 6. Location: could it move to `detail/`?

Strict detail/ rule (per `feedback_detail_strict_rule.md`):
**`detail/` ⟺ not installed.**

- Does any *installed* public header `#include` this file (directly or transitively)?
  - If **yes** → must stay at module top-level (or move to a different public subdir like `cl/`).
  - If **no** → eligible for `detail/`.
- Even if eligible, second question: **should** it move?  Criteria:
  - All consumers are .cpps inside the same module → strong candidate.
  - Consumers span multiple modules' .cpps but no public header → still a candidate (detail/ is an install boundary, not a visibility one).
  - The header is part of an obvious public framework API → even if currently unused, keep it public.

### 7. Can inline implementations move to `.cpp`?

A header with heavy inline bodies bloats compile times and exposes
unnecessary implementation detail.  For each non-trivial method body
in the header, ask:

- Is the method definition short and performance-critical (accessor, one-line) → keep inline.
- Is the method body 5+ lines, or calls into another TU's state, or uses types that would otherwise be forward-decl-only → **move to .cpp**.
- Does the inline body force the header to `#include` a heavy STL header (e.g., `<regex>`, `<iostream>`, `<algorithm>` with heavy templates)? If so and the body is not latency-critical → move.
- Are there templates? Templates generally must stay in the header, but
  explicit instantiation + `extern template` can move impl to `.cpp`
  when the set of instantiations is closed.

### 8. Includes hygiene

- Which of the header's `#include`s can become forward declarations?
- Are any includes of `detail/` files present in this (installed) header → **violation, fix**.
- Are unused includes present → prune.
- Precompiled-header overlap (the module's `_pch/<module>.h` covers common needs — the header should not re-pull those).

### 9. Is there a better place for it?

- Does it sit in the wrong module?  Example: `IntrinsicCalibrator` moved
  `io → cv` in Session 48; `ImageUndistortion` moved `io → filter`.
- Could the header be folded into a sibling? Tiny one-concept headers
  sometimes make more sense as a section of a larger file.

### 10. Documentation

- Does the public API have doxygen on entry points?
- Do examples in comments still compile?
- Is the `\defgroup` sane?

---

## Decision matrix (per header)

After walking the checks, classify:

| Verdict | Meaning | Action |
|---|---|---|
| **KEEP** | Load-bearing public API; idiomatically fine. | No action. |
| **KEEP, POLISH** | Load-bearing but has idiom smells. | Targeted refactor (inline-to-cpp, typedef-to-using, raw-ptr-to-unique_ptr, etc). |
| **PRIVATIZE** (move to `detail/`) | Not in any public header's surface; only .cpp consumers. | git mv to `detail/<group>/`, drop from `install_headers`. |
| **TRIM** | Mostly fine but has dead members / stale comments. | Edit in place. |
| **REPLACE with stdlib** | Functionality duplicates modern C++ stdlib or a common dep. | Migrate consumers, delete. |
| **MERGE** | Would fit better as a section of a neighbouring header. | Fold, git rm the empty shell. |
| **RETIRE** | Zero in-tree consumers, not plausibly useful as framework API. | Delete (like `DynamicGUI` Session 49). |
| **RELOCATE** | Wrong module. | Move to the right one (like `IntrinsicCalibrator` io→cv). |

---

## Audit output format

For each module, produce a table (easily grep-able) like:

```md
| Header | Inc count | Purpose (1 line) | Idioms | Verdict | Notes |
|---|---|---|---|---|---|
| Foo.h | 12 | RAII wrapper around X | clean | KEEP | |
| Bar.h | 0 | hand-rolled RNG | replace with <random> | REPLACE | 4 callers migrate to std::mt19937_64 |
| Baz.h | 3 | impl detail for Foo | all callers are .cpps in same module | PRIVATIZE | move to detail/foo-support/ |
```

Followed by a "Recommended actions" section grouping verdicts into
discrete PR-sized chunks.

---

## How to use this checklist

1. Run the bulk-usage-count script against the module (see below).
2. For each header, read the file top to bottom (fast — most headers are <100 lines).
3. Answer the 10 questions above; most take seconds.
4. Assign a verdict.
5. Write the action plan.
6. Execute the action plan in discrete commits (one per verdict class, not one per file).

### Bulk usage count command

```bash
for f in icl/<module>/*.h; do
  base=$(basename "$f")
  cnt=$(grep -rn "#include.*icl/<module>/$base" icl/ tests/ benchmarks/ 2>/dev/null \
        | grep -v "^$f:" | wc -l | tr -d ' ')
  printf "%4d  %s\n" "$cnt" "$base"
done | sort -rn
```

This gives inclusion frequency; pairs with §1 above.

---

## Past audits (results archived here)

### `icl/utils/` — Session 49+

Inclusion counts (at audit time, 44 headers):

```
 381  CompatMacros.h          14  Array2D.h                7  SignalHandler.h
 117  StringUtils.h           14  Lockable.h               7  FPSLimiter.h
  86  Exception.h             13  Any.h                    6  FPSEstimator.h
  77  Macros.h                12  VisualizationDescription 6  ConsoleProgress.h
  67  Point.h                 11  StrTok.h                 5  Timer.h
  46  Size.h                  11  StackTimer.h             5  TextTable.h
  41  Time.h                  11  SSETypes.h               5  SSEUtils.h
  37  Configurable.h          10  PluginRegistry.h         5  BackendDispatching.h
  35  File.h                   8  EnumDispatch.h           4  ParamList.h
  34  Rect.h                   7  SteppingRange.h          3  MultiTypeMap.h
  32  BasicTypes.h             3  FixedArray.h             2  UncopiedInstance.h
  29  Range.h                  2  ProcessMonitor.h         1  FastMedianList.h
  28  Random.h                 1  ConfigurableProxy.h      0  PThreadFix.h
  26  Thread.h                 0  Utils.h
  25  ProgArg.h
  20  ConfigFile.h
  19  ClippedCast.h
```

### Results table

| Header | Inc | Purpose (1 line) | Issues | Verdict | Notes |
|---|---|---|---|---|---|
| `PThreadFix.h` | 0 | empty "kept for backward compat" shim (comment says so) | dead code | **RETIRE** | file is 9 lines; safe to delete |
| `Utils.h` | 0 | umbrella + doxygen `\defgroup`s for the module | XML group references removed XML.h; double-included Rect.h | **KEEP, POLISH** | remove XML mention, dedup Rect include; umbrella + doxygen remains useful for framework |
| `FastMedianList.h` | 1 | histogram-based median over a fixed range | raw `new int[]`/`delete[]`; assign op leaks; deprecated-but-still-present `t2/t3/t4` vars; only 1 "consumer" is Utils.h umbrella | **RETIRE** | no real consumer; 20-line `std::vector<int>` rewrite available on demand |
| `ConfigurableProxy.h` | 1 | forwarding wrapper around a Configurable* | inline bodies in header; OK | **KEEP** | `io/GenericGrabber.h` inherits — load-bearing public API |
| `UncopiedInstance.h` | 2 | CRTP mixin: on-copy reset to default-constructed T | public inheritance from T (misleading type relationship); magic "silent reset" semantics | **KEEP** | used by Configurable.h (mutex) and ViewBasedTemplateMatcher.h; rename to e.g. `DefaultConstructOnCopy<T>` is a style-only nice-to-have |
| `ProcessMonitor.h` | 2 | Linux /proc parser for CPU/mem stats | raw `Data *m_data` instead of `unique_ptr<Data>`; `getInstance()` returns raw ptr; Linux-only (but builds on macOS, so parts must work or be stubbed) | **KEEP, POLISH** | only consumer is qt/GUI.cpp .cpp (not header) → also a **PRIVATIZE** candidate, but GUI.cpp may expose it to users of the "ps" widget. Flag as TODO |
| `FixedArray.h` | 3 | `std::array<T,N>`-like with x/y/z/w aliases via union | union-type-punning is technically UB (but widely-relied-on); could be `std::array` + CRTP accessors | **KEEP, POLISH** | framework-ish; 3 real consumers in cv app + CL kernels + FixedMatrix |
| `MultiTypeMap.h` | 3 | map<string, type-erased value> via RTTI strings + reinterpret_cast | reinvents `std::any`; `static T _NULL;` hazard; value/array bit-hack; raw ownership | **KEEP, POLISH** | qt::DataStore inherits publicly — rewriting means touching all DataStore users. Long-term: migrate to `std::map<std::string, std::any>`. |
| `ParamList.h` | 4 | key-value param bag with CSV init | 10-arg ctor with default empty strings (very pre-C++11); `const_cast<ParamList*>(this)` in ctor (no-op) | **KEEP, POLISH** | heavy markers-module user (31 files). Replace 10-arg ctor with `std::initializer_list<std::pair<Key,Value>>` in a small follow-up. |
| `BackendDispatching.h` | 5 | selector over (Backend, ctx) — keyed on PluginRegistry | clean modern C++ (Session 48 rework) | **KEEP** | |
| `SSEUtils.h` | 5 | SSE intrinsic wrappers | per SSE work; clean | **KEEP** | |
| `TextTable.h` | 5 | ASCII table formatter | OK | **KEEP** | |
| `Timer.h` | 5 | start/stop stopwatch with sub-timers | `startTimer()/stopTimer()` verbosity, raw int returns, `int m_iTimerMode` mode-flag instead of enum | **KEEP, POLISH** | could be a thin wrapper over `std::chrono::steady_clock`; minor |
| `ConsoleProgress.h` | 6 | terminal progress bar | OK, scoped to utils | **KEEP** | |
| `FPSEstimator.h` | 6 | averaging framerate counter | `virtual ~` + virtual methods but no one derives → drop virtual | **KEEP, POLISH** | drop `virtual` from dtor/methods; add `FPS_LOG_THIS_FUNCTION` macro refers to `showFps` but method is `showFPS` — bug! |
| `FPSLimiter.h` | 7 | frame-rate limiter (sleep-to-cap) | same virtual-for-nothing pattern as FPSEstimator | **KEEP, POLISH** | same pattern — drop virtuals if no derivatives |
| `SignalHandler.h` | 7 | POSIX signal → std::function dispatch | ~30 lines of commented-out old API in the public header; stale docstring describing the old `handleSignals` virtual which no longer exists | **KEEP, TRIM** | keep API, delete the commented-out corpse |
| `SteppingRange.h` | 7 | `Range<T>` + step value | virtual `contains()` — derives from virtual-dtor Range; OK | **KEEP** | |
| `EnumDispatch.h` | 8 | compile-time dispatch on runtime int | tiny, modern C++17 fold expression | **KEEP** | |
| `PluginRegistry.h` | 10 | Session 48 plugin primitive | clean | **KEEP** | |
| `SSETypes.h` | 11 | SSE / NEON typedefs | | **KEEP** | |
| `StackTimer.h` | 11 | macro-driven per-function timer | raw `new Timer`/`delete` in RAII (could be unique_ptr); static `char acBuf[100]` + snprintf; uses printf instead of iostream or std::format | **KEEP, POLISH** | old-C++ style but works; cleanup non-urgent |
| `StrTok.h` | 11 | string tokenizer class | clean after Session N (std::string_view ctor) | **KEEP** | |
| `VisualizationDescription.h` | 12 | serializable 2D overlay description | framework-central; used across cv/markers/qt | **KEEP** | |
| `Any.h` | 13 | string-serializable "anything"; inherits std::string | naming collision with `std::any` (different semantics); public std::string inheritance is a classic smell | **KEEP** | not replaceable by std::any (different contract); rename impractical |
| `Array2D.h` | 14 | shared_ptr<T[]>-backed 2D array | clean modern C++ | **KEEP** | |
| `Lockable.h` | 14 | public-inheritance mixin exposing lock/unlock/getMutex | old pattern; modern C++ prefers composing `std::mutex` and using `std::scoped_lock(obj.getMutex())` at call sites | **KEEP** | prior session kept it; migration is bigger than the benefit |
| `ClippedCast.h` | 19 | saturating numeric cast | modern (C++17 `constexpr if`); clean post-recent-cleanup | **KEEP** | |
| `ConfigFile.h` | 20 | Session 49 just PIMPL-ized | clean now | **KEEP** | |
| `ProgArg.h` | 25 | command-line argument parsing | ICL-native; external libs (CLI11, argparse) exist but would be a big migration | **KEEP** | |
| `Thread.h` | 26 | OO wrapper over std::thread | Session-48 cleanup already happened; `runningNoLock()` duplicates `running()` since it's atomic | **KEEP, TRIM** | drop the redundant `runningNoLock()` |
| `Random.h` | 28 | RNG helpers | wraps `<random>`; convenient ICL-flavored API | **KEEP** | |
| `Range.h` | 29 | templated [min, max] range | virtual dtor (needed — SteppingRange derives) | **KEEP** | |
| `BasicTypes.h` | 32 | `icl8u`, `icl16s`, `icl32s`, `icl32f`, `icl64f` aliases + depth enum | foundational | **KEEP** | |
| `Rect.h` | 34 | Session 48 templatized | clean | **KEEP** | |
| `File.h` | 35 | file abstraction (predates `<filesystem>`) | could eventually migrate to `std::filesystem` + `std::ifstream`, but pervasive consumers | **KEEP** | long-term migration candidate; not urgent |
| `Configurable.h` | 37 | property-system base class | load-bearing (every Op + every Grabber derives) | **KEEP** | |
| `Time.h` | 41 | time point / duration type | predates `<chrono>`; pervasive | **KEEP** | migration candidate someday; too pervasive for now |
| `Size.h` | 46 | Session 48 templatized | clean | **KEEP** | |
| `Point.h` | 67 | Session 48 templatized | clean | **KEEP** | |
| `Macros.h` | 77 | ICLASSERT_*, WARNING_LOG, ERROR_LOG, etc. | naked `#define` macros; modern alternative would be constexpr `inline` functions + `std::source_location` | **KEEP** | foundational; logging rewrite is a separate mega-project |
| `Exception.h` | 86 | ICL exception types | clean, conventional | **KEEP** | |
| `StringUtils.h` | 117 | `str<T>`, `parse<T>`, `tok`, `match`, `cat`, `parseVecStr` | pervasive; some pieces could migrate to `std::format`/`std::from_chars` | **KEEP** | migration piecemeal over time |
| `CompatMacros.h` | 381 | Windows DLL export macros + MSVC 2012 math polyfills + GL includes | dead MSVC<2013 polyfills (`_MSC_VER < 1800`); GL includes don't belong in a shared compat header | **KEEP, POLISH** | drop MSVC polyfills; consider moving GL includes to a dedicated `GLIncludes.h` only used where GL is needed |

### Summary of actions

**RETIRE (2 headers — free deletion):**
- `PThreadFix.h` — empty shim
- `FastMedianList.h` — 1 phantom consumer (umbrella only)

**POLISH (8 headers — small targeted cleanups):**
- `Utils.h` — drop XML doxygen mention, dedup Rect include
- `ConfigurableProxy.h` — consider moving inline bodies to .cpp (optional)
- `ProcessMonitor.h` — raw ptr → `unique_ptr`; `getInstance()` → return reference; consider PRIVATIZE
- `FixedArray.h` — consider `std::array` + accessors migration (bigger)
- `MultiTypeMap.h` — long-term: migrate to `std::map<string, std::any>` (big)
- `ParamList.h` — 10-arg ctor → `initializer_list`
- `FPSEstimator.h` — drop virtuals; fix `showFps` vs `showFPS` macro bug
- `FPSLimiter.h` — drop virtuals if no derivers
- `SignalHandler.h` — delete commented-out corpse
- `StackTimer.h` — `new Timer`/`delete` → value member or `unique_ptr`
- `Thread.h` — drop redundant `runningNoLock()`
- `UncopiedInstance.h` — rename to `DefaultConstructOnCopy<T>` (style-only)
- `Any.h` — no rename practical but note std::any collision
- `CompatMacros.h` — drop MSVC<2013 polyfills; split out GL includes
- `Timer.h` — mode-int → enum (optional)

**KEEP (no action):**
- Everything else (25 headers).

### Recommended PR-sized chunks

1. **Trivial cleanup** (one commit): retire PThreadFix + FastMedianList; polish Utils.h doxygen; delete SignalHandler commented corpse; fix FPSEstimator macro bug; drop Thread::runningNoLock; drop FPS{Estimator,Limiter} virtuals.
2. **CompatMacros diet** (one commit): drop MSVC<2013 polyfills; extract GL includes into separate header.
3. **ParamList modernization** (one commit): replace 10-arg ctor with initializer_list; touch 31 callers.
4. **ProcessMonitor polish** (one commit, optional): unique_ptr; privatize to utils/detail/.
5. **Long-term deferred items** (not this session):
   - MultiTypeMap → std::any migration
   - File → std::filesystem migration
   - Time → std::chrono migration
   - Lockable → std::mutex member pattern migration
