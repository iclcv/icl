/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
 **                         University of Bielefeld                 **
 **                Contact: nivision@techfak.uni-bielefeld.de       **
 **                Website: www.iclcv.org                           **
 **                                                                 **
 ** File   : ICLFilter/src/GenericSurfDetector.cpp                  **
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

#include <ICLAlgorithms/GenericSurfDetector.h>
using namespace icl;

namespace icl{

struct GenericSurfDetector::Data{

	IMPLEMENTATION m_impl;

	std::vector<GenericPoint> m_points;

	std::vector<GenericPoint> m_fpoints;

	//matches
	std::vector<std::pair<GenericPoint, GenericPoint> > m_matches;

	OpenSurfDetector *m_opensurfDetector;

	OpenCVSurfDetector *m_opencvDetector;

	Data(IMPLEMENTATION impl):m_impl(impl){
		if(impl==GenericSurfDetector::OPENCV){
			m_opensurfDetector = 0;
			m_opencvDetector = new OpenCVSurfDetector();
		}else{
			m_opensurfDetector = new OpenSurfDetector();
			m_opencvDetector = 0;
		}
	}

	//opencv
	Data(IMPLEMENTATION impl, const ImgBase *obj, double threshold=500,
			int extended=1, int octaves=3, int octavelayer=4):
				m_impl(impl),m_opensurfDetector(0),m_opencvDetector(new OpenCVSurfDetector(obj, threshold,
						extended, octaves, octavelayer)){}

	//opensurf
	Data(IMPLEMENTATION impl, const ImgBase *obj, bool upright=false,
			int octaves=4, int intervals=4, int init_samples=2, float thresh=0.004f):
				m_impl(impl),m_opensurfDetector(new OpenSurfDetector(obj,upright, octaves, intervals,
						init_samples, thresh)),m_opencvDetector(0){}
	~Data(){
		if(m_opencvDetector)
			delete m_opencvDetector;
		if(m_opensurfDetector)
			delete m_opensurfDetector;
	}
};

GenericSurfDetector::GenericSurfDetector(const ImgBase *obj,double threshold, int extended,
		int octaves, int octavelayer):
		m_data(new GenericSurfDetector::Data(GenericSurfDetector::OPENCV, obj,
				threshold, extended, octaves, octavelayer)){}

GenericSurfDetector::GenericSurfDetector(const ImgBase *obj,  bool upright, int octaves,
		int intervals, int init_samples, float thresh):
		m_data(new GenericSurfDetector::Data(GenericSurfDetector::OPENSURF, obj,
				upright, octaves, intervals, init_samples, thresh)){}

GenericSurfDetector::GenericSurfDetector(IMPLEMENTATION impl):
		m_data(new GenericSurfDetector::Data(impl)){}

GenericSurfDetector::~GenericSurfDetector(){
	delete m_data;
}

void GenericSurfDetector::setParams(double threshold, int extended,
		int octaves, int octavelayer){
	if(m_data->m_impl == GenericSurfDetector::OPENCV){
		(m_data->m_opencvDetector)->setParams(threshold,extended,octaves,octavelayer);
	}
}

void GenericSurfDetector::setParams(bool upright, int octaves, int intervals,
		int init_samples,float thresh){
	if(m_data->m_impl == GenericSurfDetector::OPENSURF){
		(m_data->m_opensurfDetector)->setParams(upright,octaves,intervals,init_samples,thresh);
	}
}

void GenericSurfDetector::setObjectImg(const ImgBase *objectImg) throw (ICLException){
	if(!objectImg)
		throw ICLException("object image is null");
	if(m_data->m_impl == GenericSurfDetector::OPENCV){
		return (m_data->m_opencvDetector)->setObjectImg(objectImg);
	} else {
		return (m_data->m_opensurfDetector)->setObjectImg(objectImg);
	}
}

///returns back converted image
const ImgBase *GenericSurfDetector::getObjectImg() throw (ICLException){
	if(m_data->m_impl == GenericSurfDetector::OPENCV){
		return (m_data->m_opencvDetector)->getObjectImg();
	} else {
		return (m_data->m_opensurfDetector)->getObjectImg();
	}
}

const std::vector<GenericSurfDetector::GenericPoint> &GenericSurfDetector::getObjectImgFeatures(){
	m_data->m_points.clear();
	if(m_data->m_impl == GenericSurfDetector::OPENCV){
		const std::vector<CvSURFPoint> &points = (m_data->m_opencvDetector)->getObjectImgFeatures();
		m_data->m_points.reserve(points.size());
		for(unsigned int i=0;i< points.size();++i){
			m_data->m_points.push_back(GenericSurfDetector::GenericPoint(new OpenCVGenericPointImpl(&(points[i]))));
		}
	} else {
		const std::vector<Ipoint> &points = (m_data->m_opensurfDetector)->getObjectImgFeatures();
		m_data->m_points.reserve(points.size());
		for(unsigned int i=0;i< points.size();++i){
			m_data->m_points.push_back(GenericSurfDetector::GenericPoint(new OpenSurfGenericPointImpl(&(points[i]))));
		}
	}
	return m_data->m_points;
}

const std::vector<GenericSurfDetector::GenericPoint> &GenericSurfDetector::extractFeatures(const ImgBase *src) throw (ICLException){
	if(!src)
		throw ICLException("src is null");
	m_data->m_fpoints.clear();
	if(m_data->m_impl == GenericSurfDetector::OPENSURF){
		const std::vector<Ipoint> &points = (m_data->m_opensurfDetector)->extractFeatures(src);
		m_data->m_fpoints.reserve(points.size());
		for(unsigned int i=0;i<points.size();++i){
			GenericSurfDetector::GenericPoint p(new OpenSurfGenericPointImpl(&(points[i])));
			m_data->m_fpoints.push_back(p);
		}
	}else {
		const std::vector<CvSURFPoint> &points = (m_data->m_opencvDetector)->extractFeatures(src);
		m_data->m_fpoints.reserve(points.size());
		for(unsigned int i=0;i<points.size();++i){
			GenericSurfDetector::GenericPoint p(new OpenCVGenericPointImpl(&(points[i])));
			m_data->m_fpoints.push_back(p);
		}
	}
	return m_data->m_fpoints;
}

const std::vector<std::pair<GenericSurfDetector::GenericPoint, GenericSurfDetector::GenericPoint> >
&GenericSurfDetector::match(const ImgBase *image) throw (ICLException){
	if(!image)
		throw ICLException("image is null");
	m_data->m_matches.clear();
	if(m_data->m_impl == GenericSurfDetector::OPENSURF){
		const std::vector<std::pair<Ipoint, Ipoint> > &matches = (m_data->m_opensurfDetector)->match(image);
		(m_data->m_matches).reserve(matches.size());
		for(unsigned int i=0;i<matches.size();++i){
			GenericSurfDetector::GenericPoint fi(new OpenSurfGenericPointImpl(&(matches[i].first)));
			GenericSurfDetector::GenericPoint se(new OpenSurfGenericPointImpl(&(matches[i].second)));
			std::pair<GenericSurfDetector::GenericPoint,GenericSurfDetector::GenericPoint> pa(fi,se);
			(m_data->m_matches).push_back(pa);
		}
	}else {
		const std::vector<std::pair<CvSURFPoint, CvSURFPoint> > &matches = (m_data->m_opencvDetector)->match(image);
		(m_data->m_matches).reserve(matches.size());
		for(unsigned int i=0;i<matches.size();++i){
			GenericSurfDetector::GenericPoint fi(new OpenCVGenericPointImpl(&(matches[i].first)));
			GenericSurfDetector::GenericPoint se(new OpenCVGenericPointImpl(&(matches[i].second)));
			std::pair<GenericSurfDetector::GenericPoint,GenericSurfDetector::GenericPoint> pa(fi,se);
			(m_data->m_matches).push_back(pa);
		}
	}
	return m_data->m_matches;
}

void GenericSurfDetector::setRotationInvariant(bool upright) throw (ICLException){
	if(m_data->m_impl == GenericSurfDetector::OPENSURF){
		m_data->m_opensurfDetector->setRotationInvariant(upright);
	}else{
		throw ICLException("this is not for opencv");
	}
}

void GenericSurfDetector::setOctaves(int octaves){
	if(m_data->m_impl == GenericSurfDetector::OPENCV){
		m_data->m_opencvDetector->setOctaves(octaves);
	}else{
		m_data->m_opensurfDetector->setOctaves(octaves);
	}
}

void GenericSurfDetector::setOctavelayer(int octavelayer){
	if(m_data->m_impl == GenericSurfDetector::OPENCV){
		m_data->m_opencvDetector->setOctavelayer(octavelayer);
	}else{
		m_data->m_opensurfDetector->setIntervals(octavelayer);
	}
}

void GenericSurfDetector::setInitSamples(int init_samples) throw (ICLException){
	if(m_data->m_impl == GenericSurfDetector::OPENSURF
			&& m_data->m_opensurfDetector != 0){
		m_data->m_opensurfDetector->setInitSamples(init_samples);
	}else{
		throw ICLException("this is not for opencv");
	}
}

void GenericSurfDetector::setThreshold(double threshold){
	if(m_data->m_impl == GenericSurfDetector::OPENCV){
		m_data->m_opencvDetector->setThreshold(threshold);
	}else{
		m_data->m_opensurfDetector->setRespThresh(threshold);
	}
}

void GenericSurfDetector::setExtended(int extended) throw (ICLException){
	if(m_data->m_impl == GenericSurfDetector::OPENCV){
		m_data->m_opencvDetector->setExtended(extended);
	}else{
		throw ICLException("this not for opensurf");
	}
}

bool GenericSurfDetector::getRotationInvariant() throw (ICLException){
	if(m_data->m_impl == GenericSurfDetector::OPENSURF
			&& m_data->m_opensurfDetector != 0){
		return m_data->m_opensurfDetector->getRotationInvariant();
	}else{
		throw ICLException("this is not for opencv");
	}
}

int GenericSurfDetector::getOctaves(){
	if(m_data->m_impl == GenericSurfDetector::OPENCV){
		return m_data->m_opencvDetector->getOctaves();
	}else{
		return m_data->m_opensurfDetector->getOctaves();
	}
}

int GenericSurfDetector::getOctavelayer(){
	if(m_data->m_impl == GenericSurfDetector::OPENCV){
		return m_data->m_opencvDetector->getOctavelayer();
	}else{
		return m_data->m_opensurfDetector->getIntervals();
	}
}

int GenericSurfDetector::getInitSamples() throw (ICLException){
	if(m_data->m_impl == GenericSurfDetector::OPENSURF){
		return m_data->m_opensurfDetector->getInitSamples();
	}else{
		throw ICLException("This is not for opencvsurfdetector");
	}
}

double GenericSurfDetector::getThreshold(){
	if(m_data->m_impl == GenericSurfDetector::OPENCV){
		return m_data->m_opencvDetector->getThreshold();
	}else {
		return m_data->m_opensurfDetector->getRespThresh();
	}
}

int GenericSurfDetector::getExtended() throw (ICLException){
	if(m_data->m_impl == GenericSurfDetector::OPENCV
			&& m_data->m_opencvDetector != 0){
		return m_data->m_opencvDetector->getExtended();
	} else {
		throw ICLException("this is not for opensurf");
	}
}


#ifdef HAVE_QT
void GenericSurfDetector::visualizeFeature(ICLDrawWidget &w,const GenericSurfDetector::GenericPoint &p){
	const OpenCVGenericPointImpl *cvpoint = dynamic_cast<const OpenCVGenericPointImpl*> (p.impl.get());
	if(cvpoint == 0){
		const OpenSurfGenericPointImpl *surfpoint = dynamic_cast<const OpenSurfGenericPointImpl*> (p.impl.get());
		OpenSurfDetector::visualizeFeature(w,*(surfpoint->p));
	} else {
		OpenCVSurfDetector::visualizeFeature(w,*(cvpoint->p));
	}
}
void GenericSurfDetector::visualizeFeatures(ICLDrawWidget &w, const std::vector<GenericSurfDetector::GenericPoint> &features){
	for(unsigned int i=0;i<features.size();++i)
		visualizeFeature(w,features.at(i));
}
void GenericSurfDetector::visualizeMatches(ICLDrawWidget &w_object,ICLDrawWidget &w_result,
		const std::vector<std::pair<GenericSurfDetector::GenericPoint, GenericSurfDetector::GenericPoint> > &matches){
	for(unsigned int i=0;i<matches.size();++i){
		visualizeFeature(w_object, matches[i].second);
		visualizeFeature(w_result,matches[i].first);
	}
}
#endif

}
