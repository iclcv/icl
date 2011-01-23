/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : include/ICLGeom/OpenCVCamCalib.h                       **
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
#ifndef ICL_OPENCVCAMCALIB_H_
#define ICL_OPENCVCAMCALIB_H_

#include <ICLCore/Img.h>
#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/DynMatrix.h>

#include <ICLUtils/Macros.h>
#include <ICLUtils/Uncopyable.h>

#include <ICLOpenCV/OpenCV.h>
#ifdef HAVE_OPENCV211
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc/imgproc_c.h>
//#include <opencv2/core/core_c.h>
//#include <opencv2/core/types_c.h>
#else
#include <cv.h>
#endif
namespace icl{

/**
 Cameracalibration using OpenCV functions.
 */
class OpenCVCamCalib : public Uncopyable{

	class Data;
	///Class for internal params and buffers.
	Data *m_data;

public:
	///Constructor
	/**boardWidth and boardHeight should not be equal
	  @param boardWidth width of the chessboard
	  @param boardHeight of the chessboard
	  @param boardCount minimum number of chessboards to be found on images before calibration
	 */
	OpenCVCamCalib(unsigned int boardWidth=6, unsigned int boardHeight=9, unsigned int boardCount=8);

	///Destructor
	~OpenCVCamCalib();

	///Adds points from images to computation.
	/*@param img image to be searched for chessboard and points
	  @return overall current number of found chessboard for calibration
	 */
	int addPoints(const ImgBase *img);

	///Tries to calibrates the camera, if minimal number of found and valid chessboards  is greater zero
	void calibrateCam();

	///Computes the undistorted image.
	/*@return the new undistorted image*/
	ImgBase *undisort(const ImgBase *img);

	///resets internal data and sets given params
	/*@param width of the chessboard
	  @param height of the chessboard
	  @param count minimum number of chessboards to be found on images before calibration*/
	void resetData(int width, int height, int count);

	///Returns DynMatrix of intrinsic params
	/*@return intrinsic params*/
	DynMatrix<icl64f> *getIntrinsics();

	///Returns DynMatrix of distortion params
	/*@return distortion params*/
	DynMatrix<icl64f> *getDistortion();

	///loads intrinsic params from file
	void loadParams(const char* xmlfilename);

	///saves intrinsic params to file
	void saveParams(const char* xmlfilename);

};
}

#endif /* ICL_OPENCVCAMCALIB_H_ */
