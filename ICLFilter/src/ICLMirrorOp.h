#include "ICLBaseAffineOp.h"
#ifndef MIRROR_OP_H
#define MIRROR_OP_H


namespace icl{
  
  /// Class to mirror images vertically or horizontally
  class MirrorOp : public BaseAffineOp {
    public:
    /// Constructor
    MirrorOp (axis eAxis);
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
