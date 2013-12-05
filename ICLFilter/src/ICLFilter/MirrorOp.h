/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/MirrorOp.h                     **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Andre Justus                      **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLFilter/BaseAffineOp.h>
#include <ICLUtils/Uncopyable.h>

namespace icl{
  namespace filter{
    
    /// Class to mirror images vertically or horizontally \ingroup UNARY \ingroup AFFINE
    class ICL_FILTER_API MirrorOp : public BaseAffineOp, public utils::Uncopyable {
      public:
      /// Constructor
      /**
        @param eAxis the axis on which the mirroring is performed
      */
      MirrorOp (core::axis eAxis);
      
      /// Destructor
      virtual ~MirrorOp(){}
      
      /// Applies the mirror transform to the images
      void apply (const core::ImgBase *poSrc, core::ImgBase **ppoDst);
  
      private:    
      /// array of class methods used to transform depth8u and depth32f images
      void (MirrorOp::*m_aMethods[core::depthLast+1])(const core::ImgBase *poSrc, core::ImgBase *poDst);
      
      template<typename T>
      void mirror (const core::ImgBase *poSrc, core::ImgBase *poDst);
      
      core::axis  m_eAxis;
      utils::Size  m_oSize;
      utils::Point m_oSrcOffset, m_oDstOffset;
    };
  } // namespace filter
}


