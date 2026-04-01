// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLCore/Img.h>
#include <ICLFilter/LocalThresholdOp.h>
#include <ICLCore/CCFunctions.h>

#include <ICLMarkers/FiducialDetector.h>
#include <ICLMarkers/FiducialDetectorPlugin.h>
#include <ICLMarkers/FiducialImpl.h>
#include <ICLMarkers/Fiducial.h>
#include <ICLMarkers/FiducialDetectorPluginBCH.h>
#include <ICLMarkers/FiducialDetectorPluginART.h>
#include <ICLMarkers/FiducialDetectorPluginAmoeba.h>
#include <ICLMarkers/FiducialDetectorPluginICL1.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::filter;
using namespace icl::geom;

namespace icl{
  namespace markers{

    struct Preprocessor{
      virtual const Img8u &pp(const ImgBase *src) = 0;
      virtual ~Preprocessor() {}
    };

    struct BinaryPP : public Preprocessor, public Configurable{
      LocalThresholdOp lt;
      BinaryPP(){
        lt.deactivateProperty("^UnaryOp.*");
        addChildConfigurable(&lt);
      }
      virtual const Img8u &pp(const ImgBase *src){
        static ImgBase *buf = nullptr;
        lt.apply(src, &buf);
        return *buf->asImg<icl8u>();
      }
    };

    struct FormatPP : public Preprocessor{
      format dstFmt;
      Img8u buf;
      FormatPP(format dstFmt):buf(Size(1,1),dstFmt){}
      virtual const Img8u &pp(const ImgBase *src){
        if(src->getDepth() == depth8u && src->getFormat() == dstFmt){
          return *src->asImg<icl8u>();
        }else{
          buf.setSize(src->getSize());
          cc(src,&buf);
          return buf;
        }
      }
    };

    struct FiducialDetector::Data{
      std::string plugintype;
      std::shared_ptr<FiducialDetectorPlugin> plugin;
      Img8u buffer;
      std::vector<FiducialImpl*> fidImpls;
      std::vector<Fiducial> fids;
      std::shared_ptr<Camera> camera;
      std::shared_ptr<Preprocessor> pp;

      struct IntermediaImages{
        IntermediaImages():input(0),pp(0){}
        ImgBase *input;
        ImgBase *pp;
      } iis;
    };

    FiducialDetectorPlugin* FiducialDetector::getPlugin(){
    	return data->plugin.get();
    }

    FiducialDetector::FiducialDetector(const std::string &plugin,
                                       const Any &markersToLoad,
                                       const ParamList &params,
                                       const Camera *camera):
      data(new Data){
      data->plugin = 0;
      data->plugintype = plugin;
      if(plugin == "bch"){
        data->plugin.reset(new FiducialDetectorPluginBCH);
      }else if(plugin == "art"){
        data->plugin.reset(new FiducialDetectorPluginART);
      }else if(plugin == "amoeba"){
        data->plugin.reset(new FiducialDetectorPluginAmoeba);
      }else if(plugin == "icl1"){
        data->plugin.reset(new FiducialDetectorPluginICL1);
      }else{
        throw ICLException("FiducialDetector: invalid plugin type: " + plugin);
      }

      if(markersToLoad.length()) loadMarkers(markersToLoad,params);
      if(camera) setCamera(*camera);

      addChildConfigurable(data->plugin.get());

	  switch(data->plugin->getPreProcessing()){
        case FiducialDetectorPlugin::Binary:{
          BinaryPP *p = new BinaryPP;
          data->pp.reset(p);
          addChildConfigurable(p,"thresh");
          break;
         }
        case FiducialDetectorPlugin::Gray:
          data->pp.reset(new FormatPP(formatGray));
          break;
        case FiducialDetectorPlugin::Color:
          data->pp.reset(new FormatPP(formatRGB));
          break;
        default:
          throw ICLException("FiducialDetector: invalid preprocessing type returned by plugin");
      }
    }

    FiducialDetector::~FiducialDetector(){
      delete data;
    }

    void FiducialDetector::setCamera(const Camera &camera){
      data->camera.reset(new Camera(camera));
      data->plugin->camera = data->camera.get();
    }

    const Camera &FiducialDetector::getCamera() const{
      if(!data->camera) throw ICLException("FiducialDetector::getCamera: camera has not been set");
      return *data->camera;
    }

    const std::string &FiducialDetector::getPluginType() const{
      return data->plugintype;
    }

    void  FiducialDetector::loadMarkers(const Any &which, const ParamList &params){
      data->plugin->addOrRemoveMarkers(true,which,params);
    }

    void FiducialDetector::unloadMarkers(const Any &which){
      data->plugin->addOrRemoveMarkers(false,which,ParamList());
    }

    const std::vector<Fiducial> &FiducialDetector::detect(const ImgBase *image){
      ICLASSERT_THROW(image,ICLException("FiducialDetector::detect: image was 0"));

      data->iis.input = const_cast<ImgBase*>(image);
      const Img8u &ppImage = data->pp->pp(image);
      data->iis.pp = const_cast<ImgBase*>(static_cast<const ImgBase*>(&ppImage));
      data->fidImpls.clear();

      data->plugin->detect(data->fidImpls,ppImage);

      data->fids.resize(data->fidImpls.size());
      for(unsigned int i=0;i<data->fids.size();++i){
        data->fids[i] = Fiducial(data->fidImpls[i]);
      }
      return data->fids;
    }

    /// returns the list of supported features
    Fiducial::FeatureSet  FiducialDetector::getFeatures() const{
      Fiducial::FeatureSet  fs;
      data->plugin->getFeatures(fs);
      return fs;
    }

    std::string FiducialDetector::getIntermediateImageNames() const{
      std::string names="input,pp";
      std::string fromPlugin = data->plugin->getIntermediateImageNames();
      if(fromPlugin.length()){
        return names + ',' + fromPlugin;
      }else{
        return names;
      }
    }

    const ImgBase *FiducialDetector::getIntermediateImage(const std::string &name) const{
      if(name == "input") return data->iis.input;
      if(name == "pp") return data->iis.pp;
      return data->plugin->getIntermediateImage(name);
    }

    Img8u FiducialDetector::createMarker(const Any &whichOne,const Size &size, const ParamList &params){
      return data->plugin->createMarker(whichOne,size,params);
    }

    REGISTER_CONFIGURABLE_DEFAULT(FiducialDetector);

  } // namespace markers
}
