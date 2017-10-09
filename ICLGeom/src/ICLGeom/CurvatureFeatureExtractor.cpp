/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/CurvatureFeatureExtractor.cpp      **
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

#include <ICLGeom/CurvatureFeatureExtractor.h>
#include <ICLGeom/SegmenterUtils.h>

namespace icl{
  namespace geom{

    math::DynMatrix<bool> CurvatureFeatureExtractor::apply(const core::Img32f &depthImg, core::DataSegment<float,4> &xyz, math::DynMatrix<bool> &initialMatrix,
                          std::vector<SurfaceFeatureExtractor::SurfaceFeature> features,
                          std::vector<std::vector<int> > &surfaces, core::DataSegment<float,4> &normals, bool useOpenObjects, bool useOccludedObjects,
                          float histogramSimilarity, int distance, float maxError, int ransacPasses, float distanceTolerance, float outlierTolerance){
      int w = depthImg.getSize().width;
      math::DynMatrix<bool> curvature = math::DynMatrix<bool>(initialMatrix.rows(),initialMatrix.rows(), true);//result matrix
      //initialize
      for(size_t i=0; i<initialMatrix.rows(); i++){
        for(size_t j=0; j<initialMatrix.rows(); j++){
          //only test pairs of non-adjacent curved surfaces
          if(initialMatrix(i,j)==true ||
              (features[i].curvatureFactor!=SurfaceFeatureExtractor::CURVED_1D && features[i].curvatureFactor!=SurfaceFeatureExtractor::CURVED_2D) ||
              (features[j].curvatureFactor!=SurfaceFeatureExtractor::CURVED_1D && features[j].curvatureFactor!=SurfaceFeatureExtractor::CURVED_2D) ){
            curvature(i,j)=false;
          }
        }
      }
      for(size_t i=0; i<curvature.rows(); i++){
        for(size_t j=i+1; j<curvature.cols(); j++){//dont check pairs twice
          if(curvature(i,j)==true){//candidate
            bool proceed=true;

            //joint criterion: similar surface shape and orientation (normal histogram matching)
            float similarityScore = SurfaceFeatureExtractor::matchNormalHistograms(features[i].normalHistogram, features[j].normalHistogram);
            if(similarityScore<histogramSimilarity){
              proceed=false;
            }

            //compute cases
            if(proceed){
              proceed=false;
              if(useOpenObjects){
                proceed = computeOpenObject(normals, features[i], features[j],
                                    surfaces[i], surfaces[j], distance, w);
              }
              if(useOccludedObjects && !proceed){//only if first case does not match
                proceed = computeOccludedObject(depthImg, xyz, normals, features[i], features[j],
                                    surfaces[i], surfaces[j], w, maxError, ransacPasses, distanceTolerance, outlierTolerance);
              }
            }

            if(!proceed){//remove if no case succeeded
              curvature(i,j)=false;
              curvature(j,i)=false;
            }
          }
        }
      }

      return curvature;
    }


    bool CurvatureFeatureExtractor::computeOpenObject(core::DataSegment<float, 4> &normals, SurfaceFeatureExtractor::SurfaceFeature feature1, SurfaceFeatureExtractor::SurfaceFeature feature2,
                                    std::vector<int> &surface1, std::vector<int> &surface2, int distance, int w){
      //1. neighbouring in image space
      std::pair<utils::Point,utils::Point> bBox1 = feature1.boundingBox2D; //min, max
      std::pair<utils::Point,utils::Point> bBox2 = feature2.boundingBox2D;
      bool proceed=false;
      if(bBox1.second.x>bBox2.first.x && bBox2.second.x>bBox1.first.x){ //overlap in x
        proceed=true;
      }
      if(proceed){//maximum distance in y
        proceed=false;
        if(bBox1.first.y-bBox2.second.y<distance && bBox2.first.y-bBox1.second.y<distance){
          proceed=true;
        }
      }
      //2. one surface concave and one surface convex
      if(proceed){
        float direction1 = computeConvexity(normals, feature1, surface1, w);
        float direction2 = computeConvexity(normals, feature2, surface2, w);
        //>=0 convex (front), <0 concave (back)
        if((direction1>=0 && direction2<0 && bBox2.second.y>bBox1.second.y) || (direction1<0 && direction2>=0 && bBox1.second.y>bBox2.second.y)){
          return true;
        }
      }

      return false;
    }


    bool CurvatureFeatureExtractor::computeOccludedObject(const core::Img32f &depthImg, core::DataSegment<float,4> &xyz, core::DataSegment<float, 4> &normals,
                                    SurfaceFeatureExtractor::SurfaceFeature feature1, SurfaceFeatureExtractor::SurfaceFeature feature2,
                                    std::vector<int> &surface1, std::vector<int> &surface2, int w, float maxError, int ransacPasses, float distanceTolerance, float outlierTolerance){
      //select most populated bin (same bin for both histograms)
      float maxBinValue=0;
      utils::Point maxBin(0,0);
      for(int y=0; y<feature1.normalHistogram.getSize().height; y++){
        for(int x=0; x<feature1.normalHistogram.getSize().width; x++){
          float binValue = std::min(feature1.normalHistogramChannel(x,y), feature2.normalHistogramChannel(x,y));
          if(binValue>maxBinValue){
            maxBinValue=binValue;
            maxBin.x=x;
            maxBin.y=y;
          }
        }
      }

      //backproject the points
      std::vector<int> pointIDs1 = backprojectPointIDs(normals, maxBin, surface1);
      std::vector<int> pointIDs2 = backprojectPointIDs(normals, maxBin, surface2);
      std::vector<Vec> points1 = createPointsFromIDs(xyz, pointIDs1);
      std::vector<Vec> points2 = createPointsFromIDs(xyz, pointIDs2);

      //fit line with RANSAC (faster than linear regression)
      float minError = 100000;
      std::pair<utils::Point,utils::Point> pointPairImg;
      for(int i=0; i<ransacPasses; i++){
        float currentError=0;
        std::pair<Vec,Vec> currentPointPair;
        std::pair<int,int> currentPointPairID;
        currentPointPairID.first = pointIDs1[rand()%pointIDs1.size()];
        currentPointPairID.second = pointIDs2[rand()%pointIDs2.size()];
        currentPointPair.first = xyz[currentPointPairID.first];
        currentPointPair.second = xyz[currentPointPairID.second];
        for(unsigned int i=0; i<points1.size(); i++){
          currentError+=linePointDistance(currentPointPair, points1[i]);
        }
        for(unsigned int i=0; i<points2.size(); i++){
          currentError+=linePointDistance(currentPointPair, points2[i]);
        }
        currentError/=points1.size()+points2.size();
        if(currentError<minError){
          minError=currentError;
          pointPairImg.first=idToPoint(currentPointPairID.first,w);
          pointPairImg.second=idToPoint(currentPointPairID.second,w);
        }
      }

      //occlusion check
      if(minError<maxError){
        return SegmenterUtils::occlusionCheck((core::Img32f&)depthImg, pointPairImg.first, pointPairImg.second, distanceTolerance, outlierTolerance);
      }
      return false;
    }


    float CurvatureFeatureExtractor::computeConvexity(core::DataSegment<float, 4> &normals, SurfaceFeatureExtractor::SurfaceFeature feature, std::vector<int> &surface, int w){
      //select extremal bins in histogram
      std::pair<utils::Point,utils::Point> histoExtremalBins = computeExtremalBins(feature);

      //backproject to image space and calculate mean
      std::pair<utils::Point,utils::Point> imgBackproject = backproject(normals, histoExtremalBins, surface, w);

      //scalar product to determine concave and convex
      float direction = computeConvexity(histoExtremalBins, imgBackproject);

      return direction;
    }


    std::pair<utils::Point,utils::Point> CurvatureFeatureExtractor::computeExtremalBins(SurfaceFeatureExtractor::SurfaceFeature feature){
      //normal histogram bounding box
      std::pair<utils::Point,utils::Point> histoBBox;
      histoBBox.first.x=1000;
      histoBBox.first.y=1000;
      histoBBox.second.x=-1000;
      histoBBox.second.y=-1000;
      for(int y=0; y<feature.normalHistogram.getSize().height; y++){
        for(int x=0; x<feature.normalHistogram.getSize().width; x++){
          if(feature.normalHistogramChannel(x,y)>=0.005){
            if(x<histoBBox.first.x) histoBBox.first.x=x;
            if(y<histoBBox.first.y) histoBBox.first.y=y;
            if(x>histoBBox.second.x) histoBBox.second.x=x;
            if(y>histoBBox.second.y) histoBBox.second.y=y;
          }
        }
      }
      //normal histogram extremal bins
      std::pair<utils::Point,utils::Point> histoExtremalBins; //min, max
      if(histoBBox.second.x-histoBBox.first.x>=histoBBox.second.y-histoBBox.first.y){//sample x
        int x1 = histoBBox.first.x;
        histoExtremalBins.first.x=x1;
        int x2 = histoBBox.second.x;
        histoExtremalBins.second.x=x2;
        for(int y=0; y<feature.normalHistogram.getSize().height; y++){
          if(feature.normalHistogramChannel(x1,y)>=0.005){
            histoExtremalBins.first.y=y;
          }
          if(feature.normalHistogramChannel(x2,y)>=0.005){
            histoExtremalBins.second.y=y;
          }
        }
      }else{//sampleY
        int y1 = histoBBox.first.y;
        histoExtremalBins.first.y=y1;
        int y2 = histoBBox.second.y;
        histoExtremalBins.second.y=y2;
        for(int x=0; x<feature.normalHistogram.getSize().width; x++){
          if(feature.normalHistogramChannel(x,y1)>=0.005){
            histoExtremalBins.first.x=x;
          }
          if(feature.normalHistogramChannel(x,y2)>=0.005){
            histoExtremalBins.second.x=x;
          }
        }
      }
      return histoExtremalBins;
    }


    std::pair<utils::Point,utils::Point> CurvatureFeatureExtractor::backproject(core::DataSegment<float, 4> &normals,
                        std::pair<utils::Point,utils::Point> &histoExtremalBins, std::vector<int> &surface, int w){
      std::vector<int> imgMinPoints=backprojectPointIDs(normals, histoExtremalBins.first, surface);
      std::vector<int> imgMaxPoints=backprojectPointIDs(normals, histoExtremalBins.second, surface);

      std::pair<utils::Point,utils::Point> imgMeans;
      imgMeans.first = computeMean(imgMinPoints, w);
      imgMeans.second = computeMean(imgMaxPoints, w);
      return imgMeans;
    }


    std::vector<int> CurvatureFeatureExtractor::backprojectPointIDs(core::DataSegment<float,4> &normals, utils::Point bin, std::vector<int> &surface){
      std::vector<int> pointIDs;
      for(unsigned int i=0; i<surface.size(); i++){
        if(round(normals[surface[i]].x*5.0+5.0)==bin.x && round(normals[surface[i]].y*5.0+5.0)==bin.y){
          pointIDs.push_back(surface[i]);
        }
      }
      return pointIDs;
    }


    utils::Point CurvatureFeatureExtractor::computeMean(std::vector<int> &imgIDs, int w){
      std::vector<utils::Point> points = createPointsFromIDs(imgIDs, w);
      utils::Point imgMean(0,0);

      for(unsigned int i=0; i<points.size(); i++){
        imgMean.x+=points[i].x;
        imgMean.y+=points[i].y;
      }
      imgMean.x/=points.size();
      imgMean.y/=points.size();
      return imgMean;
    }


    std::vector<utils::Point> CurvatureFeatureExtractor::createPointsFromIDs(std::vector<int> &imgIDs, int w){
      std::vector<utils::Point> points(imgIDs.size());
      for(unsigned int i=0; i<imgIDs.size(); i++){
        int id = imgIDs[i];
        int y = (int)floor((float)id/(float)w);
        int x = id-y*w;
        points[i]=utils::Point(x,y);
      }
      return points;
    }


    std::vector<Vec> CurvatureFeatureExtractor::createPointsFromIDs(core::DataSegment<float,4> &xyz, std::vector<int> &imgIDs){
      std::vector<Vec> points(imgIDs.size());
      for(unsigned int i=0; i<imgIDs.size(); i++){
        points[i]=xyz[imgIDs[i]];
      }
      return points;
    }


    float CurvatureFeatureExtractor::computeConvexity(std::pair<utils::Point,utils::Point> histoExtremalBins, std::pair<utils::Point,utils::Point> imgBackproject){
      utils::Point histoVec;
      utils::Point imgVec;
      histoVec.x=histoExtremalBins.second.x-histoExtremalBins.first.x;
      histoVec.y=histoExtremalBins.second.y-histoExtremalBins.first.y;
      float lengthHisto = sqrt(histoVec.x*histoVec.x+histoVec.y*histoVec.y);//normalize
      imgVec.x=imgBackproject.second.x-imgBackproject.first.x;
      imgVec.y=imgBackproject.second.y-imgBackproject.first.y;
      float lengthImg = sqrt(imgVec.x*imgVec.x+imgVec.y*imgVec.y);
      float direction = (histoVec.x/lengthHisto)*(imgVec.x/lengthImg)+(histoVec.y/lengthHisto)*(imgVec.y/lengthImg);
      return direction;
    }


    float CurvatureFeatureExtractor::linePointDistance(std::pair<Vec,Vec> line, Vec point){
      float d = norm3(cross(point-line.first,point-line.second))/norm3(line.second-line.first);
      return d;
    }


    utils::Point CurvatureFeatureExtractor::idToPoint(int id, int w){
      int y = (int)floor((float)id/(float)w);
      int x = id-y*w;
      return utils::Point(x,y);
    }

  } // namespace geom
}
