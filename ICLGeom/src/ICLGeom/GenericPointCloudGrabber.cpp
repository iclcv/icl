/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/GenericPointCloudGrabber.cp        **
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

#include <ICLGeom/GenericPointCloudGrabber.h>

#include <ICLUtils/PluginRegister.h>

namespace icl{
  using namespace utils;

  namespace geom{
 
    struct GenericPointCloudGrabber::Data{
      PointCloudGrabber *impl;
      
    };
    
    
    GenericPointCloudGrabber::GenericPointCloudGrabber():m_data(new Data)
    {
      m_data->impl = 0;
    }

    GenericPointCloudGrabber::GenericPointCloudGrabber(const std::string &sourceType, 
                                                       const std::string &srcDescription):m_data(new Data){
      m_data->impl = 0;
      init(sourceType,srcDescription);
    }

    GenericPointCloudGrabber::GenericPointCloudGrabber(const ProgArg &pa):m_data(new Data){
      m_data->impl = 0;
      init(pa[0],pa[1]);
    }
    
    GenericPointCloudGrabber::~GenericPointCloudGrabber(){
      ICL_DELETE(m_data->impl);
      delete m_data;
    }

    void GenericPointCloudGrabber::reinit(const std::string &description) throw (utils::ICLException){
      if(isNull()){
        throw ICLException("cannot re-initialize GenericPointCloudGrabber: instance null!");
      }
      m_data->impl->reinit(description);
    }

    Camera GenericPointCloudGrabber::getDepthCamera() const throw (utils::ICLException){
      if(isNull()){
        throw ICLException("GenericPointCloudGrabber::getDepthCamera(): called on null instance");
      }
      return m_data->impl->getDepthCamera();
    }

    Camera GenericPointCloudGrabber::getColorCamera() const throw (utils::ICLException){
      if(isNull()){
        throw ICLException("GenericPointCloudGrabber::getColorCamera(): called on null instance");
      }
      return m_data->impl->getColorCamera();
    }

    void GenericPointCloudGrabber::setCameraWorldFrame(const math::FixedMatrix<float,4,4> &T) throw (utils::ICLException){
      if(isNull()){
        throw ICLException("GenericPointCloudGrabber::setCameraWorldFrame(): called on null instance");
      }
      m_data->impl->setCameraWorldFrame(T);
    }

    
    void GenericPointCloudGrabber::init(const std::string &sourceType, const std::string &srcDescription){
      if(sourceType == "list"){
        std::cout << PluginRegister<PointCloudGrabber>::instance().getRegisteredInstanceDescription() 
                  << std::endl;
        

        throw ICLException("GenericPointCloudGrabber list presented successfully");
      }
      if(m_data->impl){
        removeChildConfigurable(m_data->impl);
        delete m_data->impl;
      }

      std::map<std::string,std::string> data;
      data["creation-string"] = srcDescription;
      m_data->impl = PluginRegister<PointCloudGrabber>::instance().createInstance(sourceType,data);
      if(!m_data->impl){
        throw ICLException("GenericPointCloudGrabber::init::unable to create"
                           " GenericPointGrabber instance of type" + sourceType);
      }
      
      addChildConfigurable(m_data->impl);
    }

    void GenericPointCloudGrabber::init(const utils::ProgArg &pa){
      init(pa[0],pa[1]);
    }

    
    bool GenericPointCloudGrabber::isNull() const{
      return !m_data->impl;
    }
      
    void GenericPointCloudGrabber::grab(PointCloudObjectBase &dst){
      if(isNull()) throw ICLException("GenericPointCloudGrabber::grab: called on a null instance");
      return m_data->impl->grab(dst);
    }

    const core::Img32f *GenericPointCloudGrabber::getDepthImage() const{
      if(isNull()) throw ICLException("GenericPointCloudGrabber::getDepthImage: called on a null instance");
      return m_data->impl->getDepthImage();
    }

    const core::Img8u *GenericPointCloudGrabber::getColorImage() const{
      if(isNull()) throw ICLException("GenericPointCloudGrabber::getColorImage: called on a null instance");
      return m_data->impl->getColorImage();
    }

  }
}

