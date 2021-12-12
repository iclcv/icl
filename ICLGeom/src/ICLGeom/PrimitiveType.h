/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PrimitiveType.h                    **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
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

namespace icl{
  namespace geom{

    enum PrimitiveType{
      vertexPrimitive   = 1<<0, //<! vertex
      linePrimitive     = 1<<1, //<! line primitive (adressing two vertices -> start and end position of the line)
      trianglePrimitive = 1<<2, //<! triange primitive (adressing three vertices)
      quadPrimitive     = 1<<3, //<! quad primitve (adressing four vertices)
      polygonPrimitive  = 1<<4, //<! polygon primitive (adressing at least 3 vertices)
      texturePrimitive  = 1<<5, //<! texture primitive (using 4 vertices like a quad as textured rectangle)
      textPrimitive     = 1<<6, //<! text primitive (internally implmented as texture or as billboard)
      noPrimitive       = 1<<7, //<! internally used type
      PRIMITIVE_TYPE_COUNT = 8  //<! also for internal use only
    };
  } // namespace geom
}
