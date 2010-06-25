/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLFilter/OpenCVSurfDetector.h                 **
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
*********************************************************************/

#ifndef ICL_OPENCVSURFDETECTOR_H_
#define ICL_OPENCVSURFDETECTOR_H_

#include <ICLCore/ImgBase.h>
#include <ICLCore/Img.h>
#ifdef HAVE_QT
#include <ICLQt/DrawWidget.h>
#endif
#include <ICLOpenCV/OpenCV.h>
//#include <ml.h>
#include <cv.h>

namespace icl{

class OpenCVSurfDetector{

private:
	///Forwarddeklaration.
	class Data;

	///Class for internal params and buffers.
	Data *m_data;

	void match(std::vector<CvSURFPoint> *ipts);

	void computeSURF();

	int nearestNeighbor(const float* vec, int laplacian,
			const CvSeq* model_keypoints, const CvSeq* model_descriptors);

public:

    // If obj= 0, no ref-image available
	OpenCVSurfDetector(const ImgBase *obj=0, double threshold=500, int extended=1,
			int octaves=3, int octavelayer=4);

	/**Destructor*/
	~OpenCVSurfDetector();

	void setObjectImg(const ImgBase *objectImg) throw (ICLException);

	///returns back converted image
	const ImgBase *getObjectImg() throw (ICLException);

	const std::vector<CvSURFPoint> &getObjectImgFeatures();

	void setOctaves(int octaves);

	void setOctavelayer(int octavelayer);

	void setExtended(int extended);

	void setThreshold(double threshold);

	//getter
	int getOctaves();

	int getOctavelayer();

	int getExtended();

	double getThreshold();

	void setParams(double threshold, int extended,
			int octaves, int octavelayer);

	const std::vector<CvSURFPoint> &extractFeatures(const ImgBase *src) throw (ICLException);

	const std::vector<std::pair<CvSURFPoint, CvSURFPoint> > &match(const ImgBase *image) throw (ICLException);

	const std::vector<std::pair<CvSURFPoint, CvSURFPoint> > &match(const ImgBase *currentImage,
			const ImgBase *newObjImage) throw (ICLException){
		setObjectImg(newObjImage);
		return match(currentImage);
	}

#ifdef HAVE_QT
	static void visualizeFeature(ICLDrawWidget &w,const CvSURFPoint &p);
	static void visualizeFeatures(ICLDrawWidget &w, const std::vector<CvSURFPoint> &features);
	static void visualizeMatches(ICLDrawWidget &w_object,ICLDrawWidget &w_result, std::vector<std::pair<CvSURFPoint, CvSURFPoint> > &matches);
#endif

};
}

#endif /* ICL_OPENCVSURFDETECTOR_H_ */
