/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/SoftObject.h                 **
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

#include <ICLPhysics/PhysicsObject.h>
#include <ICLUtils/Configurable.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletSoftBody/btSoftBody.h>

namespace icl{
namespace physics{
  /// This class combines a soft-body physics object and it's graphical representation in ICL
  /** Actually, this sub-class is just an explicit restriction of the general PhysicalObject
      class, that can represent:
      - collision objects
      - rigid object
      - soft body objects
  */
  class ICLPhysics_API SoftObject : public PhysicsObject, public utils::Configurable{
    /// internal property wrapped from bullet
    bool m_usePoseMatching;

    /// internal property wrapped from bullet
    bool m_useVolumeConversion;

    protected:

    /// protected constructor ..
    SoftObject();

    /// this MUST be called by the real implementation class AFTER the real softbody instance is created
    void createAllProperties();

    public:
    /// constructor that uses an obj-file to create a softbody
    SoftObject(const std::string &objFileName, PhysicsWorld *world);
    /// returns internal physical object as softBody
    /** Soft bodys are simple soft-body physical objects */
    virtual btSoftBody *getSoftBody();

    /// returns internal physical object as softBody (const)
    /** Soft bodys are simple soft-body physical objects */
    virtual const btSoftBody *getSoftBody() const;

    /// this is now linked via register callback
    void propertyChanged(const Configurable::Property &prop);

  };
}
}
