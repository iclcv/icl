/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLFilter module of ICL                       **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef MIRROR_OP_H
#define MIRROR_OP_H

#include <ICLFilter/BaseAffineOp.h>
#include <ICLUtils/Uncopyable.h>

namespace icl{
  
  /// Class to mirror images vertically or horizontally \ingroup UNARY \ingroup AFFINE
  class MirrorOp : public BaseAffineOp, public Uncopyable {
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
