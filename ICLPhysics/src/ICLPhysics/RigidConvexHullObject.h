/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/RigidConvexHullObject.h      **
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

#include <ICLPhysics/RigidObject.h>

namespace icl{
  namespace physics{
    /// ConvexHullObject witht he features of a RigidObject that can be created from a pointcloud
    class ICLPhysics_API RigidConvexHullObject : public RigidObject{
      public:
      /// Constructor that takes the Position and a subset of vertices specified by the index list, as well as the mass
      RigidConvexHullObject(float x, float y, float z,
                                   const std::vector<int> &indices, const std::vector<geom::Vec> &vertices,
                                   geom::Vec offset = geom::Vec(0,0,0,0),
                                   float mass=1.0);


      /// Constructor that takes the Position and a list of vertices, as well as the mass
      RigidConvexHullObject(float x, float y, float z,
                                   const std::vector<geom::Vec> &vertices,
                                   geom::Vec offset = geom::Vec(0,0,0,0),
                                   float mass=1.0);
    };
  }
}
