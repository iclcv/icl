#pragma once

#include <ICLGeom/SceneMouseHandler.h>


namespace icl {  
  namespace physics{
    class PhysicsWorld;
    class PhysicsScene;
    class RigidObject;
    class Object2PointConstraint;
  
    /// This mousehandler can pick up object and move them around
    class ICLPhysics_API PhysicsMouseHandler:public geom::SceneMouseHandler{
      geom::Scene *m_parentScene;
      PhysicsWorld *m_physicsWorld;
      RigidObject *m_selectedObject;
      geom::Vec hitpoint, relPos, pointer3d;
      Object2PointConstraint *m_constraint;
      int m_camIndex;
      
      public:
      /// Constructor that creates the Camera for the given scene and physicsworld at the given index
      PhysicsMouseHandler(const int pCameraIndex, geom::Scene *pParentScene, PhysicsWorld *pPhysicsWorld);
      
      /// Constructor that creates the Camera for the given PhysicsScene at the given index
      PhysicsMouseHandler(const int pCameraIndex, PhysicsScene *pPhysicsScene);
      
      virtual void process(const qt::MouseEvent &pMouseEvent);
      
      virtual ~PhysicsMouseHandler();
      
      private:
      void removeConstraint();
    };
  }
}
