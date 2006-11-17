#ifndef MORPHOLOGICAL_H
#define MORPHOLOGICAL_H

#include <FilterMask.h>
#include "Img.h"
namespace icl {
  
  /// Class for Morphological operations
  class Morphological : public FilterMask {
  public:

    /// Constructor that creates a Morphologicalwiener filter object, with specified mask size
    /** @param maskSize of odd width and height
        Even width or height is increased to next higher odd value.
    */
    Morphological (const Size &maskSize,char* pcMask);

    /// Change mask size
    void setMask (Size size,char* pcMask);

    void MorphStateFree();
    void MorphAdvStateFree();
    /// Filters an image using the Wiener algorithm. ImgBase version, IPP-only!
    void Erode (const ImgBase *poSrc, ImgBase **ppoDst);
    void Erode3x3 (const ImgBase *poSrc, ImgBase **ppoDst);
    void Dilate (const ImgBase *poSrc, ImgBase **ppoDst);
    void Dilate3x3 (const ImgBase *poSrc, ImgBase **ppoDst);
    void DilateBorderReplicate (const ImgBase *poSrc, ImgBase **ppoDst);
    void ErodeBorderReplicate (const ImgBase *poSrc, ImgBase **ppoDst);
    void OpenBorder(const ImgBase *poSrc, ImgBase **ppoDst);
    void CloseBorder(const ImgBase *poSrc, ImgBase **ppoDst);
    void InitMorphState(ImgBase **ppoImg);
    void InitMorphAdvState(ImgBase **ppoImg);
#ifdef WITH_IPP_OPTIMIZATION
  protected:

    template<typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, const Ipp8u*, IppiSize, IppiPoint)>
    void ippiMorphologicalCall (const Img<T> *src, Img<T> *dst);
    template<typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize)>
    void ippiMorphologicalCall3x3 (const Img<T> *src, Img<T> *dst);

    template<IppStatus (*ippiFunc) (int, const Ipp8u*, IppiSize, IppiPoint,IppiMorphState*)>
    void ippiMorphologyInitCall (int roiWidth, IppiMorphState* pState );

   template<typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, _IppiBorderType, IppiMorphState*)>
    void ippiMorphologicalBorderReplicateCall (const Img<T> *src, Img<T> *dst);

    template<typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, IppiBorderType, IppiMorphAdvState*)>
    void ippiMorphologicalBorderCall (const Img<T> *src, Img<T> *dst);

    void InitMorphState (Img8u *img); 
    void InitMorphAdvState (Img8u *img);
    void InitMorphState (Img32f *img); 
    void InitMorphAdvState (Img32f *img);



    void OpenBorder(const Img32f *src, Img32f *dst);
    void CloseBorder(const Img32f *src, Img32f *dst);
    void OpenBorder(const Img8u *src, Img8u *dst);
    void CloseBorder(const Img8u *src, Img8u *dst);

    /// Filters an image using the Wiener algorithm. Img8u version, IPP-only!
    void Erode (const Img8u *src, Img8u *dst);
    void Erode3x3 (const Img8u *src, Img8u *dst);
    /// Filters an image using the Wiener algorithm. Img32f version, IPP-only!
    void Erode (const Img32f *src, Img32f *dst);
    void Erode3x3 (const Img32f *src, Img32f *dst);
    /// Filters an image using the Wiener algorithm. Img8u version, IPP-only!
    void Dilate (const Img8u *src, Img8u *dst);
    void Dilate3x3 (const Img8u *src, Img8u *dst);
    /// Filters an image using the Wiener algorithm. Img32f version, IPP-only!
    void Dilate (const Img32f *src, Img32f *dst);
    void Dilate3x3 (const Img32f *src, Img32f *dst);

    void DilateBorderReplicate(const Img8u *src, Img8u *dst);
    void DilateBorderReplicate(const Img32f *src, Img32f *dst);

    void ErodeBorderReplicate(const Img8u *src, Img8u *dst);
    void ErodeBorderReplicate(const Img32f *src, Img32f *dst);
    typedef IppiMorphState ICLMorphState ;
    typedef IppiMorphAdvState ICLMorphAdvState;
#else
    typedef void ICLMorphState;
    typedef void ICLMorphAdvState;
#endif
  private:
//    Ipp8u* pcMask;
    icl8u * pcMask;

    ICLMorphState* pState;
    ICLMorphAdvState* pAdvState;
  };
} // namespace icl
#endif


