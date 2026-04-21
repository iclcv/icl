// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke

#pragma once

#include <icl/utils/CompatMacros.h>
// Point.h hosts the `detail::BiTuple` CRTP mixin shared with SizeT —
// see its preamble.  We need it here for SizeT to inherit from.
#include <icl/utils/Point.h>
#include <iosfwd>
#include <string>
#include <type_traits>

#ifdef ICL_HAVE_IPP
#include <ipp.h>
#else
/// Fallback for the IppiSize struct when IPP is not available.
/** Placed in the global namespace to match the real IPP struct, so
    code using IppiSize (and the `toIppiSize()` method) compiles
    identically with and without IPP. */
struct IppiSize { int width, height; };
#endif

namespace icl::utils {
  /// Templated 2D size.
  /** ICL provides two instantiations as typedefs:
      - `Size` (aka `SizeT<int>`) — integer pixel dimensions
      - `Size32f` (aka `SizeT<float>`) — sub-pixel dimensions (e.g.
        for an OpenGL-texture-coord context)

      Arithmetic, equality, and narrow/widen conversion machinery are
      inherited from `detail::BiTuple`; only size-specific features
      (`getDim`, `toIppiSize`, named resolution constants,
      string-parse constructor) live here. */
  template<typename T>
  class SizeT : public detail::BiTuple<SizeT<T>, T> {
    public:
    /// width
    T width{T(0)};
    /// height
    T height{T(0)};

    /// required by BiTuple's cross-type conversion helpers
    template<typename U> using rebind = SizeT<U>;

    // `null` is inherited from BiTuple (= SizeT{}).

    // ---------- Named screen / video resolutions ----------
    // Defined for every instantiation — SizeT<float>::VGA is
    // SizeT<float>{640.f, 480.f}.
    inline static const SizeT QQVGA { T(160),  T(120) };  ///< Quater QVGA
    inline static const SizeT CGA   { T(320),  T(200) };  ///< Color Graphics Adapter
    inline static const SizeT QVGA  { T(320),  T(240) };  ///< Quarter VGA
    inline static const SizeT HVGA  { T(480),  T(320) };  ///< Half VGA
    inline static const SizeT EGA   { T(640),  T(350) };  ///< Enhanced Graphics Adapter
    inline static const SizeT VGA   { T(640),  T(480) };  ///< Video Graphics Array
    inline static const SizeT WVGA  { T(800),  T(480) };  ///< Wide VGA
    inline static const SizeT SVGA  { T(800),  T(600) };  ///< Super VGA
    inline static const SizeT QHD   { T(960),  T(540) };  ///< Quarter HD
    inline static const SizeT DVGA  { T(960),  T(640) };  ///< Double VGA
    inline static const SizeT XGA   { T(1024), T(768) };  ///< Extended Graphics Array
    inline static const SizeT XGAP  { T(1152), T(864) };  ///< XGA Plus
    inline static const SizeT DSVGA { T(1200), T(800) };  ///< Double SVGA
    inline static const SizeT HD720 { T(1280), T(720) };  ///< Half-definition (720p)
    inline static const SizeT WXGA  { T(1280), T(800) };  ///< Wide XGA
    inline static const SizeT WXGAP { T(1440), T(900) };  ///< Wide XGA Plus
    inline static const SizeT SXVGA { T(1280), T(960) };  ///< Quad VGA
    inline static const SizeT SXGA  { T(1280), T(1024) }; ///< Super XGA
    inline static const SizeT WSXGA { T(1600), T(900) };  ///< Wide Super XGA
    inline static const SizeT SXGAP { T(1400), T(1050) }; ///< Super XGA Plus
    inline static const SizeT WSXGAP{ T(1600), T(1050) }; ///< Wide Super XGA Plus
    inline static const SizeT UXGA  { T(1600), T(1200) }; ///< Ultra XGA
    inline static const SizeT HD1080{ T(1920), T(1080) }; ///< 1920×1080 (1080p)
    inline static const SizeT WUXGA { T(1920), T(1080) }; ///< Wide UXGA
    inline static const SizeT UD    { T(3840), T(2160) }; ///< Ultra definition (4K)

    // Video formats
    inline static const SizeT CIF   { T(352),  T(288) };  ///< Common Intermediate Format
    inline static const SizeT SIF   { T(360),  T(240) };  ///< Source Input Format
    inline static const SizeT SQCIF { T(128),  T(96)  };  ///< Semi Quarter CIF
    inline static const SizeT QCIF  { T(176),  T(144) };  ///< Quarter CIF
    inline static const SizeT PAL   { T(768),  T(576) };  ///< Phase-Alternating Line
    inline static const SizeT NTSC  { T(640),  T(480) };  ///< NTSC

    /// default constructor
    constexpr SizeT() = default;

    /// coordinate constructor
    constexpr SizeT(T width_, T height_) : width(width_), height(height_) {}

    /// creates a size from a string — "VGA", "CIF", or "1024x768"
    explicit SizeT(const std::string &name);

    /// implicit widening from integer size to floating-point size
    /** Lossless — integer pixel dimensions fit exactly in `float`. */
    template<typename U>
      requires (!std::is_same_v<U, T>
                && std::is_integral_v<U>
                && std::is_floating_point_v<T>)
    constexpr SizeT(const SizeT<U> &o)
      : width(static_cast<T>(o.width)), height(static_cast<T>(o.height)) {}

    /// explicit cross-type construction for anything lossy (truncates)
    /** For rounded narrowing prefer `.rounded()`, `.floored()`,
        `.ceiled()`, or `.truncated()` at the call site. */
    template<typename U>
      requires (!std::is_same_v<U, T>
                && !(std::is_integral_v<U> && std::is_floating_point_v<T>))
    explicit constexpr SizeT(const SizeT<U> &o)
      : width(static_cast<T>(o.width)), height(static_cast<T>(o.height)) {}

    /// BiTuple element access (i == 0 → width, else → height)
    constexpr T &operator[](int i) { return i ? height : width; }
    constexpr const T &operator[](int i) const { return i ? height : width; }

    /// returns width * height
    constexpr auto getDim() const { return width * height; }

    /// explicit conversion to IppiSize (int dimensions)
    IppiSize toIppiSize() const {
      return { static_cast<int>(width), static_cast<int>(height) };
    }
  };

  /// ostream operator: formats as "WIDTHxHEIGHT"
  template<typename T>
  std::ostream &operator<<(std::ostream &os, const SizeT<T> &s);

  /// istream operator: accepts "WIDTHxHEIGHT" or a named resolution
  /// ("VGA", "HD720", …) — the named form is only recognized for
  /// integer Size (SizeT<int>); any other T only accepts numeric form.
  template<typename T>
  std::istream &operator>>(std::istream &is, SizeT<T> &s);

  // ---------- established ICL type aliases ----------

  /// integer 2D size (pixel dimensions)
  using Size = SizeT<int>;

  /// single-precision 2D size (sub-pixel dimensions)
  using Size32f = SizeT<float>;

  } // namespace icl::utils
