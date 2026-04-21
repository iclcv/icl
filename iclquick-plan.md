# Quick.h Migration Plan: ImgQ → Image

## Current State

- `ImgQ = Img<icl32f>` — everything forced to float depth
- ~60 public functions, all operating on `ImgQ`
- Some already have `template<T>` versions with `ImgQ` affinity overloads
- Two redundant buffer pools (neither thread-safe)
- Global mutable drawing state (COLOR, FILL, FONT) — not thread-safe

## Goals

1. Replace `ImgQ` with `Image` (type-erased, depth-preserving). Retire `ImgQ` typedef.
2. Use `UnaryOp`/`BinaryOp` internally wherever possible (both already have `apply(Image)` methods).
3. Introduce `QuickContext` to unify buffer management and drawing state with thread safety.
4. Split monolithic Quick.h/Quick.cpp into logically separated files.

---

## Buffer Pool Analysis

### Two Redundant Pools Currently in Use

**Pool 1: `TEMP_IMAGES` (Quick.cpp local, lines 316-363)**
- `static std::list<ImgQ> TEMP_IMAGES` — file-scope static, float-only
- Used by: arithmetic ops, cc, thresh, concatenation (via `TEMP_IMG_*` macros)
- Strategy: scan list for an `isIndependent()` image with matching params;
  if found reuse it, else reuse any independent one (resize it), else push_back new
- Returns by reference — caller shallow-copies the returned ImgQ
- **Not thread-safe**: bare static, no mutex

**Pool 2: `ImgBuffer` (core module singleton, ImgBuffer.cpp)**
- `static shared_ptr<ImgBuffer>` singleton with `vector<ImgBase*> bufs[5]` (one per depth)
- Used by: zeros, ones, load, create, grab, filter, blur, copy, copyroi (the templated functions)
- Same `isIndependent()` strategy, but already multi-depth
- Returns raw pointer — caller copies out of it
- **Also not thread-safe**: no mutex

### Problems

1. Two redundant pools doing the same thing with the same algorithm
2. Neither is thread-safe — concurrent Quick calls from different threads corrupt the lists
3. `TEMP_IMAGES` is float-only — can't serve multi-depth Image results
4. `ImgBuffer` returns raw `Img<T>*` — awkward ownership semantics
5. Global mutable drawing state (`COLOR[4]`, `FILL[4]`, `FONTSIZE`, `FONTFAMILY`,
   `savedColor/savedFill`) — not thread-safe, impossible to use from two independent
   subsystems simultaneously

### The `isIndependent()` Trick

Both pools exploit shared_ptr reference counting: if `isIndependent()` returns true,
nobody else holds a reference to the channel data, so it's safe to reclaim. This is
clever but fragile — it only works if callers make deep copies or let the shallow copy
fall out of scope before the next call. A chain like
`show(filter(load("a.png"),"sobel") + load("b.png"))` works because temporaries die
at the semicolon, but holding onto returned images across calls keeps them "pinned"
in the pool forever, growing it.

---

## QuickContext Design

### Image::memoryUsage()

New method on `Image` — needed by the pool for tracking:

```cpp
size_t Image::memoryUsage() const {
    if (isNull()) return 0;
    return size_t(getDim()) * getChannels() * getSizeOf(getDepth());
}
```

Returns the pixel data footprint in bytes (`w * h * channels * bytesPerElement`).
Object overhead (ImgBase vtable, shared_ptr control block) is negligible and ignored.

### The Context Class

```cpp
class QuickContext {
public:
    // Default cap: 256 MB — enough for typical quick-prototyping,
    // small enough to prevent runaway growth
    explicit QuickContext(size_t memoryCap = 256 * 1024 * 1024);

    // Buffer pool (replaces both TEMP_IMAGES and ImgBuffer usage in Quick)
    Image getBuffer(depth d, const ImgParams &params);
    Image getBuffer(depth d, const Size &s, int channels);

    // Pool diagnostics
    size_t memoryUsage() const;     // current pool footprint in bytes
    size_t memoryCap() const;       // configured limit
    void setMemoryCap(size_t cap);
    void clearBuffers();            // manual flush

    // Drawing state (replaces global COLOR/FILL/FONT)
    float color[4] = {255, 0, 0, 255};
    float fill[4]  = {0, 0, 0, 0};
    int fontSize = 12;
    std::string fontFamily = "Times";

    // Color save/restore stack (replaces savedColor/savedFill)
    void pushColorState();
    void popColorState();

private:
    std::vector<Image> buffers_;
    size_t memoryCap_;
    size_t currentUsage_ = 0;    // tracked incrementally

    void evict(size_t needed);   // drop independent buffers until enough room
};
```

### Buffer Pool with Memory Cap

`getBuffer()` follows this strategy:

1. **Exact match**: scan for independent buffer with matching depth + params → reuse (free)
2. **Resize**: any independent buffer → resize it (adjust `currentUsage_` for delta)
3. **Evict**: if `currentUsage_ + needed > memoryCap_`, drop independent buffers
   (oldest/smallest first) until there's room
4. **Over cap, nothing reclaimable**: all pooled buffers are pinned (someone holds a
   reference). Allocate a normal `Image` on the heap — **not added to the pool**.
   Emit a `WARNING_LOG` so the user knows their cap is undersized or they're holding
   too many references. The unpooled image works correctly, gets passed around, and
   is freed when the last reference dies — it just won't be recycled.
5. **Room in pool**: allocate, add to `buffers_`, track `currentUsage_`

```cpp
Image QuickContext::getBuffer(depth d, const ImgParams &params) {
    size_t needed = size_t(params.getDim()) * params.getChannels() * getSizeOf(d);

    // 1. Exact match among independent buffers
    for (auto &buf : buffers_) {
        if (buf.isIndependent() && buf.getDepth() == d && buf.getParams() == params)
            return buf;
    }

    // 2. Any independent buffer → resize
    for (auto &buf : buffers_) {
        if (buf.isIndependent()) {
            currentUsage_ -= buf.memoryUsage();
            buf.ensureCompatible(d, params.getSize(), params.getChannels());
            currentUsage_ += buf.memoryUsage();
            return buf;
        }
    }

    // 3. Try eviction
    if (currentUsage_ + needed > memoryCap_)
        evict(needed);

    // 4. Still over cap → unpooled allocation + warning
    if (currentUsage_ + needed > memoryCap_) {
        WARNING_LOG("QuickContext: pool memory cap exceeded ("
                    << (currentUsage_ >> 20) << "/" << (memoryCap_ >> 20)
                    << " MB). Allocating unpooled image. "
                    "Consider increasing the cap or releasing held images.");
        return Image(params.getSize(), d, params.getChannels());
    }

    // 5. Room in pool
    buffers_.emplace_back(params.getSize(), d, params.getChannels());
    currentUsage_ += needed;
    return buffers_.back();
}
```

### Thread-Local Activation

```cpp
// Quick.cpp anonymous namespace — non-owning routing pointer
thread_local QuickContext* tl_active_context = nullptr;

// Default context (lazy singleton, lives until program exit)
QuickContext& defaultContext() {
    static QuickContext ctx;
    return ctx;
}

// Get active context for this thread
QuickContext& activeContext() {
    return tl_active_context ? *tl_active_context : defaultContext();
}

// RAII activation handle
class QuickScope {
    QuickContext* prev_;
public:
    explicit QuickScope(QuickContext &ctx)
        : prev_(tl_active_context) {
        tl_active_context = &ctx;
    }
    ~QuickScope() { tl_active_context = prev_; }
    QuickScope(const QuickScope&) = delete;
    QuickScope& operator=(const QuickScope&) = delete;
};
```

### Ownership Model

**`tl_active_context` is a non-owning observer pointer.** It never allocates or
deletes anything — it just routes calls to the right context. There are no
`new QuickContext` calls in this design.

Contexts are created as:
- **Stack locals** — destroyed at scope end
- **Class members** — destroyed with the owning object
- **The default singleton** — destroyed at program exit via static destructor

The `QuickScope` RAII guard ensures the pointer is always restored before the
context it points to is destroyed (reverse destruction order guarantees this).

### Usage Patterns

**Casual use (unchanged feel — default context):**
```cpp
#include <icl/qt/Quick.h>
Image img = load("test.png");
color(255, 0, 0);
line(img, 0, 0, 100, 100);
show(img);
```

**Explicit context (library/subsystem use):**
```cpp
class MyTracker {
    QuickContext qctx_;  // private buffer pool + draw state

    void process(const Image &frame) {
        QuickScope scope(qctx_);  // activate for this thread

        Image edges = filter(frame, "sobelx"); // uses qctx_ buffers
        color(0, 255, 0);                       // sets qctx_.color
        cross(edges, 100, 100);                  // draws with qctx_.color
    }   // scope dtor restores previous context
};
```

**Multi-threaded (each thread gets its own context):**
```cpp
void worker(int id) {
    QuickContext ctx;          // per-thread context
    QuickScope scope(ctx);    // activate
    Image img = load("frame.png");
    // safe: no shared mutable state
}
```

### What Happens to ImgBuffer?

Keep `ImgBuffer` as-is in the core module — it's used by code outside Quick.
Quick stops using it; Quick's own pool lives in `QuickContext`. Two independent
systems, no coupling.

---

## File Split

### Migration Strategy: Quick2 Parallel Build

**Quick.h stays untouched.** All new code goes into `Quick2.h` and its sub-files.
This allows incremental migration: each demo/app/library file can be switched from
`#include <icl/qt/Quick.h>` to `#include <icl/qt/Quick2.h>` one at a time, adapting
`ImgQ` → `Image` usage as we go. Both headers coexist — no big-bang switch.

Once all consumers have migrated:
- Delete Quick.h and Quick.cpp
- Rename Quick2.h → Quick.h (or keep Quick2 as the canonical name)

### New File Structure

```
icl/qt/
  Quick.h/.cpp             ← UNCHANGED — old ImgQ-based API, stays working
  Quick2.h                 ← NEW multi-includer (includes all below + using-namespace block)
  QuickContext.h/.cpp      ← QuickContext, QuickScope, buffer pool, activeContext()
  QuickCreate.h/.cpp       ← zeros, ones, load, create, grab
  QuickFilter.h/.cpp       ← filter, blur, cc, rgb/hls/lab/gray, scale, channel,
                              levels, thresh, copy, copyroi, norm, flipx, flipy
  QuickMath.h/.cpp          ← arithmetic operators (img+img, img+scalar, scalar+img),
                              unary minus, math functions (exp, ln, sqr, sqrt, abs),
                              logical ops (||, &&), binary ops (binOR/XOR/AND)
  QuickCompose.h/.cpp       ← concatenation (operator,  operator%  operator|),
                              ImgROI struct, roi(), data()
  QuickDraw.h/.cpp          ← color/fill/font state access, line, cross, rect,
                              triangle, circle, pix, text, label, linestrip, polygon
  QuickIO.h/.cpp            ← save, show, showSetup, print, tic, toc,
                              openFileDialog, saveFileDialog, textInputDialog,
                              execute_process
```

### Quick2.h (the multi-includer)

```cpp
#pragma once

#include <icl/qt/QuickContext.h>
#include <icl/qt/QuickCreate.h>
#include <icl/qt/QuickFilter.h>
#include <icl/qt/QuickMath.h>
#include <icl/qt/QuickCompose.h>
#include <icl/qt/QuickDraw.h>
#include <icl/qt/QuickIO.h>

#ifndef ICL_NO_USING_NAMESPACES
using namespace icl;
using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::filter;
using namespace icl::io;
using namespace icl::qt;
using namespace icl::geom;
using namespace icl::cv;
using namespace icl::markers;
using namespace icl::physics;
#endif
```

### Include Dependencies Between Quick Files

```
QuickContext   ← foundation: no Quick deps, only core/Image + utils
QuickCreate   ← depends on QuickContext (buffer pool)
QuickFilter   ← depends on QuickContext (buffer pool), uses filter/UnaryOp
QuickMath     ← depends on QuickContext (buffer pool), uses filter/BinaryArithmeticalOp etc.
QuickCompose  ← depends on QuickContext (buffer pool)
QuickDraw     ← depends on QuickContext (draw state: color/fill/font)
QuickIO       ← depends on QuickContext (timer state), conditionally on Qt
```

All sub-headers use `activeContext()` from QuickContext to access the current
context's buffer pool or drawing state. No file depends on another Quick file
except through QuickContext.

### Rationale for Groupings

- **QuickContext** is the foundation — everything else depends on it
- **QuickCreate** is pure construction, no state mutation beyond buffers
- **QuickFilter** groups all image-to-image transformations (natural UnaryOp territory)
- **QuickMath** groups all operators and math — these are conceptually "expressions"
  and share the same internal dispatch pattern (BinaryArithmeticalOp/UnaryArithmeticalOp)
- **QuickCompose** groups spatial/channel assembly + ROI — these are structural
  operations on image geometry, not pixel values
- **QuickDraw** is self-contained: stateful (color/fill/font) + all pixel-painting code.
  This is the largest file but the functions are tightly coupled through shared helpers
  (hline, vline, draw_circle_outline)
- **QuickIO** groups all side-effectful I/O: file read/write, display, dialogs, timing

---

## Function Inventory & Migration Complexity

### Creators → QuickCreate (6 functions) — Easy

| Function | Current | New | Notes |
|----------|---------|-----|-------|
| `zeros(w,h,ch)` | → `ImgQ` | → `Image` | Already templated; add Image overload defaulting to depth32f |
| `ones(w,h,ch)` | → `ImgQ` | → `Image` | Same as zeros |
| `load(filename)` | → `ImgQ` | → `Image` | Return native depth from file — no conversion! |
| `load(filename,fmt)` | → `ImgQ` | → `Image` | Same, just cc after |
| `create(name,fmt)` | → `ImgQ` | → `Image` | TestImages already returns ImgBase* |
| `grab(dev,spec,...)` | → `ImgQ` | → `Image` | GenericGrabber already returns Image |

### Converters — Remove (13 functions)

| Function | Current | Action | Notes |
|----------|---------|--------|-------|
| `cvt8u/16s/32s/32f/64f` | `ImgQ → Img<T>` | **Remove** | `Image::convert(depth)` replaces all 5 |
| `cvt(Img<T>)` (5 overloads) | → `ImgQ` | **Remove** | No longer needed — Image wraps any depth |
| `cvt(ImgBase*/ImgBase&/Image)` | → `ImgQ` | **Remove** | Same |

### Filters → QuickFilter (13 functions) — Easy

| Function | Current | New | Notes |
|----------|---------|-----|-------|
| `filter(img,"name")` | `Img<T> → Img<T>` | → `Image` | Already uses UnaryOp; call `op.apply(Image)` |
| `blur(img, radius)` | `Img<T> → Img<T>` | → `Image` | Uses ConvolutionOp — same approach |
| `copy(img)` | `Img<T> → Img<T>` | → `Image` | `Image::deepCopy()` |
| `copyroi(img)` | `Img<T> → Img<T>` | → `Image` | visit + deepCopyROI |
| `norm(img)` | `Img<T> → Img<T>` | → `Image` | `Image::normalizeAllChannels()` |
| `cc(img, fmt)` | `ImgQ → ImgQ` | → `Image` | Wrap `core::cc(ImgBase*,ImgBase*)` |
| `rgb/hls/lab/gray(img)` | `ImgQ → ImgQ` | → `Image` | Thin wrappers around cc() |
| `scale(img, factor)` | `ImgQ → ImgQ` | → `Image` | `Image::scaledCopy()` |
| `scale(img, w, h)` | `ImgQ → ImgQ` | → `Image` | Same |
| `channel(img, ch)` | `ImgQ → ImgQ` | → `Image` | `Image::selectChannel()` |
| `levels(img, n)` | `ImgQ → ImgQ` | → `Image` | `LUTOp.apply(Image)` |
| `thresh(img, t)` | `ImgQ → ImgQ` | → `Image` | `UnaryCompareOp.apply(Image)` or visit() |
| `flipx/flipy(img)` | `ImgQ → ImgQ` | → `Image` | `Image::mirrored()` already exists |

### Arithmetic + Math + Logic → QuickMath (23 functions) — Easy/Medium

| Function | Current | New | Complexity | Notes |
|----------|---------|-----|------------|-------|
| `img + img` | `ImgQ` | → `Image` | Easy | `BinaryArithmeticalOp.apply(Image,Image)` |
| `img - img` | `ImgQ` | → `Image` | Easy | Same |
| `img * img` | `ImgQ` | → `Image` | Easy | Same |
| `img / img` | `ImgQ` | → `Image` | Easy | Same |
| `img + float` | `ImgQ` | → `Image` | Easy | `UnaryArithmeticalOp.apply(Image)` |
| `img - float` | `ImgQ` | → `Image` | Easy | Same |
| `img * float` | `ImgQ` | → `Image` | Easy | Same |
| `img / float` | `ImgQ` | → `Image` | Easy | Same |
| `float + img` | `ImgQ` | → `Image` | Easy | Commutative → delegates |
| `float - img` | `ImgQ` | → `Image` | Medium | Non-commutative; negate + add |
| `float * img` | `ImgQ` | → `Image` | Easy | Commutative |
| `float / img` | `ImgQ` | → `Image` | Medium | Non-commutative; needs visit() |
| `-img` | `ImgQ` | → `Image` | Easy | `img * (-1)` |
| `exp(img)` | `ImgQ` | → `Image` | Easy | `UnaryArithmeticalOp::expOp` |
| `ln(img)` | `ImgQ` | → `Image` | Easy | Same |
| `sqr(img)` | `ImgQ` | → `Image` | Easy | Same |
| `sqrt(img)` | `ImgQ` | → `Image` | Easy | Same |
| `abs(img)` | `ImgQ` | → `Image` | Easy | Same |
| `img \|\| img` | `ImgQ` | → `Image` | Medium | visit() pixel loop, depth promotion |
| `img && img` | `ImgQ` | → `Image` | Medium | Same |
| `binOR<T>` | `ImgQ` | → `Image` | Medium | Template on cast type, needs visit() |
| `binXOR<T>` | `ImgQ` | → `Image` | Medium | Same |
| `binAND<T>` | `ImgQ` | → `Image` | Medium | Same |

### Concatenation + ROI → QuickCompose (6 items) — Medium

| Function | Current | New | Notes |
|----------|---------|-----|-------|
| `img , img` (horiz) | `ImgQ` | → `Image` | Depth promotion + ROI copy across depths |
| `img % img` (vert) | `ImgQ` | → `Image` | Same |
| `img \| img` (channel) | `ImgQ` | → `Image` | Same |
| `ImgROI` struct | uses `ImgQ` | → `Image` | Replace ImgQ member with Image |
| `roi(img)` | `ImgQ&` | → `Image&` | Follows from ImgROI change |
| `data(img)` | `ImgQ&` | → `Image&` | Same |

### Drawing → QuickDraw (13 functions) — Hard

| Function | Current | New | Complexity | Notes |
|----------|---------|-----|------------|-------|
| `color/fill/colorinfo` | globals | context | Easy | Read/write `activeContext().color/fill` |
| `line(img,...)` | `ImgQ&` | → `Image&` | Hard | Direct pixel access with alpha blending |
| `cross(img,...)` | `ImgQ&` | → `Image&` | Easy | Delegates to line() |
| `rect(img,...)` | `ImgQ&` | → `Image&` | Hard | Direct pixel access + fill |
| `triangle(img,...)` | `ImgQ&` | → `Image&` | Hard | Direct pixel access, hline fill |
| `circle(img,...)` | `ImgQ&` | → `Image&` | Hard | Direct pixel access |
| `pix(img,...)` | `ImgQ&` | → `Image&` | Hard | Direct pixel access |
| `text(img,...)` | `ImgQ&` | → `Image&` | Hard | Direct pixel access with alpha mask |
| `linestrip/polygon` | `ImgQ&` | → `Image&` | Easy | Delegate to line/triangle |
| `label(img,text)` | `ImgQ` | → `Image` | Easy | Delegates to text() |
| `font/fontsize` | globals | context | Easy | Read/write `activeContext().fontSize/fontFamily` |

### Output + Timer + Dialogs → QuickIO (9 functions) — Trivial/None

| Function | Current | New | Notes |
|----------|---------|-----|-------|
| `save(ImgBase&,file)` | `ImgBase&` | add `Image` overload | Already generic |
| `show(ImgBase&)` | `ImgBase&` | add `Image` overload | Already generic |
| `print<T>(Img<T>)` | templated | → `Image` | `Image::print()` |
| `showSetup(...)` | config | context | Move to QuickContext |
| `tic/toc` | globals | context | Move timer to QuickContext |
| `openFileDialog` | Qt-only | no change | |
| `saveFileDialog` | Qt-only | no change | |
| `textInputDialog` | Qt-only | no change | |
| `execute_process` | utility | no change | |

---

## Complexity Summary

| Difficulty | Count | Description |
|-----------|-------|-------------|
| None/Trivial | ~25 | No change needed, or one-liner wrapper |
| Easy | ~25 | Delegate to existing UnaryOp/BinaryOp apply(Image) |
| Medium | ~10 | Concatenation, logical ops, ROI — depth promotion + visit() |
| Hard | ~6 | Drawing primitives (line, rect, triangle, circle, pix, text) |

---

## Key Design Decisions

### 1. Depth Promotion Rules

Natural order: `icl8u < icl16s < icl32s < icl32f < icl64f`.
For `Image op Image`, result depth = `max(depth_a, depth_b)`.
`BinaryArithmeticalOp` already handles this internally.

### 2. Drawing Functions — The Hard Part

All drawing code does:
```cpp
float &v = image(x,y,c);
v = (1.0 - A) * v + A * COLOR[c];
```

**Chosen approach: visit() + template the inner pixel-access logic.**

Use `visit()` at the entry point of each drawing function. The alpha blending
formula works on all numeric types with appropriate casts — integer types truncate,
which is acceptable for 0-255 drawing colors. This avoids wasteful float conversion
for large images and keeps the depth the user chose.

Helper functions (`hline`, `vline`, `draw_circle_outline`, etc.) become templates
or generic lambdas passed through visit().

### 3. ImgQ Deprecation

Keep `using ImgQ = Img<icl32f>` as deprecated typedef for backward compatibility.
New code uses `Image`. The `#define ICL_QUICK_DEPTH / ICL_QUICK_TYPE` macros go away.

### 4. Existing Templated Functions

Functions already templated (zeros, ones, load, create, grab, filter, blur, copy,
copyroi, norm, print) keep their template versions for power users who want a
specific `Img<T>`. The non-template overloads change from returning `ImgQ` to
returning `Image`.

---

## Implementation Order

### Phase 1: Build Quick2 (Quick.h untouched)

#### Step 1: QuickContext + Image::memoryUsage() (~2-3h)
- Add `Image::memoryUsage()` to core/Image.h/.cpp
- Implement `QuickContext` class with memory-capped buffer pool and drawing state
- Implement `QuickScope` RAII activation
- `thread_local` routing via `activeContext()`
- Default singleton context

#### Step 2: Quick2 Sub-Files — Creators + Filters (~3-4h)
- Create QuickCreate.h/.cpp — zeros, ones, load, create, grab returning `Image`
- Create QuickFilter.h/.cpp — all filter functions using `Image` + UnaryOp/BinaryOp
- No cvt* functions (Image::convert() replaces them)

#### Step 3: Quick2 Sub-Files — Math + Compose (~3-4h)
- Create QuickMath.h/.cpp — arithmetic operators, math functions, logical ops on `Image`
- Create QuickCompose.h/.cpp — concatenation operators, ImgROI with `Image`

#### Step 4: Quick2 Sub-Files — Draw + IO (~4-5h)
- Create QuickDraw.h/.cpp — drawing primitives with visit(), state via activeContext()
- Create QuickIO.h/.cpp — save, show, print, tic/toc, Qt dialogs

#### Step 5: Quick2.h Multi-Includer (~30min)
- Create Quick2.h that includes all sub-headers + using-namespace block
- Update CMakeLists.txt to compile the new .cpp files

### Phase 2: Incremental Migration (Quick.h → Quick2.h)

#### Step 6: Migrate Consumers One-by-One (~3-5h)
- For each file that `#include`s Quick.h:
  1. Change to `#include <icl/qt/Quick2.h>`
  2. Replace `ImgQ` with `Image`
  3. Adapt any `cvt*()` calls to `Image::convert()`
  4. Build + test
- Prioritize examples/demos first (simpler), library code last

#### Step 7: Retire Quick.h
- Once all consumers have migrated: delete Quick.h and Quick.cpp
- Optionally rename Quick2.h → Quick.h (or keep Quick2 as canonical)
- Remove ICL_QUICK_DEPTH / ICL_QUICK_TYPE macros
- Remove ImgQ typedef
- Remove old TEMP_IMAGES pool

---

## Files to Migrate (Quick.h → Quick2.h, one at a time)

```
-- Easy migrations (demos/examples — self-contained, good first targets) --
icl/qt/examples/quick.cpp                          ← main Quick demo, migrate first
icl/qt/demos/onscreen-button.cpp
icl/core/demos/canvas.cpp
icl/core/demos/pseudo-color.cpp
icl/cv/demos/signature-extraction.cpp
icl/io/demos/png_write_test.cpp
icl/io/apps/create.cpp
icl/math/demos/polynomial-regression.cpp
icl/math/demos/llm-2D.cpp
icl/markers/demos/simple-marker-demo.cpp
icl/physics/demos/physics-maze-MazeObject.cpp
icl/qt/apps/xv.cpp
icl/cv/apps/crop.cpp
icl/cv/apps/lens-undistortion-calibration-UndistortionUtil.cpp

-- Library code (more careful, may have downstream consumers) --
icl/qt/Common.h                                    ← switch to include Quick2.h
icl/qt/DrawWidget.h
icl/qt/DataStore.h
icl/qt/GLImg.h
icl/qt/GUI.cpp
icl/core/Img.h                                    ← ImgQ forward ref → remove
icl/core/Channel.h
icl/cv/QuickDocumentation.h                        ← update docs
icl/cv/TemplateTracker.cpp
icl/filter/UnaryOpPipe.h
icl/filter/WarpOp.h
icl/io/FileWriter.h
icl/geom/Scene.cpp
icl/geom/SceneMouseHandler.cpp
icl/geom/Segmentation3D.cpp
icl/markers/FiducialDetectorPluginICL1.cpp
icl/physics/PhysicsPaper.cpp
icl/physics/PhysicsPaper3.cpp

-- Final step --
icl/qt/Quick.h + Quick.cpp                         ← delete once all above are migrated
```

**Total estimate: ~20-25 hours across 4-5 sessions.**
