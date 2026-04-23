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
    m_data->temporalSmoothing->setFilterSize(getPropertyValue("pre.temporal smoothing size"));
    m_data->temporalSmoothing->setDifference(getPropertyValue("pre.temporal smoothing diff"));
    static core::ImgBase *filteredImage = 0;
    bool useTempSmoothing = getPropertyValue("pre.enable temporal smoothing");
    if(useTempSmoothing==true){//temporal smoothing
      m_data->temporalSmoothing->apply(&depthImage,&filteredImage);
    }

    std::string usedFilter=getPropertyValue("pre.filter");
    bool useAveraging=getPropertyValue("pre.averaging");
    std::string usedAngle=getPropertyValue("pre.edge angle method");
    std::string usedSmoothing=getPropertyValue("pre.smoothing");
    int normalrange = getPropertyValue("pre.normal range");
    int neighbrange = getPropertyValue("pre.edge neighborhood");
    float threshold = getPropertyValue("pre.edge threshold");
    int avgrange = getPropertyValue("pre.averaging range");

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
    float depthScaling=getPropertyValue("general.depth scaling");
    GeomColor c(1.,0.,0.,1.);
	    obj.selectRGBA32f().fill(c);
    if(useTempSmoothing==true){
      m_data->creator->create(*filteredImage->as32f(), obj, 0, depthScaling);
    }else{
      m_data->creator->create(*depthImage.as32f(), obj, 0, depthScaling);
    }

    //high level segmenation
    bool enableSegmentation = getPropertyValue("general.enable segmentation");

    if(enableSegmentation){
      bool useROI = getPropertyValue("general.use ROI");
      bool stabelizeSegmentation = getPropertyValue("general.stabelize segmentation");

      int surfaceMinSize = getPropertyValue("surfaces.min surface size");
      int surfaceRadius = getPropertyValue("surfaces.assignment radius");
      float surfaceDistance = getPropertyValue("surfaces.assignment distance");

      bool cutfreeEnable = getPropertyValue("cutfree.enable cutfree adjacency feature");
      float cutfreeEuclDist = getPropertyValue("cutfree.ransac euclidean distance");
      int cutfreePasses = getPropertyValue("cutfree.ransac passes");
      int cutfreeTolerance = getPropertyValue("cutfree.ransac tolerance");
      float cutfreeMinAngle = getPropertyValue("cutfree.min angle");

      bool coplanEnable = getPropertyValue("coplanarity.enable coplanarity feature");
      float coplanMaxAngle = getPropertyValue("coplanarity.max angle");
      float coplanDistance = getPropertyValue("coplanarity.distance tolerance");
      float coplanOutlier = getPropertyValue("coplanarity.outlier tolerance");
      int coplanNumTriangles = getPropertyValue("coplanarity.num triangles");
      int coplanNumScanlines = getPropertyValue("coplanarity.num scanlines");

      bool curveEnable = getPropertyValue("curvature.enable curvature feature");
      float curveHistogram = getPropertyValue("curvature.histogram similarity");
      bool curveUseOpen = getPropertyValue("curvature.enable open objects");
      int curveMaxDist = getPropertyValue("curvature.max distance");
      bool curveUseOccluded = getPropertyValue("curvature.enable occluded objects");
      float curveMaxError = getPropertyValue("curvature.max error");
      int curvePasses = getPropertyValue("curvature.ransac passes");
      float curveDistance = getPropertyValue("curvature.distance tolerance");
      float curveOutlier = getPropertyValue("curvature.outlier tolerance");

      bool remainingEnable = getPropertyValue("remaining.enable remaining points feature");
      int remainingMinSize = getPropertyValue("remaining.min size");
      float remainingEuclDist = getPropertyValue("remaining.euclidean distance");
      int remainingRadius = getPropertyValue("remaining.radius");
      float remainingAssignEuclDist = getPropertyValue("remaining.assign euclidean distance");
      int remainingSupportTolerance = getPropertyValue("remaining.support tolerance");

      float graphcutThreshold = getPropertyValue("graphcut.threshold");

      if(useROI){
        m_data->segmentation->setROI(getPropertyValue("general.ROI min x"),
                                    getPropertyValue("general.ROI max x"),
                                    getPropertyValue("general.ROI min y"),
                                    getPropertyValue("general.ROI max y"),
                                    getPropertyValue("general.ROI min z"),
                                    getPropertyValue("general.ROI max z"));

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
	    return m_data->segmentation->getLabelImage(getPropertyValue("general.stabelize segmentation"));
	  }


  core::Img8u ConfigurableDepthImageSegmenter::getColoredLabelDisplay(){
    return m_data->segmentation->getColoredLabelImage(getPropertyValue("general.stabelize segmentation"));
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