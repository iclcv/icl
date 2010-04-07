/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLOpenCV/src/OpenCV.cpp                               **
** Module : ICLOpenCV                                              **
** Authors: Christof Elbrechter                                    **
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
#ifdef HAVE_OPENCV
#include <ICLOpenCV/OpenCV.h>
namespace icl{

IplImage *ensureCompatible(IplImage **dst, int depth,const CvSize& size,int channels){
	if(!dst || !*dst){
		*dst = cvCreateImage(size,depth,channels);
		return *dst;
	}
	if((*dst)->depth !=  depth || (*dst)->nChannels != channels || size.height != (*dst)->height
			|| size.width != (*dst)->width){
		//oder einfach 
		cvReleaseImage(dst);
		*dst=cvCreateImage(size,depth,channels);

		/*cvReleaseData(*dst);
		(*dst)->depth = depth;
		(*dst)->nChannels = channels;
		(*dst)->height = size.height;
		(*dst)->width = size.width;
		(*dst)->origin = 0;
		//TODO
		(*dst)->widthStep = channels*size.width;
		(*dst)->nSize = sizeof(IplImage);
		(*dst)->ID = 0;
		(*dst)->dataOrder = 0;
		(*dst)->roi =0;
		(*dst)->maskROI = 0;
		(*dst)->imageId = 0;
		(*dst)->tileInfo = 0;
		(*dst)->imageSize = size.height*size.width*channels;
		switch(depth){
			case IPL_DEPTH_8U:{
				(*dst)->imageData = new char[sizeof(icl8u)*size.width*size.height*channels];
				break;}
			case IPL_DEPTH_8S:{
				//in this case we use icl16s
				(*dst)->imageData = new char[sizeof(signed char)*size.width*size.height*channels];
				break;}
			case IPL_DEPTH_16S:{
				(*dst)->imageData = new char[sizeof(icl16s)*size.width*size.height*channels];
				break;}
			case IPL_DEPTH_32S:{
				(*dst)->imageData = new char[sizeof(icl32s)*size.width*size.height*channels];
				break;}
			case IPL_DEPTH_32F:{
				(*dst)->imageData = new char[sizeof(icl32f)*size.width*size.height*channels];
				break;}
			case IPL_DEPTH_64F:{
				(*dst)->imageData = new char[sizeof(icl64f)*size.width*size.height*channels];
				break;}
			default :{
				//this should not happen
				throw ICLException("Invalid source depth");
			}
		}
		(*dst)->imageDataOrigin = (*dst)->imageData;*/
	}
	return *dst;
}

template<typename DST_T>
inline Img<DST_T> *ipl_to_img_dstpref(CvArr *src,Img<DST_T> *dst){
	IplImage imageHeader;
	IplImage* image = cvGetImage(src, &imageHeader);
	dst->setParams(ImgParams(Size(image->width,image->height),image->nChannels));
	switch(image->depth){
	case IPL_DEPTH_8U:{
		interleavedToPlanar((icl8u*)image->imageData,dst,image->widthStep);
		break;}
	case IPL_DEPTH_8S:{
		//in this case we use icl16s
		interleavedToPlanar((signed char*)image->imageData,dst,image->widthStep);
		break;}
	case IPL_DEPTH_16S:{
		interleavedToPlanar((icl16s*)image->imageData,dst,image->widthStep);
		break;}
	case IPL_DEPTH_32S:{
		interleavedToPlanar((icl32s*)image->imageData,dst,image->widthStep);
		break;}
	case IPL_DEPTH_32F:{
		interleavedToPlanar((icl32f*)image->imageData,dst,image->widthStep);
		break;}
	case IPL_DEPTH_64F:{
		interleavedToPlanar((icl64f*)image->imageData,dst,image->widthStep);
		break;}
	default :{
		//this should not happen
		throw ICLException("Invalid source depth");
	}
	}
	return dst;
}

template<typename SRC_T,typename DST_T>
inline void ipl_to_img_srcpref(IplImage *src, ImgBase **dst){
	ensureCompatible(dst,getDepth<DST_T>(),Size(src->width,src->height),src->nChannels);
	SRC_T *data = (SRC_T*) src->imageData;
	interleavedToPlanar(data,(*dst)->asImg<DST_T>(),src->widthStep);
}

ImgBase *ipl_to_img(CvArr *src,ImgBase **dst,DepthPreference e) throw (icl::ICLException){
	if(!src){
		throw icl::ICLException("Source is NULL");
	}
	IplImage imageHeader;
	IplImage* image = cvGetImage(src, &imageHeader);
	if(dst && *dst && e==PREFERE_DST_DEPTH){
		switch((*dst)->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                               \
		case depth##D:{						                   \
			ipl_to_img_dstpref<icl##D>(image,(*dst)->asImg<icl##D>());   \
			break;}
		ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
		}
	} else if(e==PREFERE_DST_DEPTH){
		throw icl::ICLException("Cannot determine depth of destinationimage");
	} else { // DepthPreference == PREFERE_SRC_DEPTH
		switch(image->depth){
		case IPL_DEPTH_8U:{
			ipl_to_img_srcpref<icl8u,icl8u>(image,dst);
			break;}
		case IPL_DEPTH_8S:{
			//in this case we use icl16s
			ipl_to_img_srcpref<signed char,icl16s>(image,dst);
			break;}
		case IPL_DEPTH_16S:{
			ipl_to_img_srcpref<icl16s,icl16s>(image,dst);
			break;}
		case IPL_DEPTH_32S:{
			ipl_to_img_srcpref<icl32s,icl32s>(image,dst);
			break;}
		case IPL_DEPTH_32F:{
			ipl_to_img_srcpref<icl32f,icl32f>(image,dst);
			break;}
		case IPL_DEPTH_64F:{
			ipl_to_img_srcpref<icl64f,icl64f>(image,dst);
			break;}
		default :{
			//this should not happen
			throw ICLException("Invalid source depth");
		}
		}
	}
	for(int i=0;i<(*dst)->getChannels()/2;++i){
		(*dst)->swapChannels(i,(*dst)->getChannels()-1-i);
	}
	return *dst;
}

template<typename SRC_T,typename DST_T>
inline void img_to_ipl_srcpref(const ImgBase *src, IplImage **dst){
	switch(src->getDepth()){
	case depth8u:{
		ensureCompatible(dst,IPL_DEPTH_8U,cvSize(src->getWidth(),src->getHeight()),src->getChannels());
		break;}
	case depth16s:{
		ensureCompatible(dst,IPL_DEPTH_16S,cvSize(src->getWidth(),src->getHeight()),src->getChannels());
		break;}
	case depth32s:{
		ensureCompatible(dst,IPL_DEPTH_32S,cvSize(src->getWidth(),src->getHeight()),src->getChannels());
		break;}
	case depth32f:{
		ensureCompatible(dst,IPL_DEPTH_32F,cvSize(src->getWidth(),src->getHeight()),src->getChannels());
		break;}
	case depth64f:{
		ensureCompatible(dst,IPL_DEPTH_64F,cvSize(src->getWidth(),src->getHeight()),src->getChannels());
		break;}
	default :{
		//this should not happen
		throw ICLException("Invalid source depth");
	}
	}
	Img<SRC_T> tmp = *src->asImg<SRC_T>();
	for(int i=0;i<src->getChannels()/2;++i){
		tmp.swapChannels(i,src->getChannels()-1-i);
	}
	planarToInterleaved(&tmp,(DST_T*)(*dst)->imageData,(*dst)->widthStep);
}

template<typename SRC_T>
inline CvArr *img_to_ipl_dstpref(Img<SRC_T> *src,CvArr *dst){
	IplImage imageHeader;
	IplImage* image = cvGetImage(dst, &imageHeader);
	for(int i=0;i<src->getChannels()/2;++i){
		src->swapChannels(i,src->getChannels()-1-i);
	}
	switch(image->depth){
	case IPL_DEPTH_8U:{
		//TODO
		dst=ensureCompatible(&image,IPL_DEPTH_8U,cvSize(src->getWidth(),src->getHeight()),src->getChannels());
		planarToInterleaved(src,(icl8u*)image->imageData);
		break;}
	case IPL_DEPTH_8S:{
		//in this case we use icl16s
		planarToInterleaved(src,(signed char*)image->imageData);
		break;}
	case IPL_DEPTH_16S:{
		planarToInterleaved(src,(icl16s*)image->imageData);
		break;}
	case IPL_DEPTH_32S:{
		planarToInterleaved(src,(icl32s*)image->imageData);
		break;}
	case IPL_DEPTH_32F:{
		planarToInterleaved(src,(icl32f*)image->imageData);
		break;}
	case IPL_DEPTH_64F:{
		planarToInterleaved(src,(icl64f*)image->imageData);
		break;}
	default :{
		//this should not happen
		throw ICLException("Invalid source depth");
	}
	}
	return dst;
}

IplImage *img_to_ipl(const ImgBase *src, IplImage **dst,DepthPreference e) throw (ICLException){
	if(!src){
		throw ICLException("Source is NULL");
	}
	if(dst && *dst && e==PREFERE_DST_DEPTH){
		switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                               \
		case depth##D:{                                         \
			Img<icl##D> tmp = *src->asImg<icl##D>();                     \
			img_to_ipl_dstpref(&tmp,*dst);              \
			break;}
		ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
		}
	} else if(e==PREFERE_DST_DEPTH){
		throw ICLException("Cannot determine depth of destinationimage");
	} else { // DepthPreference == PREFERE_SRC_DEPTH
		switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                               \
		case depth##D:{						                   \
			img_to_ipl_srcpref<icl##D,icl##D>(src,dst);   \
			break;}
		ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
		}
	}
	return *dst;
}


template<typename T>
inline CvMat* img_2_cvmat(const Img<T> *src, CvMat *dst,int channel){
	const T *srcdata = src->begin(channel);
	int dim = src->getSize().width*src->getSize().height;
	int ltype = int(dst->type & CV_MAT_DEPTH_MASK);
	switch(ltype){
	case CV_8UC1:{
		for(int i=0;i<dim;++i){
			dst->data.ptr[i]=(icl8u)srcdata[i];
		}
		break;
	}
	case CV_16SC1:{
		for(int i=0;i<dim;++i){
			dst->data.s[i]=(short)srcdata[i];
		}
		break;
	}
	case CV_32SC1:{
		for(int i=0;i<dim;++i){
			dst->data.i[i]=(int)srcdata[i];
		}
		break;
	}
	case CV_32FC1:{
		for(int i=0;i<dim;++i){
			dst->data.fl[i]=icl32f(srcdata[i]);
		}
		break;
	}
	case CV_64FC1:{
		for(int i=0;i<dim;++i){
			dst->data.db[i]=(icl64f)srcdata[i];
		}
		break;
	}
	default :{
		throw ICLException("Unknown or not possible type of dst");
	}
	}
	return dst;
}

CvMat* img_to_cvmat(const ImgBase *src, CvMat *dst,int channel) throw (icl::ICLException){
	if(!src){
		throw ICLException("Source is NULL");
	}
	//channelcheck
	if(channel<0 || channel>=src->getChannels()){
		throw ICLException("Invalid channel");
	}

	if(!dst ||src->getWidth() != dst->width || src->getHeight() != dst->height){
		if(!dst)
			delete dst;
		switch(src->getDepth()){
		case depth8u: {
			dst=cvCreateMat(src->getWidth(),src->getHeight(),CV_8UC1);
			break;
		}
		case depth16s:{
			dst=cvCreateMat(src->getWidth(),src->getHeight(),CV_16SC1);
			break;
		}
		case depth32s :{
			dst=cvCreateMat(src->getWidth(),src->getHeight(),CV_32SC1);
			break;
		}
		case depth32f: {
			dst=cvCreateMat(src->getWidth(),src->getHeight(),CV_32FC1);
			break;
		}
		case depth64f: {
			dst=cvCreateMat(src->getWidth(),src->getHeight(),CV_64FC1);
			break;
		}
		default :{
			throw ICLException("Invalid source depth");
		}
		}
	}

	switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                               \
		case depth##D:{						                   \
			img_2_cvmat(src->asImg<icl##D>(),dst,channel);     \
			break;}
	ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
	}
	return dst;
}


CvMat *ensureCompatible(CvMat **dst, int depth,int rows, int cols){
	if(!dst || !*dst){
		*dst = cvCreateMat(rows,cols,depth);
		return *dst;
	}
	int ltype = int(depth & CV_MAT_DEPTH_MASK);
	if((*dst)->type !=  ltype || (*dst)->rows != rows
			|| (*dst)->cols != cols){
		cvReleaseMat(dst);
		*dst = cvCreateMat(rows,cols,depth);
	}
	return *dst;
}

CvMat *img_to_cvmat_shallow(const ImgBase *src,CvMat *dst) throw (ICLException){
	if(!src){
		throw icl::ICLException("Source is NULL");
	}
	if(src->getChannels()>1){
		throw icl::ICLException("to many channels for shallow copy");
	}
	switch(src->getDepth()){
	case depth8u:{
		ensureCompatible(&dst, CV_8UC1,src->getHeight(),src->getHeight());
		cvReleaseData(dst);
		dst->data.ptr = (unsigned char*)((src->asImg<icl8u>())->begin(0));
		break;}
	case depth16s:{
		ensureCompatible(&dst, CV_16SC1,src->getHeight(),src->getHeight());
		cvReleaseData(dst);
		dst->data.s = (short*)((src->asImg<icl16s>())->begin(0));
		break;}
	case depth32s:{
		ensureCompatible(&dst, CV_32SC1,src->getHeight(),src->getHeight());
		cvReleaseData(dst);
		dst->data.i = (int*)((src->asImg<icl32s>())->begin(0));
		break;}
	case depth32f:{
		ensureCompatible(&dst, CV_32FC1,src->getHeight(),src->getHeight());
		cvReleaseData(dst);
		dst->data.fl = (float*)((src->asImg<icl32f>())->begin(0));
		break;}
	case depth64f:{
		ensureCompatible(&dst, CV_64FC1,src->getHeight(),src->getHeight());
		cvReleaseData(dst);
		dst->data.db = (double*)((src->asImg<icl64f>())->begin(0));
		break;}
	default :{
		//this should not happen
		throw ICLException("Invalid source depth");
	}
	}
	return dst;
}

IplImage *img_to_ipl_shallow(ImgBase *src,IplImage *dst)throw (ICLException){
	if(!src){
		throw icl::ICLException("Source is NULL");
	}
	if(src->getChannels()>1){
		throw icl::ICLException("to many channels for shallow copy");
	}
	switch(src->getDepth()){
	case depth8u:{
		ensureCompatible(&dst, IPL_DEPTH_8U,cvSize(src->getWidth(),src->getHeight()),1);
		cvReleaseData(dst);
		dst->imageData = (char*)((src->asImg<icl8u>())->begin(0));
		break;}
	case depth16s:{
		ensureCompatible(&dst, IPL_DEPTH_16S,cvSize(src->getWidth(),src->getHeight()),1);
		cvReleaseData(dst);
		dst->imageData = (char*)((src->asImg<icl16s>())->begin(0));
		break;}
	case depth32s:{
		ensureCompatible(&dst, IPL_DEPTH_32S,cvSize(src->getWidth(),src->getHeight()),1);
		cvReleaseData(dst);
		dst->imageData = (char*)((src->asImg<icl32s>())->begin(0));
		break;}
	case depth32f:{
		ensureCompatible(&dst, IPL_DEPTH_32F,cvSize(src->getWidth(),src->getHeight()),1);
		cvReleaseData(dst);
		dst->imageData = (char*)((src->asImg<icl32f>())->begin(0));
		break;}
	case depth64f:{
		ensureCompatible(&dst, IPL_DEPTH_64F,cvSize(src->getWidth(),src->getHeight()),1);
		cvReleaseData(dst);
		dst->imageData = (char*)((src->asImg<icl64f>())->begin(0));
		break;}
	default :{
		//this should not happen
		throw ICLException("Invalid source depth");
	}
	}
	dst->imageDataOrigin = dst->imageData;
	return dst;
}
}
#endif

