/***********************************************************************
 **                Image Component Library (ICL)                       **
 **                                                                    **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld             **
 **                         Neuroinformatics Group                     **
 ** Website: www.iclcv.org and                                         **
 **          http://opensource.cit-ec.de/projects/icl                  **
 **                                                                    **
 ** File   : ICLGeom/src/ICLGeom/ConfigurableDepthImageSegmenter.cpp   **
 ** Module : ICLGeom                                                   **
 ** Authors: Andre Ueckermann                                          **
 **                                                                    **
 **                                                                    **
 ** GNU LESSER GENERAL PUBLIC LICENSE                                  **
 ** This file may be used under the terms of the GNU Lesser General    **
 ** Public License version 3.0 as published by the                     **
 **                                                                    **
 ** Free Software Foundation and appearing in the file LICENSE.LGPL    **
 ** included in the packaging of this file.  Please review the         **
 ** following information to ensure the license requirements will      **
 ** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                   **
 **                                                                    **
 ** The development of this software was supported by the              **
 ** Excellence Cluster EXC 277 Cognitive Interaction Technology.       **
 ** The Excellence Cluster EXC 277 is a grant of the Deutsche          **
 ** Forschungsgemeinschaft (DFG) in the context of the German          **
 ** Excellence Initiative.                                             **
 **                                                                    **
 ***********************************************************************/

#include <icl/geom/ConfigurableDepthImageSegmenter.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/geom/FeatureGraphSegmenter.h>
#include <icl/geom/PointCloudCreator.h>
#include <icl/geom/ObjectEdgeDetector.h>
#include <icl/filter/MotionSensitiveTemporalSmoothing.h>

namespace icl::geom {
  struct ConfigurableDepthImageSegmenter::Data {
				Data(Mode mode, Camera depthCam, Camera colorCam,
						 PointCloudCreator::DepthImageMode depth_mode){
          depthCamera=depthCam;
						creator = new PointCloudCreator(depthCam, colorCam, depth_mode);
						use_extern_edge_image = false;
          init(mode);
	    }

				Data(Mode mode, Camera depthCam,
						 PointCloudCreator::DepthImageMode depth_mode){
          depthCamera=depthCam;
						creator = new PointCloudCreator(depthCam, depth_mode);
						use_extern_edge_image = false;
          init(mode);
      }

	    ~Data() {
	    }

    void init(Mode mode) {
        temporalSmoothing = new filter::MotionSensitiveTemporalSmoothing(2047, 15);

        if(mode==CPU){
          temporalSmoothing->setUseCL(false);
          objectEdgeDetector = new ObjectEdgeDetector(ObjectEdgeDetector::CPU);
          segmentation = new FeatureGraphSegmenter(FeatureGraphSegmenter::CPU);
        }else{
          temporalSmoothing->setUseCL(true);
          objectEdgeDetector = new ObjectEdgeDetector(ObjectEdgeDetector::BEST);
          segmentation = new FeatureGraphSegmenter(FeatureGraphSegmenter::BEST);
        }
    }

    PointCloudCreator *creator;
    ObjectEdgeDetector *objectEdgeDetector;
    filter::MotionSensitiveTemporalSmoothing *temporalSmoothing;
    FeatureGraphSegmenter *segmentation;

    Camera depthCamera;

    core::Img8u edgeImage;
    core::Img8u normalImage;

			bool use_extern_edge_image;
  };


		ConfigurableDepthImageSegmenter::ConfigurableDepthImageSegmenter(Mode mode, Camera depthCam,
																																		 PointCloudCreator::DepthImageMode depth_mode) :
				m_data(new Data(mode, depthCam, depth_mode)){
      initProperties();
  }

		ConfigurableDepthImageSegmenter::ConfigurableDepthImageSegmenter(Mode mode, Camera depthCam, Camera colorCam,
																																		 icl::geom::PointCloudCreator::DepthImageMode depth_mode) :
					m_data(new Data(mode, depthCam, colorCam, depth_mode)){
      initProperties();
  }

  void ConfigurableDepthImageSegmenter::initProperties() {
      addProperty("general.enable segmentation",utils::prop::Flag{}, true);
      addProperty("general.stabelize segmentation",utils::prop::Flag{}, true);
      addProperty("general.depth scaling",utils::prop::Range{.min=0.9f, .max=1.1f}, 1.05);
      addProperty("general.use ROI",utils::prop::Flag{}, false);
      addProperty("general.ROI min x",utils::prop::Range{.min=-1500, .max=500}, -1200);
      addProperty("general.ROI max x",utils::prop::Range{.min=-500, .max=1500}, 1200);
      addProperty("general.ROI min y",utils::prop::Range{.min=-1500, .max=800}, -100);
      addProperty("general.ROI max y",utils::prop::Range{.min=-500, .max=1500}, 1050);
      addProperty("general.ROI min z",utils::prop::Range{.min=-500, .max=500}, 0);
      addProperty("general.ROI max z",utils::prop::Range{.min=0, .max=1500}, 1050);

      addProperty("pre.enable temporal smoothing",utils::prop::Flag{}, true);
      addProperty("pre.temporal smoothing size",utils::prop::Range{.min=1, .max=15, .step=1}, 6);
      addProperty("pre.temporal smoothing diff",utils::prop::Range{.min=1, .max=22, .step=1}, 10);
      addProperty("pre.filter",utils::prop::Menu{"unfiltered", "median3x3", "median5x5"}, "median3x3");
      addProperty("pre.normal range",utils::prop::Range{.min=1, .max=15, .step=1}, 1);
      addProperty("pre.averaging",utils::prop::Flag{}, true);
      addProperty("pre.averaging range",utils::prop::Range{.min=1, .max=15, .step=1}, 2);
      addProperty("pre.smoothing",utils::prop::Menu{"linear", "gaussian"}, "linear");
      addProperty("pre.edge threshold",utils::prop::Range{.min=0.7f, .max=1.f}, 0.89);
      addProperty("pre.edge angle method",utils::prop::Menu{"max", "mean"}, "mean");
      addProperty("pre.edge neighborhood",utils::prop::Range{.min=1, .max=15}, 1);

      addProperty("surfaces.min surface size",utils::prop::Range{.min=5, .max=75, .step=1}, 50);
      addProperty("surfaces.assignment radius",utils::prop::Range{.min=2, .max=15, .step=1}, 7);
      addProperty("surfaces.assignment distance",utils::prop::Range{.min=3.0f, .max=25.0f}, 15.0);

      addProperty("cutfree.enable cutfree adjacency feature",utils::prop::Flag{}, true);
      addProperty("cutfree.ransac euclidean distance",utils::prop::Range{.min=2.0f, .max=20.0f}, 8.0);
      addProperty("cutfree.ransac passes",utils::prop::Range{.min=5, .max=50, .step=1}, 20);
      addProperty("cutfree.ransac tolerance",utils::prop::Range{.min=5, .max=50, .step=1}, 30);
      addProperty("cutfree.min angle",utils::prop::Range{.min=0.0f, .max=70.0f}, 30.0);

      addProperty("coplanarity.enable coplanarity feature",utils::prop::Flag{}, true);
      addProperty("coplanarity.max angle",utils::prop::Range{.min=0.0f, .max=60.0f}, 30.0);
      addProperty("coplanarity.distance tolerance",utils::prop::Range{.min=1.0f, .max=10.0f}, 3.0);
      addProperty("coplanarity.outlier tolerance",utils::prop::Range{.min=1.0f, .max=10.0f}, 5.0);
      addProperty("coplanarity.num triangles",utils::prop::Range{.min=10, .max=50, .step=1}, 20);
      addProperty("coplanarity.num scanlines",utils::prop::Range{.min=1, .max=20, .step=1}, 9);

      addProperty("curvature.enable curvature feature",utils::prop::Flag{}, true);
      addProperty("curvature.histogram similarity",utils::prop::Range{.min=0.1f, .max=1.0f}, 0.5);
      addProperty("curvature.enable open objects",utils::prop::Flag{}, true);
      addProperty("curvature.max distance",utils::prop::Range{.min=1, .max=20, .step=1}, 10);
      addProperty("curvature.enable occluded objects",utils::prop::Flag{}, true);
      addProperty("curvature.max error",utils::prop::Range{.min=1.0f, .max=20.0f}, 10.0);
      addProperty("curvature.ransac passes",utils::prop::Range{.min=5, .max=50, .step=1}, 20);
      addProperty("curvature.distance tolerance",utils::prop::Range{.min=1.0f, .max=10.0f}, 3.0);
      addProperty("curvature.outlier tolerance",utils::prop::Range{.min=1.0f, .max=10.0f}, 5.0);

      addProperty("remaining.enable remaining points feature",utils::prop::Flag{}, true);
      addProperty("remaining.min size",utils::prop::Range{.min=5, .max=50, .step=1}, 10);
      addProperty("remaining.euclidean distance",utils::prop::Range{.min=2.0f, .max=20.0f}, 10.0);
      addProperty("remaining.radius",utils::prop::Range{.min=0, .max=10, .step=1}, 0);
      addProperty("remaining.assign euclidean distance",utils::prop::Range{.min=2.0f, .max=20.0f}, 10.0);
      addProperty("remaining.support tolerance",utils::prop::Range{.min=0, .max=30, .step=1}, 9);

      addProperty("graphcut.threshold",utils::prop::Range{.min=0.0f, .max=1.1f}, 0.5);

      setConfigurableID("segmentation");
  }


  ConfigurableDepthImageSegmenter::~ConfigurableDepthImageSegmenter(){
    delete m_data;
  }


  void ConfigurableDepthImageSegmenter::apply(const core::Img32f &depthImage, PointCloudObject &obj){// const core::Img8u &colorImage){

    //pre segmentation
    m_data->temporalSmoothing->setFilterSize(prop("pre.temporal smoothing size").value);
    m_data->temporalSmoothing->setDifference(prop("pre.temporal smoothing diff").value);
    static core::ImgBase *filteredImage = 0;
    bool useTempSmoothing = prop("pre.enable temporal smoothing").value;
    if(useTempSmoothing==true){//temporal smoothing
      m_data->temporalSmoothing->apply(&depthImage,&filteredImage);
    }

    std::string usedFilter=prop("pre.filter").value;
    bool useAveraging=prop("pre.averaging").value;
    std::string usedAngle=prop("pre.edge angle method").value;
    std::string usedSmoothing=prop("pre.smoothing").value;
    int normalrange = prop("pre.normal range").value;
    int neighbrange = prop("pre.edge neighborhood").value;
    float threshold = prop("pre.edge threshold").value;
    int avgrange = prop("pre.averaging range").value;

    bool usedFilterFlag=true;
    if(usedFilter.compare("median3x3")==0){ //median 3x3
      m_data->objectEdgeDetector->setMedianFilterSize(3);
    }
    else if(usedFilter.compare("median5x5")==0){ //median 5x5
      m_data->objectEdgeDetector->setMedianFilterSize(5);
    }else{
      usedFilterFlag=false;
    }

    m_data->objectEdgeDetector->setNormalCalculationRange(normalrange);
    m_data->objectEdgeDetector->setNormalAveragingRange(avgrange);

    if(usedAngle.compare("max")==0){//max
      m_data->objectEdgeDetector->setAngleNeighborhoodMode(0);
    }
    else if(usedAngle.compare("mean")==0){//mean
      m_data->objectEdgeDetector->setAngleNeighborhoodMode(1);
    }

    bool usedSmoothingFlag = true;
    if(usedSmoothing.compare("linear")==0){//linear
      usedSmoothingFlag = false;
    }
    else if(usedSmoothing.compare("gaussian")==0){//gauss
      usedSmoothingFlag = true;
    }

    m_data->objectEdgeDetector->setAngleNeighborhoodRange(neighbrange);
    m_data->objectEdgeDetector->setBinarizationThreshold(threshold);

			if (!m_data->use_extern_edge_image) {
				if(useTempSmoothing==true){
					m_data->edgeImage=m_data->objectEdgeDetector->calculate(*filteredImage->as32f(), usedFilterFlag,
																								 useAveraging, usedSmoothingFlag);
				}else{
					m_data->edgeImage=m_data->objectEdgeDetector->calculate(*depthImage.as32f(), usedFilterFlag,
																								 useAveraging, usedSmoothingFlag);
				}
				m_data->objectEdgeDetector->applyWorldNormalCalculation(m_data->depthCamera);
				m_data->normalImage=m_data->objectEdgeDetector->getRGBNormalDisplay();
			}


    obj.lock();

    //create pointcloud
    float depthScaling=prop("general.depth scaling").value;
    GeomColor c(1.,0.,0.,1.);
	    obj.selectRGBA32f().fill(c);
    if(useTempSmoothing==true){
      m_data->creator->create(*filteredImage->as32f(), obj, 0, depthScaling);
    }else{
      m_data->creator->create(*depthImage.as32f(), obj, 0, depthScaling);
    }

    //high level segmenation
    bool enableSegmentation = prop("general.enable segmentation").value;

    if(enableSegmentation){
      bool useROI = prop("general.use ROI").value;
      bool stabelizeSegmentation = prop("general.stabelize segmentation").value;

      int surfaceMinSize = prop("surfaces.min surface size").value;
      int surfaceRadius = prop("surfaces.assignment radius").value;
      float surfaceDistance = prop("surfaces.assignment distance").value;

      bool cutfreeEnable = prop("cutfree.enable cutfree adjacency feature").value;
      float cutfreeEuclDist = prop("cutfree.ransac euclidean distance").value;
      int cutfreePasses = prop("cutfree.ransac passes").value;
      int cutfreeTolerance = prop("cutfree.ransac tolerance").value;
      float cutfreeMinAngle = prop("cutfree.min angle").value;

      bool coplanEnable = prop("coplanarity.enable coplanarity feature").value;
      float coplanMaxAngle = prop("coplanarity.max angle").value;
      float coplanDistance = prop("coplanarity.distance tolerance").value;
      float coplanOutlier = prop("coplanarity.outlier tolerance").value;
      int coplanNumTriangles = prop("coplanarity.num triangles").value;
      int coplanNumScanlines = prop("coplanarity.num scanlines").value;

      bool curveEnable = prop("curvature.enable curvature feature").value;
      float curveHistogram = prop("curvature.histogram similarity").value;
      bool curveUseOpen = prop("curvature.enable open objects").value;
      int curveMaxDist = prop("curvature.max distance").value;
      bool curveUseOccluded = prop("curvature.enable occluded objects").value;
      float curveMaxError = prop("curvature.max error").value;
      int curvePasses = prop("curvature.ransac passes").value;
      float curveDistance = prop("curvature.distance tolerance").value;
      float curveOutlier = prop("curvature.outlier tolerance").value;

      bool remainingEnable = prop("remaining.enable remaining points feature").value;
      int remainingMinSize = prop("remaining.min size").value;
      float remainingEuclDist = prop("remaining.euclidean distance").value;
      int remainingRadius = prop("remaining.radius").value;
      float remainingAssignEuclDist = prop("remaining.assign euclidean distance").value;
      int remainingSupportTolerance = prop("remaining.support tolerance").value;

      float graphcutThreshold = prop("graphcut.threshold").value;

      if(useROI){
        m_data->segmentation->setROI(prop("general.ROI min x").value,
                                    prop("general.ROI max x").value,
                                    prop("general.ROI min y").value,
                                    prop("general.ROI max y").value,
                                    prop("general.ROI min z").value,
                                    prop("general.ROI max z").value);

      }
      m_data->segmentation->setMinSurfaceSize(surfaceMinSize);
      m_data->segmentation->setAssignmentParams(surfaceDistance, surfaceRadius);
      m_data->segmentation->setCutfreeParams(cutfreeEuclDist, cutfreePasses, cutfreeTolerance, cutfreeMinAngle);
      m_data->segmentation->setCoplanarityParams(coplanMaxAngle, coplanDistance, coplanOutlier, coplanNumTriangles, coplanNumScanlines);
      m_data->segmentation->setCurvatureParams(curveHistogram, curveUseOpen, curveMaxDist, curveUseOccluded, curveMaxError,
                              curvePasses, curveDistance, curveOutlier);
      m_data->segmentation->setRemainingPointsParams(remainingMinSize, remainingEuclDist, remainingRadius, remainingAssignEuclDist, remainingSupportTolerance);
      m_data->segmentation->setGraphCutThreshold(graphcutThreshold);

				if(useTempSmoothing){
					core::Img8u lI=m_data->segmentation->apply(obj.selectXYZH(),m_data->edgeImage,*filteredImage->as32f(), m_data->objectEdgeDetector->getNormals(),
																										 stabelizeSegmentation, useROI, cutfreeEnable, coplanEnable, curveEnable, remainingEnable);
					obj.setColorsFromImage(lI);
      }else{
					core::Img8u lI=m_data->segmentation->apply(obj.selectXYZH(), m_data->edgeImage, *depthImage.as32f(), m_data->objectEdgeDetector->getNormals(),
																										 stabelizeSegmentation, useROI, cutfreeEnable, coplanEnable, curveEnable, remainingEnable);
					obj.setColorsFromImage(lI);
      }
    }

  obj.unlock();
  }

	std::vector<geom::SurfaceFeatureExtractor::SurfaceFeature>
	ConfigurableDepthImageSegmenter::getSurfaceFeatures() {
		return m_data->segmentation->getSurfaceFeatures();
	}

  const core::DataSegment<float,4> ConfigurableDepthImageSegmenter::getNormalSegment() {
		return m_data->objectEdgeDetector->getNormals();
  }

	const core::Img32f ConfigurableDepthImageSegmenter::getAngleDisplay() {
		return m_data->objectEdgeDetector->getAngleDisplay();
	}

  const core::Img8u ConfigurableDepthImageSegmenter::getNormalDisplay(){
    return m_data->normalImage;
  }


  const core::Img8u ConfigurableDepthImageSegmenter::getEdgeDisplay(){
    return m_data->edgeImage;
  }


	  core::Img32s ConfigurableDepthImageSegmenter::getLabelDisplay(){
	    return m_data->segmentation->getLabelImage(prop("general.stabelize segmentation").value);
	  }


  core::Img8u ConfigurableDepthImageSegmenter::getColoredLabelDisplay(){
    return m_data->segmentation->getColoredLabelImage(prop("general.stabelize segmentation").value);
  }

  core::Img8u ConfigurableDepthImageSegmenter::getMappedColorImage(const core::Img8u &image) {
      core::Img8u mapped(image.getParams());
      if (!m_data->creator->hasColorCamera())
          return mapped;
      m_data->creator->mapImage(&image,bpp(mapped));
      return mapped;
  }

	void ConfigurableDepthImageSegmenter::mapImageToDepth(const core::ImgBase *src, core::ImgBase **dst) {
		m_data->creator->mapImage(src,dst);
	}

	void ConfigurableDepthImageSegmenter::setNormals(core::DataSegment<float,4> &normals) {
		m_data->objectEdgeDetector->setNormals(normals);
	}

	void ConfigurableDepthImageSegmenter::setEdgeSegData(core::Img8u &edges, core::Img8u &normal_img) {
		m_data->edgeImage = edges;
		m_data->normalImage = normal_img;
	}

	void ConfigurableDepthImageSegmenter::setUseExternalEdges(bool use_external_edges) {
		m_data->use_extern_edge_image = use_external_edges;
	}

  std::vector<std::vector<int> > ConfigurableDepthImageSegmenter::getSurfaces(){
    return m_data->segmentation->getSurfaces();
  }


  std::vector<std::vector<int> > ConfigurableDepthImageSegmenter::getSegments(){
    return m_data->segmentation->getSegments();
  }

  } // namespace icl::geom