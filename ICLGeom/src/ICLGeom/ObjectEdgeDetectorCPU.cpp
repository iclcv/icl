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
#include <ICLGeom/ObjectEdgeDetectorData.h>

#include <ICLFilter/MedianOp.h>

namespace icl {

using namespace utils;
using namespace math;
using namespace core;

namespace geom {

struct ObjectEdgeDetectorCPU::Data {
	Data() {
		oedData = new ObjectEdgeDetectorData();
		
		//set default values
		params = oedData->getParameters();
		isInitialized=false;

		medianFilter = new filter::MedianOp(utils::Size(3,3));
		medianFilter->setClipToROI(false);
	}
	~Data() {
		delete medianFilter;
	}
	int w, h;
	DataSegment<float,4> normals;
	DataSegment<float,4> avgNormals;
	DataSegment<float,4> worldNormals;
	Array2D<Vec4> normalsA;
	Array2D<Vec4> avgNormalsA;
	Array2D<Vec4> worldNormalsA;
	core::Img32f rawImage;
	core::Img32f filteredImage;
	core::Img32f angleImage;
	core::Img8u binarizedImage;
	core::Img8u normalImage;
	bool isInitialized;
	
	filter::MedianOp* medianFilter;
	ObjectEdgeDetectorData* oedData;
	ObjectEdgeDetectorData::m_params params;

};

ObjectEdgeDetectorCPU::ObjectEdgeDetectorCPU() :
		m_data(new Data()) {
}

ObjectEdgeDetectorCPU::~ObjectEdgeDetectorCPU() {
	delete m_data;
}

void ObjectEdgeDetectorCPU::initialize(Size size){
    if(m_data->isInitialized){
		m_data->isInitialized=false;  
    }
    //create arrays and images in given size
	m_data->w = size.width;
	m_data->h = size.height;
	
	m_data->normalsA=Array2D<Vec4>(m_data->w,m_data->h);
	m_data->avgNormalsA=Array2D<Vec4>(m_data->w,m_data->h);
	m_data->worldNormalsA=Array2D<Vec4>(m_data->w,m_data->h);
	m_data->normals=DataSegment<float,4>(&m_data->normalsA(0,0).x, sizeof(Vec4), m_data->normalsA.getDim(), m_data->normalsA.getWidth());
	m_data->avgNormals=DataSegment<float,4>(&m_data->avgNormalsA(0,0).x, sizeof(Vec4), m_data->avgNormalsA.getDim(), m_data->avgNormalsA.getWidth());
	m_data->worldNormals=DataSegment<float,4>(&m_data->worldNormalsA(0,0).x, sizeof(Vec4), m_data->worldNormalsA.getDim(), m_data->worldNormalsA.getWidth());

	m_data->rawImage.setSize(Size(m_data->w, m_data->h));
	m_data->rawImage.setChannels(1);
	m_data->filteredImage.setSize(Size(m_data->w, m_data->h));
	m_data->filteredImage.setChannels(1);
	m_data->angleImage.setSize(Size(m_data->w, m_data->h));
	m_data->angleImage.setChannels(1);
	m_data->binarizedImage.setSize(Size(m_data->w, m_data->h));
	m_data->binarizedImage.setChannels(1);
	m_data->normalImage.setSize(Size(m_data->w, m_data->h));
	m_data->normalImage.setChannels(3);
}

void ObjectEdgeDetectorCPU::setDepthImage(const Img32f &depthImg) {
	m_data->rawImage = depthImg;
}

void ObjectEdgeDetectorCPU::applyMedianFilter() {
	m_data->medianFilter->adaptSize(Size(m_data->params.medianFilterSize,m_data->params.medianFilterSize));
	m_data->medianFilter->apply(&m_data->rawImage, bpp(m_data->filteredImage));
}

const Img32f &ObjectEdgeDetectorCPU::getFilteredDepthImage() {
	return m_data->filteredImage;
}

void ObjectEdgeDetectorCPU::setFilteredDepthImage(const Img32f &filteredImg) {
  m_data->filteredImage = filteredImg;
}

void ObjectEdgeDetectorCPU::applyNormalCalculation() {
	const int r = m_data->params.normalRange;
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

	if (m_data->params.useNormalAveraging && !m_data->params.useGaussSmoothing) {
		applyLinearNormalAveraging();
	} else if (m_data->params.useNormalAveraging && m_data->params.useGaussSmoothing) {
		applyGaussianNormalSmoothing();
	}
}

void ObjectEdgeDetectorCPU::applyLinearNormalAveraging() {
	const int r = m_data->params.normalAveragingRange;
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
				m_data->avgNormalsA[i] = avg;
			}
		}
	}
}

void ObjectEdgeDetectorCPU::applyGaussianNormalSmoothing() {
	ObjectEdgeDetectorData::m_kernel kernelData = m_data->oedData->getKernel(m_data->params.normalAveragingRange);
	float norm = kernelData.norm;
	int l = kernelData.l;
	DynMatrix<float> kernel = kernelData.kernel;
	
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
			    m_data->avgNormalsA[i] = avg;
		    }
	    }
    }
}

const DataSegment<float,4> ObjectEdgeDetectorCPU::getNormals() {
	if (m_data->params.useNormalAveraging == true) {
		return m_data->avgNormals;
	} else {
		return m_data->normals;
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
				if (m_data->params.useNormalAveraging == true) {
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

const DataSegment<float,4> ObjectEdgeDetectorCPU::getWorldNormals() {
	return m_data->worldNormals;
}

const core::Img8u &ObjectEdgeDetectorCPU::getRGBNormalImage() {
	return m_data->normalImage;
}

void ObjectEdgeDetectorCPU::setNormals(DataSegment<float,4> pNormals) {
	if (m_data->params.useNormalAveraging == true) {
		pNormals.deepCopy(m_data->avgNormals);
	} else {
		pNormals.deepCopy(m_data->normals);
	}
}

void ObjectEdgeDetectorCPU::applyAngleImageCalculation() {
	DataSegment<float,4> norm;
	if (m_data->params.useNormalAveraging == true) {
		norm = m_data->avgNormals;
	} else {
		norm = m_data->normals;
	}
	const int w = m_data->w, h = m_data->h;
	core::Channel32f angleI = m_data->angleImage[0];
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int i = x + w * y;
			if (y < m_data->params.neighborhoodRange
					|| y >= h - (m_data->params.neighborhoodRange)
					|| x < m_data->params.neighborhoodRange
					|| x >= w - (m_data->params.neighborhoodRange)) {
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
				for (int z = 1; z <= m_data->params.neighborhoodRange; z++) {
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
				snr /= m_data->params.neighborhoodRange;
				snl /= m_data->params.neighborhoodRange;
				snt /= m_data->params.neighborhoodRange;
				snb /= m_data->params.neighborhoodRange;
				sntr /= m_data->params.neighborhoodRange;
				sntl /= m_data->params.neighborhoodRange;
				snbr /= m_data->params.neighborhoodRange;
				snbl /= m_data->params.neighborhoodRange;

				if (m_data->params.neighborhoodMode == 0) {//max
					angleI(x, y) = maxAngle(snr, snl, snt, snb,
                                            snbl, snbr, sntl, sntr);
				} else if (m_data->params.neighborhoodMode == 1) {//mean
					angleI(x, y) = (snr + snl + snt + snb
							+ sntr + sntl + snbr + snbl) / 8;
				} else {
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
					> m_data->params.binarizationThreshold) {
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
	m_data->params.medianFilterSize = size;
}

void ObjectEdgeDetectorCPU::setNormalCalculationRange(int range) {
	m_data->params.normalRange = range;
}

void ObjectEdgeDetectorCPU::setNormalAveragingRange(int range) {
	m_data->params.normalAveragingRange = range;
}

void ObjectEdgeDetectorCPU::setAngleNeighborhoodMode(int mode) {
	m_data->params.neighborhoodMode = mode;
}

void ObjectEdgeDetectorCPU::setAngleNeighborhoodRange(int range) {
	m_data->params.neighborhoodRange = range;
}

void ObjectEdgeDetectorCPU::setBinarizationThreshold(float threshold) {
	m_data->params.binarizationThreshold = threshold;
}

void ObjectEdgeDetectorCPU::setUseNormalAveraging(bool use) {
	m_data->params.useNormalAveraging = use;
}

void ObjectEdgeDetectorCPU::setUseGaussSmoothing(bool use) {
	m_data->params.useGaussSmoothing = use;
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
	m_data->params.useNormalAveraging = average;
	m_data->params.useGaussSmoothing = gauss;
	applyNormalCalculation();
	applyAngleImageCalculation();
	applyImageBinarization();
	return getBinarizedAngleImage();
}


float ObjectEdgeDetectorCPU::scalar(FixedColVector<float,4> &a, FixedColVector<float,4> &b){
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

float ObjectEdgeDetectorCPU::flipAngle(float angle){
    if (angle < cos(M_PI / 2)){
	    angle = cos(M_PI - acos(angle));
	}
	return angle;
}

float ObjectEdgeDetectorCPU::scalarAndFlip(FixedColVector<float,4> &a, FixedColVector<float,4> &b){
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
