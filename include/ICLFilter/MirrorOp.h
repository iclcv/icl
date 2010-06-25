/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/MirrorOp.h                           **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Andre Justus                      **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

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
