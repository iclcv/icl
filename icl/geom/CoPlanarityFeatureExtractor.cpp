// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Andre Ueckermann, Christof Elbrechter

#include <icl/geom/CoPlanarityFeatureExtractor.h>
#include <icl/geom/SegmenterUtils.h>

namespace icl::geom {
  math::DynMatrixBase<bool> CoPlanarityFeatureExtractor::apply(math::DynMatrixBase<bool> &initialMatrix, std::vector<SurfaceFeatureExtractor::SurfaceFeature> features,
                        const core::Img32f &depthImage, std::vector<std::vector<int> > &surfaces, float maxAngle,
                        float distanceTolerance, float outlierTolerance, int triangles, int scanlines){
    math::DynMatrixBase<bool> coplanar = math::DynMatrixBase<bool>(initialMatrix.rows(),initialMatrix.rows(), true);//result matrix
    //initialize
    for(size_t i=0; i<initialMatrix.rows(); i++){
      for(size_t j=0; j<initialMatrix.rows(); j++){
        //only test pairs of non-adjacent planar surfaces
        if(initialMatrix(j, i)==true || features[i].curvatureFactor!=SurfaceFeatureExtractor::PLANAR || features[j].curvatureFactor!=SurfaceFeatureExtractor::PLANAR){
          coplanar(j, i)=false;
        }
      }
    }
    for(size_t i=0; i<coplanar.rows(); i++){
      for(size_t j=i+1; j<coplanar.cols(); j++){//dont check pairs twice
        if(coplanar(j, i)==true){//candidate
          bool proceed=true;

          //criterion 1: both surfaces have similar mean normal (same orientation)
          proceed=criterion1(features[i].meanNormal, features[j].meanNormal, maxAngle);

          //criterion 2: both surfaces have the same level (combined surface has similar normal)
          if(proceed){
            proceed = criterion2(depthImage, surfaces[i], surfaces[j], features[i].meanNormal, features[j].meanNormal, maxAngle, triangles);
          }

          //criterion3: both surfaces separated by occlusion
          if(proceed){
            proceed=criterion3(depthImage, surfaces[i], surfaces[j], distanceTolerance, outlierTolerance, scanlines);
          }

          if(!proceed){//remove if one of the criterions failed
            coplanar(j, i)=false;
            coplanar(i, j)=false;
          }
        }
      }
    }
    return coplanar;
  }


  float CoPlanarityFeatureExtractor::getAngle(Vec n1, Vec n2){
    float a1=(n1[0]*n2[0]+n1[1]*n2[1]+n1[2]*n2[2]);
    float angle=acos(a1)*180.0/M_PI;//angle between the surface normals
    return angle;
  }


  utils::Point CoPlanarityFeatureExtractor::getRandomPoint(std::vector<int> surface, int imgWidth){
    int id=surface[rand()%surface.size()];
    int y = static_cast<int>(floor(static_cast<float>(id)/static_cast<float>(imgWidth)));
    int x = id-y*imgWidth;
    utils::Point p(x,y);
    return p;
  }


  Vec CoPlanarityFeatureExtractor::getNormal(Vec p0, Vec p1, Vec p2){
    Vec fa=p1-p0;
    Vec fb=p2-p0;
    Vec n1(fa[1]*fb[2]-fa[2]*fb[1],//normal
           fa[2]*fb[0]-fa[0]*fb[2],
           fa[0]*fb[1]-fa[1]*fb[0],
           0);
    Vec n01=n1/norm3(n1);//normalized normal
    return n01;
  }


  bool CoPlanarityFeatureExtractor::criterion1(Vec n1, Vec n2, float maxAngle){
    float angle=getAngle(n1,n2);
    if(angle>90) angle=180.-angle;//flip

    if(angle>maxAngle){
      return false;
    }
    return true;
  }


  bool CoPlanarityFeatureExtractor::criterion2(const core::Img32f &depthImage, std::vector<int> &surface1, std::vector<int> &surface2,
                                              Vec n1, Vec n2, float maxAngle, int triangles){
    std::vector<int> a,b;
    int w = depthImage.getSize().width;
    if(surface1.size()>surface2.size()){//find bigger surface
      b=surface2;
      a=surface1;
    }else{
      a=surface2;
      b=surface1;
    }

    Vec meanNormal((n1[0]+n2[0])/2.,
                   (n1[1]+n2[1])/2.,
                   (n1[2]+n2[2])/2.,
                   0);//mean of both surface normals
    Vec meanComb(0,0,0,0);//mean of combined surface normals

    core::Channel32f depthImageC = depthImage[0];
    for(int p=0; p<triangles; p++){
      //random combined plane normals
      utils::Point p0=getRandomPoint(b, w);
      utils::Point p1=getRandomPoint(a, w);
      utils::Point p2=getRandomPoint(a, w);

      while (p1.x==p2.x && p1.y==p2.y){//not the same point
        p2=getRandomPoint(a, w);
      }

      Vec a(p0.x,p0.y,depthImageC(p0.x,p0.y),0);
      Vec b(p1.x,p1.y,depthImageC(p1.x,p1.y),0);
      Vec c(p2.x,p2.y,depthImageC(p2.x,p2.y),0);

      Vec n01 = getNormal(a,b,c);

     	float ang = getAngle(n01,meanNormal);
     	if(ang>90.){//flip
     	  n01*=-1;
     	  ang=180.-ang;
      }
      meanComb+=n01;
    }
    meanComb/=triangles;

    float ang=getAngle(meanNormal,meanComb);

    if(ang>maxAngle){
      return false;
    }
    return true;
  }


  bool CoPlanarityFeatureExtractor::criterion3(const core::Img32f &depthImage, std::vector<int> &surface1, std::vector<int> &surface2,
                                              float distanceTolerance, float outlierTolerance, int scanlines){
    int w = depthImage.getSize().width;
    int occlusions=0;
    for(int l=0; l<scanlines; l++){
      utils::Point p1=getRandomPoint(surface1, w);
      utils::Point p2=getRandomPoint(surface2, w);

      if(SegmenterUtils::occlusionCheck(const_cast<core::Img32f&>(depthImage), p1, p2, distanceTolerance, outlierTolerance)){
        occlusions++;
      }
    }

    if(occlusions<0.8*scanlines){
      return false;
    }
    return true;
  }


  } // namespace icl::geom