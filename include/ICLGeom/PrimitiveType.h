/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/PrimitiveType.h                        **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_PRIMITIVE_TYPE_H
#define ICL_PRIMITIVE_TYPE_H

namespace icl{

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
}

#endif
