/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLIO/src/ImageUndistortion.cpp                        **
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
#include <ICLIO/ImageUndistortion.h>

namespace icl{

ImageUndistortion::ImageUndistortion(const std::string &filename){
	std::ifstream s(filename.c_str());
	s >> (*this);
}

ImageUndistortion::ImageUndistortion(){}

std::istream &operator>>(std::istream &is, ImageUndistortion &udist){
	XMLDocument *doc = new XMLDocument;
	doc->loadNext(is);
	ConfigFile f(doc);
	if(udist.params.size() != 0 )
		udist.params.resize(0);

#define LOAD_FROM_STREAM(KEY) \
    if (f.contains("config." #KEY)) udist.params.push_back(f["config." #KEY]);
	/*udist.params.push_back(f["config.intrin.fx"]);
	udist.params.push_back(f["config.intrin.fy"]);
	udist.params.push_back(f["config.intrin.ix"]);
	udist.params.push_back(f["config.intrin.iy"]);
	udist.params.push_back(f["config.intrin.skew"]);
	udist.params.push_back(f["config.dist.k1"]);
	udist.params.push_back(f["config.dist.k2"]);
	udist.params.push_back(f["config.dist.k3"]);
	udist.params.push_back(f["config.dist.k4"]);
	udist.params.push_back(f["config.dist.k5"]);*/
    LOAD_FROM_STREAM(intrin.fx);
    LOAD_FROM_STREAM(intrin.fy);
    LOAD_FROM_STREAM(intrin.ix);
    LOAD_FROM_STREAM(intrin.iy);
    LOAD_FROM_STREAM(intrin.skew);
    LOAD_FROM_STREAM(dist.k1);
    LOAD_FROM_STREAM(dist.k2);
    LOAD_FROM_STREAM(dist.k3);
    LOAD_FROM_STREAM(dist.k4);
	LOAD_FROM_STREAM(dist.k5);
    #undef LOAD_FROM_STREAM

	/*udist.params.push_back(f["config.intrin.fx"]);
	udist.params.push_back(f["config.intrin.fy"]);
	udist.params.push_back(f["config.intrin.ix"]);
	udist.params.push_back(f["config.intrin.iy"]);
	udist.params.push_back(f["config.intrin.skew"]);
	udist.params.push_back(f["config.dist.k1"]);
	udist.params.push_back(f["config.dist.k2"]);
	udist.params.push_back(f["config.dist.k3"]);
	udist.params.push_back(f["config.dist.k4"]);
	udist.params.push_back(f["config.dist.k5"]);*/
	FixedMatrix<icl64f,3,3> KK_new;
	KK_new[0] = udist.params[0]; KK_new[1] = udist.params[0]*udist.params[4]; KK_new[2] = udist.params[2];
	KK_new[3] = 0.0; KK_new[4] = udist.params[1]; KK_new[5] = udist.params[3];
	KK_new[6] = 0.0; KK_new[7] = 0.0; KK_new[8] = 1.0;
	udist.KK_new_inv = KK_new.inv();
	return is;
}

std::ostream &operator<<(std::ostream &s, ImageUndistortion &udist){
	ConfigFile f;
	f["config.intrin.fx"] = udist.params[0];
	f["config.intrin.fy"] = udist.params[1];
	f["config.intrin.ix"] = udist.params[2];
	f["config.intrin.iy"] = udist.params[3];
	f["config.intrin.skew"] = udist.params[4];
	f["config.dist.k1"] = udist.params[5];
	f["config.dist.k2"] = udist.params[6];
	f["config.dist.k3"] = udist.params[7];
	f["config.dist.k4"] = udist.params[8];
	f["config.dist.k5"] = udist.params[9];
	return s << f;
}

const Point32f ImageUndistortion::undistort5Param(const Point32f &point) const {
		FixedMatrix<icl64f,1,3> p; p[0] = point.x; p[1] = point.y; p[2] = 1.0;
		FixedMatrix<icl64f,1,3> rays = KK_new_inv*p;

		FixedMatrix<icl64f,1,2> x;
		x[0] = rays[0]/rays[2];
		x[1] = rays[1]/rays[2];

		FixedMatrix<icl64f,1,5> k;
		k[0] = params[5]; k[1] = params[6]; k[2] = params[7]; k[3] = params[8]; k[4] = params[9];
		// Add distortion:
		FixedMatrix<icl64f,1,2> pd;

		double r2 = x[0]*x[0]+x[1]*x[1];
		double r4 = r2*r2;
		double r6 = r4*r2;

		// Radial distortion:
		double cdist = 1 + k[0] * r2 + k[1] * r4 + k[4] * r6;

		pd[0] = x[0] * cdist;
		pd[1] = x[1] * cdist;

		// tangential distortion:
		double a1 = 2*x[0]*x[1];
		double a2 = r2 + 2*x[0]*x[0];
		double a3 = r2 + 2*x[1]*x[1];

		FixedMatrix<icl64f,1,2> delta_x;
		delta_x[0]= k[2]*a1 + k[3]*a2 ;
		delta_x[1]= k[2] * a3 + k[3]*a1;

		pd = pd + delta_x;
		// Reconvert in pixels:
		double px2 = params[0]*(pd[0]+params[4]*pd[1])+params[2];
		double py2 = params[1]*pd[1]+params[3];

		return Point32f(px2,py2);
	}
}

