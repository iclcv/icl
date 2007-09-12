#ifndef OP_ROI_HANDLER_H
#define OP_ROI_HANDLER_H

#include <iclImgBase.h>

namespace icl {
  /// Utility base class for Image Operators \ingroup OTHER
  /** The OpROIHandler class builds a utility base class for ICL unary and binary
      operator classes. Each Operator is performed on the ROI of the source image(s) 
      only. The destination image is <em> always </em> adapted in its parameters to the
      necessary values. We distinguish the following modes:

      - Adapt the destination image in its size and ROI to the source
        image. Hence the destination will have the same size and ROI as the
        source image. Nevertheless the operator is applied on the ROI only, 
        leaving the previous content of the destination image unchanged
        in this border region. So this mode requires subsequent handling
        of the border.(bClipToROI = false)

        <pre>
       source image    destination-image   
        xxxxxxxxx        xxxxxxxxx
        xxxx....x        xxxx....x
        xxxx....x   -->  xxxx....x
        xxxx....x        xxxx....x
        </pre>

      - Adapt the destination images size, such that it exactly comprises
        the ROI of the source image. (bClipToROI = true)
        <pre>
       source image    destination-image   
        xxxxxxxxx        
        xxxx....x        ....
        xxxx....x   -->  ....
        xxxx....x        ....
        </pre>

      - Only check the destination images parameters and issue a warning
        if not set correctly already. In this case the filter operation itself
        should not be performed. (bCheckOnly = true)

      To this end the Filter class provides variables m_bCheck and m_bClipToROI 
      accessible by setter and getter functions as well as several version of 
      the prepare() methods which checks and adapts the destination image if 
      neccessary.
  */
  class OpROIHandler {
    public:
    /// Destructor
    virtual ~OpROIHandler(){}

    
    private:
    friend class UnaryOp;
    friend class BinaryOp;
    /// change adaption of destination image (see class description)
    void setClipToROI (bool bClipToROI) {this->m_bClipToROI = bClipToROI;}
    void setCheckOnly (bool bCheckOnly) {this->m_bCheckOnly = bCheckOnly;}
    
    bool getClipToROI() const { return m_bClipToROI; }
    bool getCheckOnly() const { return m_bCheckOnly; }
    
    /// Filter is a base class for other classes and should be instantiated
    OpROIHandler() : m_bClipToROI (true), m_bCheckOnly (false) {}
    
    /// check+adapt destination images parameters against given values
    /// bCheckOnly mode ignores the given imgSize
    bool prepare (ImgBase **ppoDst, depth eDepth, const Size &imgSize, 
                  format eFormat, int nChannels, const Rect& roi, 
                  Time timestamp=Time::null);
    
    /// check+adapt destination image to properties of given source image
    virtual bool prepare (ImgBase **ppoDst, const ImgBase *poSrc) {
      // ensure that the checkOnly flag is set to TRUE, if source and destination images are the same pointers, 
      ICLASSERT_RETURN_VAL( !(ppoDst && (*ppoDst==poSrc) && !getCheckOnly() ), false);
      return prepare (ppoDst, poSrc->getDepth(), chooseSize (poSrc),
                      poSrc->getFormat(), poSrc->getChannels (), chooseROI (poSrc),
                      poSrc->getTime());
    }
    
    /// check+adapt destination image to properties of given source image
    /// but use explicitly given depth
    virtual bool prepare (ImgBase **ppoDst, const ImgBase *poSrc, depth eDepth) {
      // ensure that the checkOnly flag is set to TRUE, if source and destination images are the same pointers, 
      ICLASSERT_RETURN_VAL( !(ppoDst && (*ppoDst==poSrc) && !getCheckOnly() ), false);
      return prepare (ppoDst, eDepth, chooseSize (poSrc),
                      poSrc->getFormat(), poSrc->getChannels (), chooseROI (poSrc),
                      poSrc->getTime());
    }
    /// return to-be-used image size depending on bClipToROI
    const Size chooseSize (const ImgBase *poSrc) {
      return m_bClipToROI ? poSrc->getROISize () : poSrc->getSize ();
    }
    /// return to-be-used ROI depending on bClipToROI
    const Rect chooseROI (const ImgBase *poSrc) {
      return m_bClipToROI ? Rect (Point::null, poSrc->getROISize ())
        : poSrc->getROI();
    }
    
    bool m_bClipToROI, m_bCheckOnly;
  };
}
#endif
