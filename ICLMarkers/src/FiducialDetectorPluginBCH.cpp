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

  struct FiducialDetectorPluginBCH::Data{
    Img8u buffer;
    std::bitset<4096> loaded;
    int maxLoaded;
    std::vector<Size32f> sizes;
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
  }

  FiducialDetectorPluginBCH::~FiducialDetectorPluginBCH(){
    delete data;
  }


  void FiducialDetectorPluginBCH::addOrRemoveMarkers(bool add, const Any &which, const ParamList &l){
    Size s = l["size"];

    if(!which.length()){
      throw ICLException("FiducialDetectorPluginBCH::addOrRemoveMarkers: invalid marker definition");
    }
    
    switch(which[0]){
      case '[':{
        Range32s r = which;
        for(int i=r.minVal;i<=r.maxVal;++i){
          if(i<0 || i >= 4096) continue;
          data->loaded[i] = add;
          if(add) data->sizes[i] = s;
        }
        break;
      }
      case '{':{
        std::vector<int> ids = parseVecStr<int>(which.substr(1,which.length()-2));
        for(unsigned int i=0;i<ids.size();++i){
          if(i<0 || i >= 4096) continue;
          data->loaded[ids[i]] = add;
          if(add) data->sizes[i] = s;
        }
        break;
      }default:
        int i = which.as<int>();
        if(i<0 || i >= 4096) break;
        data->loaded[i] = add;
        if(add) data->sizes[i] = s;
        break;
    }
    
    data->maxLoaded=-1;
    for(int i=4095;i>=0;--i){
      if(data->loaded[i]){
        data->maxLoaded = i;
        break;
      }
    }
  }


  static inline void thresh_inplace(icl8u *p, int len, int t){
    for(int i=0;i<len;++i) p[i] = 255*(p[i]>t);
  }
  
  
  FiducialImpl *FiducialDetectorPluginBCH::classifyPatch(const Img8u &image, int *rot){
    int maxErr = getPropertyValue("max bch errors");

    image.scaledCopyROI(&data->buffer, interpolateRA);
    thresh_inplace(data->buffer.begin(0),36, 127);

    DecodedBCHCode2D p = decode_bch_2D(data->buffer,data->maxLoaded,false);

    static Fiducial::FeatureSet supported = Fiducial::AllFeatures;
    static Fiducial::FeatureSet computed = ( Fiducial::Center2D | 
                                             Fiducial::Rotation2D |
                                             Fiducial::Corners2D );
    
    if(p && p.errors <= maxErr ){
      FiducialImpl *impl = new FiducialImpl(this,supported,computed,
                                            p.id, -1,data->sizes[p.id]);
      *rot = (int)p.rot;
      return impl;
    }else {
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

}
