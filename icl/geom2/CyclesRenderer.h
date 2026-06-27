// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/Raytracer.h>
#include <memory>
#include <string>

namespace icl::geom2 {

  class Scene2;

  /// Sample-count / denoise preset (see CyclesRenderer). Preview = fast/noisy,
  /// Interactive = balanced (live viewers), Final = high sample count (stills).
  enum class RenderQuality { Preview, Interactive, Final };

  /// Photoreal renderer for a geom2 Scene2, backed by Blender Cycles (path
  /// tracer). GL-free, so it renders **headlessly anywhere** — no QApplication,
  /// no GL context, no window (unlike GLSceneCapture / Scene2::renderToImage).
  ///
  /// ── Quick start (one-shot, e.g. a headless capture) ───────────────────────
  /// \code
  ///   CyclesRenderer cyc(scene, RenderQuality::Final);
  ///   cyc.setSceneScale(1.0f);     // geom2 is in mm — see "Scene scale" below
  ///   cyc.setSamples(128);
  ///   cyc.renderBlocking(0);       // runs all samples, then returns
  ///   io::save(cyc.getImage(), "out.png");
  /// \endcode
  /// See demo `geom2-headless-cycles-capture` for the full minimal program.
  ///
  /// ── The three DRIVE MODELS — pick exactly ONE, never mix ──────────────────
  /// Cycles owns a long-lived render session; how you pump it differs:
  ///
  ///   • renderBlocking(cam) — SYNCHRONOUS one-shot. Runs to the target sample
  ///     count, then returns; getImage() holds the final frame. Use for offscreen
  ///     / batch / headless captures and tests. Blocks the calling thread (incl.
  ///     through Cycles' first-time kernel compile), so never call it on a GUI
  ///     thread you need responsive.
  ///
  ///   • render(cam) — POLL-DRIVEN progressive. Call it once per frame from a
  ///     SINGLE thread (typically a worker run() loop). It is a dirty-flag state
  ///     machine: it auto-restarts when the camera or scene changes
  ///     (hash + pending-sync), otherwise extends the image one step at a time
  ///     and goes idle once converged. getImage() returns the latest (still
  ///     refining) frame each call. Use for live/interactive viewers.
  ///     FOOTGUN: do not mutate any scene/Configurable property *every* frame
  ///     (e.g. setPropertyValue(...) unconditionally) — it keeps the dirty flag
  ///     set forever, so it restarts every frame and never converges. Only touch
  ///     the scene on real changes. (Note: "enable lighting" is a GL-only Scene2
  ///     property; Cycles ignores it and always lights physically.)
  ///
  ///   • start(cam)/stop() + setOnImageReady(cb) — ASYNCHRONOUS. start() spawns
  ///     an internal management thread that drives the session AND reads the
  ///     scene, pushing each refinement to your callback. Pull the result with
  ///     getImage() if you prefer. stop() (also the dtor) joins it.
  ///
  ///   ⚠ NEVER combine start() with render()/renderBlocking(): two drivers then
  ///   race the same Cycles session and the shared Scene2 (non-atomic init flag,
  ///   double Session construction) → use-after-free crash. If your app drives
  ///   Cycles from a worker loop and/or mutates the scene there, use render()
  ///   (or renderBlocking()) ONLY — not start().
  ///
  /// ── Getting a NON-BLACK / correct image (the gotchas) ─────────────────────
  ///   • Scene scale: geom2 units are MILLIMETRES, but Cycles' default scale is
  ///     0.001 → a 280 mm board becomes 0.28 units and falls outside sane light/
  ///     camera ranges (→ black). ALWAYS call setSceneScale(1.0f).
  ///   • Light colour is 0..255 (LightNode), not 0..1. A GeomColor(1,1,1,1)
  ///     copied from a GL demo is ~1/255 ≈ black light → black render. Use e.g.
  ///     GeomColor(255, 247, 235, 255). (A scene that looks lit in a GL view but
  ///     renders black in Cycles is the tell — GL lighting is a separate path.)
  ///   • Need at least one LightNode in range of the geometry.
  ///   • Textures: set a core::Image on Material (setBaseColorMap, …). Both
  ///     formatRGB (3ch) and 4-channel RGBA work; sRGB is assumed for colour maps.
  ///
  /// Geometry/material/transform edits are picked up via the invalidate*() calls;
  /// a plain render()/renderBlocking() after edits also resyncs.
  class CyclesRenderer : public Raytracer {
  public:
    explicit CyclesRenderer(Scene2 &scene,
                            RenderQuality quality = RenderQuality::Interactive);
    ~CyclesRenderer();

    CyclesRenderer(const CyclesRenderer &) = delete;
    CyclesRenderer &operator=(const CyclesRenderer &) = delete;
    CyclesRenderer(CyclesRenderer &&) noexcept;
    CyclesRenderer &operator=(CyclesRenderer &&) noexcept;

    // --- drive models (pick ONE — see class doc) ---
    void start(int camIndex = 0) override;   ///< async: spawn management thread
    void stop() override;                    ///< join the async thread (dtor does too)
    void setOnImageReady(std::function<void(const core::Img8u &)> cb) override; ///< async push cb
    void render(int camIndex = 0) override;          ///< poll: one progressive step (call per frame)
    void renderBlocking(int camIndex = 0) override;  ///< sync: run all samples, then return
    const core::Img8u &getImage() const override;    ///< latest frame (pull); empty until first render

    void setSamples(int samples) override;     ///< target sample count (can only increase mid-render)
    void setMaxBounces(int bounces) override;  ///< path-tracing ray depth
    void setExposure(float exposure) override;
    void setBrightness(float brightness) override;

    void invalidateAll() override;        ///< full resync (geometry + materials + lights)
    void invalidateTransforms() override; ///< cheap: object transforms changed (BVH refit)
    void invalidateNode(Node *node) override;

    void setSamplesPerStep(int n) override;  ///< samples per render() step (progressive granularity)
    float getProgress() const override;
    int getUpdateCount() const override;  ///< # of frames delivered (use to detect first output)
    bool isRendering() const override;
    bool isAvailable() const override { return true; }
    void setDenoising(bool enabled) override;  ///< OIDN — quality up, ~500ms, off by default

    void setQuality(RenderQuality quality);
    RenderQuality getQuality() const;
    void setResolutionScale(float scale);  ///< render at scale× camera resolution (speed knob)
    void setDevice(const std::string &device);  ///< e.g. "GPU"/"CPU" (default: best available)
    /// World units per geom2 unit. geom2 is in MILLIMETRES; Cycles' default is
    /// 0.001 which shrinks scenes out of useful range → ALWAYS set 1.0f. See class doc.
    void setSceneScale(float scale);

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
  };

} // namespace icl::geom2
