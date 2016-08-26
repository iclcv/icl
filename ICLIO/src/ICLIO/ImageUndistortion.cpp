/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/ImageUndistortion.cpp                  **
** Module : ICLIO                                                  **
** Authors: Christian Groszewski                                   **
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

#include <ICLIO/ImageUndistortion.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLUtils/XML.h>
#include <fstream>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;

namespace icl{
  namespace io{
  
    struct ImageUndistortion::Impl{
      virtual ~Impl(){}
      Img32f warpMap;
      std::string model;
      std::vector<double> params;
      std::vector<double> warpMapParams;

      Size imageSize;
      virtual Point32f undistort(const Point32f &point) const{
        const double &x0 = params[0];
        const double &y0 = params[1];
        const double &f = params[2]/100000000.0;
        const double &s = params[3];
        
        float x = s*(point.x-x0);
        float y = s*(point.y-y0);
        float p = 1 - f * (x*x + y*y);
        float xd = (p*x + x0);
        float yd = (p*y + y0);
        return Point32f(xd,yd);
      }
      virtual size_t getNumParams() const { return 4; }
      virtual void setParams(const std::vector<double> &params){
        if(params.size() != getNumParams()){
          throw ICLException("invalid parameter count for ImageUndistorion model "  + model);
        }
        this->params = params;
      }
    };
    
    struct MatlabModel5ParamsImpl : public ImageUndistortion::Impl{
      FixedMatrix<icl64f,3,3> Kinv;
      void updateKinv(){
        Kinv[0] = params[0]; 
        Kinv[1] = params[0]*params[4]; 
        Kinv[2] = params[2];
        Kinv[3] = 0.0; 
        Kinv[4] = params[1]; 
        Kinv[5] = params[3];
        Kinv[6] = 0.0; 
        Kinv[7] = 0.0; 
        Kinv[8] = 1.0;

        //Kinv = Kinv.inv();
        Kinv = Kinv.pinv(true);
      }
      virtual size_t getNumParams() const { return 10; }
      virtual Point32f undistort(const Point32f &point) const{
        FixedMatrix<icl64f,1,3> p; p[0] = point.x; p[1] = point.y; p[2] = 1.0;
        FixedMatrix<icl64f,1,3> rays = Kinv*p;
        
        FixedMatrix<icl64f,1,2> x;
        x[0] = rays[0]/rays[2];
        x[1] = rays[1]/rays[2];
        
        FixedMatrix<icl64f,1,5> k;
        k[0] = params[5]; k[1] = params[6]; k[2] = params[7]; k[3] = params[8]; k[4] = params[9];
        // Add distortion:
        FixedMatrix<icl64f,1,2> pd;
        
        double r2 = x[0]*x[0]+x[1]*x[1];
        double r4 = r2*r2;
        double r6 = r4*r2;
        
        // Radial distortion:
        double cdist = 1 + k[0] * r2 + k[1] * r4 + k[4] * r6;
        
        pd[0] = x[0] * cdist;
        pd[1] = x[1] * cdist;
        
        // tangential distortion:
        double a1 = 2*x[0]*x[1];
        double a2 = r2 + 2*x[0]*x[0];
        double a3 = r2 + 2*x[1]*x[1];
        
        FixedMatrix<icl64f,1,2> delta_x;
        delta_x[0]= k[2]*a1 + k[3]*a2 ;
        delta_x[1]= k[2] * a3 + k[3]*a1;
        
        pd = pd + delta_x;
        // Reconvert in pixels:
        double px2 = params[0]*(pd[0]+params[4]*pd[1])+params[2];
        double py2 = params[1]*pd[1]+params[3];
        
        return Point32f(px2,py2);
      }
    };
    
    ImageUndistortion::ImageUndistortion():impl(0){};
    
    ImageUndistortion::ImageUndistortion(const std::string &model, const std::vector<double> &params, const Size &imageSize){
      if(model == "SimpleARTBased"){
        impl = new Impl;
        impl->params = params;
      }else if(model == "MatlabModel5Params"){
        MatlabModel5ParamsImpl *impl = new MatlabModel5ParamsImpl;
        this->impl = impl;
        impl->params = params;
        impl->updateKinv();
      }else{
        throw ICLException("ImageUndistortion(...): invalid model name");
      }
      impl->model = model;
      impl->imageSize = imageSize;
      impl->warpMap.detach();
      
    }
  
    ImageUndistortion::ImageUndistortion(const ImageUndistortion &other):impl(0){
      *this=other;
    }
    ImageUndistortion &ImageUndistortion::operator=(const ImageUndistortion &other){
      ICL_DELETE(impl);
      if(other.impl){
        if(other.impl->model == "SimpleARTBased") this->impl = new Impl(*other.impl);
        else this->impl = new MatlabModel5ParamsImpl(*reinterpret_cast<MatlabModel5ParamsImpl*>(other.impl));
      }
      return *this;
    }
  
  
    ImageUndistortion::ImageUndistortion(const std::string &filename):impl(0){
      std::ifstream s(filename.c_str());
      s >> (*this);
    }
    
    const Point32f ImageUndistortion::operator()(const Point32f &p) const{
      ICLASSERT_THROW(!isNull(), ICLException("unable to use function call operator () on a null-ImageUndistortion instance"));
      return impl->undistort(p);
    }
    
    const Size &ImageUndistortion::getImageSize() const{
      ICLASSERT_THROW(!isNull(), ICLException("unable to query the image size from a null-ImageUndistortion instance"));
      return impl->imageSize;
    }
    const std::vector<double> &ImageUndistortion::getParams() const{
      ICLASSERT_THROW(!isNull(), ICLException("unable to query the undistortion parameters from a null-ImageUndistortion instance"));
      return impl->params;
    }

    void ImageUndistortion::setParams(const std::vector<double> &params){
      ICLASSERT_THROW(!isNull(), ICLException("unable to set the undistortion parameters for a null-ImageUndistortion instance"));
      impl->setParams(params);
    }

    const std::string &ImageUndistortion::getModel() const{
      ICLASSERT_THROW(!isNull(), ICLException("unable to query the model from a null-ImageUndistortion instance"));
      return impl->model;
    }
  
    std::istream &operator>>(std::istream &is, ImageUndistortion &dest){
      utils::XMLDocument *doc = new utils::XMLDocument;
      doc->loadNext(is);
      ConfigFile f(doc);
      f.setPrefix("config.");
      std::vector<double> params;
  
  #define CHECK_THROW(KEY)                                                \
      if(!f.contains("" #KEY)){                                           \
        throw ICLException("unable to parse xml-file to "                \
                           "ImageUndistortion: entry " #KEY               \
                           " is missing!");                               \
      }
      CHECK_THROW(model);
      std::string model = f["model"].as<std::string>();
      
      if(model == "MatlabModel5Params"){
  #define LFS(KEY)                                                        \
        CHECK_THROW(KEY)                                                  \
        params.push_back(f["" #KEY]);                             
        
        LFS(intrin.fx);
        LFS(intrin.fy);
        LFS(intrin.ix);
        LFS(intrin.iy);
        LFS(intrin.skew);
        LFS(udist.k1);
        LFS(udist.k2);
        LFS(udist.k3);
        LFS(udist.k4);
        LFS(udist.k5);
      }else if(model == "SimpleARTBased"){
        LFS(center.x);
        LFS(center.y);
        LFS(udist.f);
        LFS(udist.scale);
      }else{
        throw ICLException("unable to parse xml-file to ImageUndistortion: invalide model name");
      }
      
      CHECK_THROW(size.width);
      CHECK_THROW(size.height);
      
      dest = ImageUndistortion(model, params, Size(f["size.width"], f["size.height"]));
  
  #undef LSF
  #undef CHECK_THROW
      return is;
    }
    
    std::ostream &operator<<(std::ostream &s,const ImageUndistortion &udist){
      ICLASSERT_THROW(!udist.isNull(), ICLException("unable to serialize a null-ImageUndistortion instance"));
      ConfigFile f;
      f.setPrefix("config.");
      
      f["size.width"] = udist.getImageSize().width;
      f["size.height"] = udist.getImageSize().height;
      f["model"] = udist.getModel();
      
      const std::vector<double> &p = udist.getParams();
      if(udist.getModel() == "MatlabModel5Params"){
        f["intrin.fx"] = p[0];
        f["intrin.fy"] = p[1];
        f["intrin.ix"] = p[2];
        f["intrin.iy"] = p[3];
        f["intrin.skew"] = p[4];
        f["udist.k1"] = p[5];
        f["udist.k2"] = p[6];
        f["udist.k3"] = p[7];
        f["udist.k4"] = p[8];
        f["udist.k5"] = p[9];
      }else{
        f["center.x"] = p[0];
        f["center.y"] = p[1];
        f["udist.f"] = p[2];
        f["udist.scale"] = p[3];
      }
      return s << f;
    }
  
    const Img32f &ImageUndistortion::createWarpMap() const{
      if(!impl->warpMap.getChannels()){
        impl->warpMap.setChannels(2);
        impl->warpMap.setSize(getImageSize());
      }
      if(impl->params != impl->warpMapParams){
        impl->warpMapParams = impl->params;
        
        Channel32f cs[2] = { impl->warpMap[0], impl->warpMap[1] };
        const Size size = getImageSize();
        for(int xi=0;xi<size.width;++xi){
          for(int yi=0;yi<size.height; ++yi){
            Point32f p = impl->undistort(Point32f(xi,yi));
            cs[0](xi,yi) = p.x;
            cs[1](xi,yi) = p.y; 
          }
        }
      }
      return impl->warpMap;
    }
  } // namespace io
}
