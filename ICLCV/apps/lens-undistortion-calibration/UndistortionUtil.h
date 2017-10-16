/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/UndistortionUtil.h           **
** Module : ICLMarkers                                             **
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

#include <ICLUtils/Point32f.h>
#include <ICLUtils/FixedArray.h>
#include <ICLUtils/Size.h>
#include <ICLCore/Img.h>
#include <ICLFilter/WarpOp.h>

namespace icl{


  class UndistortionUtil : public utils::Configurable {
    utils::Size imageSize;
    float k[9]; // k0...k4 and ix, iy and fixed: fx, fy
    core::Img32f warpMapBuffer;
    filter::WarpOp warpOp;
    bool inInit;
    bool warpMapDirty;
    core::Img8u outBuf;
    void propertyCallback(const utils::Configurable::Property &p);
    bool inverted;

    public:

    UndistortionUtil(const utils::Size &imageSize,
                     bool inverted=false);

    const core::Img32f &getWarpMap() const {
      return warpMapBuffer;
    }

    void save();

    void init(const utils::Size &imageSize,
              const std::vector<float> &k,
              const utils::Point32f &coffset=utils::Point32f::null,
              bool updateMap=false);

    void setParamVector(const float k[7]);

    void updateWarpMap();

     static utils::Point32f undistort_point(const utils::Point32f &pd,
                                            const float k[9]){
       const float fx = k[7], fy = k[8];
       const float rx = (pd.x - k[5])/fx, ry = (pd.y - k[6])/fy;
       const float r2 = rx*rx + ry*ry;
       const float cr = 1.0f + k[0]*r2 + k[1]*r2*r2 + k[4]*r2*r2*r2;

       const float dx = 2*k[2]*rx*ry + k[3] *(r2 + 2*rx*rx);
       const float dy = 2*k[3]*rx*ry + k[2] *(r2 + 2*ry*ry);

       const float rxu = rx * cr + dx;
       const float ryu = ry * cr + dy;

       return utils::Point32f(rxu*fx + k[5],
                              ryu*fy + k[6]);
     }

    static utils::Point32f undistort_point_inverse(const utils::Point32f &pd,
                                                   const float k[7]);


    inline utils::Point32f undistort_inverse(const utils::Point32f &pd){
      return undistort_point_inverse(pd, this->k);
    }

    inline utils::Point32f undistort(const utils::Point32f &pd){
      return undistort_point(pd, this->k);
    }

    inline utils::Point32f operator()(const utils::Point32f &pd){
      return undistort(pd);
    }

    const core::Img8u &undistort(const core::Img8u &src);


  };
}
