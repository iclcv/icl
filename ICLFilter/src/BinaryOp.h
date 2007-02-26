#ifndef BINARY_OP_H
#define BINARY_OP_H

#include "OpROIHandler.h"
namespace icl{
  /// Abstract base class for operators of type result=f(imageA,imageB)
  /** <b>TODO!!!</b> clip to ROI and prepare logic here !*/
  class BinaryOp{
    public:
    virtual ~BinaryOp(){}
    /// pure virtual apply function
    virtual void apply(const ImgBase *operand1,const ImgBase *operand2, ImgBase **result)=0;

    void setClipToROI (bool bClipToROI) { m_oROIHandler.setClipToROI(bClipToROI); }
    void setCheckOnly (bool bCheckOnly) { m_oROIHandler.setCheckOnly(bCheckOnly); }
    
    bool getClipToROI() const { return m_oROIHandler.getClipToROI(); }
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
    virtual bool prepare (ImgBase **ppoDst, const ImgBase *poSrc, depth eDepth) {
      return m_oROIHandler.prepare(ppoDst, poSrc, eDepth);
    }
    
    static inline bool check(const ImgBase *operand1,const ImgBase *operand2 , bool checkDepths = true){
      if(!checkDepths){
        return operand1->getChannels() == operand2->getChannels() &&
          operand1->getROISize() == operand2->getROISize() ;
      }else{
        return operand1->getChannels() == operand2->getChannels() &&
          operand1->getROISize() == operand2->getROISize() &&
          operand1->getDepth() == operand2->getDepth() ;
      }            
    }
    private:
    OpROIHandler m_oROIHandler;

  };
}

#endif
