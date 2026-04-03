// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

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
