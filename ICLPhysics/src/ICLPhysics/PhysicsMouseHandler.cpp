/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/PhysicsMouseHandler.cpp      **
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
#include <ICLGeom/Scene.h>
#include <ICLPhysics/PhysicsMouseHandler.h>
#include <ICLPhysics/RigidObject.h>
#include <ICLPhysics/PhysicsWorld.h>
#include <ICLPhysics/Object2PointConstraint.h>
#include <ICLPhysics/PhysicsScene.h>


#include <iostream>
using namespace std;
namespace icl {
  namespace physics{

    PhysicsMouseHandler::PhysicsMouseHandler(const int pCameraIndex, geom::Scene *pParentScene, PhysicsWorld *pPhysicsWorld):
    SceneMouseHandler(pCameraIndex, pParentScene),
    m_parentScene(pParentScene),
    m_physicsWorld(pPhysicsWorld),
    m_selectedObject(0),
    m_constraint(0),
    m_camIndex(0){}

    PhysicsMouseHandler::PhysicsMouseHandler(const int pCameraIndex, PhysicsScene *pPhysicsScene):
    SceneMouseHandler(pCameraIndex, pPhysicsScene),
    m_parentScene(pPhysicsScene),
    m_physicsWorld(pPhysicsScene),
    m_selectedObject(0),
    m_constraint(0),
    m_camIndex(pCameraIndex){}

    void PhysicsMouseHandler::process(const qt::MouseEvent &pMouseEvent){
      const geom::Camera &cam = m_parentScene->getCamera(m_camIndex);
      //check if shift and left mouse is pressed for "grab-mode"
      if((pMouseEvent.isLeft() && pMouseEvent.isModifierActive(qt::ShiftModifier)) || m_selectedObject){
        if(pMouseEvent.isPressEvent()){
          //select the object to be moved
          geom::ViewRay ray = cam.getViewRay(pMouseEvent.getPos());
          geom::Vec normal;
          PhysicsObject* obj;

          if(m_physicsWorld->rayCast(ray,10000.f,obj,normal,hitpoint))
          {
            m_selectedObject = dynamic_cast<RigidObject*>(obj);
            if(m_selectedObject) {
              if(m_selectedObject->hasContactResponse()) {
                geom::Mat trans = m_selectedObject->getTransformation();
                geom::Mat invTrans = trans.inv();
                relPos = invTrans * hitpoint;
                removeConstraint();
                m_constraint = new Object2PointConstraint(m_selectedObject, relPos, hitpoint, 500.f, 0.002f);
                m_physicsWorld->addConstraint(m_constraint);
              } else {
                m_selectedObject = 0;
              }
            }
          }
        //deselect the current obejct
        }else if(pMouseEvent.isReleaseEvent()){
          if(m_selectedObject){
            m_selectedObject = 0;
            removeConstraint();
          }
        //update pointerposition and the constraint
        }else if(m_selectedObject){
          pointer3d = cam.estimate3DPosition(pMouseEvent.getPos(), geom::PlaneEquation(hitpoint,cam.getNorm()));
          m_constraint->setPoint(pointer3d);
          m_selectedObject->activate();
        }

      //otherwise stick to standard behaviour
      }else{
        SceneMouseHandler::process(pMouseEvent);
      }
    }

    PhysicsMouseHandler::~PhysicsMouseHandler(){
      removeConstraint();
    }

    void PhysicsMouseHandler::removeConstraint(){
      if(m_constraint){
        m_physicsWorld->removeConstraint(m_constraint);
        delete m_constraint;
        m_constraint = 0;
      }
    }
  }
}
