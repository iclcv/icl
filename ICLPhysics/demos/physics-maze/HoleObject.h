#pragma once

#include <ICLPhysics/RigidCylinderObject.h>
#include <ICLPhysics/RigidSphereObject.h>

namespace icl{
  namespace physics{

    class HoleObject : public RigidCylinderObject{
      public:
      void onHit(PhysicsObject* self, PhysicsObject* other, geom::Vec pos) {
        if(dynamic_cast<RigidSphereObject*>(other))timer = 5;
      }
      //timer that controls how long the hole flashes yellow
      int timer;

      HoleObject(float x, float y, float z):
        RigidCylinderObject(x, y, z, 1, 3,1) {
          timer = 0;
          setVisible(geom::Primitive::all,false);
          addCylinder(0, 0, 0, 11, 11, 1, 16);
          setColor(geom::Primitive::all, geom::geom_green());
          setCollisionCallback(utils::function(this, &HoleObject::onHit));
          setContactResponse(false);
          PhysicsObject::setContactResponse(false);
        }
      void updateSceneObject() {
        RigidCylinderObject::updateSceneObject();
        if(timer > 0) {
            setColor(geom::Primitive::all, geom::geom_yellow());
            timer--;
        } else {
            setColor(geom::Primitive::all, geom::geom_green());
        }
      }
    };
  }
}
