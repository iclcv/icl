/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/RSBPointCloudGrabber.cpp           **
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

#define BOOST_SIGNALS_NO_DEPRECATION_WARNING
#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/converter/Repository.h>
#include <rsb/converter/ProtocolBufferConverter.h>


#include <ICLGeom/RSBPointCloudGrabber.h>
#include <ICLGeom/PointCloudSerializer.h>
#include <ICLGeom/ProtoBufSerializationDevice.h>

#include <ICLUtils/Thread.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/PluginRegister.h>

using namespace boost;
using namespace rsb;
using namespace rsb::converter;

namespace icl{
  using namespace io;
  using namespace utils;
  using namespace core;

  namespace geom{
  
    struct RSBPointCloudGrabber::Data{
      Mutex mutex;
      bool initialized;
      RSBPointCloud buffer;
      ProtoBufSerializationDevice sdev;
      SmartPtr<Camera> depthCam;
      SmartPtr<Camera> colorCam;
      Data():sdev(&buffer){}
      
      struct Handler : public DataHandler<RSBPointCloud>{
        RSBPointCloudGrabber::Data *data;
        Handler(RSBPointCloudGrabber::Data *data):data(data){}
        virtual void notify(shared_ptr<RSBPointCloud> image){
          Mutex::Locker lock(data->mutex);
          data->buffer.CopyFrom(*image);
        }
      };
      
      shared_ptr<rsb::Handler> handler;
      ListenerPtr listener;
    };
    
    
    RSBPointCloudGrabber::RSBPointCloudGrabber(const std::string &scope, 
                                               const std::string &trasportList,
                                               Camera *depthCam,
                                               Camera *colorCam){
      m_data = new Data;
      m_data->initialized = false;
      m_data->depthCam = depthCam;
      m_data->colorCam = colorCam;
      if(scope.length()) init(scope,trasportList);

    }
    
    RSBPointCloudGrabber::~RSBPointCloudGrabber(){
      delete m_data;
    }
    
    void RSBPointCloudGrabber::init(const std::string &scope, const std::string &transportList){
      if(m_data->initialized){
        m_data->listener->removeHandler(m_data->handler);
      }
      Scope rsbScope(scope);
      Factory &factory = rsb::getFactory();
      ParticipantConfig rsbCfg = factory.getDefaultParticipantConfig();
      typedef std::set<ParticipantConfig::Transport> TSet;
      typedef std::vector<ParticipantConfig::Transport> TVec;
      
      TSet ts2 = rsbCfg.getTransports(true);
      TVec ts(ts2.begin(),ts2.end());
      std::vector<std::string> transports = tok(transportList,",");

      for(TVec::iterator it = ts.begin(); it != ts.end(); ++it){
        ParticipantConfig::Transport &t = *it;
        if( find(transports.begin(), transports.end(), it->getName()) == transports.end() ){
          t.setEnabled(false);
        }else{
          it->setEnabled(true);
        }
      }
      rsbCfg.setTransports(TSet(ts.begin(),ts.end()));
      m_data->listener = factory.createListener(rsbScope,rsbCfg);
      m_data->handler = shared_ptr<Handler>(new Data::Handler(m_data));
      m_data->listener->addHandler(m_data->handler);
    }
      
    void RSBPointCloudGrabber::grab(PointCloudObjectBase &dst){
      Mutex::Locker lock(m_data->mutex);
      while(!m_data->buffer.IsInitialized()){
        m_data->mutex.unlock();
        Thread::msleep(10);
        m_data->mutex.lock();
      }
      PointCloudSerializer::deserialize(dst, m_data->sdev);      
    }

    Camera RSBPointCloudGrabber::getDepthCamera() const throw (utils::ICLException){
      if(!m_data->depthCam){
        throw ICLException("RSBPointCloudGrabber::getDepthCamera(): the depth camera can only "
                           "be returned if explicitly given to the grabber creation string "
                           "e.g. \"app -pci rsb /foo,depth-camfile.xml,color-cam-file.xml\"");
                           
      }
      return *m_data->depthCam;
    }
    
    Camera RSBPointCloudGrabber::getColorCamera() const throw (utils::ICLException){
      if(!m_data->colorCam){
        throw ICLException("RSBPointCloudGrabber::getColorCamera(): the color camera can only "
                           "be returned if explicitly given to the grabber creation string "
                           "e.g. \"app -pci rsb /foo,depth-camfile.xml,color-cam-file.xml\"");
      }

      return *m_data->colorCam;
    }
    
    static std::vector<std::string> extract_cams(std::string &s){
      std::vector<std::string> ts = tok(s,",");
      if(ts.size() == 1) return std::vector<std::string>();
      if(ts.size() == 2) {
        s = ts[0];
        return std::vector<std::string>(1,ts[1]);
      }else if(ts.size() == 3){
        s = ts[0];
        return std::vector<std::string>(ts.begin()+1,ts.end());
      }else{
        throw ICLException("create_rsb_point_cloud_grabber from program argument: invalid syntax!");
      }
    }
    
    static PointCloudGrabber *create_rsb_point_cloud_grabber(const std::map<std::string,std::string> &d){
      std::map<std::string,std::string>::const_iterator it = d.find("creation-string");
      if(it == d.end()) return 0;
      const std::string &params = it->second;
      std::vector<std::string> ts = tok(params,":");
      if(ts.size() ==  1){
        std::vector<std::string> cams = extract_cams(ts[0]);
        if(cams.size() == 1){
          return new RSBPointCloudGrabber(ts[0], "spread", new Camera(cams[0]));
        }else if(cams.size() == 2){
          return new RSBPointCloudGrabber(ts[0], "spread", new Camera(cams[0]), new Camera(cams[1]));
        }else{
          return new RSBPointCloudGrabber(ts[0]);
        }
      }else if(ts.size() == 2){
        std::vector<std::string> cams = extract_cams(ts[0]);
        if(cams.size() == 1){
          return new RSBPointCloudGrabber(ts[1], ts[0], new Camera(cams[0]));
        }else if(cams.size() == 2){
          return new RSBPointCloudGrabber(ts[1], ts[0], new Camera(cams[0]), new Camera(cams[1]));
        }else{
          return new RSBPointCloudGrabber(ts[1], ts[0]);
        }
      }else{
        throw ICLException("unable to create RSBPointCloudGrabber from given parameter string '"+params+"'");
      }
      return 0;
    }
    
    REGISTER_PLUGIN(PointCloudGrabber,rsb,create_rsb_point_cloud_grabber,
                    "RSB based point cloud grabber",
                    "creation-string: [comma-sep-transport-list:]rsb-scope[,depth-cam-file[,color-cam-file]]");
  } // namespace geom
}

