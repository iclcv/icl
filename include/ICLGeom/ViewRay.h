/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLGeom module of ICL                         **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_VIEW_RAY_H
#define ICL_VIEW_RAY_H

#include <ICLGeom/GeomDefs.h>
#include <ICLUtils/Exception.h>
#include <iostream>

namespace icl{
  /** \cond */
  class PlaneEquation;
  /** \endcond */

  /// This is a view-ray's line equation in parameter form
  /** A view-ray is described by the equation
      \f[ V: \mbox{offset} + \lambda \cdot \mbox{direction} \f]
  */
  struct ViewRay{
    /// Constructor with given offset and direction vector
    explicit ViewRay(const Vec &offset=Vec(), const Vec &direction=Vec());
    
    /// line offset
    Vec offset;
    
    /// line direction
    Vec direction;
    
    /// calculates line-plane intersection 
    /** @see static Camera::getIntersection function */
    Vec getIntersection(const PlaneEquation &plane) const throw (ICLException);
    
    /// evaluates ViewRay' line equation at given lambda
    Vec operator()(float lambda) const;
  };

  /// ostream operator
  std::ostream &operator<<(std::ostream &s, const ViewRay &vr);
}

#endif
