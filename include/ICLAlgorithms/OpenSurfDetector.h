/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : include/ICLFilter/OpenSurfDetector.h                   **
 ** Module : ICLFilter                                              **
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

#ifndef ICL_OPENSURFDETECTOR_H_
#define ICL_OPENSURFDETECTOR_H_

#include <ICLCore/ImgBase.h>
#include <ICLCore/Img.h>
#ifdef HAVE_QT
#include <ICLQt/DrawWidget.h>
#endif
#include <ICLOpenCV/OpenCV.h>
#include <opensurf/surflib.h>
namespace icl{
/**
 This class is a wrapper for opensurf.
 Instances of this class detect surf. You can set some parameters to
 make it more stable. For more informations on opensurf visit
 http://www.chrisevansdev.com/computer-vision-opensurf.html*/
class OpenSurfDetector{

private:
	///Forwarddeklaration.
	class Data;

	///Class for internal params and buffers.
	Data *m_data;

	///internal match function
	/**
	 Finds matches between surf of object image and passed
	 surf of given image.*/
	void match(std::vector<Ipoint> *ipts);

	///computes surf
	void computeSURF();

public:

	///Creates a new OpenSurfDetector object with standard parameters.
	/**
	 Creates a new OpenSurfDetector object. Standard parameters are
	 are set, but one can change these later.
	 @param obj the object image
	 @param upright
	 @param octaves
	 @param intervals
	 @param init_samples
	 @param thresh*/
	OpenSurfDetector(const ImgBase *obj=0, bool upright=false, int octaves=4,
			int intervals=4, int init_samples=2, float thresh=0.004f);

	///Destructor
	~OpenSurfDetector();

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
	const std::vector<Ipoint> &getObjectImgFeatures();

	///sets rotationinvariance
	/**
	 Note due to compatibility with  opensurf setting
	 this parameter false has the behavior you expect.
	 @param upright rotationinvariance or not*/
	void setRotationInvariant(bool upright);

	///sets the octave count
	/** @param octaves the octavenumber*/
	void setOctaves(int octaves);

	///sets the intervalls
	/** @param intervals the intervalnumber*/
	void setIntervals(int intervals);

	///sets the initsamples
	/** @param init_samples the  samplesnumber*/
	void setInitSamples(int init_samples);

	///sets the responsethresh
	/** @param thresh the thresh*/
	void setRespThresh(float thresh);

	///returns wether rotationinvariance is set or not
	/** @return value for rotationinvariance*/
	bool getRotationInvariant();

	///returns octave count
	/** @return the current octaves number*/
	int getOctaves();

	///returns interval count
	/** @return the current intervals number*/
	int getIntervals();

	///returns initsamples
	/**@return the current initsamples number*/
	int getInitSamples();

	///returns responsethresh
	/** @return the current reponsethresh*/
	float getRespThresh();

	///sets desired parameters
	/**
	 Sets desired parameters and computes surf, but only if the
	 parameters have really changed and if a reference image is
	 available.
	 @param upright rotationinvariance
	 @param octaves
	 @param intervals
	 @param init_samples
	 @param thresh*/
	void setParams(bool upright, int octaves, int intervals, int init_samples,
			float thresh);

	///extracs surf from given image
	/**
	 Extracts surf from given image. If the image is null
	 an empty vector is returned.
	 @param src source image
	 @return computed features*/
	const std::vector<Ipoint> &extractFeatures(const ImgBase *src);

	///computes matches between reference image and passed image.
	/**
	 If the passed image or the reference image is null an
	 exception is thrown. First point of the pair is the point for the image,
	 second for the reference image.
	 @param images
	 @return matches as std::pair in vector*/
	const std::vector<std::pair<Ipoint, Ipoint> > &match(const ImgBase *image) throw (ICLException);

	///sets new newObjImage as new reference image and computes matches between passed reference image and passed image.
	/**
	 If one of the passed images null an exception is thrown.
	 First point of the pair is the point for the image, second for the reference image.
	 @param images
	 @return matches as std::pair in vector*/
	const std::vector<std::pair<Ipoint, Ipoint> > &match(const ImgBase *currentImage,
			const ImgBase *newObjImage) throw (ICLException){
		setObjectImg(newObjImage);
		return match(currentImage);
	}

#ifdef HAVE_QT
	///draws a point on a widget
	/**
	 @param w drawwidget
	 @param p point to be drawn*/
	static void visualizeFeature(ICLDrawWidget &w,const Ipoint &p);

	///draws all points in vector on a widget
	/**
	 @param w drawwidget
	 @param features vector of points to be drawn*/
	static void visualizeFeatures(ICLDrawWidget &w, const std::vector<Ipoint> &features);

	///draws matches on widgets
	/**
	 @param w_object drawwidget for the reference image
	 @param w_result drawwidget for image
	 @param matches vector of matchesto be drawn*/
	static void visualizeMatches(ICLDrawWidget &w_object,ICLDrawWidget &w_result, std::vector<std::pair<Ipoint, Ipoint> > &matches);
#endif

};
}

#endif /* ICL_OPENSURFDETECTOR_H_ */
