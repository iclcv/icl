#ifndef BINARY_OP_H
#define BINARY_OP_H

#include "iclOpROIHandler.h"

namespace icl{
  /// Abstract base class for binary image operations \ingroup BINARY
  /** A list of all binary operators can be found here: \n
      \ref BINARY
  **/
  class BinaryOp{
    public:
    
    /// default constructor
    BinaryOp();
    
    /// copy constructor
    BinaryOp(const BinaryOp &other);
    
    /// assignment operator
    BinaryOp &operator=(const BinaryOp &other);
    
    /// virtual destructor
    virtual ~BinaryOp();
    
    
    /// pure virtual apply function
    virtual void apply(const ImgBase *operand1,const ImgBase *operand2, ImgBase **result)=0;
    
    /// applyfunction without explicit destination image 
    /** Usually this function must not be reimplemented, because it's default operation does simply use
        an internal buffer to call apply(const ImgBase*,const ImgBase*,ImgBase**). */
    virtual const ImgBase *apply(const ImgBase *operand1, const ImgBase *operand2);

    /// sets if the image should be clip to ROI or not
    /**
      @param bClipToROI true=yes, false=no
    */
    void setClipToROI (bool bClipToROI) { m_oROIHandler.setClipToROI(bClipToROI); }

    /// sets if the destination image should be adapted to the source, or if it is only checked if it can be adapted.
    /**
      @param bCheckOnly true = destination image is only checked, false = destination image will be checked and adapted.
    */
    void setCheckOnly (bool bCheckOnly) { m_oROIHandler.setCheckOnly(bCheckOnly); }
    
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

    protected:
    bool prepare (ImgBase **ppoDst, depth eDepth, const Size &imgSize, 
                  format eFormat, int nChannels, const Rect& roi, 
                  Time timestamp=Time::null){
      return m_oROIHandler.prepare(ppoDst, eDepth,imgSize,eFormat, nChannels, roi, timestamp);
    }
    
    /// check+adapt destination image to properties of given source image
    virtual bool prepare (ImgBase **ppoDst, const ImgBase *poSrc) {
      return m_oROIHandler.prepare(ppoDst, poSrc);
    }
    
    /// check+adapt destination image to properties of given source image
    /// but use explicitly given depth
    virtual bool prepare (ImgBase **ppoDst, 
                          const ImgBase *poSrc, 
                          depth eDepth) {
      return m_oROIHandler.prepare(ppoDst, poSrc, eDepth);
    }
    
    static inline bool check(const ImgBase *operand1,
                             const ImgBase *operand2 , 
                             bool checkDepths = true) {
      if(!checkDepths) {
        return operand1->getChannels() == operand2->getChannels() &&
          operand1->getROISize() == operand2->getROISize() ;
      } else {
        return operand1->getChannels() == operand2->getChannels() &&
          operand1->getROISize() == operand2->getROISize() &&
          operand1->getDepth() == operand2->getDepth() ;
      }            
    }
    
  private:
    OpROIHandler m_oROIHandler;

    /// internal image buffer which is used for the apply function without destination image argument
    ImgBase *m_buf;
  };
}

#endif
