/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/MultiCamFiducialDetector.cpp **
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

#include <ICLMarkers/MultiCamFiducialDetector.h>
#include <ICLMarkers/MultiCamFiducialImpl.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::geom;

namespace icl{
  namespace markers{

   struct MultiCamFiducialDetector::Data{
      bool camsDeeplyCopied;
      std::vector<Camera*> cams;
      std::vector<FiducialDetector*> detectors;
      std::vector<std::vector<Fiducial> > results;
      std::vector<MultiCamFiducialImpl> impls;
      int numImplsUsed;
      std::vector<MultiCamFiducial> output;


      ~Data(){
        if(camsDeeplyCopied){
          for(unsigned int i=0;i<cams.size();++i){
            delete cams[i];
          }
        }
        for(unsigned int i=0;i<detectors.size();++i){
          delete detectors[i];
        }
      }
    };


    void MultiCamFiducialDetector::property_callback(const Configurable::Property &p){
      Any value = getPropertyValue(p.name);
      for(unsigned int i=1;i<m_data->detectors.size();++i){
        m_data->detectors[i]->setPropertyValue(p.name, value);
      }
    }

    MultiCamFiducialDetector::MultiCamFiducialDetector():m_data(0){}

    MultiCamFiducialDetector::MultiCamFiducialDetector(const std::string &pluginType,
                                                       const Any &markersToLoad,
                                                       const ParamList &params,
                                                       const std::vector<Camera*> &cams,
                                                       bool syncProperties,
                                                       bool deepCopyCams) throw (ICLException):m_data(0){
      init(pluginType,markersToLoad,params,cams,syncProperties,deepCopyCams);
    }


    void MultiCamFiducialDetector::init(const std::string &pluginType,
                                        const Any &markersToLoad,
                                        const ParamList &params,
                                        const std::vector<Camera*> &cams,
                                        bool syncProperties,
                                        bool deepCopyCams) throw (ICLException){
      if(m_data) delete m_data;
      m_data = new Data;

      m_data->camsDeeplyCopied = deepCopyCams;
      if(deepCopyCams){
        std::vector<Camera*> copiedCams(cams.size());
        for(unsigned int i=0;i<copiedCams.size();++i) copiedCams[i] = new Camera(*cams[i]);
        m_data->cams = copiedCams;
      }else{
        m_data->cams = cams;
      }

      for(unsigned int i=0;i<cams.size();++i){
        m_data->detectors.push_back(new FiducialDetector(pluginType,markersToLoad,params));
        m_data->detectors[i]->setCamera(*m_data->cams[i]);
        if(!(i && syncProperties)){
          addChildConfigurable(m_data->detectors[i], syncProperties ? str("") : str("detector ")+str(i));
        }
      }
      if(syncProperties){
        Configurable::registerCallback(function(this,&MultiCamFiducialDetector::property_callback));
      }

      m_data->results.resize(cams.size());
    }


    const std::vector<MultiCamFiducial> &MultiCamFiducialDetector::detect(const std::vector<const ImgBase*> &images,
                                                                          int minCamsFound) throw (ICLException){
      ICLASSERT_THROW(m_data, ICLException(str(__FUNCTION__)+": this is null"));
      ICLASSERT_THROW(m_data->detectors.size() == images.size(),
                      ICLException(str(__FUNCTION__)+ ": given image count is wrong (got "
                                   + str(images.size()) + " but expected "
                                   + str(m_data->detectors.size()) + ")" ));

      int maxID = -1;
      for(unsigned int i=0;i<m_data->detectors.size();++i){
        std::vector<Fiducial> &r = m_data->results[i];
        r = m_data->detectors[i]->detect(images[i]);
         for(unsigned int j=0;j<r.size();++j){
          int id = r[j].getID();
          if(id > maxID) maxID = id;
        }
      }
      if(maxID == -1){
        m_data->output.clear();
        return m_data->output;
      }
      m_data->numImplsUsed = maxID +1;
      if((int)m_data->impls.size() < m_data->numImplsUsed){
        m_data->impls.resize(m_data->numImplsUsed);
      }

      for(int i=0;i<m_data->numImplsUsed;++i){
        m_data->impls[i].numFound = 0;
      }

      for(unsigned int i=0;i<m_data->results.size();++i){
        std::vector<Fiducial> &r = m_data->results[i];
        for(unsigned int j=0;j<r.size();++j){
          int id = r[j].getID();
          MultiCamFiducialImpl &m = m_data->impls[id];
          if(!m.numFound) {
            m.init(id);
          }
          ++m.numFound;
          m.fids.push_back(r[j]);
          m.cams.push_back(m_data->cams[i]);
        }
      }

      m_data->output.clear();
      for(int i=0;i<m_data->numImplsUsed;++i){
        if(m_data->impls[i].numFound >= minCamsFound){
          m_data->output.push_back(MultiCamFiducial(&m_data->impls[i]));
        }
      }

      return m_data->output;
     }

    const FiducialDetector &MultiCamFiducialDetector::getFiducialDetector(int idx) const{
      ICLASSERT_THROW(m_data, ICLException(str(__FUNCTION__)+": this is null"));
      return *m_data->detectors[idx];
    }

    FiducialDetector &MultiCamFiducialDetector::getFiducialDetector(int idx){
      ICLASSERT_THROW(m_data, ICLException(str(__FUNCTION__)+": this is null"));
      return *m_data->detectors[idx];
    }

    void MultiCamFiducialDetector::loadMarkers(const Any &which, const ParamList &params) throw (ICLException){
      ICLASSERT_THROW(m_data, ICLException(str(__FUNCTION__)+": this is null"));
      for(int i=0;i<getNumCameras();++i){
        m_data->detectors[i]->loadMarkers(which,params);
      }
    }

    void MultiCamFiducialDetector::unloadMarkers(const Any &which){
      ICLASSERT_THROW(m_data, ICLException(str(__FUNCTION__)+": this is null"));
      for(int i=0;i<getNumCameras();++i){
        m_data->detectors[i]->unloadMarkers(which);
      }
    }

    std::string MultiCamFiducialDetector::getIntermediateImageNames() const{
      ICLASSERT_THROW(m_data, ICLException(str(__FUNCTION__)+": this is null"));
      std::ostringstream str;
      for(int i=0;i<getNumCameras();++i){
        std::vector<std::string> iin = tok(m_data->detectors[i]->getIntermediateImageNames(),",");
        for(unsigned int j=0;j<iin.size();++j){
          str << "cam " << i << ":" << iin[j] << ( (j<iin.size()-1) ? "," : "");
        }
        str << ((i<getNumCameras()-1) ? "," : "");
      }
      return str.str();
    }

    int MultiCamFiducialDetector::getCameraIDFromIntermediteImageName(const std::string &name){
      return parse<int>(name.substr(4));
    }

    const ImgBase *MultiCamFiducialDetector::getIntermediateImage(const std::string &name) const throw (ICLException){
      ICLASSERT_THROW(m_data, ICLException(str(__FUNCTION__)+": this is null"));
      int idx = parse<int>(name.substr(4));
      if(idx >= 0 && idx < getNumCameras()){
        size_t colonPos = name.find(':');
        return m_data->detectors[idx]->getIntermediateImage(name.substr(colonPos+1));
      }else{
        return 0;
      }
    }

    int MultiCamFiducialDetector::getNumCameras() const{
      ICLASSERT_THROW(m_data, ICLException(str(__FUNCTION__)+": this is null"));
      return m_data->detectors.size();
    }


    REGISTER_CONFIGURABLE_DEFAULT(MultiCamFiducialDetector);
  } // namespace markers
}
