/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLGeom/src/ICLGeom/EuclideanBlobSegmenter.cpp         **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLCV/RegionGrower.h>
#include <ICLGeom/EuclideanBlobSegmenter.h>
#include <ICLGeom/PointCloudObjectBase.h>
#include <ICLGeom/PlanarRansacEstimator.h>
#include <ICLGeom/SegmenterUtils.h>


namespace icl {
  namespace geom {
    using namespace core;
    struct EuclideanBlobSegmenter::Data {
	    Data(Mode mode) {
        if(mode==BEST || mode==GPU){
          ransac=new PlanarRansacEstimator(PlanarRansacEstimator::GPU);
	        segUtils=new SegmenterUtils(SegmenterUtils::GPU);
        }else{
          ransac=new PlanarRansacEstimator(PlanarRansacEstimator::CPU);
	        segUtils=new SegmenterUtils(SegmenterUtils::CPU);
        }
        
        minClusterSize = 25;
	      RANSACeuclDistance = 5.0;
	      RANSACpasses = 20;
	      RANSACtolerance = 30;
	      RANSACsubset = 2;
	      BLOBSeuclDistance = 15;
	      
	      xMinROI = 0, xMaxROI = 0;
	      yMinROI = 0, yMaxROI = 0;
	      zMinROI = 0, zMaxROI = 0;
	    }

	    ~Data() {
	    }
      
      PlanarRansacEstimator* ransac;
      SegmenterUtils* segUtils;
      
	    float xMinROI, xMaxROI, yMinROI, yMaxROI, zMinROI, zMaxROI;
	    std::vector<std::vector<int> > surfaces;
      std::vector<std::vector<int> > blobs;
      unsigned int minClusterSize;
      int RANSACeuclDistance; 
      int RANSACpasses; 
      int RANSACtolerance; 
      int RANSACsubset;
      int BLOBSeuclDistance;
      
      core::DataSegment<float,4> xyzData;
      core::Img8u normalEdgeImage;
      core::Img32f depthImage;
      core::Img8u maskImage;
      core::Img8u maskImageBlobs;
      core::Img32s labelImage;
      core::Img32s labelImageSurface;
    };
    
    
    EuclideanBlobSegmenter::EuclideanBlobSegmenter(Mode mode) :
	    m_data(new Data(mode)) {
    }


    EuclideanBlobSegmenter::~EuclideanBlobSegmenter() {
	    delete m_data;
    }
    

    Img8u EuclideanBlobSegmenter::apply(DataSegment<float, 4> xyz, const Img8u &edgeImg, 
                                        const Img32f &depthImg, bool stabelize, bool useROI) {
	    m_data->xyzData = xyz;
	    m_data->normalEdgeImage = edgeImg;
	    m_data->depthImage = depthImg;
	
	    regionGrow(useROI);
	    blobSegmentation(useROI);
	    return getColoredLabelImage(stabelize);
    }


    void EuclideanBlobSegmenter::setROI(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax) {
	    m_data->xMinROI = xMin;
	    m_data->xMaxROI = xMax;
	    m_data->yMinROI = yMin;
	    m_data->yMaxROI = yMax;
	    m_data->zMinROI = zMin;
	    m_data->zMaxROI = zMax;
    }


    void EuclideanBlobSegmenter::setMinClusterSize(unsigned int size) {
	    m_data->minClusterSize = size;
    }


    void EuclideanBlobSegmenter::setRansacParams(float distance, int passes, int subset){
      m_data->RANSACeuclDistance=distance;
      m_data->RANSACpasses=passes;
      m_data->RANSACsubset=subset;
    }


    void EuclideanBlobSegmenter::setBLOBSeuclDistance(int distance) {
	    m_data->BLOBSeuclDistance = distance;
    }


    Img32s EuclideanBlobSegmenter::getLabelImage(bool stabelize) {
	    if(stabelize){
	      return m_data->segUtils->stabelizeSegmentation(m_data->labelImage);
	    }
	    return m_data->labelImage;
    }


    core::Img8u EuclideanBlobSegmenter::getColoredLabelImage(bool stabelize) {
      Img32s lI=getLabelImage(stabelize);
      return m_data->segUtils->createColorImage(lI);
    }


    std::vector<std::vector<int> > EuclideanBlobSegmenter::getSurfaces() {
	    return m_data->surfaces;
    }


    std::vector<std::vector<int> > EuclideanBlobSegmenter::getBlobs() {
	    return m_data->blobs;
    }


    void EuclideanBlobSegmenter::regionGrow(bool useROI) {
      m_data->surfaces.clear();
      
      //create mask
      if(useROI){
		    m_data->maskImage=m_data->segUtils->createROIMask(m_data->xyzData, m_data->depthImage, m_data->xMinROI, 
		            m_data->xMaxROI, m_data->yMinROI, m_data->yMaxROI, m_data->zMinROI, m_data->zMaxROI);
		  }else{
		    m_data->maskImage=m_data->segUtils->createMask(m_data->depthImage);
		  } 

      //region growing on edge image
      cv::RegionGrower rg;
    
      m_data->labelImage = rg.applyEqualThreshold(m_data->normalEdgeImage, m_data->maskImage, 255, m_data->minClusterSize);        
      m_data->surfaces=rg.getRegions();      
    }


    void EuclideanBlobSegmenter::blobSegmentation(bool useROI) {
      //find biggest surface (support plane)
      int maxID=-1;
      unsigned int maxSize=0;
      for(unsigned int i=0; i<m_data->surfaces.size(); i++){
        if(m_data->surfaces.at(i).size()>maxSize){
          maxID=i;
          maxSize=m_data->surfaces.at(i).size();
        }
      }
      
      //RANSAC with the plane (find model) 
      if(maxID < 0) return;
      
      PlanarRansacEstimator::Result result=m_data->ransac->apply(m_data->xyzData, 
                m_data->surfaces.at(maxID), m_data->surfaces.at(maxID), m_data->RANSACeuclDistance/2, m_data->RANSACpasses, 
                m_data->RANSACsubset, m_data->RANSACtolerance, PlanarRansacEstimator::MAX_ON);
      
      //create mask
      if(useROI){
		    m_data->maskImageBlobs=m_data->segUtils->createROIMask(m_data->xyzData, m_data->depthImage, m_data->xMinROI, 
		            m_data->xMaxROI, m_data->yMinROI, m_data->yMaxROI, m_data->zMinROI, m_data->zMaxROI);
		  }else{
		    m_data->maskImageBlobs=m_data->segUtils->createMask(m_data->depthImage);
		  } 
      
      //assign points to the surface
      m_data->labelImageSurface.setChannels(1);
      m_data->labelImageSurface.setSize(m_data->depthImage.getSize());      
      m_data->ransac->relabel(m_data->xyzData, m_data->maskImageBlobs, m_data->labelImage, m_data->labelImageSurface, 1, 
                maxID+1, m_data->RANSACeuclDistance, result);

      regionGrowBlobs();    
    }


    void EuclideanBlobSegmenter::regionGrowBlobs() {      
      m_data->blobs.clear();
      //region growing on blobs
      cv::RegionGrower rg; 
      const Img32s &result = rg.applyFloat4EuclideanDistance(m_data->xyzData, m_data->maskImageBlobs, m_data->BLOBSeuclDistance, m_data->minClusterSize, 2);
      utils::Size s=m_data->depthImage.getSize();
      
      //relabel the label image
      for(int y=0; y<s.height; y++){
        for(int x=0; x<s.width; x++){
          if(m_data->labelImageSurface(x,y,0)!=1){
            m_data->labelImage(x,y,0)=result(x,y,0);
          }
          else{
            m_data->labelImage(x,y,0)=1;
          }
        }     
      }
      m_data->blobs=rg.getRegions();  
    }

  } // namespace geom
}
