/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : include/ICLIO/ImageUndistortion.h                      **
 ** Module : ICLIO                                                  **
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
#ifndef ICL_IMAGEUNDISTORTION_H_
#define ICL_IMAGEUNDISTORTION_H_

#include <ICLUtils/ConfigFile.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/XML.h>
#include <ICLUtils/FixedMatrix.h>
#include <ICLUtils/Size.h>
#include <fstream>
#include <string>
#include <vector>
namespace icl{

class ImageUndistortion{

	Size imgsize;

	const Point32f undistortSimple(const Point32f &point) const {
		const double &x0 = params[0];
		const double &y0 = params[1];
		const double &f = params[2]/100000000.0;
		const double &s = params[3];

		float x = s*(point.x-x0);
		float y = s*(point.y-y0);
		float p = 1 - f * (x*x + y*y);
		float xd = (p*x + x0);
		float yd = (p*y + y0);
		return Point32f(xd,yd);
	}

	const Point32f undistort5Param(const Point32f &point) const;

public:
	FixedMatrix<icl64f,3,3> KK_new_inv;
	std::vector<double> params;

	ImageUndistortion(const std::string &filename);

	ImageUndistortion();

	enum Model{
		SimpleARTBasedUndistortion, MatlabModel5Params
	} model;

	const Point32f undistort(const Point32f &point,Model  model = MatlabModel5Params) const {
		switch(model){
		case SimpleARTBasedUndistortion: return undistortSimple(point);
		case MatlabModel5Params: return undistort5Param(point);
		}
	}

	void setSize(int width,int height){
		imgsize = Size(width,height);
	}

	const Size getSize() {
	    return imgsize;
	}

 	const Size getSize() const {
	    return imgsize;
	}

};

std::istream &operator>>(std::istream &is, ImageUndistortion &udist);

std::ostream &operator<<(std::ostream &s, ImageUndistortion &udist);

}

#endif /* ICL_IMAGEUNDISTORTION_H_ */


