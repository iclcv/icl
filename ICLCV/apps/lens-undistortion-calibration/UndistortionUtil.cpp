/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/UndistortionUtil.cpp         **
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

#include "UndistortionUtil.h"
#include <ICLMath/QuadTree.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLQt/Quick.h>

namespace icl{
  using namespace utils;
  using namespace core;
  using namespace math;

  UndistortionUtil::UndistortionUtil(const Size &imageSize, bool inverted):
    inInit(false),warpMapDirty(true),inverted(inverted){
    addProperty("interpolation","menu","nearest,linear","linear");
    for(int i=0;i<5;++i){
      if(i < 4){
        addProperty("k"+str(i),"range","[-0.3,0.3]",0);
      }else{
        addProperty("k"+str(i),"range","[-0.1,0.1]",0);
      }
    }
    addProperty("ix-offset","range","[-100,100]", 0);
    addProperty("iy-offset","range","[-100,100]", 0);
    addProperty("reset","command","","");
    
    init(imageSize, std::vector<float>(5,0), Point32f::null, true);

    registerCallback(function(this, &UndistortionUtil::propertyCallback));
  }
    
  void UndistortionUtil::propertyCallback(const utils::Configurable::Property &p){
    if(inInit) return;
    if(p.name == "reset"){
      init(imageSize,std::vector<float>(5,0));
    }
    if(p.name == "interpolation"){
      if(p.value == "linear"){
        warpOp.setScaleMode(interpolateLIN);
      }else{
        warpOp.setScaleMode(interpolateNN);
      }
    }else if(p.name[0] == 'k'){
      k[parse<int>(p.name.substr(1))] = parse<float>(p.value);
      warpMapDirty = true;
    }else if(p.name[0] == 'i'){
      float v = parse<float>(p.value);
      if(p.name[1] == 'x') k[5] = imageSize.width/2.0 + v;
      else if(p.name[1] == 'y') k[6] = imageSize.height/2.0 + v;
      warpMapDirty = true;
    }
  }

  void UndistortionUtil::save(){
    ConfigFile f;
    f.setPrefix("config.");
    
    f["size.width"] = imageSize.width;
    f["size.height"] = imageSize.height;
    f["model"] = str("MatlabModel5Params");
    
    
    f["intrin.fx"] = imageSize.width/2.;
    f["intrin.fy"] = imageSize.height/2.;
    f["intrin.ix"] = double(k[5]);
    f["intrin.iy"] = double(k[6]);
    f["intrin.skew"] = 0.;
    f["udist.k1"] = double(k[0]);
    f["udist.k2"] = double(k[1]);
    f["udist.k3"] = double(k[2]);
    f["udist.k4"] = double(k[3]);
    f["udist.k5"] = double(k[4]);
    
    try{
      std::string fn = saveFileDialog();
      if(fn.length()){
        f.save(fn);
      }
    }catch(...){};
  }
  void UndistortionUtil::setParamVector(const float k[7]){
    std::copy(k,k+7, this->k);
    for(int i=0;i<5;++i){
      setPropertyValue("k"+str(i), k[i]);
    }
    setPropertyValue("ix-offset",k[5]-imageSize.width/2.0f);
    setPropertyValue("iy-offset",k[6]-imageSize.height/2.0f);
  }

  void UndistortionUtil::init(const utils::Size &imageSize,
                   const std::vector<float> &k,
                   const utils::Point32f &coffset,
                   bool updateMap){
    this->k[7] = imageSize.width*0.5;
    this->k[8] = imageSize.height*0.5;

    inInit = true;
    this->imageSize = imageSize;
    if(k.size() != 5) throw utils::ICLException("invalid parameter size");
    std::copy(k.begin(), k.end(), this->k);
    this->k[5] = imageSize.width/2.0 + coffset.x;
    this->k[6] = imageSize.height/2.0 + coffset.y;
    
    if(updateMap){
      updateWarpMap();
    }
    for(int i=0;i<5;++i){
      setPropertyValue("k"+str(i), k[i]);
    }
    setPropertyValue("ix-offset",coffset.x);
    setPropertyValue("ix-offset",coffset.y);
    
    inInit = false;
    warpMapDirty = true;
  }
  /*
  static void nadaraya_watson_esimate(const std::vector<FixedColVector<float,4> > &ps,
                                      float x, float y, float &dx, float &dy){
    float nx = 0, ny = 0, denom = 0;
    for(size_t i=0;i<ps.size();++i){
      const FixedColVector<float,4> &p = ps[i];
      const float xi=p[0], yi=p[1], vxi = p[2], vyi=p[3];
      const float d = exp( - ::sqrt( sqr(x-xi) + sqr(y-yi) ) );
      nx += d*vxi;
      ny += d*vyi;
      denom += d;
    }
    if(denom){
      dx = nx/denom;
      dy = ny/denom;
    }else{
      dx = dy = -1;
    }
  }
      */
  static inline Point32f abs_diff(const Point32f &a, const Point32f &b){
    return Point32f(fabs(a.x-b.x), fabs(a.y-b.y));
  }
  
  /*  static bool is_alternating(const Point32f &a, const Point32f &b){
    return fabs(a.x+b.x) < 0.00001 && fabs(a.y+b.y) < 0.00001;
  }
      */

  
  utils::Point32f UndistortionUtil::undistort_point_inverse(const utils::Point32f &s,
                                                 const float k[7]){
    Point32f p = s;
    Point32f fx;
    const float lambda = 0.5;
    const float h = 0.01;
    const float use_lambda = lambda/h;
    const float t = 0.05; // if we are that close to the target pixel, we stop
                          // note that our step-width has to be linked to this
    int nn = 0;
    do{
      fx = abs_diff(undistort_point(p,k), s);
      Point32f fxh = abs_diff(undistort_point(p+Point32f(h,h),k), s);
      Point32f grad = (fxh - fx) * use_lambda;
      grad.x *= fx.x;
      grad.y *= fx.y;
      p -= grad;
      if(++nn > 100) break;
    }while(fx.x > t || fx.y > t);

    return p;
  }

#if 0
  utils::Point32f UndistortionUtil::undistort_point_inverse(const utils::Point32f &s,
                                                 const float k[7]){
    Point32f p = s;
    Point32f fx;
    const float lambda = 0.5;
    int nn = 0;
    do{
      fx = abs_diff(undistort_point(p,k), s);
      Point32f fxh = abs_diff(undistort_point(p+Point32f(1,1),k), s);
      Point32f grad = (fxh - fx) * lambda;
      grad.x *= fx.x;
      grad.y *= fx.y;
      p -= grad;
      if(++nn > 100) break;
    }while(fx.x > 1 || fx.y > 1);

    return p;
  }
#endif
  
  void UndistortionUtil::updateWarpMap(){
    warpMapBuffer.setChannels(2);
    warpMapBuffer.setSize(imageSize);
    warpMapBuffer.fill(-1);
    Channel32f cs[2] = { warpMapBuffer[0], warpMapBuffer[1] };
    int idx = 0;
    unsigned int w  = imageSize.width, h = imageSize.height;

#if 1
    if(inverted){
      for(unsigned int yi=0;yi<h; ++yi){
        for(unsigned int xi=0;xi<w; ++xi, ++idx){
          Point32f p = undistort_inverse(Point32f(xi,yi));
          cs[0](xi,yi) = p.x;
          cs[1](xi,yi) = p.y;
        }
      }
    }else{
      for(unsigned int yi=0;yi<h; ++yi){
        for(unsigned int xi=0;xi<w; ++xi, ++idx){
          Point32f p = undistort(Point32f(xi,yi));
          cs[0](xi,yi) = p.x;
          cs[1](xi,yi) = p.y;
        }
      }
    }
#endif
    
#if 0
    static const float T = 1;//0.2;
    static const float D = 1;
    static const Point32f H(D,D);
    static const float lambda = 16; //0.5;
    static const float n = lambda/D;

    std::vector<int> nns;
    for(unsigned int yi=0;yi<h; ++yi){
      for(unsigned int xi=0;xi<w; ++xi, ++idx){
        // find x y so that undistort(x,y) = xi, yi
        Point32f s(xi,yi);
        Point32f p = s;
        int nn = 0;
        Point32f fx(10,10);
        float lambda = 0.5;
        Point32f lastGrad;
        while(fx.x > 1 || fx.y > 1){
          fx = abs_diff(undistort(p), s);
          Point32f fxh = abs_diff(undistort(p+H), s);
          Point32f grad = (fxh - fx)*lambda;
          grad.x *= fx.x;
          grad.y *= fx.y;

          if(is_alternating(grad,lastGrad)){
            lambda *= 0.8;
          }

             lastGrad = grad;
          p -= grad;
          ++nn;
          if(nn > 1000){
            SHOW(grad);
            DEBUG_LOG("could not find minimum after 1000 steps");
          }
        }
        nns.push_back(nn);
        //cs[0][idx] = p.x;
        //cs[1][idx] = p.y; 
        if((unsigned)p.x < w && (unsigned)p.y < h){
          cs[0](xi,yi) = p.x;
          cs[1](xi,yi) = p.y;
        }
      }
    }
    std::sort(nns.begin(), nns.end());
    std::cout << "all:";
    for(size_t i=0;i<nns.size();i+=100){
      std::cout << nns[i] << "  ";
    }
    std::cout << std::endl;
#endif

#if 0
    // idea: implement this stuff using opencl!
    // avoid quadtree by simply using a windowed lookup on the GPU ...
    typedef FixedColVector<float,4> V4;
    typedef QuadTree<float,4,1,4096,V4> QTree;
    QTree tree(-100,-100, w+200, h+200);
    
    DEBUG_LOG("creating quadtree");
    for(unsigned int yi=0;yi<h; ++yi){
      for(unsigned int xi=0;xi<w; ++xi, ++idx){
        Point32f p = undistort(Point32f(xi,yi));
        tree.insert(V4(p.x,p.y,xi,yi));
      }
    }
    DEBUG_LOG("performing nadaraya-watson-based estimation");
    static const int r = 3;
    for(unsigned int yi=0;yi<h; ++yi){
      for(unsigned int xi=0;xi<w; ++xi, ++idx){
        std::vector<V4> ps = tree.query(xi-r,yi-r,2*r,2*r);
        nadaraya_watson_esimate(ps, xi, yi, cs[0](xi,yi), cs[1](xi,yi));
      }
    }
#endif

#if 0
    typedef FixedColVector<float,4> V4;
    typedef QuadTree<float,4,1,4096,V4> QTree;
    QTree tree(-100,-100, w+200, h+200);
    
    DEBUG_LOG("creating quadtree");
    for(unsigned int yi=0;yi<h; ++yi){
      for(unsigned int xi=0;xi<w; ++xi, ++idx){
        Point32f p = undistort(Point32f(xi,yi));
        tree.insert(V4(p.x,p.y,xi,yi));
      }
    }
    DEBUG_LOG("performing nadaraya-watson-based estimation");
    static const int r = 3;
    for(unsigned int yi=0;yi<h; ++yi){
      for(unsigned int xi=0;xi<w; ++xi, ++idx){
        std::vector<V4> ps = tree.query(xi-r,yi-r,2*r,2*r);
        nadaraya_watson_esimate(ps, xi, yi, cs[0](xi,yi), cs[1](xi,yi));
      }
    }
#endif
#if 0
    for(unsigned int yi=0;yi<h; ++yi){
      for(unsigned int xi=0;xi<w; ++xi, ++idx){
        Point32f p = undistort(Point32f(xi,yi));
        //cs[0][idx] = p.x;
        //cs[1][idx] = p.y; 
        if((unsigned)p.x < w && (unsigned)p.y < h){
          cs[0](p.x,p.y) = xi;
          cs[1](p.x,p.y) = yi;
        }
      }
    }
#endif

    warpOp.setWarpMap(warpMapBuffer);
    warpMapDirty = false;
  }
  
  const Img8u &UndistortionUtil::undistort(const Img8u &src){
    if(warpMapDirty){
      updateWarpMap();
    }
    outBuf.fill(0);
    warpOp.apply(&src,bpp(outBuf));
    return outBuf;
  }

}
