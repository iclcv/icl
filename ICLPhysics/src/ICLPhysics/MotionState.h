/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/MotionState.h                **
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
