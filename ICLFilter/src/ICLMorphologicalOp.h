#include <ICLNeighborhoodOp.h>
#include <ICLImg.h>
#ifndef MORPHOLOGICAL_OP_H
#define MORPHOLOGICAL_OP_H

namespace icl {
  
  /// Class for Morphological operations  (Only available for Img8u and Img32f, IPP only!)
  /** TODO !*/
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
        @param eoptype operation type
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
