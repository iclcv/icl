/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/BendingConstraint.cpp        **
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
#include <ICLPhysics/BendingConstraint.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Macros.h>
#include <ICLPhysics/PhysicsDefs.h>

#include <iostream>

namespace icl{

  using namespace utils;
  using namespace geom;
namespace physics{

  typedef BendingConstraint::Link Link;
  typedef BendingConstraint::Material Material;

  BendingConstraint::BendingConstraint(Link *link, utils::Point a, utils::Point b):
    link(link),material(link?link->m_material:0),a(a),b(b){
  }

  void BendingConstraint::setStiffness(float val){
    ICLASSERT_THROW(link,utils::ICLException("BendingConstraint::setStiffness called on NULL instance"));
    Link &l = *link;
    Material &m = *material;
    l.m_c0 = (l.m_c0 * m.m_kLST) / val;
    m.m_kLST = val;
  }

  void BendingConstraint::updateLinkPointer(std::map<Material*,Link*> &lookup){
    link = lookup[material];
  }

  std::pair<Vec,Vec> BendingConstraint::getLine() const{
    static const float f = + 0.1;
    return std::make_pair(Vec(bullet2icl(link->m_n[0]->m_x[0] + f * link->m_n[0]->m_n[0]),
                              bullet2icl(link->m_n[0]->m_x[1] + f * link->m_n[0]->m_n[1]),
                              bullet2icl(link->m_n[0]->m_x[2] + f * link->m_n[0]->m_n[2]),1),
                          Vec(bullet2icl(link->m_n[1]->m_x[0] + f * link->m_n[1]->m_n[0]),
                              bullet2icl(link->m_n[1]->m_x[1] + f * link->m_n[1]->m_n[1]),
                              bullet2icl(link->m_n[1]->m_x[2] + f * link->m_n[1]->m_n[2]),1));
  }
  float BendingConstraint::getStiffness() const{
    return material->m_kLST;
  }

}
}
