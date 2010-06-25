/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
 **                         University of Bielefeld                 **
 **                Contact: nivision@techfak.uni-bielefeld.de       **
 **                Website: www.iclcv.org                           **
 **                                                                 **
 ** File   : ICLFilter/src/OpenCVSurfDetector.cpp                   **
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

#include <ICLAlgorithms/OpenCVSurfDetector.h>
using namespace icl;
namespace icl{

struct OpenCVSurfDetector::Data{
	//copy of the original object image
	IplImage *m_objectImg_org;
	//working copy of the original object image
	IplImage *m_objectImg;
	//temporary buffer
	IplImage *m_tmp_ipl_img;
	//default value = 500;
	float m_thresh;
	//default value  = 1;
	int m_extended;
	//default value  = 3;
	int m_octaves;
	//default value  = 4;
	int m_octavelayer;

	std::vector<CvSURFPoint> m_obj_ipts;

	std::vector<CvSURFPoint> m_ipts;

	//matches
	std::vector<std::pair<CvSURFPoint, CvSURFPoint> > m_matches;

	CvSeq *m_objectKeypoints;
	CvSeq *m_objectDescriptors;

	CvSeq *m_imageKeypoints;
	CvSeq *m_imageDescriptors;

	CvMemStorage* m_storage;
	CvSURFParams m_params;

	CvSeqReader m_objreader;
	CvSeqReader m_imgreader;

	int extended;
	double hessianThreshold;

	int nOctaves;
	int nOctaveLayers;

	Data(double threshold=500, int extended=1, int octaves=3, int octavelayer=4):
		m_objectImg_org(0),m_objectImg(0),m_tmp_ipl_img(0),m_thresh(threshold),m_extended(extended),
		m_octaves(octaves),m_octavelayer(octavelayer),m_objectKeypoints(0),
		m_objectDescriptors(0),m_imageKeypoints(0),m_imageDescriptors(0),
		m_storage(cvCreateMemStorage(0)),m_params(cvSURFParams(threshold, extended)){}

	~Data(){
		if(m_objectImg)
			cvReleaseImage(&m_objectImg);
		if(m_objectImg_org)
			cvReleaseImage(&m_objectImg_org);
		if(m_tmp_ipl_img)
			cvReleaseImage(&m_tmp_ipl_img);
		if(m_storage)
			cvReleaseMemStorage(&m_storage);
	}
};


OpenCVSurfDetector::OpenCVSurfDetector(const ImgBase *obj,double threshold,
		int extended, int octaves, int octavelayer):
		m_data(new OpenCVSurfDetector::Data(threshold, extended, octaves, octavelayer)){
	if(obj)
		setObjectImg(obj);
}

OpenCVSurfDetector::~OpenCVSurfDetector(){
	delete m_data;
}

void OpenCVSurfDetector::setObjectImg(const ImgBase *objectImage) throw (ICLException){
	if(!objectImage)
		throw ICLException("object image is null");
	//release all buffers
	if(m_data->m_objectImg)
		cvReleaseImage(&(m_data->m_objectImg));
	if(m_data->m_objectImg_org)
		cvReleaseImage(&(m_data->m_objectImg_org));
	if(m_data->m_tmp_ipl_img)
		cvReleaseImage(&(m_data->m_tmp_ipl_img));
	if(objectImage){
		icl::img_to_ipl(objectImage,&(m_data->m_objectImg_org));
		//only on greyscale
		(m_data->m_objectImg) = cvCreateImage(cvGetSize(m_data->m_objectImg_org), IPL_DEPTH_8U, 1);
		if((m_data->m_objectImg_org)->nChannels > 1){
			cvCvtColor(m_data->m_objectImg_org ,m_data->m_objectImg, CV_RGB2GRAY);
		}else{
			cvCopy(m_data->m_objectImg_org, m_data->m_objectImg);
		}
		computeSURF();
	}else{
		//set default values
		(m_data->m_obj_ipts).clear();
		m_data->m_octaves = 3;
		m_data->m_octavelayer = 4;
		m_data->m_extended = 1;
		m_data->m_thresh= 500;
	}
}

void OpenCVSurfDetector::computeSURF(){
	// Extract surf points
	cvExtractSURF(m_data->m_objectImg, 0, &(m_data->m_objectKeypoints),
			&(m_data->m_objectDescriptors),m_data->m_storage, m_data->m_params );
	(m_data->m_obj_ipts).clear();
	for(int i = 0; i < m_data->m_objectKeypoints->total; i++ ){
		CvSURFPoint *r = (CvSURFPoint*) cvGetSeqElem(m_data->m_objectKeypoints, i);
		(m_data->m_obj_ipts).push_back(*r);
	}
}

const ImgBase *OpenCVSurfDetector::getObjectImg() throw (ICLException){
	if(m_data->m_objectImg_org)
		return icl::ipl_to_img(m_data->m_objectImg_org);
	else
		throw ICLException("Object image is null");
}

void OpenCVSurfDetector::setThreshold(double threshold){
	if(m_data->m_thresh != threshold){
		m_data->m_thresh = threshold;
		computeSURF();
	}
}

void OpenCVSurfDetector::setOctaves(int octaves){
	if(m_data->m_octaves != octaves){
		m_data->m_octaves = octaves;
		computeSURF();
	}
}

void OpenCVSurfDetector::setOctavelayer(int octavelayer){
	if(m_data->m_octavelayer != octavelayer){
		m_data->m_octavelayer = octavelayer;
		computeSURF();
	}
}

void OpenCVSurfDetector::setExtended(int extended){
	if(extended == 0 || extended == 1)
		if(m_data->m_extended != extended){
			m_data->m_extended = extended;
			computeSURF();
		}
}

int OpenCVSurfDetector::getOctaves(){
	return m_data->m_octaves;
}

int OpenCVSurfDetector::getOctavelayer(){
	return m_data->m_octavelayer;
}

int OpenCVSurfDetector::getExtended(){
	return m_data->m_extended;
}

double OpenCVSurfDetector::getThreshold(){
	return m_data->m_thresh;
}

void OpenCVSurfDetector::setParams(double threshold, int extended,
		int octaves, int octavelayer){
	bool changed = false;
	if(m_data->m_octaves != octaves){
		m_data->m_octaves = octaves;
		changed = true;
	}
	if(m_data->m_octavelayer != octavelayer){
		m_data->m_octavelayer = octavelayer;
		changed = true;
	}
	if(m_data->m_extended != extended){
		m_data->m_extended = extended;
		changed = true;
	}
	if(m_data->m_thresh != threshold){
		m_data->m_thresh = threshold;
		changed = true;
	}
	if(changed)
		computeSURF();
}

const std::vector<CvSURFPoint> &OpenCVSurfDetector::extractFeatures(const ImgBase *src)	throw (ICLException){
	if(!src)
		throw ICLException("image is null");
	(m_data->m_ipts).clear();
	if(src){
		IplImage *ipl = 0;
		icl::img_to_ipl(src,&ipl);
		//only on greyscale
		(m_data->m_tmp_ipl_img) = cvCreateImage(cvGetSize(ipl), IPL_DEPTH_8U, 1);
		if(ipl->nChannels > 1){
			cvCvtColor(ipl ,m_data->m_tmp_ipl_img, CV_RGB2GRAY);
		}else{
			cvCopy(ipl, m_data->m_tmp_ipl_img);
		}
		//only on greyscale
		cvExtractSURF(m_data->m_tmp_ipl_img, 0, &(m_data->m_imageKeypoints),
				&(m_data->m_imageDescriptors),m_data->m_storage, m_data->m_params);
		m_data->m_ipts.reserve(m_data->m_imageKeypoints->total);
		for(int i = 0; i < m_data->m_imageKeypoints->total; i++ ){
			CvSURFPoint *r = (CvSURFPoint*) cvGetSeqElem(m_data->m_imageKeypoints, i);
			(m_data->m_ipts).push_back(*r);
		}
		cvReleaseImage(&(m_data->m_tmp_ipl_img));
		cvReleaseImage(&ipl);
	}
	return m_data->m_ipts;
}

int OpenCVSurfDetector::nearestNeighbor(const float* vec, int laplacian,
		const CvSeq* model_keypoints, const CvSeq* model_descriptors){
	int length = (int)(model_descriptors->elem_size/sizeof(float));
	if(length % 4 != 0)
		return -1;
	int i, neighbor = -1;
	double d, dist1 = 1e6, dist2 = 1e6;
	CvSeqReader reader, kreader;
	cvStartReadSeq( model_keypoints, &kreader, 0 );
	cvStartReadSeq( model_descriptors, &reader, 0 );
	register double t0,t1,t2,t3;
	for( i = 0; i < model_descriptors->total; i++ ){
		const CvSURFPoint* kp = (const CvSURFPoint*)kreader.ptr;
		const float* mvec = (const float*)reader.ptr;
		CV_NEXT_SEQ_ELEM( kreader.seq->elem_size, kreader );
		CV_NEXT_SEQ_ELEM( reader.seq->elem_size, reader );
		if( laplacian != kp->laplacian )
			continue;
		d = 0;
		for( int j = 0; j < length; j += 4 ){
			t0 = vec[j] - mvec[j];
			t1 = vec[j+1] - mvec[j+1];
			t2 = vec[j+2] - mvec[j+2];
			t3 = vec[j+3] - mvec[j+3];
			d += t0*t0 + t1*t1 + t2*t2 + t3*t3;
			if( d > dist2 )
				break;
		}
		if( d < dist1 ){
			dist2 = dist1;
			dist1 = d;
			neighbor = i;
		}else if ( d < dist2 )
			dist2 = d;
	}
	if ( dist1 < 0.6*dist2 )
		return neighbor;
	return -1;
}

const std::vector<std::pair<CvSURFPoint, CvSURFPoint> > &OpenCVSurfDetector::match(const ImgBase *image) throw (ICLException){
	if(!image)
		throw ICLException("image is null");
	if(!(m_data->m_objectImg))
		throw ICLException("object image is null");
	(m_data->m_matches).clear();
	if(m_data->m_objectImg_org){
		extractFeatures(image);
		cvStartReadSeq(m_data->m_objectKeypoints, &(m_data->m_objreader));
		cvStartReadSeq(m_data->m_objectDescriptors, &(m_data->m_imgreader));
		int nearest_neighbor = -1;
		CvSURFPoint* om = 0;
		CvSURFPoint* im = 0;
		for(int i = 0; i < (m_data->m_objectDescriptors)->total; i++ ){
			const CvSURFPoint* kp = (const CvSURFPoint*)(m_data->m_objreader).ptr;
			const float* descriptor = (const float*)(m_data->m_imgreader).ptr;
			CV_NEXT_SEQ_ELEM((m_data->m_objreader).seq->elem_size, (m_data->m_objreader));
			CV_NEXT_SEQ_ELEM((m_data->m_imgreader).seq->elem_size, (m_data->m_imgreader));
			nearest_neighbor = nearestNeighbor( descriptor, kp->laplacian, m_data->m_imageKeypoints, m_data->m_imageDescriptors );
			//TODO copy implementation of naiveNearestNeighbor

			//TODO end
			if( nearest_neighbor >= 0 ){
				om = (CvSURFPoint*)cvGetSeqElem((m_data->m_objectKeypoints),i);
				im = (CvSURFPoint*)cvGetSeqElem((m_data->m_imageKeypoints), nearest_neighbor);
				std::pair<CvSURFPoint,CvSURFPoint> p(*im,*om);
				(m_data->m_matches).push_back(p);
			}
		}
	}
	return m_data->m_matches;
}

const std::vector<CvSURFPoint> &OpenCVSurfDetector::getObjectImgFeatures(){
	return m_data->m_obj_ipts;
}

#ifdef HAVE_QT
void OpenCVSurfDetector::visualizeFeature(ICLDrawWidget &w,const CvSURFPoint &p){
	CvPoint center;
	int radius;
	center.x = cvRound(p.pt.x);
	center.y = cvRound(p.pt.y);
	radius = cvRound(p.size*1.2/9.*2);
	w.nofill();
	w.color(0,255,0,255);
	w.circle(center.x,center.y,radius);
}

void OpenCVSurfDetector::visualizeFeatures(ICLDrawWidget &w, const std::vector<CvSURFPoint> &features){
	for(unsigned int i=0;i<features.size();++i)
		visualizeFeature(w,features.at(i));
}

void OpenCVSurfDetector::visualizeMatches(ICLDrawWidget &w_object,ICLDrawWidget &w_result,
		std::vector<std::pair<CvSURFPoint, CvSURFPoint> > &matches){
	for(unsigned int i=0;i<matches.size();++i){
		visualizeFeature(w_object, matches[i].first);
		visualizeFeature(w_result,matches[i].second);
	}
}
#endif
}
