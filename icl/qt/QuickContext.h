// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/core/Image.h>
#include <icl/core/ImgParams.h>

#include <string>
#include <vector>

namespace icl::filter {
  class UnaryOp;
  class BinaryOp;
}
namespace icl::io {
  class GenericGrabber;
}

namespace icl::qt {

  /// Context for Quick2 functions: owns a memory-capped buffer pool and drawing state
  /** QuickContext replaces the global mutable state (COLOR, FILL, FONT, TEMP_IMAGES)
      that Quick.h used. Each context is independent — different threads or subsystems
      can use their own contexts without interference.

      The buffer pool recycles Image objects to avoid repeated heap allocations in
      expression-heavy Quick2 code. Pool size is bounded by a configurable memory cap;
      when the cap is reached, independent (unreferenced) buffers are evicted. If nothing
      can be evicted, a fresh unpooled Image is allocated with a warning.

      Use QuickScope for RAII activation of a context on the current thread.
      Code that doesn't create a QuickScope uses the process-wide default context.

      \code
      QuickContext ctx(128 * 1024 * 1024); // 128 MB cap
      QuickScope scope(ctx);
      Image img = load("test.png");        // uses ctx's buffer pool
      color(255, 0, 0);                    // sets ctx's draw color
      line(img, 0, 0, 100, 100);           // draws with ctx's color
      \endcode
  */
  class ICLQt_API QuickContext {
  public:
    /// Creates a context with the given memory cap (default 256 MB)
    explicit QuickContext(size_t memoryCap = 256 * 1024 * 1024);

    ~QuickContext();

    QuickContext(const QuickContext&) = delete;
    QuickContext& operator=(const QuickContext&) = delete;

    /// @name Buffer Pool
    /// @{

    /// Returns a recycled or new Image with the given depth and parameters
    /** Tries to reuse an independent (unreferenced) buffer from the pool.
        If the pool is over its memory cap, evicts independent buffers.
        If nothing can be evicted, allocates an unpooled Image with a warning. */
    core::Image getBuffer(core::depth d, const core::ImgParams &params);

    /// Convenience overload
    core::Image getBuffer(core::depth d, const utils::Size &s, int channels);

    /// Current pool memory footprint in bytes
    size_t memoryUsage() const;

    /// Configured memory cap in bytes
    size_t memoryCap() const;

    /// Change the memory cap
    void setMemoryCap(size_t cap);

    /// Drop all independent buffers from the pool
    void clearBuffers();

    /// Toggle per-event pool tracing to stderr (alloc/resize/evict/unpooled).
    /** When enabled every change to the pool prints a one-line trace with
        the event, delta, current usage, and buffer size/depth/channels.
        Intended for diagnosing pool-accounting bugs (e.g. currentUsage
        underflow). Off by default. */
    void setTracing(bool enabled);

    /// Whether per-event pool tracing is currently enabled
    bool isTracing() const;

    /// If enabled, getBuffer() throws ICLException instead of logging a
    /// WARNING and falling back to an unpooled allocation when the pool
    /// cap is exceeded. Useful for tests that need to assert the
    /// overflow path is taken without polluting stderr. Off by default. */
    void setThrowOnCapExceeded(bool enabled);

    /// Whether getBuffer() throws when the pool cap is exceeded
    bool throwsOnCapExceeded() const;

    /// @}
    /// @name Op Application
    /// @{

    /// Gets a pool buffer sized by op.getDestinationParams(), applies the op, returns result
    core::Image applyOp(filter::UnaryOp &op, const core::Image &src);

    /// Rvalue overload — accepts temporary ops for one-liners
    inline core::Image applyOp(filter::UnaryOp &&op, const core::Image &src) {
      return applyOp(op, src);
    }

    /// BinaryOp version
    core::Image applyOp(filter::BinaryOp &op, const core::Image &src1, const core::Image &src2);

    /// BinaryOp rvalue overload
    inline core::Image applyOp(filter::BinaryOp &&op, const core::Image &src1, const core::Image &src2) {
      return applyOp(op, src1, src2);
    }

    /// @}
    /// @name Grabber Management
    /// @{

    /// Returns a cached grabber for the given device, creating one if needed
    std::shared_ptr<io::GenericGrabber> getGrabber(const std::string &dev,
                                                    const std::string &devSpec);

    /// Releases (uncaches) a grabber for the given device
    void releaseGrabber(const std::string &dev, const std::string &devSpec);

    /// @}
    /// @name Drawing State
    /// @{

    float drawColor[4] = {255, 0, 0, 255};
    float fillColor[4] = {0, 0, 0, 0};
    int fontSize = 12;
    std::string fontFamily = "Times";

    /// Save current color/fill state onto an internal stack
    void pushColorState();

    /// Restore last saved color/fill state
    void popColorState();

    /// @}

  private:
    struct Data;
    Data *m_data;
  };

  /// RAII guard that activates a QuickContext on the current thread
  /** The previous context (if any) is restored when the scope ends.
      Scopes nest correctly — each QuickScope saves and restores the
      previous thread-local context pointer.

      \code
      QuickContext ctx;
      {
          QuickScope scope(ctx);  // ctx is now active
          // ... Quick2 calls use ctx ...
      }   // previous context restored
      \endcode
  */
  class ICLQt_API QuickScope {
    QuickContext *m_prev;
  public:
    explicit QuickScope(QuickContext &ctx);
    ~QuickScope();
    QuickScope(const QuickScope&) = delete;
    QuickScope& operator=(const QuickScope&) = delete;
  };

  /// Returns the active QuickContext for the current thread
  /** If no QuickScope is active, returns the process-wide default context. */
  ICLQt_API QuickContext& activeContext();

} // namespace icl::qt
