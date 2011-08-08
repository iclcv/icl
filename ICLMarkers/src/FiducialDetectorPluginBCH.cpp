/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/FiducialDetectorPluginBCH.cpp           **
** Module : ICLMarkers                                             **
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

#include <ICLMarkers/FiducialDetectorPluginBCH.h>
#include <ICLMarkers/BCHCode.h>
#include <ICLMarkers/Fiducial.h>

namespace icl{
  
  namespace{
    struct PatternBinarization{  virtual void apply(icl8u *p) = 0;  };
    
    struct PatternBinarizationThresh : public PatternBinarization{
      int t;
      PatternBinarizationThresh(int t=0):t(t){}
      virtual void apply(icl8u *p){
        for(int i=0;i<36;++i) p[i] = 255*(p[i]>t);
      }
    };
    
    struct PatternBinarizationKMeans : public PatternBinarizationThresh{
      int maxIterations;
      PatternBinarizationKMeans(int maxIterations):
        maxIterations(maxIterations){}

      virtual void apply(icl8u *p){
        float mean = std::accumulate(p,p+36,0.0f)/36.0f;
        for(int j=0;j<maxIterations;++j){
          Point l,r;
          for(int i=0;i<36;++i){
            (p[i] < mean ? l : r) += Point(p[i],1);
          }
          mean = round( (float(l[0])/l[1] + float(r[0])/r[1]) / 2  );
        }
        PatternBinarizationThresh::t = mean;
        PatternBinarizationThresh::apply(p);
      }
    };
  }

  struct FiducialDetectorPluginBCH::Data{
    Img8u buffer;
    std::bitset<4096> loaded;
    int maxLoaded;
    std::vector<Size32f> sizes;

    SmartPtr<PatternBinarization> bin;
    int maxBCHErr;
    BCHCoder bch;
  };
  
  FiducialDetectorPluginBCH::FiducialDetectorPluginBCH():
    data(new Data){
    data->buffer.setChannels(1);
    data->buffer.setSize(Size(6,6));

    data->loaded.reset();
    data->maxLoaded = -1;
    data->sizes.resize(4096);

    addProperty("max bch errors","range","[0,4]:1",3);
    addProperty("match factor","range","[1,10]:1",2);
    addProperty("border width","range","[1,10]:1",2);
    addProperty("binarize.threshold","range","[1,254]:1",127);
    addProperty("binarize.k-means steps","range","[1:5]:1",1);
    addProperty("binarize.mode","menu","k-means,threshold","k-means");
  }

  FiducialDetectorPluginBCH::~FiducialDetectorPluginBCH(){
    delete data;
  }


  void FiducialDetectorPluginBCH::addOrRemoveMarkers(bool add, const Any &which, const ParamList &l){
    Size s = l["size"];

    std::vector<int> ids = parse_list_str(which);
    for(unsigned int i=0;i<ids.size();++i){
      int x = ids[i];
      if(x<0 || x>= 4096) continue;
      data->loaded[x] = add;
      if(add) data->sizes[x] = s;
    }
    
    data->maxLoaded=-1;
    for(int i=4095;i>=0;--i){
      if(data->loaded[i]){
        data->maxLoaded = i;
        break;
      }
    }

  }


 
  
  void FiducialDetectorPluginBCH::prepareForPatchClassification(){
    data->maxBCHErr = getPropertyValue("max bch errors");
    std::string mode = getPropertyValue("binarize.mode");
    if(mode == "threshold"){
      data->bin = new PatternBinarizationThresh(getPropertyValue("binarize.threshold").as<int>());
    }else{
      data->bin = new PatternBinarizationKMeans(getPropertyValue("binarize.k-means steps").as<int>());
    }
  }
  
  FiducialImpl *FiducialDetectorPluginBCH::classifyPatch(const Img8u &image, int *rot, bool returnRejectedQuads){
    image.scaledCopyROI(&data->buffer, interpolateRA);
    
    data->bin->apply(data->buffer.begin(0));
    
    DecodedBCHCode2D p = data->bch.decode2D(data->buffer,data->maxLoaded,false);

    static Fiducial::FeatureSet supported = Fiducial::AllFeatures;
    static Fiducial::FeatureSet computed = ( Fiducial::Center2D | 
                                             Fiducial::Rotation2D |
                                             Fiducial::Corners2D );
    
    if(p && (p.id >=0) && (p.id < 4096) && (data->loaded[p.id]) && p.errors <= data->maxBCHErr){
      FiducialImpl *impl = new FiducialImpl(this,supported,computed,
                                            p.id, -1,data->sizes[p.id]);
      *rot = (int)p.rot;
      return impl;
    }else if (returnRejectedQuads){
      *rot = 0;
      return new FiducialImpl(this,supported,computed, 999999, -1,Size(1,1)); // dummy ID
    }else{
      return 0;
    }
  }

  void FiducialDetectorPluginBCH::getQuadRectificationParameters(Size &markerSizeWithBorder,
                                                                 Size &markerSizeWithoutBorder){
    int f = getPropertyValue("match factor");
    int b = getPropertyValue("border width");
    markerSizeWithBorder = Size(f*(2*b+6), f*(2*b+6));
    markerSizeWithoutBorder = Size(f*6,f*6);
  }

  Img8u FiducialDetectorPluginBCH::createMarker(const Any &whichOne,const Size &size, const ParamList &params){
    return BCHCoder::createMarkerImage(whichOne,params["border width"],size);
  }


}
