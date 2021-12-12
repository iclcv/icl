/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/BendingConstraint.h          **
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

#include <BulletSoftBody/btSoftBody.h>
#include <ICLUtils/Point.h>
#include <ICLGeom/GeomDefs.h>

#include <string>
#include <map>

namespace icl{
namespace physics{
  struct ICLPhysics_API BendingConstraint{
    typedef btSoftBody::tLinkArray LinkArray;
    typedef btSoftBody::Material Material;
    typedef btSoftBody::Link Link;

    Link *link;
    Material *material;
    utils::Point a,b;

    BendingConstraint(Link *link, utils::Point a=utils::Point::null,
                      utils::Point b=utils::Point::null);

    void setStiffness(float val);

    void updateLinkPointer(std::map<Material*,Link*> &lookup);

    float getStiffness() const;

    std::pair<geom::Vec,geom::Vec> getLine() const;
  };
}
}
