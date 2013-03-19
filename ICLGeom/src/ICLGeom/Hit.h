/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/Hit.h                              **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
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

#include <ICLGeom/GeomDefs.h>
#include <iostream>

namespace icl{
  namespace geom{
    
    /** \cond */
    class SceneObject;
    /** \endcond */
    
    /// utility structure that defines a hit between a ViewRay and SceneObjects
    struct Hit{
      /// constructor (initializes obj with 0 and dist with -1)
      Hit():obj(0),dist(-1){}
      
      /// hit SceneObject
      SceneObject *obj; 
  
      /// exact position in the world where it was hit
      Vec pos;   
  
      /// distance to the originating viewrays origin       
      float dist;
  
      /// for sorting by closest distance ot viewray origin
      inline bool operator<(const Hit &h) const { return dist < h.dist; }
  
      /// can be used to check wheter there was a hit at all
      operator bool() const { return obj; }
  
      /// friendly implemented ostream operator ...
      friend std::ostream &operator<<(std::ostream &str, const Hit &h){
        return (h ? (str << "Hit(obj=" << (void*) h.obj << ", dist=" << h.dist 
                     << ", pos=" << h.pos.transp() << ")" ) 
                : (str << "Hit(NULL)"));
      }
    };
  
  
  
  } // namespace geom
}

