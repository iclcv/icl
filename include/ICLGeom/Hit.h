/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/Hit.h                                  **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
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

#ifndef ICL_HIT_H
#define ICL_HIT_H

#include <ICLGeom/GeomDefs.h>
#include <iostream>

namespace icl{
  
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



}

#endif
