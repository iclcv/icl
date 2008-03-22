#ifndef MORPHOLOGICAL_OP_H
#define MORPHOLOGICAL_OP_H

#include <iclNeighborhoodOp.h>
#include <iclImg.h>
#include <iclUncopyable.h>

namespace icl {
  
  /// Class for Morphological operations  \ingroup UNARY \ingroup NBH
  /** (Only available for Img8u and Img32f, IPP only!) 
      
      \section DST_SIZE Destination Image Sizes
      Destination image ROI size depend not only on given input ROI size
      and mask-size, but also on the used optype.
      In case of default operations (dilate, erode, ... destination ROI
      size is calculated as in the top level NeighborhoodOp class.
      <b>But Note:</b> for dilate and erode border, destination image roi
      size becomes <b>equal</b> to the source images one.
  */
  class MorphologicalOp : public NeighborhoodOp, public Uncopyable {
  public:

  /// this enum specifiy all possible morphological operations
  enum optype { 
    dilate=0,
    erode=1,
    dilate3x3=2,
    erode3x3=3,
    dilateBorderReplicate=4,
    erodeBorderReplicate=5,
    openBorder=6,
    closeBorder=7,
    tophatBorder=8,
    blackhatBorder=9,
    gradientBorder=10
  };
    /// Constructor that creates a Morphological object, with specified mask size
    /** @param maskSize of odd width and height
                        Even width or height is increased to next higher odd value.
        @param pcMask pointer to the Mask
        @param eoptype operation type
    */
    MorphologicalOp (const Size &maskSize,char* pcMask, optype eoptype);
  
    /// Destructor
    ~MorphologicalOp ();
  
    /// Change mask
    void setMask (Size size,char* pcMask);
    
    /// returns mask
    /** 
      @return mask
    */
    icl8u* getMask() const;

    /// returns mask size
    /** 
      @return mask size
    */
    Size getMaskSize() const;
    
    void setOptype(optype type);
    
    /// returns the type of the selected morphological operation
    /** 
      @return optype
    */
    optype getOptype() const;
    
    /// Performs morph of an image with given optype and mask.
    void apply (const ImgBase *poSrc, ImgBase **ppoDst);
    

#ifdef WITH_IPP_OPTIMIZATION
  private:

    template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, const Ipp8u*, IppiSize, IppiPoint)>
    IppStatus ippiMorphologicalCall (const Img<T> *src, Img<T> *dst);
    template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize)>
    IppStatus ippiMorphologicalCall3x3 (const Img<T> *src, Img<T> *dst);

    template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, _IppiBorderType, IppiMorphState*)>
    IppStatus ippiMorphologicalBorderReplicateCall (const Img<T> *src, Img<T> *dst,IppiMorphState *state);

    template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, IppiBorderType, IppiMorphAdvState*)>
    IppStatus ippiMorphologicalBorderCall (const Img<T> *src, Img<T> *dst, IppiMorphAdvState *advState);

    typedef IppiMorphState ICLMorphState ;
    typedef IppiMorphAdvState ICLMorphAdvState;
#else
    typedef void ICLMorphState;
    typedef void ICLMorphAdvState;
#endif
  private:
    icl8u * m_pcMask;
    Size m_oMaskSizeMorphOp; // actually masksize of NeighborhoodOp and MorphOp may be different
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
