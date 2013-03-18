/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/FiducialImpl.cpp             **
** Module : ICLMarkers                                             **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#include <ICLMarkers/FiducialImpl.h>

namespace icl{
  namespace markers{
    
    FiducialImpl::FiducialImpl(const FiducialImpl &o):
      parent(o.parent),supported(o.supported),computed(o.computed),
      id(o.id),index(o.index){
      
      info2D = o.info2D ? new Info2D(*o.info2D) : 0;
      info3D = o.info3D ? new Info3D(*o.info3D) : 0;
    }
    
    FiducialImpl &FiducialImpl::operator=(const FiducialImpl &o){
      parent = o.parent;
      supported = o.supported;
      computed = o.computed;
      id = o.id;
      index = o.index;
      if(o.info2D){
        if(!info2D) info2D = new Info2D(*o.info2D);
        else *info2D = *o.info2D;
      }else{
        ICL_DELETE(info2D);
      }
      if(o.info3D){
        if(!info3D) info3D = new Info3D(*o.info3D);
        else *info3D = *o.info3D;
      }else{
        ICL_DELETE(info3D);
      }
      return *this;
    }
  
  } // namespace markers
}
