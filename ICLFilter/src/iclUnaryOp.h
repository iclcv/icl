#ifndef UNARY_OP_H
#define UNARY_OP_H

#include "iclOpROIHandler.h"

namespace icl{
  
  /** \cond */
  class MultiThreader;
  /** \endcond */
  
  /// Abstract Base class for Unary Operators \ingroup UNARY
  /** A list of unary operators can be found here:\n
      \ref UNARY
  **/
  class UnaryOp{
    public:

    /// Explicit empty constructor
    UnaryOp();

    /// Explicit copy constructor
    UnaryOp(const UnaryOp &other);

    /// Explicit declaration of the assignment operator
    UnaryOp &operator=(const UnaryOp &other);

    
    /// Destructor
    virtual ~UnaryOp();
      
    /// pure virtual apply function, that must be implemented in all derived classes
    virtual void apply(const ImgBase *operand1, ImgBase **dst)=0;

    /// *NEW* apply function for multithreaded filtering
    virtual void applyMT(const ImgBase *operand1, ImgBase **dst, unsigned int nThreads);
    
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
    virtual bool prepare (ImgBase **ppoDst, const ImgBase *poSrc, depth eDepth) {
      return m_oROIHandler.prepare(ppoDst, poSrc, eDepth);
    }

    MultiThreader *m_poMT;
    
    private:
  
    OpROIHandler m_oROIHandler;
  };    
}
#endif
