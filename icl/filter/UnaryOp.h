// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Configurable.h>
#include <icl/core/Image.h>
#include <icl/core/ImgParams.h>
#include <icl/filter/OpROIHandler.h>
#include <mutex>

namespace icl::filter {
  /// Abstract Base class for Unary Operators \ingroup UNARY
  /** A list of unary operators can be found here:\n
      \ref UNARY
  **/
  class ICLFilter_API UnaryOp : public utils::Configurable{
    void initConfigurable();

    public:

    /// Explicit empty constructor
    UnaryOp();

    /// Explicit copy constructor
    UnaryOp(const UnaryOp &other);

    /// Explicit declaration of the assignment operator
    UnaryOp &operator=(const UnaryOp &other);


    /// Destructor
    virtual ~UnaryOp();

    /// Pure virtual apply — all subclasses implement this
    virtual void apply(const core::Image &src, core::Image &dst) = 0;

    /// Legacy ImgBase** wrapper — final, delegates to Image-based apply
    virtual void apply(const core::ImgBase *src, core::ImgBase **dst) final;

    /// Single-arg apply: uses internal buffer, returns reference to it
    [[nodiscard]] const core::Image& apply(const core::Image &src);

    /// function operator
    [[nodiscard]] inline const core::Image& operator()(const core::Image &src){
      return apply(src);
    }

    /// Returns the expected destination image parameters for a given source
    /** Default: clipToROI → src ROI size; else → src full size, same depth/channels/format.
        Subclasses with different output geometry (e.g. NeighborhoodOp) should override. */
    virtual std::pair<core::depth, core::ImgParams> getDestinationParams(const core::Image &src) const;


    /// sets if the image should be clip to ROI or not
    /**
      @param bClipToROI true=yes, false=no
    */
    void setClipToROI (bool bClipToROI) {
      m_oROIHandler.setClipToROI(bClipToROI);
      prop("UnaryOp.clip to ROI").value = bClipToROI ? "on" : "off";
      call_callbacks("UnaryOp.clip to ROI",this);
    }

    /// sets if the destination image should be adapted to the source, or if it is only checked if it can be adapted.
    /**
      @param bCheckOnly true = destination image is only checked, false = destination image will be checked and adapted.
    */
    void setCheckOnly (bool bCheckOnly) {
      m_oROIHandler.setCheckOnly(bCheckOnly);
      prop("UnaryOp.check only").value = bCheckOnly ? "on" : "off";
      call_callbacks("UnaryOp.check only",this);
    }

    /// returns the ClipToROI status
    /**
      @return true=ClipToROI is enable, false=ClipToROI is disabled
    */
    bool getClipToROI() const { return m_oROIHandler.getClipToROI(); }

    /// returns the CheckOnly status
    /**
      @return true=CheckOnly is enable, false=CheckOnly is disabled
    */
    bool getCheckOnly() const { return m_oROIHandler.getCheckOnly(); }


    /// sets value of a property (always call call_callbacks(propertyName) or Configurable::setPropertyValue)
    virtual void setPropertyValue(const std::string &propertyName, const utils::Any &value);

    // Note: the old fromString() / listFromStringOps() / getFromStringSyntax() /
    // applyFromString() registry was retired — `utils::Configurable::
    // create_configurable(name)` and `get_registered_configurables()` (used by
    // filter-playground and icl-configurable-info) cover the same use case
    // with richer per-property metadata.


    /// Same as Configurable::registerCallback, but wraps the callback so it
    /// acquires `m_applyMutex` before firing. Lets subclasses that mutate
    /// internal state from a property change rely on the base for the
    /// callback-side lock; they still need to acquire `m_applyMutex` at the
    /// top of apply() to close the race (reader-side). Shadows the base's
    /// non-virtual registerCallback — call sites on a UnaryOp-or-derived
    /// object resolve to this overload via static dispatch.
    void registerCallback(const Callback &cb);
    using utils::Configurable::registerCallback;

    protected:

    /// Serializes the apply() reader against property callbacks that mutate
    /// subclass state. Recursive so nested locking (e.g. apply → setMask
    /// path firing a callback) doesn't deadlock. See
    /// project_configurable_op_threadsafety.md for rationale — this replaces
    /// the per-Op mutex pattern that GaborOp / WienerOp used to roll manually.
    mutable std::recursive_mutex m_applyMutex;


    /// Image-based prepare: ensures dst matches the given parameters
    bool prepare(core::Image &dst, core::depth d, const utils::Size &s,
                 core::format fmt, int channels, const utils::Rect &roi,
                 utils::Time t = utils::Time::null);

    /// Image-based prepare: uses getDestinationParams() to determine dst parameters
    bool prepare(core::Image &dst, const core::Image &src);

    /// Image-based prepare: as above, but with explicit depth override
    bool prepare(core::Image &dst, const core::Image &src, core::depth d);

    /// Legacy prepare (for subclasses not yet migrated)
    bool prepare (core::ImgBase **ppoDst, core::depth eDepth, const utils::Size &imgSize,
                  core::format eFormat, int nChannels, const utils::Rect& roi,
                  utils::Time timestamp=utils::Time::null){
      return m_oROIHandler.prepare(ppoDst, eDepth,imgSize,eFormat, nChannels, roi, timestamp);
    }

    /// Legacy prepare
    virtual bool prepare (core::ImgBase **ppoDst, const core::ImgBase *poSrc) {
      return m_oROIHandler.prepare(ppoDst, poSrc);
    }

    /// Legacy prepare
    virtual bool prepare (core::ImgBase **ppoDst, const core::ImgBase *poSrc, core::depth eDepth) {
      return m_oROIHandler.prepare(ppoDst, poSrc, eDepth);
    }

    private:

    OpROIHandler m_oROIHandler;

    core::Image m_buf;
  };


#define DYNAMIC_UNARY_OP_CREATION_FUNCTION(NAME)       \
  extern "C" {                                         \
    UnaryOp *create_##NAME(const std::string &s){      \
      return NAME(s);                                  \
    }                                                  \
  }

  } // namespace icl::filter