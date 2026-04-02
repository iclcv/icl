// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Types.h>
#include <ICLUtils/Size.h>
#include <ICLUtils/Rect.h>
#include <ICLUtils/Time.h>
#include <ICLUtils/Range.h>
#include <ICLUtils/Exception.h>

#include <memory>
#include <string>
#include <vector>
#include <iosfwd>

namespace icl::core {
  // Forward declarations — Image.h does NOT pull in ImgBase.h or Img.h
  class ImgBase;
  template<class T> class Img;

  /// Value-semantic, type-erased image handle wrapping ImgBase
  /** Image provides a safe, convenient interface around the ImgBase/Img<T>
      class hierarchy. It uses shared_ptr<ImgBase> internally, giving
      automatic memory management and cheap shallow copies.

      Key improvements over raw ImgBase*:
      - No manual memory management (no delete, no ownership confusion)
      - ensureCompatible() is a method (no ImgBase** double pointers)
      - Value semantics: pass by value, return from functions
      - visit() for type-safe depth dispatch without switch statements

      Copy semantics are shallow (like shared_ptr). Use deepCopy()
      for an independent copy. Use detach() to make shared channel
      data independent.

      \section TYPED_ACCESS Typed Access
      Use as<T>() to get a reference to the underlying Img<T>.
      Use visit() with a generic lambda for type-safe dispatch:
      \code
      img.visit([](auto &typed) {
          // typed is Img<icl8u>&, Img<icl32f>&, etc.
          typed(0, 0, 0) = 42;
      });
      \endcode
      Note: callers using as<T>() or visit() must include Img.h.
  */
  class ICLCore_API Image {
    std::shared_ptr<ImgBase> m_impl;

  public:
    /// @name Constructors
    /// @{

    /// Creates a null (empty) image
    Image();

    /// Creates an image with given size, depth, and channel count
    Image(const utils::Size &s, depth d, int channels = 1,
          format fmt = formatMatrix);

    /// Wraps an existing ImgBase pointer (takes ownership)
    explicit Image(ImgBase *p);

    /// Wraps an existing shared pointer
    Image(const std::shared_ptr<ImgBase> &p);

    /// Shallow copy from an ImgBase (or Img<T> via inheritance)
    /** Creates a new Image that shares channel data with the source.
        Non-explicit: allows implicit conversion from Img<T> to Image,
        so any Img<T> can be passed where const Image& is expected.
        Only metadata is copied — pixel data is shared (refcounted). */
    Image(const ImgBase &img);

    /// Default shallow copy (shared_ptr semantics)
    Image(const Image &) = default;
    Image(Image &&) = default;
    Image& operator=(const Image &) = default;
    Image& operator=(Image &&) = default;

    /// Element-wise equality comparison
    /** Both images must have the same depth, size, and channel count.
        Pixel comparison uses epsilon for floating-point depths. */
    bool operator==(const Image &other) const;

    /// Element-wise inequality
    bool operator!=(const Image &other) const { return !(*this == other); }

    /// @}
    /// @name Null Check
    /// @{

    /// Returns true if the image has no backing data
    bool isNull() const;

    /// Convenience: true if not null
    explicit operator bool() const;

    /// @}
    /// @name Metadata Accessors
    /// @{

    utils::Size getSize() const;
    int getWidth() const;
    int getHeight() const;
    int getDim() const;        ///< width * height
    utils::Rect getImageRect() const; ///< Rect(0,0,w,h)
    int getChannels() const;
    depth getDepth() const;
    format getFormat() const;
    utils::Time getTime() const;
    void setTime(const utils::Time &t);
    void setFormat(format fmt);

    /// @}
    /// @name ROI (Region of Interest)
    /// @{

    utils::Rect getROI() const;
    utils::Size getROISize() const;
    utils::Point getROIOffset() const;
    void setROI(const utils::Rect &roi);
    void setROI(const utils::Point &offset, const utils::Size &size);
    void setFullROI();
    bool hasFullROI() const;

    /// @}
    /// @name Buffer Management
    /// @{

    /// Ensures the image has the given parameters, reallocating if needed.
    /** This replaces the ensureCompatible(ImgBase**, ...) pattern.
        If depth changes, a new image is allocated (other shared holders
        keep the old one). If only size/channels/format change, the
        existing buffer is adapted in-place. */
    void ensureCompatible(depth d, const utils::Size &s, int channels,
                          format fmt = formatMatrix);

    /// Ensures this image matches another's parameters
    void ensureCompatible(const Image &other);

    void setSize(const utils::Size &s);
    void setChannels(int n);

    /// @}
    /// @name Data Independence
    /// @{

    /// Makes channel data independent from other images sharing it.
    /** If channel == -1, detaches all channels. */
    void detach(int channel = -1);

    /// Returns true if all channel data is exclusively owned
    bool isIndependent() const;

    /// @}
    /// @name Copy / Convert
    /// @{

    /// Returns a deep (independent) copy
    [[nodiscard]] Image deepCopy() const;

    /// Returns a copy converted to the given depth
    [[nodiscard]] Image convert(depth d) const;

    /// Converts this image into dst (dst determines the target depth)
    void convertTo(Image &dst) const;

    /// Converts only the ROI of this image into dst (dst determines target depth)
    void convertROITo(Image &dst) const;

    /// Returns a scaled copy
    [[nodiscard]] Image scaledCopy(const utils::Size &newSize,
                     scalemode sm = interpolateNN) const;

    /// @}
    /// @name Channel Operations
    /// @{

    /// Returns a shallow single-channel image
    [[nodiscard]] Image selectChannel(int c) const;

    /// Returns a shallow multi-channel selection
    [[nodiscard]] Image selectChannels(const std::vector<int> &channels) const;

    /// @}
    /// @name In-Place Operations
    /// @{

    /// Fills all (or one) channel(s) with a value
    void clear(int channel = -1, icl64f val = 0);

    /// Mirrors the image in-place along the given axis
    void mirror(axis a);

    /// Returns a mirrored copy (flipped along given axis)
    [[nodiscard]] Image mirrored(axis a) const;

    /// Scales the image in-place
    void scale(const utils::Size &s, scalemode sm = interpolateNN);

    /// Normalizes all channels to the given range
    void normalizeAllChannels(const utils::Range<icl64f> &dstRange);

    /// @}
    /// @name Min / Max
    /// @{

    icl64f getMin(int channel) const;
    icl64f getMax(int channel) const;
    utils::Range<icl64f> getMinMax(int channel) const;

    /// @}
    /// @name Raw Pointer Access (legacy interop)
    /// @{

    ImgBase* ptr();
    const ImgBase* ptr() const;

    /// @}
    /// @name Swap
    /// @{

    void swap(Image &other);

    /// @}
    /// @name Debug
    /// @{

    void print(const std::string &title = "image") const;

    /// @}

    // ================================================================
    // Template methods — defined below the class.
    // Callers using these must include <ICLCore/Img.h>.
    // ================================================================

    /// @name Type Check
    /// @{

    /// Returns true if the underlying image has pixel type T
    template<class T> bool is() const {
      return !isNull() && getDepth() == core::getDepth<T>();
    }

    /// @}
    /// @name Typed Access
    /// @{

    /// Returns a reference to the underlying Img<T>.
    /** Caller must ensure getDepth() matches T (use is<T>() to check). */
    template<class T>
    Img<T>& as() {
      return *static_cast<Img<T>*>(m_impl.get());
    }

    /// const version
    template<class T>
    const Img<T>& as() const {
      return *static_cast<const Img<T>*>(m_impl.get());
    }

    // Convenience typed accessors (match ImgBase::asImg<T>() / as8u() etc.)
    Img<icl8u>&  as8u()        { return as<icl8u>();  }
    Img<icl16s>& as16s()       { return as<icl16s>(); }
    Img<icl32s>& as32s()       { return as<icl32s>(); }
    Img<icl32f>& as32f()       { return as<icl32f>(); }
    Img<icl64f>& as64f()       { return as<icl64f>(); }
    const Img<icl8u>&  as8u()  const { return as<icl8u>();  }
    const Img<icl16s>& as16s() const { return as<icl16s>(); }
    const Img<icl32s>& as32s() const { return as<icl32s>(); }
    const Img<icl32f>& as32f() const { return as<icl32f>(); }
    const Img<icl64f>& as64f() const { return as<icl64f>(); }

    /// @}
    /// @name Visitor (type-safe depth dispatch)
    /// @{

    /// Calls f with the typed Img<T>& matching the runtime depth.
    /** Example:
        \code
        img.visit([](auto &typed) { ... });
        \endcode
        The lambda receives Img<icl8u>&, Img<icl16s>&, etc.
        All branches must return the same type (or void). */
    template<class F>
    auto visit(F &&f) {
      switch(getDepth()){
        case depth8u:  return f(as<icl8u>());
        case depth16s: return f(as<icl16s>());
        case depth32s: return f(as<icl32s>());
        case depth32f: return f(as<icl32f>());
        default:       return f(as<icl64f>());
      }
    }

    /// const version — lambda receives const Img<T>&
    template<class F>
    auto visit(F &&f) const {
      switch(getDepth()){
        case depth8u:  return f(as<icl8u>());
        case depth16s: return f(as<icl16s>());
        case depth32s: return f(as<icl32s>());
        case depth32f: return f(as<icl32f>());
        default:       return f(as<icl64f>());
      }
    }

    /// Calls f(typed, other_typed) for two images of the same depth.
    template<class F>
    auto visitWith(Image &other, F &&f) {
      switch(getDepth()){
        case depth8u:  return f(as<icl8u>(),  other.as<icl8u>());
        case depth16s: return f(as<icl16s>(), other.as<icl16s>());
        case depth32s: return f(as<icl32s>(), other.as<icl32s>());
        case depth32f: return f(as<icl32f>(), other.as<icl32f>());
        default:       return f(as<icl64f>(), other.as<icl64f>());
      }
    }

    /// const version — f receives (const Img<T>&, Img<T>&)
    template<class F>
    auto visitWith(Image &other, F &&f) const {
      switch(getDepth()){
        case depth8u:  return f(as<icl8u>(),  other.as<icl8u>());
        case depth16s: return f(as<icl16s>(), other.as<icl16s>());
        case depth32s: return f(as<icl32s>(), other.as<icl32s>());
        case depth32f: return f(as<icl32f>(), other.as<icl32f>());
        default:       return f(as<icl64f>(), other.as<icl64f>());
      }
    }

    /// For ROI-aware line-based channel/pixel iteration on typed Img<T>,
    /// see Visitors.h (visitROILines, visitROILinesPerChannelWith, etc.)
    /// and VisitorsN.h (visitROILinesN, visitROILinesNWith).

  };

  /// Stream output (prints size, depth, channels, format)
  ICLCore_API std::ostream& operator<<(std::ostream &s, const Image &img);

  } // namespace icl::core