// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/Raytracer.h>
#include <memory>
#include <string>

namespace icl::geom2 {

  class Scene2;

  enum class RenderQuality { Preview, Interactive, Final };

  /// Renders a geom2 Scene using Blender Cycles as the backend.
  class CyclesRenderer : public Raytracer {
  public:
    explicit CyclesRenderer(Scene2 &scene,
                            RenderQuality quality = RenderQuality::Interactive);
    ~CyclesRenderer();

    CyclesRenderer(const CyclesRenderer &) = delete;
    CyclesRenderer &operator=(const CyclesRenderer &) = delete;
    CyclesRenderer(CyclesRenderer &&) noexcept;
    CyclesRenderer &operator=(CyclesRenderer &&) noexcept;

    void start(int camIndex = 0) override;
    void stop() override;
    void setOnImageReady(std::function<void(const core::Img8u &)> cb) override;
    void render(int camIndex = 0) override;
    void renderBlocking(int camIndex = 0) override;
    const core::Img8u &getImage() const override;

    void setSamples(int samples) override;
    void setMaxBounces(int bounces) override;
    void setExposure(float exposure) override;
    void setBrightness(float brightness) override;

    void invalidateAll() override;
    void invalidateTransforms() override;
    void invalidateNode(Node *node) override;

    void setSamplesPerStep(int n) override;
    float getProgress() const override;
    int getUpdateCount() const override;
    bool isRendering() const override;
    bool isAvailable() const override { return true; }
    void setDenoising(bool enabled) override;

    void setQuality(RenderQuality quality);
    RenderQuality getQuality() const;
    void setResolutionScale(float scale);
    void setDevice(const std::string &device);
    void setSceneScale(float scale);

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
  };

} // namespace icl::geom2
