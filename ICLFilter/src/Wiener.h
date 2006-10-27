#ifndef WIENER_H
#define WIENER_H

#include <FilterMask.h>
#include "Img.h"
namespace icl {
  
  /// Class for Wiener Filter
  /** 
Wiener filters are commonly used in
image processing applications to remove additive noise from degraded images, to restore a blurry
image, and in similar operations.
  */
  class Wiener : public FilterMask {
  public:

    /// Constructor that creates a wiener filter object, with specified mask size
    /** @param maskSize of odd width and height
        Even width or height is increased to next higher odd value.
    */
    Wiener (const Size &maskSize);

    /// Change mask size
    void setMask (Size size);
#ifdef WITH_IPP_OPTIMIZATION
    /// Filters an image using the Wiener algorithm. ImgI version, IPP-only!
    void FilterWiener (const ImgI *poSrc, ImgI **ppoDst,icl32f *noise);

  protected:
    template<typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, IppiSize, IppiPoint, icl32f*, icl8u*)>
    void ippiWienerCall (const Img<T> *src, Img<T> *dst,icl32f *noise);
    /// Filters an image using the Wiener algorithm. Img8u version, IPP-only!
    void FilterWiener (const Img8u *src, Img8u *dst,icl32f *noise);
    /// Filters an image using the Wiener algorithm. Img32f version, IPP-only!
    void FilterWiener (const Img32f *src, Img32f *dst, icl32f *noise);

#endif
  private:
    icl8u *m_oBuf;
  };
} // namespace icl
#endif


