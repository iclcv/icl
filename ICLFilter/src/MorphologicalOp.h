#ifndef MORPHOLOGICAL_OP_H
#define MORPHOLOGICAL_OP_H

#include <NeighborhoodOp.h>
#include <Img.h>
namespace icl {
  
  /// Class for Morphological operations  (Only available for Img8u and Img32f, IPP only!)

/**
    /// Performs erosion of an image using a general rectangular mask.
    void applyErode (const ImgBase *poSrc, ImgBase **ppoDst);
    /// Performs erosion of an image using a 3x3 mask
    void applyErode3x3 (const ImgBase *poSrc, ImgBase **ppoDst);
    /// Performs dilation of an image using a general rectangular mask.
    void applyDilate (const ImgBase *poSrc, ImgBase **ppoDst);
    /// Performs dilation of an image using a 3x3 mask
    void applyDilate3x3 (const ImgBase *poSrc, ImgBase **ppoDst);
    /// Performs dilation of an image.
    void applyDilateBorderReplicate (const ImgBase *poSrc, ImgBase **ppoDst);
    /// Performs erosion of an image.
    void applyErodeBorderReplicate (const ImgBase *poSrc, ImgBase **ppoDst);
    /// Perfoms opening operation of an image.
    void applyOpenBorder(const ImgBase *poSrc, ImgBase **ppoDst);
    /// Perfoms closing operation of an image.
    void applyCloseBorder(const ImgBase *poSrc, ImgBase **ppoDst);
    /// Performs top-hat operation of an image.
    void applyTophatBorder(const ImgBase *poSrc, ImgBase **ppoDst);
    /// Performs black-hat operation of an image.
    void applyBlackhatBorder(const ImgBase *poSrc, ImgBase **ppoDst);
    /// Calculates morphological gradient of an image.
    void applyGradientBorder(const ImgBase *poSrc, ImgBase **ppoDst);
*/


  class MorphologicalOp : public NeighborhoodOp {
  public:

/// Performs dilation of an image.
  enum optype { 
    dilate,
    erode,
    dilate3x3,
    erode3x3,
    dilateBorderReplicate,
    erodeBorderReplicate,
    openBorder,
    closeBorder,
    tophatBorder,
    blackhatBorder,
    gradientBorder
  };
    /// Constructor that creates a Morphological object, with specified mask size
    /** @param maskSize of odd width and height
        Even width or height is increased to next higher odd value.
        @param pcMask pointer to the Mask
    */
    MorphologicalOp (const Size &maskSize,char* pcMask, optype eoptype);
    ~MorphologicalOp ();
    /// Change mask size
    void setMask (Size size,char* pcMask);
    /// Performs morph of an image with given optype and mask.
    icl8u* getMask() const;
    Size getMaskSize() const;
    void setOptype(optype type);
    optype getOptype() const;

    void apply (const ImgBase *poSrc, ImgBase **ppoDst);
    

#ifdef WITH_IPP_OPTIMIZATION
  private:

    template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, const Ipp8u*, IppiSize, IppiPoint)>
    void ippiMorphologicalCall (const Img<T> *src, Img<T> *dst);
    template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize)>
    void ippiMorphologicalCall3x3 (const Img<T> *src, Img<T> *dst);

   template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, _IppiBorderType, IppiMorphState*)>
    void ippiMorphologicalBorderReplicateCall (const Img<T> *src, Img<T> *dst,IppiMorphState *state);

    template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, IppiBorderType, IppiMorphAdvState*)>
    void ippiMorphologicalBorderCall (const Img<T> *src, Img<T> *dst, IppiMorphAdvState *advState);

    typedef IppiMorphState ICLMorphState ;
    typedef IppiMorphAdvState ICLMorphAdvState;
#else
    typedef void ICLMorphState;
    typedef void ICLMorphAdvState;
#endif
  private:
    icl8u * m_pcMask;
    Size m_sMasksize;
    ICLMorphState* m_pState8u;
    ICLMorphState* m_pState32f;
    ICLMorphAdvState* m_pAdvState8u;
    ICLMorphAdvState* m_pAdvState32f;
    bool m_bMorphState8u;
    bool m_bMorphState32f;
    bool m_bMorphAdvState8u;
    bool m_bMorphAdvState32f;
    bool m_bHas_changed;
    bool m_bHas_changedAdv;
    void deleteMorphStates();
    void checkMorphAdvState8u(const Size roiSize);
    void checkMorphAdvState32f(const Size roiSize);
    void checkMorphState8u(const Size roiSize);
    void checkMorphState32f(const Size roiSize);
  
    
    optype m_eType;

  };
} // namespace icl
#endif
