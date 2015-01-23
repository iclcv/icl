/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLGeom/src/ICLGeom/FeatureGraphSegmenter.cpp          **
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

#include <ICLGeom/FeatureGraphSegmenter.h>

#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/PointCloudObjectBase.h>
#include <ICLGeom/CutfreeAdjacencyFeatureExtractor.h>
#include <ICLGeom/CoPlanarityFeatureExtractor.h>
#include <ICLGeom/CurvatureFeatureExtractor.h>
#include <ICLGeom/RemainingPointsFeatureExtractor.h>
#include <ICLGeom/SegmenterUtils.h>
#include <ICLMath/GraphCutter.h>
#include <ICLCV/RegionDetector.h>
#include <ICLGeom/PlanarRansacEstimator.h>

namespace icl {
  namespace geom {
    struct FeatureGraphSegmenter::Data {
	    Data(Mode mode) {
        if(mode==BEST || mode==GPU){
          cutfree=new CutfreeAdjacencyFeatureExtractor(CutfreeAdjacencyFeatureExtractor::GPU);
	        segUtils=new SegmenterUtils(SegmenterUtils::GPU);
        }else{
          cutfree=new CutfreeAdjacencyFeatureExtractor(CutfreeAdjacencyFeatureExtractor::CPU);
	        segUtils=new SegmenterUtils(SegmenterUtils::CPU);
        }
  
        region=new cv::RegionDetector(25, 4000000, 254, 255, false);
        
	      xMinROI = 0, xMaxROI = 0;
	      yMinROI = 0, yMaxROI = 0;
	      zMinROI = 0, zMaxROI = 0;
        
        minSurfaceSize = 25;
        
        assignmentRadius=5;
	      assignmentDistance=10;

        cutfreeRansacEuclideanDistance=5.0;
        cutfreeRansacPasses=20;
        cutfreeRansacTolerance=30;
        cutfreeMinAngle=30.;
        
        coplanarityMaxAngle=30.0;
        coplanarityDistanceTolerance=3.0;
        coplanarityOutlierTolerance=5.0;
        coplanarityNumTriangles=20;
        coplanarityNumScanlines=9;
        
        curvatureHistogramSimilarity=0.5;
        curvatureUseOpenObjects=true;
        curvatureMaxDistance=10;
        curvatureUseOccludedObjects=true;
        curvatureMaxError=10.0;
        curvatureRansacPasses=20;
        curvatureDistanceTolerance=3.0;
        curvatureOutlierTolerance=5.0;                        
      
        remainingMinSize=10;
        remainingEuclideanDistance=5.0;
        remainingRadius=6;  
        
        graphCutThreshold=0.5;
	    }

	    ~Data() {
	    }
      
      CutfreeAdjacencyFeatureExtractor* cutfree;
      SegmenterUtils* segUtils;
      cv::RegionDetector* region;
	        
	    std::vector<std::vector<int> > surfaces;
      std::vector<std::vector<int> > segments;

      core::Img8u maskImage;
      core::Img32s labelImage;
      
      float xMinROI, xMaxROI, yMinROI, yMaxROI, zMinROI, zMaxROI;
      
	    unsigned int minSurfaceSize;
      
      int assignmentRadius;
      float assignmentDistance;
      
      float cutfreeRansacEuclideanDistance;
      int cutfreeRansacPasses;
      int cutfreeRansacTolerance;
      float cutfreeMinAngle;
            
      float coplanarityMaxAngle;
      float coplanarityDistanceTolerance;
      float coplanarityOutlierTolerance;
      int coplanarityNumTriangles;
      int coplanarityNumScanlines;
      
      float curvatureHistogramSimilarity;
      bool curvatureUseOpenObjects;
      int curvatureMaxDistance;
      bool curvatureUseOccludedObjects;
      float curvatureMaxError;
      int curvatureRansacPasses;
      float curvatureDistanceTolerance;
      float curvatureOutlierTolerance;                        
    
      int remainingMinSize;
      float remainingEuclideanDistance;
      int remainingRadius;
           
      float graphCutThreshold;
    };
    
    
    FeatureGraphSegmenter::FeatureGraphSegmenter(Mode mode) :
	    m_data(new Data(mode)) {
    }


    FeatureGraphSegmenter::~FeatureGraphSegmenter() {
	    delete m_data;
    }
    

    core::Img8u FeatureGraphSegmenter::apply(core::DataSegment<float, 4> xyz, const core::Img8u &edgeImg, const core::Img32f &depthImg, 
                                  core::DataSegment<float, 4> normals, bool stabelize, bool useROI, 
                                  bool useCutfreeAdjacency, bool useCoplanarity, bool useCurvature, bool useRemainingPoints) {
	
	    surfaceSegmentation(xyz, edgeImg, depthImg, m_data->minSurfaceSize, useROI);
	    	    	    
	    std::vector<SurfaceFeatureExtractor::SurfaceFeature> features=SurfaceFeatureExtractor::apply(m_data->labelImage, xyz, normals, SurfaceFeatureExtractor::ALL);
	    
	    math::DynMatrix<bool> initialMatrix = m_data->segUtils->edgePointAssignmentAndAdjacencyMatrix(xyz, m_data->labelImage, 
                              m_data->maskImage, m_data->assignmentRadius, m_data->assignmentDistance, m_data->surfaces.size());
	    
	    math::DynMatrix<bool> resultMatrix(m_data->surfaces.size(), m_data->surfaces.size(), false);
	    
	    if(useCutfreeAdjacency){
	      math::DynMatrix<bool> cutfreeMatrix = m_data->cutfree->apply(xyz, 
                  m_data->surfaces, initialMatrix, m_data->cutfreeRansacEuclideanDistance, 
                  m_data->cutfreeRansacPasses, m_data->cutfreeRansacTolerance, m_data->labelImage, features, m_data->cutfreeMinAngle);
        math::GraphCutter::mergeMatrix(resultMatrix, cutfreeMatrix);
      }
      
      if(useCoplanarity){
        math::DynMatrix<bool> coplanMatrix = CoPlanarityFeatureExtractor::apply(initialMatrix, features, depthImg, m_data->surfaces, m_data->coplanarityMaxAngle,
                          m_data->coplanarityDistanceTolerance, m_data->coplanarityOutlierTolerance, m_data->coplanarityNumTriangles, m_data->coplanarityNumScanlines);
    	   math::GraphCutter::mergeMatrix(resultMatrix, coplanMatrix);
      }

      if(useCurvature){
        math::DynMatrix<bool> curveMatrix = CurvatureFeatureExtractor::apply(depthImg, xyz, initialMatrix, features, m_data->surfaces, normals, 
                                            m_data->curvatureUseOpenObjects, m_data->curvatureUseOccludedObjects, m_data->curvatureHistogramSimilarity, 
                                            m_data->curvatureMaxDistance, m_data->curvatureMaxError, m_data->curvatureRansacPasses, m_data->curvatureDistanceTolerance, 
                                            m_data->curvatureOutlierTolerance);
        math::GraphCutter::mergeMatrix(resultMatrix, curveMatrix);
      }
	    
	    m_data->segments.clear();
	    m_data->segments = math::GraphCutter::thresholdCut(resultMatrix, m_data->graphCutThreshold);
	    
	    if(useRemainingPoints){
	      RemainingPointsFeatureExtractor::apply(xyz, depthImg, m_data->labelImage, m_data->maskImage, 
                          m_data->surfaces, m_data->segments, m_data->remainingMinSize, m_data->remainingEuclideanDistance, m_data->remainingRadius);
	    }
	    	    
	    m_data->segUtils->relabel(m_data->labelImage, m_data->segments, m_data->surfaces.size());
      
	    return getColoredLabelImage(stabelize);
    }


    void FeatureGraphSegmenter::setROI(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax) {
	    m_data->xMinROI = xMin;
	    m_data->xMaxROI = xMax;
	    m_data->yMinROI = yMin;
	    m_data->yMaxROI = yMax;
	    m_data->zMinROI = zMin;
	    m_data->zMaxROI = zMax;
    }


    void FeatureGraphSegmenter::setMinSurfaceSize(unsigned int size) {
	    m_data->minSurfaceSize = size;
    }
    
    
    void FeatureGraphSegmenter::setAssignmentParams(float distance, int radius){
      m_data->assignmentRadius=radius;
      m_data->assignmentDistance=distance;
    }
    
    
    void FeatureGraphSegmenter::setCutfreeParams(float euclideanDistance, int passes, int tolerance, float minAngle){
      m_data->cutfreeRansacEuclideanDistance=euclideanDistance;
      m_data->cutfreeRansacPasses=passes;
      m_data->cutfreeRansacTolerance=tolerance;
      m_data->cutfreeMinAngle=minAngle;
    }
    
    
    void FeatureGraphSegmenter::setCoplanarityParams(float maxAngle, float distanceTolerance, float outlierTolerance, int numTriangles, int numScanlines){
      m_data->coplanarityMaxAngle=maxAngle;
      m_data->coplanarityDistanceTolerance=distanceTolerance;
      m_data->coplanarityOutlierTolerance=outlierTolerance;
      m_data->coplanarityNumTriangles=numTriangles;
      m_data->coplanarityNumScanlines=numScanlines;
    }
     
      
    void FeatureGraphSegmenter::setCurvatureParams(float histogramSimilarity, bool useOpenObjects, int maxDistance, bool useOccludedObjects, float maxError, 
                            int ransacPasses, float distanceTolerance, float outlierTolerance){
      m_data->curvatureHistogramSimilarity=histogramSimilarity;
      m_data->curvatureUseOpenObjects=useOpenObjects;
      m_data->curvatureMaxDistance=maxDistance;
      m_data->curvatureUseOccludedObjects=useOccludedObjects;
      m_data->curvatureMaxError=maxError;
      m_data->curvatureRansacPasses=ransacPasses;
      m_data->curvatureDistanceTolerance=distanceTolerance;
      m_data->curvatureOutlierTolerance=outlierTolerance;                        
    }
    
    
    void FeatureGraphSegmenter::setRemainingPointsParams(int minSize, float euclideanDistance, int radius){
      m_data->remainingMinSize=minSize;
      m_data->remainingEuclideanDistance=euclideanDistance;
      m_data->remainingRadius=radius;
    }
  
    
    void FeatureGraphSegmenter::setGraphCutThreshold(float threshold){
      m_data->graphCutThreshold=threshold;
    }
    
    
    std::vector<std::vector<int> > FeatureGraphSegmenter::getSegments() {
	    return m_data->segments;
    }
    
    
    std::vector<std::vector<int> > FeatureGraphSegmenter::getSurfaces() {
	    return m_data->surfaces;
    }
   

    core::Img32s FeatureGraphSegmenter::getLabelImage(bool stabelize) {
	    if(stabelize){
	      return m_data->segUtils->stabelizeSegmentation(m_data->labelImage);
	    }
	    return m_data->labelImage;
    }


    core::Img8u FeatureGraphSegmenter::getColoredLabelImage(bool stabelize) {
      core::Img32s lI=getLabelImage(stabelize);
      return m_data->segUtils->createColorImage(lI);
    }

    
    core::Img8u FeatureGraphSegmenter::getMaskImage(){
      return m_data->maskImage;
    }

    
    void FeatureGraphSegmenter::surfaceSegmentation(core::DataSegment<float,4> &xyz, const core::Img8u &edgeImg, 
                                const core::Img32f &depthImg, int minSurfaceSize, bool useROI){
      m_data->surfaces.clear();//clear
      if(useROI){//create mask
		    m_data->maskImage=m_data->segUtils->createROIMask(xyz, (core::Img32f&)depthImg, m_data->xMinROI, 
		            m_data->xMaxROI, m_data->yMinROI, m_data->yMaxROI, m_data->zMinROI, m_data->zMaxROI);
		  }else{
		    m_data->maskImage=m_data->segUtils->createMask((core::Img32f&)depthImg);
		  } 
		  
		  m_data->labelImage.setSize(edgeImg.getSize());
      m_data->labelImage.setChannels(1);
		  m_data->labelImage.fill(0);
		  
      core::Channel8u maskImageC = m_data->maskImage[0];
      core::Channel32s labelImageC = m_data->labelImage[0];
      int w = edgeImg.getSize().width;
      
      int numCluster=0;      
      m_data->region->setConstraints (minSurfaceSize, 4000000, 254, 255);
      std::vector<std::vector<int> > remove;
      std::vector<cv::ImageRegion> regions;
      regions = m_data->region->detect(&edgeImg); 	
      for(unsigned int i=0; i<regions.size(); i++){
        numCluster++;
        std::vector<utils::Point> ps = regions[i].getPixels();
        std::vector<int> data;
        for(unsigned int j=0; j<ps.size(); j++){
          int px = ps[j][0];
          int py = ps[j][1];
          int v=px+w*py;
          if(maskImageC(px,py)==0){
            labelImageC(px,py)=numCluster;
            maskImageC(px,py)=1;
            data.push_back(v);
          }
        }
        if((int)data.size()<minSurfaceSize){
          remove.push_back(data);
          numCluster--;
        }
        else{
          m_data->surfaces.push_back(data);
        }
      }
      for(unsigned int i=0; i<remove.size(); i++){
        for(unsigned int j=0; j<remove.at(i).size(); j++){
          maskImageC[remove[i][j]]=0;
          labelImageC[remove[i][j]]=0;
        }
      }
    }    
          
  } // namespace geom
}
