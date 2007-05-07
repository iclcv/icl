#ifndef MIRROR_OP_H
#define MIRROR_OP_H

#include "iclBaseAffineOp.h"

namespace icl{
  
  /// Class to mirror images vertically or horizontally
  class MirrorOp : public BaseAffineOp {
    public:
    /// Constructor
    /**
      @param eAxis the axis on which the mirroring is performed
    */
    MirrorOp (axis eAxis);
    
    /// Destructor
    virtual ~MirrorOp(){}
    
    /// Applies the mirror transform to the images
    void apply (const ImgBase *poSrc, ImgBase **ppoDst);

    private:    
    /// array of class methods used to transform depth8u and depth32f images
    void (MirrorOp::*m_aMethods[depthLast+1])(const ImgBase *poSrc, ImgBase *poDst);
    
    template<typename T>
    void mirror (const ImgBase *poSrc, ImgBase *poDst);
    
    axis  m_eAxis;
    Size  m_oSize;
    Point m_oSrcOffset, m_oDstOffset;
  };
}


#endif
