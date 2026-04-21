#pragma once

#include <icl/physics/RigidCylinderObject.h>
#include <icl/physics/RigidSphereObject.h>
#include <icl/geom/Material.h>

namespace icl::physics {
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
          setMaterial(geom::Material::fromColor(geom::geom_green()));
          setCollisionCallback([this](PhysicsObject* self, PhysicsObject* other, geom::Vec pos){ onHit(self, other, pos); });
          setContactResponse(false);
          PhysicsObject::setContactResponse(false);
        }
      void updateSceneObject() {
        RigidCylinderObject::updateSceneObject();
        if(timer > 0) {
            setMaterial(geom::Material::fromColor(geom::geom_yellow()));
            timer--;
        } else {
            setMaterial(geom::Material::fromColor(geom::geom_green()));
        }
      }
    };
  }