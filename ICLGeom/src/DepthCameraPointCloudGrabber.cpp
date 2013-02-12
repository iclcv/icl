/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/DepthCameraPointCloudGrabber.cpp           **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Patrick Nobou                     **
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

#include <ICLGeom/DepthCameraPointCloudGrabber.h>
#include <ICLGeom/PointCloudCreator.h>

#include <ICLIO/GenericGrabber.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::io;
using namespace icl::qt;

namespace icl{
  namespace geom{
  
    struct DepthCameraPointCloudGrabber::Data{
      GenericGrabber depthGrabber;
      GenericGrabber colorGrabber;
      PointCloudCreator creator;
      const Img32f *lastDepthImage;
      const Img8u *lastColorImage;
    };
    
    const Camera &DepthCameraPointCloudGrabber::get_default_depth_cam(){
      static const Camera cam; return cam;
    }
    const Camera &DepthCameraPointCloudGrabber::get_null_color_cam(){
      static const Camera cam; return cam;
    }
    
    
    DepthCameraPointCloudGrabber::DepthCameraPointCloudGrabber(const Camera &depthCam,
                                                     const Camera &colorCam,
                                                     const std::string &depthDeviceType,
                                                     const std::string &depthDeviceID,
                                                     const std::string &colorDeviceType,
                                                     const std::string &colorDeviceID):
      m_data(new Data){
      m_data->lastColorImage = 0;
      m_data->lastDepthImage = 0;
      m_data->depthGrabber.init(depthDeviceType,depthDeviceType+"="+depthDeviceID);
      m_data->depthGrabber.useDesired(formatMatrix);
      m_data->depthGrabber.useDesired(depth32f);
      m_data->depthGrabber.useDesired(depthCam.getResolution());
  
      if(&colorCam != &get_null_color_cam()){
        m_data->creator.init(depthCam, colorCam);
        m_data->colorGrabber.init(colorDeviceType,colorDeviceType+"="+colorDeviceID);
        m_data->colorGrabber.useDesired(formatRGB);
        m_data->colorGrabber.useDesired(depth8u);
        m_data->colorGrabber.useDesired(colorCam.getResolution());
      }else{
        m_data->creator.init(depthCam);
      }
    }
    
    DepthCameraPointCloudGrabber::~DepthCameraPointCloudGrabber(){
      delete m_data;
    }
  
    
    void DepthCameraPointCloudGrabber::grab(PointCloudObjectBase &dst){
      dst.lock();
      const Img32f &depthImage = *m_data->depthGrabber.grab()->as32f();
      const Img8u *rgbImage = m_data->colorGrabber.isNull() ? 0 : m_data->colorGrabber.grab()->as8u();
      m_data->creator.create(depthImage, dst, rgbImage);
      m_data->lastDepthImage = &depthImage;
      m_data->lastColorImage = rgbImage;
      dst.unlock();
    }
  
    const Img32f &DepthCameraPointCloudGrabber::getLastDepthImage() const{
      if(!m_data->lastDepthImage){
        throw ICLException("DepthCameraPointCloudGrabber::getLastColorImage(): internal depht image was null "
                           " you must call grab(dst) first)");
      }
      return *m_data->lastDepthImage;
    }
  
    const Img8u &DepthCameraPointCloudGrabber::getLastColorImage() const throw (ICLException){
      if(!m_data->lastColorImage){
        throw ICLException("DepthCameraPointCloudGrabber::getLastColorImage(): internal color image was null (either"
                           " no color grabber is availalble, or grab(dst) was not called before)");
      }
      return *m_data->lastColorImage;
    }

    void DepthCameraPointCloudGrabber::mapImage(const core::ImgBase *src, core::ImgBase **dst, const core::Img32f *depthImageMM){
      m_data->creator.mapImage(src,dst,depthImageMM);
    }
    
    void DepthCameraPointCloudGrabber::setUseCL(bool enable){
      m_data->creator.setUseCL(enable);
    }
  
  } // namespace geom
}
