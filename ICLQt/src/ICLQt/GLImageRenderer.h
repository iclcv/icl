// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Size.h>
#include <ICLCore/Types.h>
#include <ICLCore/Image.h>
#include <ICLQt/ImageStatistics.h>

#include <vector>

namespace icl::qt {

  /// Shader-based image renderer for GL 4.1 Core Profile
  /** Replaces the legacy GLImg for 2D image display. Handles image storage,
      BCI (brightness/contrast/intensity) adjustment via shader uniforms,
      pixel queries, and statistics. Used by ICLWidget for background images.

      The renderer draws a fullscreen quad textured with the stored image,
      letterboxed to preserve aspect ratio within the current GL viewport.
  */
  class ICLQt_API GLImageRenderer {
    struct Data;
    Data *m_data;

  public:
    GLImageRenderer();
    ~GLImageRenderer();

    GLImageRenderer(const GLImageRenderer&) = delete;
    GLImageRenderer& operator=(const GLImageRenderer&) = delete;

    // --- Data management ---

    /// Buffer new image data (deep copy)
    void update(const core::ImgBase *src);

    /// Clear stored image (isNull() will return true)
    void clear();

    /// Returns true if no image is stored
    bool isNull() const;

    /// Returns stored image size (or Size::null if null)
    utils::Size getSize() const;

    /// Returns stored image width
    int getWidth() const;

    /// Returns stored image height
    int getHeight() const;

    // --- Rendering ---

    /// Render stored image to current GL viewport (letterboxed)
    void render();

    /// Upload and render an image (convenience, does not store)
    void render(const core::Image &img);

    /// Set texture interpolation: interpolateNN or interpolateLIN
    void setScaleMode(core::scalemode sm);

    // --- BCI (brightness/contrast/intensity) ---

    /// Set BCI values (-1,-1,-1 for auto-adapt from image range)
    void setBCI(int b, int c, int i);

    // --- Pixel query & statistics ---

    /// Returns pixel color at (x,y) from stored image, empty if out of bounds
    std::vector<icl64f> getColor(int x, int y) const;

    /// Returns statistics of stored image (min/max, histograms)
    const ImageStatistics &getStats() const;

    /// Returns pointer to stored image (not owned by caller)
    const core::ImgBase *extractDisplay() const;

    // --- Grid overlay state ---

    /// Enable/disable pixel grid overlay (drawing done externally)
    void setDrawGrid(bool enabled, float *color = 0);

    /// Set grid line color
    void setGridColor(float *color);

    /// Get current grid color
    const float *getGridColor() const;

    /// Returns true if grid overlay is enabled
    bool getDrawGrid() const;
  };

} // namespace icl::qt
