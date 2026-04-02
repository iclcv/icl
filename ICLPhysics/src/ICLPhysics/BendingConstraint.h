// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <BulletSoftBody/btSoftBody.h>
#include <ICLUtils/Point.h>
#include <ICLGeom/GeomDefs.h>

#include <string>
#include <map>

namespace icl::physics {
struct ICLPhysics_API BendingConstraint{
  using LinkArray = btSoftBody::tLinkArray;
  using Material = btSoftBody::Material;
  using Link = btSoftBody::Link;

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