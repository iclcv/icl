#pragma once

#include <ICLUtils/Uncopyable.h>
#include <LinearMath/btMotionState.h>

namespace icl {
  namespace physics{
  
    class RigidObject;
    
    /// This class is a MotionState for updating SceneObjects to match the PhysicsObject state.
    class ICLPhysics_API MotionState : public btMotionState, public utils::Uncopyable{
    
      private:
      /// RigidObject this MotionState belongs to
      RigidObject* scn_obj;
      
      /// Internal transform of the Object
      btTransform trans;
      
      public:
      /// Constructor that takes an initial Position an the corresponding RigidObject
      MotionState(const btTransform &initialpos, RigidObject *obj) {
          scn_obj = obj;
          trans = initialpos;
      }
      virtual ~MotionState() {
      }

      virtual void getWorldTransform(btTransform &worldTrans) const {
          worldTrans = trans;
      }

      virtual void setWorldTransform(const btTransform &worldTrans) {
          scn_obj->stateChanged();
          trans = worldTrans;
      }
    };
  }
}
