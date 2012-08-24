/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : include/ICLGeom/SuperQuadric.h                         **
 ** Module : ICLGeom                                                **
 ** Authors: Christian Groszewski                                   **
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
#pragma once

#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/DynMatrix.h>

#include <math.h>
#include <cmath>

namespace icl{

  
  /// Class representing a superquadric. 
  /** All paramter can be set and optimized independent
      if necessary.
      Main paper: Recovery of Parametric Models from Range Images: The Case for Superquadrics with Global Deformations.      */
  class SuperQuadric{

    private:

    ///lower limit for e1 and e2
    static const double E_MIN;
    ///upper limit for e1 and e2
    static const double E_MAX;
    ///lower limit for a1,a2,a3
    static const double A_MIN;
    ///upper limit for a1,a2,a3
    static const double A_MAX;

    class Data;
    Data *m_data;

    ///returns clipped value
    /**
        * @param v value to clip
        * @param vmix lower border
        * @param vmax upper border
        * @return clipped value
        */
    double clip_val(double v, double vmin, double vmax);

    ///clips current values of superquaric, in particular e1,e2,a1,a2,a3
    void clip();

    ///return transformationmatrix for given center and rotation
    /**
        * @param x value for center for x-dim
        * @param y value for center for y-dim
        * @param z value for center for z-dim
        * @param rx rotation of x-axis
        * @param ry rotation of y-axis
        * @param rz rotation of z-axis
        * @return transformationmatrix
        */
    DynMatrix<icl64f> screate_hom_transformation(double x, double y, double z, double rx, double ry, double rz);

    ///returns transformed point
    /**
        * @param x value for x-dim
        * @param y value for y-dim
        * @param z value for z-dim
        * @param T transformationmatrix
        * @return transformed point
        */
    DynMatrix<icl64f> strans_pts(const double x, const double y, const double z, const DynMatrix<icl64f> &T);

    ///returns Matrix of distances for given points
    /**
        * @param x values for xdim
        * @param y values for ydim
        * @param z values for zdim
        * @return distances for given input points
        */
    DynMatrix<icl64f> distance(const DynMatrix<icl64f> &x, const DynMatrix<icl64f> &y, const DynMatrix<icl64f> &z);

    ///creates and returns the transformationmatrix
    /**
        * @return transformation for current values of superquadric
        */
    DynMatrix<icl64f> create_hom_transformation();

    ///returns distances for given points and params
    /**
        * @param px values for xdim
        * @param py values for ydim
        * @param pz values for zdim
        * @param e1 value to use
        * @param e2 value to use
        * @param a1 value to use
        * @param a2 value to use
        * @param a3 value to use
        * @param T Transformationmatrix to use
        * @return distances as colvector
        */
    DynMatrix<icl64f> distance(const DynMatrix<icl64f> &px, const DynMatrix<icl64f> &py, const DynMatrix<icl64f> &pz,
                               const double e1, const double e2, const double a1, const double a2, const double a3, const DynMatrix<icl64f> &T);

    public:
    ///Constructor, calls updateAll, values are clipped clip is true
    /**
        * @param params values for x,y,z,e1,e2,a1,a2,a3,rx,ry,rz
        * @param clip values are clipped if true
        */
    SuperQuadric(const DynMatrix<icl64f> &params, bool clip = true);

    ///Constructorwithout clipping
    /**
        * @param x
        * @param y
        * @param z
        * @param e1
        * @param e2
        * @param a1
        * @param a2
        * @param a3
        * @param rx
        * @param ry
        * @param rz
        */
    SuperQuadric(double x=0.0, double y=0.0, double z=0.0, double e1=0.8, double e2=1.0,
                 double a1=2.0, double a2=1.0, double a3= 1.0,
                 double rx=0.0, double ry=0.0, double rz=0.0);

    ///Destructor
    ~SuperQuadric();

    ///updates all params x,y,z,e1,e2,a1,a2,a3,rx,ry,rz, values are clipped if cl = true
    /**
        * @param params row or colvector of new values
        * @param cl if true all values are clipped
        */
    void updateAll(const DynMatrix<icl64f> &params, bool clip=true);

    ///returns array of all current values for x,y,z,e1,e2,a1,a2,a3,rx,ry,rz
    /**
        * @return array with values for center of superquadric, order is x, y, z
        */
    double *getAllParams();

    ///updates all rotation values rx, ry and rz, all values are clipped cl is true
    /**
        * @param params row or colvector of new values for x, y and z
        * @param cl if true all values are clipped
        */
    void updateR(const DynMatrix<icl64f> &params, bool cl=true);

    ///returns array of current values for rx, ry and rz
    /**
        * @return array with values for rx, ry, rz
        */
    double *getR();

    ///updates all values but rotation, order is x,y,z,e1,e2,a1,a2 and a3, values are clipped if cl is true
    /**
        * @param params row or colvector of new values
        * @param cl if true all values are clipped
        */
    void updateAllR(const DynMatrix<icl64f> &params, bool cl=true);

    ///returns array of current values for x, y, z, e1, e2, a1, a2and a3
    /**
        * @return array with all values but rotation
        */
    double *getAllR();

    ///sets values for e1 and e2 and a1, a2, and a3 values are clipped if cl is true
    /**
        * @param params row or colvector of new values for x, y and z
        * @param cl if true all values are clipped
        */
    void updateAE(const DynMatrix<icl64f> &params, bool cl=true);

    ///returns array of current values for e1, e2 and a1, a2 and a3
    /**
        * @return array with values for e1, e2, a1, a2 and a3
        */
    double *getAE();

    ///sets values for e1 and e2, values are clipped if cl is true
    /**
        * @param params row or colvector of new values for e1 and e2
        * @param cl if true all values are clipped
        */
    void updateE(const DynMatrix<icl64f> &params, bool cl=true);

    ///returns array of current values for e1 and e2
    /**
        * @return array with values for e1 and e2
        */
    double *getE();

    ///sets values for a1, a2 and a3, all values are clipped if cl is true
    /**
        * @param params row or colvector of new values for a1, a2 and a3
        * @param cl if true all values are clipped
        */
    void updateA(const DynMatrix<icl64f> &params, bool cl=true);

    ///returns array of current values for a1, a2 and a3 (center of superquadric)
    /**
        * @return array with values for a1, a2, a3
        */
    double *getA();

    ///sets values for x, y and z, all values are clipped if cl is true
    /**
        * @param params row or colvector of new values for x, y and z
        * @param cl if true all values are clipped
        */
    void updateXYZ(const DynMatrix<icl64f> &params, bool cl=true);

    ///returns array of current values for x, y and z (center of superquadric)
    /**
        * @return array with values for center of superquadric, order is x, y, z
        */
    double *getXYZ();

    ///errorfunction evaluates the error for given 3dim points
    /**
        * @param x Matrix for xdim
        * @param y Matrix for ydim
        * @param z Matrix for zdim
        */
    DynMatrix<icl64f> *error(const DynMatrix<icl64f> &x, const DynMatrix<icl64f> &y, const DynMatrix<icl64f> &z);

    ///sets values for x, y and z (the center of the superquadric)
    /**
        * @param x value of x
        * @param y value of y
        * @param z value of z
        */
    void setXYZ(double x, double y, double z);

    ///sets values for a1, a2 and a3, new values are clipped automatically
    /**
        * @param a1 value of a1
        * @param a2 value of a2
        * @param a3 value of a3
        */
    void seta1a2a3(double a1, double a2, double a3);

    ///sets values for e1 and e2, new values are clipped automatically
    /**
        * @param e1 value of e1
        * @param e2 value of e2
        */
    void sete1e2(double e1, double e2);

    ///sets values for rx, ry and rz
    /**
        * @param rx value of rotationangle of x
        * @param ry value of rotationangle of y
        * @param rz value of rotationangle of z
        */
    void setrxryrz(double rx, double ry, double rz);

  };
}

