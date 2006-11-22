#ifndef MORPHOLOGICAL_H
#define MORPHOLOGICAL_H

#include <FilterMask.h>
#include "Img.h"
namespace icl {
  
  /// Class for Morphological operations
  class Morphological : public FilterMask {
  public:

    /// Constructor that creates a Morphological object, with specified mask size
    /** @param maskSize of odd width and height
        Even width or height is increased to next higher odd value.
    */
    Morphological (const Size &maskSize,char* pcMask);

    /// Change mask size
    void setMask (Size size,char* pcMask);

    void MorphStateFree();
    void MorphAdvStateFree();
/*    void MorphGrayStateFree_8u();
    void MorphGrayStateFree_32f();*/
    /// Performs erosion of an image using a general rectangular mask.
    void Erode (const ImgBase *poSrc, ImgBase **ppoDst);
    /// Performs erosion of an image using a 3x3 mask
    void Erode3x3 (const ImgBase *poSrc, ImgBase **ppoDst);
    /// Performs dilation of an image using a general rectangular mask.
    void Dilate (const ImgBase *poSrc, ImgBase **ppoDst);
    /// Performs dilation of an image using a 3x3 mask
    void Dilate3x3 (const ImgBase *poSrc, ImgBase **ppoDst);
    /// Performs dilation of an image.
    void DilateBorderReplicate (const ImgBase *poSrc, ImgBase **ppoDst);
    /// Performs erosion of an image.
    void ErodeBorderReplicate (const ImgBase *poSrc, ImgBase **ppoDst);
    /// Perfoms opening operation of an image.
    void OpenBorder(const ImgBase *poSrc, ImgBase **ppoDst);
    /// Perfoms closing operation of an image.
    void CloseBorder(const ImgBase *poSrc, ImgBase **ppoDst);
    /// Performs top-hat operation of an image.
    void TophatBorder(const ImgBase *poSrc, ImgBase **ppoDst);
    /// Performs black-hat operation of an image.
    void BlackhatBorder(const ImgBase *poSrc, ImgBase **ppoDst);
    /// Calculates morphological gradient of an image.
    void GradientBorder(const ImgBase *poSrc, ImgBase **ppoDst);
    /*
    void GrayDilateBorder(const ImgBase *poSrc, ImgBase **ppoDst);
    void GrayErodeBorder(const ImgBase *poSrc, ImgBase **ppoDst);
    */
/*    void ReconstructDilate(const ImgBase *poSrc, ImgBase **ppoDst);
    void ReconstructErode(const ImgBase *poSrc, ImgBase **ppoDst);
*/
    /// Allocates and initializes morphology state structure for the erosion or dilation operation.
    void InitMorphState(ImgBase **ppoImg);
    /// Allocates and initializes morphology state structure for advanced morphology operations.
    void InitMorphAdvState(ImgBase **ppoImg);
    //void InitMorphGrayState(ImgBase **ppoImg);
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

/*    template<typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, IppiBorderType, IppiMorphGrayState_32f*)>
    void ippiMorphologicalGrayCall (const Img<T> *src, Img<T> *dst);*/

    void InitMorphState (Img8u *img); 
    void InitMorphAdvState (Img8u *img);
    //void InitMorphGrayState (Img8u *img);
    void InitMorphState (Img32f *img); 
    void InitMorphAdvState (Img32f *img);
    //void InitMorphGrayState (Img32f *img);



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

    void TophatBorder(const Img8u *src, Img8u *dst);
    void BlackhatBorder(const Img8u *src, Img8u *dst);
    void GradientBorder(const Img8u *src, Img8u *dst);
/*    void GrayDilateBorder(const Img8u *src, Img8u *dst);
    void GrayErodeBorder(const Img8u *src, Img8u *dst);*/
/*    void ReconstructDilate(const Img8u *src, Img8u *dst);
    void ReconstructErode(const Img8u *src, Img8u *dst);
*/
    void TophatBorder(const Img32f *src, Img32f *dst);
    void BlackhatBorder(const Img32f *src, Img32f *dst);
    void GradientBorder(const Img32f *src, Img32f *dst);
/*    void GrayDilateBorder(const Img32f *src, Img32f *dst);
    void GrayErodeBorder(const Img32f *src, Img32f *dst);*/
/*    void ReconstructDilate(const Img32f *src, Img32f *dst);
    void ReconstructErode(const Img32f *src, Img32f *dst);
*/
    typedef IppiMorphState ICLMorphState ;
    typedef IppiMorphAdvState ICLMorphAdvState;
/*    typedef IppiMorphGrayState_8u ICLMorphGrayState_8u;
    typedef IppiMorphGrayState_32f ICLMorphGrayState_32f;*/
#else
    typedef void ICLMorphState;
    typedef void ICLMorphAdvState;
/*    typedef void ICLMorphGrayState_8u;
    typedef void ICLMorphGrayState_32f;*/
#endif
  private:
//    Ipp8u* pcMask;
    icl8u * pcMask;

    ICLMorphState* pState;
    ICLMorphAdvState* pAdvState;
/*    ICLMorphGrayState_8u* pGrayState_8u;
    ICLMorphGrayState_32f* pGrayState_32f;*/
  };
} // namespace icl
#endif


