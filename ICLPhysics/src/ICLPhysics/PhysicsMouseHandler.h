/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/PhysicsMouseHandler.h        **
** Module : ICLPhysics                                             **
** Author : Christof Elbrechter, Matthias Esau                     **
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
