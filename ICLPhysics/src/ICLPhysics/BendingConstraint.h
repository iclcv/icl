#pragma once

#include <BulletSoftBody/btSoftBody.h>
#include <ICLUtils/Point.h>
#include <ICLGeom/GeomDefs.h>

#include <string>
#include <map>

namespace icl{
namespace physics{
  struct ICLPhysics_API BendingConstraint{
    typedef btSoftBody::tLinkArray LinkArray;
    typedef btSoftBody::Material Material;
    typedef btSoftBody::Link Link;

    Link *link;
    Material *material;
    utils::Point a,b;
   
    BendingConstraint(Link *link, utils::Point a=utils::Point::null, 
                      utils::Point b=utils::Point::null);
    
    void setStiffness(float val);
    
    void updateLinkPointer(std::map<Material*,Link*> &lookup);
    
    float getStiffness() const;
    
    std::pair<geom::Vec,geom::Vec> getLine() const;
  };
}
}

