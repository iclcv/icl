/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/RemainingPointsFeatureExtractor.cpp**
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

#include <ICLGeom/RemainingPointsFeatureExtractor.h>
#include <ICLCore/Channel.h>
#include <ICLCore/Img.h>
#include <ICLCV/RegionGrower.h>
#include <ICLGeom/PlanarRansacEstimator.h>

namespace icl{
  namespace geom{    

    void RemainingPointsFeatureExtractor::apply(core::DataSegment<float,4> &xyz, const core::Img32f &depthImage, core::Img32s &labelImage, core::Img8u &maskImage, 
                          std::vector<std::vector<int> > &surfaces, std::vector<std::vector<int> > &segments, int minSize, float euclideanDistance, int radius, float assignEuclideanDistance, int supportTolerance){
      calculateLocalMinima(depthImage, maskImage, radius);      
      apply(xyz, labelImage, maskImage, surfaces, segments, minSize, euclideanDistance, assignEuclideanDistance, supportTolerance);
    }
    
                           
    void RemainingPointsFeatureExtractor::apply(core::DataSegment<float,4> &xyz, core::Img32s &labelImage, core::Img8u &maskImage, 
                      std::vector<std::vector<int> > &surfaces, std::vector<std::vector<int> > &segments, int minSize, float euclideanDistance, float assignEuclideanDistance, int supportTolerance){
      int numCluster=surfaces.size();
      clusterRemainingPoints(xyz, surfaces, labelImage, maskImage, minSize, euclideanDistance, numCluster);
      std::vector<std::vector<int> > neighbours;
      std::vector<std::vector<int> > neighboursPoints;
      detectNeighbours(xyz, surfaces, labelImage, neighbours, neighboursPoints, numCluster, assignEuclideanDistance); 

      ruleBasedAssignment(xyz, labelImage, surfaces, segments, neighbours, neighboursPoints, numCluster, supportTolerance);
    }
     
                      
    math::DynMatrix<bool> RemainingPointsFeatureExtractor::apply(core::DataSegment<float,4> &xyz, const core::Img32f &depthImage, core::Img32s &labelImage, core::Img8u &maskImage, 
                      std::vector<std::vector<int> > &surfaces, int minSize, float euclideanDistance, int radius, float assignEuclideanDistance){
      calculateLocalMinima(depthImage, maskImage, radius);
      return apply(xyz, labelImage, maskImage, surfaces, minSize, euclideanDistance, assignEuclideanDistance);            
    }
     
                      
    math::DynMatrix<bool> RemainingPointsFeatureExtractor::apply(core::DataSegment<float,4> &xyz, core::Img32s &labelImage, core::Img8u &maskImage, 
                      std::vector<std::vector<int> > &surfaces, int minSize, float euclideanDistance, float assignEuclideanDistance){      
      int numCluster=surfaces.size();
      clusterRemainingPoints(xyz, surfaces, labelImage, maskImage, minSize, euclideanDistance, numCluster);
      std::vector<std::vector<int> > neighbours;
      std::vector<std::vector<int> > neighboursPoints;
      detectNeighbours(xyz, surfaces, labelImage, neighbours, neighboursPoints, numCluster, assignEuclideanDistance); 
      
      //create Matrix
      math::DynMatrix<bool> remainingMatrix(surfaces.size(), surfaces.size(), false);
      for(unsigned int x=numCluster; x<surfaces.size(); x++){
        std::vector<int> nb = neighbours[x-numCluster];
        for(unsigned int y=0; y<nb.size(); y++){
          remainingMatrix(x,nb[y])=true;//nb-1
          remainingMatrix(nb[y],x)=true;//nb-1
        }
      }
      return remainingMatrix;                  
    }

    
    void RemainingPointsFeatureExtractor::calculateLocalMinima(const core::Img32f &depthImage, core::Img8u &maskImage, int radius){      
      int w=depthImage.getSize().width;
      int h=depthImage.getSize().height;
      core::Channel8u maskImageC = maskImage[0];
      core::Channel32f depthImageC = depthImage[0];
      int ii=radius;
      for(int y=0; y<h; y++){ 
        for(int x=0; x<w; x++){
          if(maskImageC(x,y)==0){
            bool localMin=false;      
            if(x>=ii && x<w-ii){
              localMin=(depthImageC(x-ii,y)<depthImageC(x,y) && depthImageC(x+ii,y)<depthImageC(x,y));
            }else if(y>=ii && y<h-ii && localMin==false){
                localMin=(depthImageC(x,y-ii)<depthImageC(x,y) && depthImageC(x,y+ii)<depthImageC(x,y));
            }else if(x>=ii && x<w-ii && y>=ii && y<h-ii && localMin==false){
                localMin=(depthImageC(x-ii,y+ii)<depthImageC(x,y) && depthImageC(x+ii,y-ii)<depthImageC(x,y));
            }else if(x>=ii && x<w-ii && y>=ii && y<h-ii && localMin==false){
                localMin=(depthImageC(x+ii,y+ii)<depthImageC(x,y) && depthImageC(x-ii,y-ii)<depthImageC(x,y));
            }
            if(localMin){
              maskImageC(x,y)=1;
            }            
          }
        }
      }
    }
    
    
    void RemainingPointsFeatureExtractor::clusterRemainingPoints(core::DataSegment<float,4> &xyz, std::vector<std::vector<int> > &surfaces, core::Img32s &labelImage, core::Img8u &maskImage, 
                                           int minSize, float euclideanDistance, int numCluster){
      //cluster remaining points by euclidean distance
      cv::RegionGrower rg; 
      const core::Img32s &result = rg.applyFloat4EuclideanDistance(xyz, maskImage, euclideanDistance, minSize, numCluster+1);      
      std::vector<std::vector<int> > regions=rg.getRegions();       
      surfaces.insert(surfaces.end(), regions.begin(), regions.end()); 
      
      //relabel the label image
      utils::Size s=labelImage.getSize();
      core::Channel32s labelImageC = labelImage[0];
      core::Channel32s resultC = result[0];
      for(int y=0; y<s.height; y++){
        for(int x=0; x<s.width; x++){
          if(labelImageC(x,y)==0){
            labelImageC(x,y)=resultC(x,y);
          }
        }     
      }
    } 
    
    
    void RemainingPointsFeatureExtractor::detectNeighbours(core::DataSegment<float,4> &xyz, std::vector<std::vector<int> > &surfaces, core::Img32s &labelImage, std::vector<std::vector<int> > &neighbours, 
                                     std::vector<std::vector<int> > &neighboursPoints, int numCluster, float assignEuclideanDistance){
      utils::Size s = labelImage.getSize();
      core::Channel32s labelImageC = labelImage[0];
      //determine neighbouring surfaces      
      for(unsigned int x=numCluster; x<surfaces.size(); x++){ 
        std::vector<int> nb;//neighbours
        std::vector<int> nbPoints;//number of connecting points
        for(unsigned int y=0; y<surfaces[x].size(); y++){
          for(int p=-1; p<=1; p++){//all 8 neighbours
            for(int q=-1; q<=1; q++){
              int p1 = surfaces[x][y];
              int p2 = surfaces[x][y]+p+s.width*q;
              if(p2>=0 && p2<s.width*s.height && p1!=p2 && labelImageC[p1]>labelImageC[p2] && labelImageC[p2]!=0){//bounds, id, value, not 0
                if(checkNotExist(labelImageC[p2]-1, nb, nbPoints) && math::dist3(xyz[p1], xyz[p2])<assignEuclideanDistance){// /4.
                  nb.push_back(labelImageC[p2]-1);//id, not label-value
                  nbPoints.push_back(1); 
                }
              }
            }
          }
        }
        neighbours.push_back(nb);
        neighboursPoints.push_back(nbPoints);  
      }
    }
        
        
    bool RemainingPointsFeatureExtractor::checkNotExist(int zw, std::vector<int> &nb, std::vector<int> &nbPoints){   
      if(zw!=0){
        for(unsigned int z=0; z<nb.size(); z++){
          if(nb[z]==zw){
            nbPoints[z]++;
            return false;
          }
        }
        return true;
      }
      return false;
    }


    void RemainingPointsFeatureExtractor::ruleBasedAssignment(core::DataSegment<float,4> &xyz, core::Img32s &labelImage, std::vector<std::vector<int> > &surfaces, std::vector<std::vector<int> > &segments, 
                                        std::vector<std::vector<int> > &neighbours, std::vector<std::vector<int> > &neighboursPoints, int numCluster, int supportTolerance){
                                        
      std::vector<int> assignment = segmentMapping(segments, surfaces.size());
      
      for(unsigned int x=numCluster; x<surfaces.size(); x++){ 
        std::vector<int> nb = neighbours[x-numCluster];
        std::vector<int> nbPoints = neighboursPoints[x-numCluster];
        if(nb.size()==0){ //no neighbours -> new segment
          std::vector<int> seg;
          seg.push_back(x);
          segments.push_back(seg);
          assignment[x] = segments.size()-1;
        }           
        else if(nb.size()==1 && surfaces[x].size()<15){ //very small -> assign
          segments[assignment[nb[0]]].push_back(x);
          assignment[x]=assignment[nb[0]];
        }                 
        else if(nb.size()==1){
          bool supported = checkSupport(labelImage, surfaces[x], nb[0], supportTolerance); 
          if(supported){ //new blob
          //if(nbPoints[0]<9){ //new blob (weak connectivity)
            std::vector<int> seg;
            seg.push_back(x);
            segments.push_back(seg);
            assignment[x] = segments.size()-1;           
          }
          else{ //assign
            segments[assignment[nb[0]]].push_back(x);
            assignment[x]=assignment[nb[0]];
          }
        }                 
        else if(nb.size()>1){
          bool same=true;
          for(unsigned int a=1; a<nb.size(); a++){
            if(assignment[nb[a]]!=assignment[nb[0]]){
              same=false;
            }
          }
          if(same==true){ //same blob->assign
            for(unsigned int p=0; p<nb.size(); p++){
              segments[assignment[nb[p]]].push_back(x);
              assignment[x]=assignment[nb[p]];
            }
          }
          else{ //different blob -> determine best match by RANSAC            
            int bestNeighbourID = ransacAssignment(xyz, surfaces, nb, x);           
            //assign to neighbour with smallest error
            segments[assignment[nb[bestNeighbourID]]].push_back(x);
            assignment[x]=assignment[nb[bestNeighbourID]];
          }
        }
      }                                                      
    }
    
    
    std::vector<int> RemainingPointsFeatureExtractor::segmentMapping(std::vector<std::vector<int> > &segments, int numSurfaces){
      //mapping for faster calculation
      std::vector<int> assignment (numSurfaces,0);
      for(unsigned int i=0; i<segments.size(); i++){
        for(unsigned int j=0; j<segments[i].size(); j++){
          assignment[segments[i][j]]=i;
        }
      }
      return assignment;
    }
    
    
    int RemainingPointsFeatureExtractor::ransacAssignment(core::DataSegment<float,4> &xyz, std::vector<std::vector<int> > &surfaces, std::vector<int> &nb, int x){
      std::vector<std::vector<Vec> > n0;
      std::vector<std::vector<float> > dist;
      for(unsigned int i=0; i<nb.size(); i++){//calculate RANSAC models on neighbours
        std::vector<Vec> n00(10);
        std::vector<float> dist0(10);
        
        PlanarRansacEstimator::calculateRandomModels(xyz, surfaces[nb[i]], n00, dist0, 10);
        n0.push_back(n00);
        dist.push_back(dist0);  
      }
      float bestNeighbourScore=10000000;
      int bestNeighbourID=0;
      for(unsigned int i=0; i<n0.size(); i++){//neighbours
        float bestPassScore=10000000;
        for(unsigned int j=0; j<n0[i].size(); j++){//passes
          float passScore=0;
          for(unsigned int p=0; p<surfaces[x].size(); p++){
            float s1 = (xyz[surfaces[x][p]][0]*n0[i][j][0]+
                        xyz[surfaces[x][p]][1]*n0[i][j][1]+
                        xyz[surfaces[x][p]][2]*n0[i][j][2])-dist[i][j];      
            passScore+=fabs(s1);
          }
          passScore/=surfaces[x].size();
          if(passScore<bestPassScore){
            bestPassScore=passScore;
          }
        }
        if(bestPassScore<bestNeighbourScore){
          bestNeighbourScore=bestPassScore;
          bestNeighbourID=i;
        }
      }
      return bestNeighbourID;
    }
    
    bool RemainingPointsFeatureExtractor::checkSupport(core::Img32s &labelImage, std::vector<int> &surface, int neighbourID, int supportTolerance){
      int count=0;
      utils::Size s = labelImage.getSize();
      core::Channel32s labelImageC = labelImage[0];
      for(unsigned int y=0; y<surface.size(); y++){
        for(int p=-1; p<=1; p++){//all 8 neighbours
          for(int q=-1; q<=1; q++){
            int p1 = surface[y];
            int p2 = surface[y]+p+s.width*q;
            if(p2>=0 && p2<s.width*s.height && p1!=p2 && labelImageC[p1]!=labelImageC[p2] && labelImageC[p2]!=0 && labelImageC[p2]-1!=neighbourID){//bounds, id-self, value-self, not 0, value-neighb.
              if(count<supportTolerance){
                count++;
              }else{
                return false;
              }
            }
          }
        }
      }
      return true;
    }
      
  } // namespace geom
}
