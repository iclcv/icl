/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLGeom/PlaneEquation.h                        **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#ifndef ICL_PLANE_EQUATION_H
#define ICL_PLANE_EQUATION_H

#include <ICLGeom/GeomDefs.h>
#include <iostream>

namespace icl{
  
  /// Utility structure for calculation of view-ray / plane intersections
  struct PlaneEquation{

    /// Constructor with given offset and direction vector
    explicit PlaneEquation(const Vec &offset=Vec(), const Vec &normal=Vec());
    
      /// line offset
    Vec offset;
    
    /// line direction
    Vec normal;
  };

  /// ostream operator
  std::ostream &operator<<(std::ostream &s, const PlaneEquation &p);
}

#endif
