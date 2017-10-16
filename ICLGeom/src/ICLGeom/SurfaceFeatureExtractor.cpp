/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/SurfaceFeatureExtractor.cpp        **
** Module : ICLGeom                                                **
** Authors: Andre Ueckermann                                       **
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

#include <ICLGeom/SurfaceFeatureExtractor.h>

namespace icl{
  namespace geom{

    SurfaceFeatureExtractor::SurfaceFeature SurfaceFeatureExtractor::apply(
                                            std::vector<Vec> &points, std::vector<Vec> &normals, int mode){
      SurfaceFeatureExtractor::SurfaceFeature feature = getInitializedStruct();
      if(normals.size()!=points.size()){
        throw utils::ICLException("points size != normals size");
      }
      feature.numPoints=points.size();
      for(unsigned int i=0; i<points.size(); i++){
    	  update(normals[i], points[i], feature, mode);
      }
      finish(feature, mode);
      return feature;
    }


    std::vector<SurfaceFeatureExtractor::SurfaceFeature> SurfaceFeatureExtractor::apply(
                                            core::Img32s labelImage, core::DataSegment<float,4> &xyzh,
                                            core::DataSegment<float,4> &normals, int mode){
  	  unsigned int w = labelImage.getSize().width;
  	  unsigned int h = labelImage.getSize().height;
  	  core::Channel32s labelI = labelImage[0];
      std::vector<SurfaceFeatureExtractor::SurfaceFeature> features;
      for(unsigned int y=0; y<h; y++){
        for(unsigned int x=0; x<w; x++){
          while((int)features.size()<labelI(x,y)){
            features.push_back(getInitializedStruct());
          }
          if(labelI(x,y)>0){
            features.at(labelI(x,y)-1).numPoints++;
            update(normals[x+y*w], xyzh[x+y*w], features.at(labelI(x,y)-1), mode, x, y);
          }
        }
      }
      for(unsigned int i=0; i<features.size(); i++){
        finish(features.at(i), mode);
      }
      return features;
    }


    SurfaceFeatureExtractor::SurfaceFeature SurfaceFeatureExtractor::getInitializedStruct(){
      SurfaceFeatureExtractor::SurfaceFeature feature;
      feature.numPoints=0;
      feature.normalHistogram.setSize(utils::Size(11,11));
      feature.normalHistogram.setChannels(1);
      feature.normalHistogram.setFormat(core::formatMatrix);
      feature.normalHistogram.fill(0);
      feature.normalHistogramChannel = feature.normalHistogram[0];
      feature.meanNormal=Vec(0,0,0,0);
      feature.meanPosition=Vec(0,0,0,0);
      feature.curvatureFactor=SurfaceFeatureExtractor::UNDEFINED;
      feature.boundingBox3D.first = Vec(1000000, 1000000, 1000000, 0);
      feature.boundingBox3D.second = Vec(-1000000, -1000000, -1000000, 0);
      feature.boundingBox2D.first = utils::Point(1000000,1000000);
      feature.boundingBox2D.second = utils::Point(-1000000, -1000000);
      feature.volume=0;
      return feature;
    }


    void SurfaceFeatureExtractor::update(Vec &normal, Vec &point, SurfaceFeature &feature, int mode, int x, int y){
      if(mode&NORMAL_HISTOGRAM){
        int xx = round(normal.x*5.0+5.0);//-1 -> 0, 0 -> 5, 1 -> 10
        int yy = round(normal.y*5.0+5.0);
        feature.normalHistogramChannel(xx,yy)++;
      }
  	  if(mode&MEAN_NORMAL){
  	    feature.meanNormal+=normal;
  	  }
  	  if(mode&MEAN_POSITION){
  	    feature.meanPosition+=point;
  	  }
  	  if(mode&BOUNDING_BOX_3D){
  	    if(point[0]<feature.boundingBox3D.first[0]) feature.boundingBox3D.first[0]=point[0];//min
  	    if(point[1]<feature.boundingBox3D.first[1]) feature.boundingBox3D.first[1]=point[1];
  	    if(point[2]<feature.boundingBox3D.first[2]) feature.boundingBox3D.first[2]=point[2];
  	    if(point[0]>feature.boundingBox3D.second[0]) feature.boundingBox3D.second[0]=point[0];//max
  	    if(point[1]>feature.boundingBox3D.second[1]) feature.boundingBox3D.second[1]=point[1];
  	    if(point[2]>feature.boundingBox3D.second[2]) feature.boundingBox3D.second[2]=point[2];
  	  }
  	  if(mode&BOUNDING_BOX_2D){
  	    if(x<feature.boundingBox2D.first.x) feature.boundingBox2D.first.x=x;//min
  	    if(y<feature.boundingBox2D.first.y) feature.boundingBox2D.first.y=y;
  	    if(x>feature.boundingBox2D.second.x) feature.boundingBox2D.second.x=x;//max
  	    if(y>feature.boundingBox2D.second.y) feature.boundingBox2D.second.y=y;
  	  }
  	}


  	void SurfaceFeatureExtractor::finish(SurfaceFeature &feature, int mode){
  	  feature.meanNormal/=feature.numPoints;
      feature.meanPosition/=feature.numPoints;
      if(mode&CURVATURE_FACTOR || mode&NORMAL_HISTOGRAM){
        int numOfBinsBigger05=0;
        for(unsigned int y = 0; y<11; y++){
          for(unsigned int x = 0; x<11; x++){
            feature.normalHistogramChannel(x,y)/=feature.numPoints;//normalized histogram
            if(feature.normalHistogramChannel(x,y)>=0.005) numOfBinsBigger05++;
          }
        }
        if(numOfBinsBigger05==0) feature.curvatureFactor = SurfaceFeatureExtractor::UNDEFINED;
        else if(numOfBinsBigger05<8) feature.curvatureFactor = SurfaceFeatureExtractor::PLANAR;
        else if(numOfBinsBigger05<20) feature.curvatureFactor = SurfaceFeatureExtractor::CURVED_1D;
        else feature.curvatureFactor = SurfaceFeatureExtractor::CURVED_2D;
      }
      if(mode&BOUNDING_BOX_3D){
        Vec min = feature.boundingBox3D.first;
        Vec max = feature.boundingBox3D.second;
        feature.volume = (max[0]-min[0])*(max[1]-min[1])*(max[2]-min[2]);
      }
	  }


	  float SurfaceFeatureExtractor::matchNormalHistograms(core::Img32f &a, core::Img32f &b){
      float sum=0;
      core::Channel32f aC = a[0];
      core::Channel32f bC = b[0];
      for(size_t i=0; i<11; i++){
        for(size_t j=0; j<11; j++){
          sum+=std::min(aC(i,j),bC(i,j));
        }
      }
      return sum;
	  }

  }
}
