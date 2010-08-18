/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLAlgorithms/OpenCVSurfDetector.h             **
** Module : ICLAlgorithms                                          **
** Authors: Christian Groszewski                                   **
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

#ifndef ICL_OPENCVSURFDETECTOR_H_
#define ICL_OPENCVSURFDETECTOR_H_

#include <ICLCore/ImgBase.h>
#include <ICLCore/Img.h>
#ifdef HAVE_QT
#include <ICLQt/DrawWidget.h>
#endif
#include <ICLOpenCV/OpenCV.h>
#ifdef HAVE_OPENCV211
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/features2d/features2d.hpp>
#else
#include <ml.h>
#include <cv.h>
#endif
namespace icl{
/**
 This class is a wrapper for opencv surf.
 Instances of this class detect surf. You can set some parameters to
 make it more stable. For more informations on opencv surf take
 a look in the documentation of OPENCV2
 http://opencv.willowgarage.com/wiki/*/
class OpenCVSurfDetector{

private:
	///Forwarddeklaration.
	class Data;

	///Class for internal params and buffers.
	Data *m_data;

	///internal match function
	/**
	  Finds matches between surf of object image and passed
	  surf of given image.
	  @param vector of surfpoints*/
	void match(std::vector<CvSURFPoint> *ipts);

	///computes surf
	void computeSURF();
	///finds nearest neigbor between points of interest
	int nearestNeighbor(const float* vec, int laplacian,
			const CvSeq* model_keypoints, const CvSeq* model_descriptors);

public:

	///Creates a new OpenSurfDetector object with standard parameters.
	/**
	  Creates a new OpenSurfDetector object. Standard parameters are
	  are set, but one can change these later.
	  @param obj the object image
	  @param threshold
	  @param extended
	  @param octaves
  	  @param octavelayer*/
	OpenCVSurfDetector(const ImgBase *obj=0, double threshold=500, int extended=1,
			int octaves=3, int octavelayer=4);

	/**Destructor*/
	~OpenCVSurfDetector();


	///sets a new image as reference image und  computes surf for it
	/**
	  If the parameter objectImg is null an Exception is thrown.
	  @param objectImg the image to be set as new reference image*/
	void setObjectImg(const ImgBase *objectImg) throw (ICLException);

	///returns a copy of the reference image
	/**
	  Return a copy of the original reference image. If the reference image
	  was not set, the method throws an Exception.
	  @return copy reference image*/
	const ImgBase *getObjectImg() throw (ICLException);

	///returns the surf of the reference image.
	/**
	  If the reference image was not set, the returned vector is empty.
	  @return found features*/
	const std::vector<CvSURFPoint> &getObjectImgFeatures();

	///sets the octave count
	/** @param octaves the octavenumber*/
	void setOctaves(int octaves);

	///sets the octavelayer count
	/** @param octavelayer the octavelayer number*/
	void setOctavelayer(int octavelayer);

	///sets value for extend
	/**@param extended*/
	void setExtended(int extended);

	///sets the threshold
	/** @param thresh the threshold*/
	void setThreshold(double threshold);

	//getter
	///returns octave number
	/** @return the current octave number*/
	int getOctaves();

	///returns octavelayer number
	/** @return the current octavelayer number*/
	int getOctavelayer();

	///returns value for extended
	/** @return the current value for extended*/
	int getExtended();

	///returns threshold
	/** @return the current threshold*/
	double getThreshold();

	///sets desired parameters
	/**
	  Sets desired parameters and computes surf, but only if the
	  parameters have really changed and if a reference image is
	  available.
	  @param thresh
	  @param extended
	  @param octaves
	  @param octavelayer*/
	void setParams(double threshold, int extended,
			int octaves, int octavelayer);

	///extracs surf from given image
	/**
	  Extracts surf from given image. If the image is null
	  an empty vector is returned.
	  @param src source image
	  @return computed features*/
	const std::vector<CvSURFPoint> &extractFeatures(const ImgBase *src) throw (ICLException);

	///computes matches between reference image and passed image.
	/**
	  If the passed image or the reference image is null an
	  exception is thrown. First point of the pair is the point for the image,
	  second for the reference image.
	  @param image
	  @return matches as std::pair in vector*/
	const std::vector<std::pair<CvSURFPoint, CvSURFPoint> > &match(const ImgBase *image) throw (ICLException);

	///sets new newObjImage as new reference image and computes matches between passed reference image and passed image.
	/**
	  If one of the passed images null an exception is thrown.
	  First point of the pair is the point for the image, second for the reference image.
	  @param image
	  @param newObjImage the new reference image
	  @return matches as std::pair in vector*/
	const std::vector<std::pair<CvSURFPoint, CvSURFPoint> > &match(const ImgBase *currentImage,
			const ImgBase *newObjImage) throw (ICLException){
		setObjectImg(newObjImage);
		return match(currentImage);
	}

#ifdef HAVE_QT
	///draws a point on a widget
	/**
	  @param w drawwidget
	  @param p point to be drawn*/
	static void visualizeFeature(ICLDrawWidget &w,const CvSURFPoint &p);
	///draws all points in vector on a widget
	/**
	  @param w drawwidget
	  @param features vector of points to be drawn*/
	static void visualizeFeatures(ICLDrawWidget &w, const std::vector<CvSURFPoint> &features);
	///draws matches on widgets
	/**
	  @param w_object drawwidget for the reference image
	  @param w_result drawwidget for image
	  @param matches vector of matches to be drawn*/
	static void visualizeMatches(ICLDrawWidget &w_object,ICLDrawWidget &w_result, std::vector<std::pair<CvSURFPoint, CvSURFPoint> > &matches);
#endif

};
}

#endif /* ICL_OPENCVSURFDETECTOR_H_ */
