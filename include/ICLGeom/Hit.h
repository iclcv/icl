#ifndef ICL_HIT_H
#define ICL_HIT_H

#include <ICLGeom/GeomDefs.h>

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
  };
}

#endif
