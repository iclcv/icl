/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/RSBImageOutput.cpp                           **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
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

#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/converter/Repository.h>
#include <rsb/converter/ProtocolBufferConverter.h>

#include <ICLCore/ImageSerializer.h>

#include <ICLIO/ImageCompressor.h>
#include <ICLIO/RSBImageOutput.h>
#include <ICLIO/RSBImage.pb.h>
#include <ICLUtils/Mutex.h>

using namespace boost;
using namespace rsb;
using namespace rsb::converter;

namespace icl{

  struct StaticRSBImageTypeRegistration{
    StaticRSBImageTypeRegistration(){
      shared_ptr<ProtocolBufferConverter<RSBImage> > p(new ProtocolBufferConverter<RSBImage>());
#if RSB_VERSION_MINOR < 8
      stringConverterRepository()->registerConverter(p);
#else
      converterRepository<std::string>()->registerConverter(p);
#endif


    }
  } static_RSBImage_type_registration;
  
  struct RSBImageOutput::Data{
    Mutex mutex;
    Informer<RSBImage>::Ptr informer;
    Informer<RSBImage>::DataPtr out;

    ListenerPtr propertyListener;
    std::string propertyScopeName;

    struct PropertyHandler : public DataHandler<std::string>{
      RSBImageOutput *out;
      PropertyHandler(RSBImageOutput *out):out(out){}
      virtual void notify(shared_ptr<std::string> s){
        std::vector<std::string> ts = tok(*s,":");
        if(ts.size() != 2){
          ERROR_LOG("invalid property definition at " << out->m_data->propertyScopeName);
        }
        out->setCompression(ImageCompressor::CompressionSpec(ts[0],ts[1]));
      }
    };
    shared_ptr<rsb::Handler> propertyHandler;

  };

  RSBImageOutput::RSBImageOutput():m_data(0){}

  RSBImageOutput::~RSBImageOutput(){
    ICL_DELETE(m_data);
  }
    
  RSBImageOutput::RSBImageOutput(const std::string &scope, const std::string &transportList):m_data(0){
    init(scope,transportList);
  }
  
  void RSBImageOutput::init(const std::string &scope, const std::string &transportList){
    ICL_DELETE(m_data);
    m_data = new Data;
    
    Scope rsbScope(scope);
    ParticipantConfig rsbCfg;
    std::vector<std::string> transports = tok(transportList,",");
    for(size_t i=0;i<transports.size();++i){
      rsbCfg.addTransport(ParticipantConfig::Transport(transports[i]));
    }
    m_data->informer = Factory::getInstance().createInformer<RSBImage>(rsbScope,rsbCfg);
    m_data->out = Informer<RSBImage>::DataPtr(new RSBImage);

    
    m_data->propertyScopeName = "/icl/RSBImageOutput/configuration"+scope;
    m_data->propertyListener = Factory::getInstance().createListener(Scope(m_data->propertyScopeName), rsbCfg);
    m_data->propertyHandler = shared_ptr<rsb::Handler>(new Data::PropertyHandler(this));
    m_data->propertyListener->addHandler(m_data->propertyHandler);
  }
    
  void RSBImageOutput::send(const ImgBase *image){
    ICLASSERT_RETURN(image);
    ICLASSERT_RETURN(!isNull());
    ICLASSERT_RETURN(image->getDim() > 0);
    ICLASSERT_RETURN(image->getChannels() > 0);
    
    Mutex::Locker lock(m_data->mutex);
    Informer<RSBImage>::DataPtr &out = m_data->out;
   
    out->set_width(image->getWidth());
    out->set_height(image->getHeight());
    out->set_channels(image->getChannels());
    out->set_time(image->getTime().toMicroSeconds());
    out->set_roix(image->getROIXOffset());
    out->set_roiy(image->getROIYOffset());
    out->set_roiw(image->getROIWidth());
    out->set_roih(image->getROIHeight());

    out->set_format((RSBImage::Format)image->getFormat());
    out->set_depth((RSBImage::Depth)image->getDepth());

    out->set_metadata(image->getMetaData());

    ImageCompressor::CompressionSpec c = getCompression();
    out->set_compressionmode(c.mode);
    out->set_compressionquality(c.quality);

    ImageCompressor::CompressedData cmp = compress(image,true);
    out->set_data(cmp.bytes,cmp.len);
    m_data->informer->publish(out);
    
    //  std::string *metadata = out->mutable_metadata();
    m_data->informer->publish(out);

  }
};
