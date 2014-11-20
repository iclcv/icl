/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/PhysicsDefs.h                **
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

#include <ICLGeom/GeomDefs.h>
#include <LinearMath/btTransform.h>

namespace icl{
  namespace physics{
	ICLPhysics_API extern float ICL_UNIT_TO_METER;
	ICLPhysics_API extern float METER_TO_BULLET_UNIT;
    #define ICL_UNIT_TO_BULLET_UNIT (ICL_UNIT_TO_METER / METER_TO_BULLET_UNIT)

    /// Converts from icl to bullet units
    inline float icl2bullet(float x) { return ICL_UNIT_TO_BULLET_UNIT * x; }
    
    /// Converts from bullet to icl units
    inline float bullet2icl(float x) { return x / ICL_UNIT_TO_BULLET_UNIT; }
    
    /// Creates an icl Mat from a bullet transform and scales accordingly.
    inline geom::Mat bullet2icl(const btTransform &T)
    {
      const btMatrix3x3 &R = T.getBasis();
      const btVector3 &t = T.getOrigin();
      return geom::Mat(R[0][0],R[0][1],R[0][2],bullet2icl(t[0]),
                 R[1][0],R[1][1],R[1][2],bullet2icl(t[1]),
                 R[2][0],R[2][1],R[2][2],bullet2icl(t[2]),
                 0,0,0,1);
    }
    

    inline btTransform icl2bullet_unscaled_mat(const geom::Mat &M)
    {
      btTransform T;
      T.setBasis(btMatrix3x3(M[0],M[1],M[2],
                             M[4],M[5],M[6],
                             M[8],M[9],M[10]));
      T.setOrigin(btVector3(M[3],M[7],M[11]));
      return T;
    }

    /// Creates a bullet transform from an icl Mat and scales accordingly.
    inline btTransform icl2bullet_scaled_mat(const geom::Mat &M)
    {
      btTransform T;
      T.setBasis(btMatrix3x3(M[0],M[1],M[2],
                             M[4],M[5],M[6],
                             M[8],M[9],M[10]));
      T.setOrigin(btVector3(icl2bullet(M[3]),
                            icl2bullet(M[7]),
                            icl2bullet(M[11])));
      return T;
    }
    
    /// Creates a bullet vector from an icl vector WITHOUT scaling.
    inline btVector3 icl2bullet_unscaled(const geom::Vec &v) { return btVector3(v[0],v[1],v[2]); }
    
    /// Creates an icl vector from a bullet vector WITHOUT scaling.
    inline geom::Vec bullet2icl_unscaled(const btVector3 &v) { return geom::Vec(v[0],v[1],v[2],1.0); }
    
    /// Creates a bullet vector from an icl vector WITH scaling.
    inline btVector3 icl2bullet_scaled(const geom::Vec &v) { return btVector3(icl2bullet(v[0]),icl2bullet(v[1]),icl2bullet(v[2])); }
    
    /// Creates an icl vector from a bullet vector WITH scaling.
    inline geom::Vec bullet2icl_scaled(const btVector3 &v) { return geom::Vec(bullet2icl(v[0]),bullet2icl(v[1]),bullet2icl(v[2]),1.0); }
  }
}
