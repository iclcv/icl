/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/PhysicsUtils.h               **
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

#include <ICLGeom/Geom.h>

namespace icl{
  namespace geom{
    class Scene;
  }
  namespace physics{

    class PhysicsWorld;
    class PhysicsObject;

    /// removes all rigid object from the given scene and world that are below minZ
    void ICLPhysics_API remove_fallen_objects(geom::Scene *scene, PhysicsWorld *world, float minZ = -2000);

    /// adds num random objects
	void ICLPhysics_API add_clutter(geom::Scene *scene, PhysicsWorld *world, int num = 30);

    /// Calculates the distance between the 2 closest points of the objects a and b
	void ICLPhysics_API calcDistance(PhysicsObject* a, PhysicsObject* b, geom::Vec &pointOnA, geom::Vec &pointOnB, float &distance);

    /// Calculates the distance between the 2 closest points of the objects a and b
	void ICLPhysics_API calcDistance(PhysicsObject* a, PhysicsObject* b, geom::Mat transA, geom::Mat transB, geom::Vec &pointOnA, geom::Vec &pointOnB, float &distance);

    /// Creates primitives from blobs. The last blob is assumed to be the Table the objects rest on.
	void ICLPhysics_API createObjectsFromBlobs(const std::vector< std::vector<int> > &indices, const std::vector<geom::Vec> &vertices, std::vector<PhysicsObject*> &objects);
  }
}
