/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLGeom/src/OpenCVCamCalib.cpp                         **
 ** Module : ICLGeom                                                **
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
#include <ICLGeom/OpenCVCamCalib.h>

using namespace icl;
namespace icl{

struct OpenCVCamCalib::Data{
	int bWidth;
	int bHeight;
	int cornerCount;
	int successes;
	int bSize;

	CvMat* image_points;
	CvMat* object_points;
	CvMat* point_counts;
	CvMat* intrinsic_matrix;
	CvMat* distortion_coeffs;
	CvPoint2D32f* corners;

	CvSize imgSize;

	Data(unsigned int boardWitdth, unsigned int boardHeigt, unsigned int boardCount):
		bWidth(boardWitdth),bHeight(boardHeigt),cornerCount(0),successes(0),bSize(bWidth*bHeight){
		image_points = cvCreateMat(boardCount*bSize, 2, CV_32FC1);
		object_points = cvCreateMat(boardCount*bSize, 3, CV_32FC1);
		point_counts = cvCreateMat(boardCount, 1, CV_32SC1);
		intrinsic_matrix = cvCreateMat(3, 3, CV_32FC1);
		distortion_coeffs = cvCreateMat(5, 1, CV_32FC1);
		corners = new CvPoint2D32f[bSize];
	}
	~Data(){
		cvReleaseMat(&image_points );
		cvReleaseMat(&object_points );
		cvReleaseMat(&point_counts );
		cvReleaseMat(&intrinsic_matrix);
		cvReleaseMat(&distortion_coeffs);
		delete corners;
	}
};

OpenCVCamCalib::OpenCVCamCalib(unsigned int boardWitdth, unsigned int boardHeigt,
		unsigned int boardCount):m_data(new OpenCVCamCalib::Data(boardWitdth,
				boardHeigt, boardCount)){}

OpenCVCamCalib::~OpenCVCamCalib(){
	delete m_data;
}

void OpenCVCamCalib::resetData(int width, int height, int count){
	m_data->bWidth = width;
	m_data->bHeight = height;
	m_data->bSize = width * height;
	delete m_data->corners;
	m_data->corners = new CvPoint2D32f[m_data->bSize];
	cvReleaseMat(&(m_data->image_points));
	m_data->image_points = cvCreateMat(count*(m_data->bSize), 2, CV_32FC1);
	cvReleaseMat(&(m_data->object_points));
	m_data->object_points = cvCreateMat(count*(m_data->bSize), 3, CV_32FC1);
	cvReleaseMat(&(m_data->point_counts));
	m_data->point_counts = cvCreateMat(count, 1, CV_32SC1);
	cvReleaseMat(&(m_data->intrinsic_matrix));
	m_data->intrinsic_matrix = cvCreateMat(3, 3, CV_32FC1);
	cvReleaseMat(&(m_data->distortion_coeffs));
	m_data->distortion_coeffs = cvCreateMat(5, 1, CV_32FC1);
	m_data->successes = 0;
}

DynMatrix<icl64f> *OpenCVCamCalib::getIntrinsics(){
	DynMatrix<icl64f> *intr = new DynMatrix<icl64f>(3,3);
	for(unsigned int i=0;i<3;++i){
		for(unsigned int j=0;j<3;++j)
			intr->at(i,j) = CV_MAT_ELEM(*(m_data->intrinsic_matrix), float, j, i);
	}
	return intr;
}

DynMatrix<icl64f> *OpenCVCamCalib::getDistortion(){
	DynMatrix<icl64f> *dist = new DynMatrix<icl64f>(1,5);
	for(unsigned int i=0;i<5;++i)
		dist->at(0,i) = CV_MAT_ELEM(*(m_data->distortion_coeffs), float, i, 0);
	return dist;
}

int OpenCVCamCalib::addPoints(const ImgBase *img){
	if(!img)
		return m_data->successes;
	IplImage *image = img_to_ipl(img);
	//TODO remove after test
	//CvSize boardSize = cvSize(m_data->bWidth, m_data->bHeight);

	//TODO allow params
	int found = cvFindChessboardCorners(image, cvSize(m_data->bWidth, m_data->bHeight), m_data->corners,
			&(m_data->cornerCount), CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
	if(found == 0)
		return m_data->successes;
	m_data->imgSize = cvGetSize(image);

	IplImage *gray_image = 0;
	if(image->nChannels == 3){
		gray_image = cvCreateImage(cvGetSize(image), 8, 1);
		cvCvtColor(image, gray_image, CV_BGR2GRAY);
	} else {
		gray_image = image;
	}

	//TODO optimize params + setting
	cvFindCornerSubPix(gray_image, m_data->corners, m_data->cornerCount, cvSize(11,11),
			cvSize(-1, -1), cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1));

	if(m_data->cornerCount == (m_data->bSize)){
		int step = (m_data->successes)*(m_data->bSize);
		for(int i=step, j=0; j<(m_data->bSize); ++i, ++j){
			CV_MAT_ELEM(*(m_data->image_points), float, i, 0 ) = (m_data->corners)[j].x;
			CV_MAT_ELEM(*(m_data->image_points), float, i, 1 ) = (m_data->corners)[j].y;
			CV_MAT_ELEM(*(m_data->object_points), float, i, 0 ) = j/(m_data->bWidth);
			CV_MAT_ELEM(*(m_data->object_points), float, i, 1 ) = j%(m_data->bWidth);
			CV_MAT_ELEM(*(m_data->object_points), float, i, 2 ) = 0.0f;
		}
		CV_MAT_ELEM(*(m_data->point_counts), int, m_data->successes, 0) = m_data->bSize;
		m_data->successes++;
	}
	if(image->nChannels == 3)
		cvReleaseImage(&gray_image);
	cvReleaseImage(&image);
	return m_data->successes;
}

void OpenCVCamCalib::calibrateCam(){
	if(m_data->successes>0){
		CvMat* object_points2 = cvCreateMat(m_data->successes*(m_data->bSize), 3, CV_32FC1 );
		CvMat* image_points2 = cvCreateMat(m_data->successes*(m_data->bSize), 2, CV_32FC1 );
		CvMat* point_counts2 = cvCreateMat(m_data->successes, 1, CV_32SC1 );
		for(int i=0; i<(m_data->successes)*(m_data->bSize); ++i){
			CV_MAT_ELEM(*image_points2, float, i, 0) = CV_MAT_ELEM(*(m_data->image_points), float, i, 0);
			CV_MAT_ELEM(*image_points2, float, i, 1) = CV_MAT_ELEM(*(m_data->image_points), float, i, 1);
			CV_MAT_ELEM(*object_points2, float, i, 0) = CV_MAT_ELEM(*(m_data->object_points), float, i, 0);
			CV_MAT_ELEM(*object_points2, float, i, 1) = CV_MAT_ELEM(*(m_data->object_points), float, i, 1);
			CV_MAT_ELEM(*object_points2, float, i, 2) = CV_MAT_ELEM(*(m_data->object_points), float, i, 2);
		}
		for(int i=0; i<m_data->successes; ++i){
			CV_MAT_ELEM(*point_counts2, int, i, 0) = CV_MAT_ELEM(*(m_data->point_counts), int, i, 0);
		}
		CV_MAT_ELEM(*(m_data->intrinsic_matrix), float, 0, 0) = 1.0;
		CV_MAT_ELEM(*(m_data->intrinsic_matrix), float, 1, 1) = 1.0;
		//TODO allow more params
		cvCalibrateCamera2(object_points2, image_points2, point_counts2, m_data->imgSize,
				m_data->intrinsic_matrix, m_data->distortion_coeffs, NULL, NULL, CV_CALIB_FIX_ASPECT_RATIO);
		cvReleaseMat(&object_points2);
		cvReleaseMat(&image_points2);
		cvReleaseMat(&point_counts2);
	}
}

//TODO maybe optimize
ImgBase *OpenCVCamCalib::undisort(const ImgBase *img){
	(m_data->imgSize).width = (img->getSize()).width;
	(m_data->imgSize).height = (img->getSize()).height;
	IplImage* mapx = cvCreateImage(m_data->imgSize, IPL_DEPTH_32F, 1 );
	IplImage* mapy = cvCreateImage(m_data->imgSize, IPL_DEPTH_32F, 1 );
	cvInitUndistortMap(m_data->intrinsic_matrix, m_data->distortion_coeffs, mapx, mapy);
	IplImage *image = img_to_ipl(img);
	IplImage *image_clone = cvCloneImage( image );
	cvRemap(image_clone, image, mapx, mapy);
	cvReleaseImage(&image_clone);
	ImgBase *iclimg = ipl_to_img(image);
	cvReleaseImage(&image);
	cvReleaseImage(&mapx);
	cvReleaseImage(&mapy);
	return iclimg;
}



//TODO
void OpenCVCamCalib::loadParams(const char* xmlfilename){
	//loading intrinsics and distortions
	cvReleaseMat(&(m_data->intrinsic_matrix));
	m_data->intrinsic_matrix = (CvMat*)cvLoad( "Intrinsics.xml" );
	cvReleaseMat(&(m_data->distortion_coeffs));
	m_data->distortion_coeffs = (CvMat*)cvLoad( "Distortion.xml" );
}

//TODO
void OpenCVCamCalib::saveParams(const char* xmlfilename){
	//saving intrinsics and distortions
	cvSave( "Intrinsics.xml", m_data->intrinsic_matrix );
	cvSave( "Distortion.xml", m_data->distortion_coeffs );
}

}
