/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLGeom/src/ICLGeom/ObjectEdgeDetectorCPU.cpp          **
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

#include <ICLCore/Channel.h>
#include <ICLGeom/ObjectEdgeDetectorCPU.h>

#include <ICLFilter/MedianOp.h>

namespace icl {

using namespace utils;
using namespace math;
using namespace core;

namespace geom {

struct ObjectEdgeDetectorCPU::Data {
	Data(const Size &size) {
		//set default values
		medianFilterSize = 3;
		normalRange = 2;
		normalAveragingRange = 1;
		neighborhoodMode = 0;
		neighborhoodRange = 3;
		binarizationThreshold = 0.89;
		useNormalAveraging = true;
		useGaussSmoothing = false;

		//create arrays and images in given size
		if (size == Size::QVGA) {
			std::cout << "Resolution: 320x240" << std::endl;
			w = 320;
			h = 240;

		} else if (size == Size::VGA) {
			std::cout << "Resolution: 640x480" << std::endl;
			w = 640;
			h = 480;
		} else {
			std::cout << "Unknown Resolution" << std::endl;
			w = size.width;
			h = size.height;
		}

		normals = new Vec4[w * h];
		avgNormals= new Vec4[w * h];
		worldNormals= new Vec4[w * h];
		for(int i=0; i<w*h;i++) {
			normals[i].x=0;
			normals[i].y=0;
			normals[i].z=0;
			normals[i].w=0;
			avgNormals[i].x=0;
			avgNormals[i].y=0;
			avgNormals[i].z=0;
			avgNormals[i].w=0;
			worldNormals[i].x=0;
			worldNormals[i].y=0;
			worldNormals[i].z=0;
			worldNormals[i].w=0;
		}

		rawImage.setSize(Size(w, h));
		rawImage.setChannels(1);
		filteredImage.setSize(Size(w, h));
		filteredImage.setChannels(1);
		angleImage.setSize(Size(w, h));
		angleImage.setChannels(1);
		binarizedImage.setSize(Size(w, h));
		binarizedImage.setChannels(1);
		normalImage.setSize(Size(w, h));
		normalImage.setChannels(3);
		medianFilter = new filter::MedianOp(utils::Size(3,3));
		medianFilter->setClipToROI(false);

	}
	~Data() {
		delete[] normals;
		delete[] avgNormals;
		delete[] worldNormals;
	}
	int w, h;
	int medianFilterSize;
	int normalRange;
	int normalAveragingRange;
	int neighborhoodMode;
	int neighborhoodRange;
	float binarizationThreshold;
	bool useNormalAveraging;
	bool useGaussSmoothing;
	Vec4* normals;
	Vec4* avgNormals;
	Vec4* worldNormals;
	core::Img32f rawImage;
	core::Img32f filteredImage;
	core::Img32f angleImage;
	core::Img8u binarizedImage;
	core::Img8u normalImage;
	
	filter::MedianOp* medianFilter;

};

ObjectEdgeDetectorCPU::ObjectEdgeDetectorCPU(Size size) :
		m_data(new Data(size)) {
}

ObjectEdgeDetectorCPU::~ObjectEdgeDetectorCPU() {
	delete m_data;
}

void ObjectEdgeDetectorCPU::setDepthImage(const Img32f &depthImg) {
	m_data->rawImage = depthImg;
}

void ObjectEdgeDetectorCPU::applyMedianFilter() {
	m_data->medianFilter->adaptSize(Size(m_data->medianFilterSize,m_data->medianFilterSize));
	m_data->medianFilter->apply(&m_data->rawImage, bpp(m_data->filteredImage));
}

const Img32f &ObjectEdgeDetectorCPU::getFilteredDepthImage() {
	return m_data->filteredImage;
}

void ObjectEdgeDetectorCPU::setFilteredDepthImage(const Img32f &filteredImg) {
  m_data->filteredImage = filteredImg;
}

void ObjectEdgeDetectorCPU::applyNormalCalculation() {
	const int r = m_data->normalRange;
	Vec fa1, fb1, n1, n01;
	Channel32f filteredI = m_data->filteredImage[0];
	
	for (int y = 0; y < m_data->h; y++) {
		for (int x = 0; x < m_data->w; x++) {
			int i = x + m_data->w * y;
			if (y < r || y >= m_data->h - r || x < r
					|| x >= m_data->w - r) {
				m_data->normals[i].x = 0; //points out of range
				m_data->normals[i].y = 0;
				m_data->normals[i].z = 0;
			} else {
				//cross product normal determination
				fa1[0] = (x + r) - (x - r);
				fa1[1] = (y - r) - (y - r);
				fa1[2] = filteredI(x + r, y - r)
						- filteredI(x - r, y - r);
				fb1[0] = (x) - (x - r);
				fb1[1] = (y + r) - (y - r);
				fb1[2] = filteredI(x, y + r)
						- filteredI(x - r, y - r);

				n1[0] = fa1[1] * fb1[2] - fa1[2] * fb1[1];
				n1[1] = fa1[2] * fb1[0] - fa1[0] * fb1[2];
				n1[2] = fa1[0] * fb1[1] - fa1[1] * fb1[0];
				n01[0] = n1[0] / norm3(n1);
				n01[1] = n1[1] / norm3(n1);
				n01[2] = n1[2] / norm3(n1);

				m_data->normals[i].x = n01[0];
				m_data->normals[i].y = n01[1];
				m_data->normals[i].z = n01[2];
				m_data->normals[i].w = 1;
			}
		}
	}

	if (m_data->useNormalAveraging && !m_data->useGaussSmoothing) {
		applyLinearNormalAveraging();
	} else if (m_data->useNormalAveraging && m_data->useGaussSmoothing) {
		applyGaussianNormalSmoothing();
	}
}

void ObjectEdgeDetectorCPU::applyLinearNormalAveraging() {
	const int r = m_data->normalAveragingRange;
	for (int y = 0; y < m_data->h; y++) {
		for (int x = 0; x < m_data->w; x++) {
			int i = x + m_data->w * y;
			if (y < r || y >= m_data->h - r || x < r
					|| x >= m_data->w - r) {
				m_data->avgNormals[i] = m_data->normals[i];
			} else {
				Vec4 avg;
				avg.x = 0, avg.y = 0, avg.z = 0, avg.w = 0;
				for (int sx = -r; sx <= r; sx++) {
					for (int sy = -r; sy <= r; sy++) {
						avg.x += m_data->normals[(x + sx)
								+ m_data->w * (y + sy)].x;
						avg.y += m_data->normals[(x + sx)
								+ m_data->w * (y + sy)].y;
						avg.z += m_data->normals[(x + sx)
								+ m_data->w * (y + sy)].z;
					}
				}
				avg.x /= ((1 + 2 * r) * (1 + 2 * r));
				avg.y /= ((1 + 2 * r) * (1 + 2 * r));
				avg.z /= ((1 + 2 * r) * (1 + 2 * r));
				avg.w = 1;
				m_data->avgNormals[i] = avg;
			}
		}
	}
}

void ObjectEdgeDetectorCPU::applyGaussianNormalSmoothing() {
	float norm = 1;
	DynMatrix<float> kernel = DynMatrix<float>(1, 1, 0.0);
	int l = 0;
	if (m_data->normalAveragingRange <= 1) {
		// nothing!
	} else if (m_data->normalAveragingRange <= 3) {
		norm = 16.;
		l = 1;
		DynMatrix<float> k1 = DynMatrix<float>(1, 3, 0.0);
		k1(0, 0) = 1.;
		k1(0, 1) = 2.;
		k1(0, 2) = 1.;
		kernel = k1 * k1.transp();
	} else if (m_data->normalAveragingRange <= 5) {
		norm = 256.;
		l = 2;
		DynMatrix<float> k1 = DynMatrix<float>(1, 5, 0.0);
		k1(0, 0) = 1.;
		k1(0, 1) = 4.;
		k1(0, 2) = 6.;
		k1(0, 3) = 4.;
		k1(0, 4) = 1.;
		kernel = k1 * k1.transp();
	} else {
		norm = 4096.;
		l = 3;
		DynMatrix<float> k1 = DynMatrix<float>(1, 7, 0.0);
		k1(0, 0) = 1.;
		k1(0, 1) = 6.;
		k1(0, 2) = 15.;
		k1(0, 3) = 20.;
		k1(0, 4) = 15.;
		k1(0, 5) = 6.;
		k1(0, 6) = 1.;
		kernel = k1 * k1.transp();
	}
    for (int y = 0; y < m_data->h; y++) {
	    for (int x = 0; x < m_data->w; x++) {
		    int i = x + m_data->w * y;
		    if (y < l || y >= m_data->h - l || x < l || x >= m_data->w - l
				    || l == 0) {
			    m_data->avgNormals[i] = m_data->normals[i];
		    } else {
			    Vec4 avg;
			    avg.x = 0, avg.y = 0, avg.z = 0, avg.w = 0;
			    for (int sx = -l; sx <= l; sx++) {
				    for (int sy = -l; sy <= l; sy++) {
					    avg.x += m_data->normals[(x + sx)
							    + m_data->w * (y + sy)].x
							    * kernel(sx + l, sy + l);
					    avg.y += m_data->normals[(x + sx)
							    + m_data->w * (y + sy)].y
							    * kernel(sx + l, sy + l);
					    avg.z += m_data->normals[(x + sx)
							    + m_data->w * (y + sy)].z
							    * kernel(sx + l, sy + l);
				    }
			    }
			    avg.x /= norm;
			    avg.y /= norm;
			    avg.z /= norm;
			    avg.w = 1;
			    m_data->avgNormals[i] = avg;
		    }
	    }
    }
}

const Vec *ObjectEdgeDetectorCPU::getNormals() {
	if (m_data->useNormalAveraging == true) {
		return (const Vec*) m_data->avgNormals;
	} else {
		return (const Vec*) m_data->normals;
	}
}

void ObjectEdgeDetectorCPU::applyWorldNormalCalculation(const Camera &cam) {
	Mat T = cam.getCSTransformationMatrix();
	FixedMatrix<float, 3, 3> R = T.part<0, 0, 3, 3>();
	Mat T2 = R.transp().resize<4, 4>(0);
	T2(3, 3) = 1;
	const core::Channel32f d = m_data->rawImage[0];
	for (int y = 0; y < m_data->h; ++y) {
		for (int x = 0; x < m_data->w; ++x) {
			int i = x + m_data->w * y;
			if (d[i] == 2047) {
				m_data->normalImage(x, y, 0) = 0;
				m_data->normalImage(x, y, 1) = 0;
				m_data->normalImage(x, y, 2) = 0;
			} else {
				Vec pWN;
				if (m_data->useNormalAveraging == true) {
					pWN = T2 * (Vec&) m_data->avgNormals[i];
				} else {
					pWN = T2 * (Vec&) m_data->normals[i];
				}
				m_data->worldNormals[i].x = -pWN[0];
				m_data->worldNormals[i].y = -pWN[1];
				m_data->worldNormals[i].z = -pWN[2];
				m_data->worldNormals[i].w = 1.;

				m_data->normalImage(x, y, 0) = (int) (fabs(pWN[0]) * 255.);
				m_data->normalImage(x, y, 1) = (int) (fabs(pWN[1]) * 255.);
				m_data->normalImage(x, y, 2) = (int) (fabs(pWN[2]) * 255.);
			}
		}
	}
}

const Vec* ObjectEdgeDetectorCPU::getWorldNormals() {
	return (const Vec*) m_data->worldNormals;
}

const core::Img8u &ObjectEdgeDetectorCPU::getRGBNormalImage() {
	return m_data->normalImage;
}

void ObjectEdgeDetectorCPU::setNormals(Vec* pNormals) {
	if (m_data->useNormalAveraging == true) {
		m_data->avgNormals = (Vec4*) pNormals;
	} else {
		m_data->normals = (Vec4*) pNormals;
	}
}

void ObjectEdgeDetectorCPU::applyAngleImageCalculation() {
	Vec4 * norm;
	if (m_data->useNormalAveraging == true) {
		norm = m_data->avgNormals;
	} else {
		norm = m_data->normals;
	}
	const int w = m_data->w, h = m_data->h;
	core::Channel32f angleI = m_data->angleImage[0];
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int i = x + w * y;
			if (y < m_data->neighborhoodRange
					|| y >= h - (m_data->neighborhoodRange)
					|| x < m_data->neighborhoodRange
					|| x >= w - (m_data->neighborhoodRange)) {
				angleI(x, y) = 0;
			} else {
				float snr = 0; //sum right
				float snl = 0; //sum left
				float snt = 0; //sum top
				float snb = 0; //sum bottom
				float sntr = 0; //sum top-right
				float sntl = 0; //sum top-left
				float snbr = 0; //sum bottom-right
				float snbl = 0; //sum bottom-left
				for (int z = 1; z <= m_data->neighborhoodRange; z++) {
					//angle between normals
					//flip if angle is bigger than 90°
					snr += scalarAndFlip(norm[i],norm[i + z]);
					snl += scalarAndFlip(norm[i],norm[i - z]);
					snt += scalarAndFlip(norm[i],norm[i + w * z]);
					snb += scalarAndFlip(norm[i],norm[i - w * z]);
					sntr += scalarAndFlip(norm[i],norm[i + w * z + z]);
					sntl += scalarAndFlip(norm[i],norm[i + w * z - z]);
					snbr += scalarAndFlip(norm[i],norm[i - w * z + z]);
					snbl += scalarAndFlip(norm[i],norm[i - w * z - z]);					
				}
				snr /= m_data->neighborhoodRange;
				snl /= m_data->neighborhoodRange;
				snt /= m_data->neighborhoodRange;
				snb /= m_data->neighborhoodRange;
				sntr /= m_data->neighborhoodRange;
				sntl /= m_data->neighborhoodRange;
				snbr /= m_data->neighborhoodRange;
				snbl /= m_data->neighborhoodRange;

				if (m_data->neighborhoodMode == 0) {//max
					angleI(x, y) = maxAngle(snr, snl, snt, snb,
                                            snbl, snbr, sntl, sntr);
				} else if (m_data->neighborhoodMode == 1) {//mean
					angleI(x, y) = (snr + snl + snt + snb
							+ sntr + sntl + snbr + snbl) / 8;
				} else {
					std::cout << "Unknown neighborhood mode" << std::endl;
				}
			}
		}
	}
}

const Img32f &ObjectEdgeDetectorCPU::getAngleImage() {
	return m_data->angleImage;
}

void ObjectEdgeDetectorCPU::setAngleImage(const Img32f &angleImg) {
  m_data->angleImage = angleImg;
}

void ObjectEdgeDetectorCPU::applyImageBinarization() {
	for (int y = 0; y < m_data->h; y++) {
		for (int x = 0; x < m_data->w; x++) {
			if (m_data->angleImage(x, y, 0)
					> m_data->binarizationThreshold) {
				m_data->binarizedImage(x, y, 0) = 255;
			} else {
				m_data->binarizedImage(x, y, 0) = 0;
			}
		}
	}
}

const Img8u &ObjectEdgeDetectorCPU::getBinarizedAngleImage() {
	return m_data->binarizedImage;
}

void ObjectEdgeDetectorCPU::setMedianFilterSize(int size) {
	m_data->medianFilterSize = size;
}

void ObjectEdgeDetectorCPU::setNormalCalculationRange(int range) {
	m_data->normalRange = range;
}

void ObjectEdgeDetectorCPU::setNormalAveragingRange(int range) {
	m_data->normalAveragingRange = range;
}

void ObjectEdgeDetectorCPU::setAngleNeighborhoodMode(int mode) {
	m_data->neighborhoodMode = mode;
}

void ObjectEdgeDetectorCPU::setAngleNeighborhoodRange(int range) {
	m_data->neighborhoodRange = range;
}

void ObjectEdgeDetectorCPU::setBinarizationThreshold(float threshold) {
	m_data->binarizationThreshold = threshold;
}

void ObjectEdgeDetectorCPU::setUseNormalAveraging(bool use) {
	m_data->useNormalAveraging = use;
}

void ObjectEdgeDetectorCPU::setUseGaussSmoothing(bool use) {
	m_data->useGaussSmoothing = use;
}

bool ObjectEdgeDetectorCPU::isCLReady() {
    return false;
}

const Img8u &ObjectEdgeDetectorCPU::calculate(const Img32f &depthImage,
		bool filter, bool average, bool gauss) {
	if (filter == false) {
		setFilteredDepthImage(depthImage);
	} else {
		setDepthImage(depthImage);
		applyMedianFilter();
	}
	m_data->useNormalAveraging = average;
	m_data->useGaussSmoothing = gauss;
	applyNormalCalculation();
	applyAngleImageCalculation();
	applyImageBinarization();
	return getBinarizedAngleImage();
}


float ObjectEdgeDetectorCPU::scalar(Vec4 a, Vec4 b){
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

float ObjectEdgeDetectorCPU::flipAngle(float angle){
    if (angle < cos(M_PI / 2)){
	    angle = cos(M_PI - acos(angle));
	}
	return angle;
}

float ObjectEdgeDetectorCPU::scalarAndFlip(Vec4 a, Vec4 b){
    return flipAngle(scalar(a,b));
}

float ObjectEdgeDetectorCPU::maxAngle(float snr, float snl, float snt, float snb,
                                      float snbl, float snbr, float sntl, float sntr){
    float max = snr;
	if (max > snl) {
		max = snl;
	}
	if (max > snt) {
		max = snt;
	}
	if (max > snb) {
		max = snb;
	}
	if (max > snbl) {
		max = snbl;
	}
	if (max > snbr) {
		max = snbr;
	}
	if (max > sntl) {
		max = sntl;
	}
	if (max > sntr) {
		max = sntr;
	}
	return max;
}

} // namespace geom
}
