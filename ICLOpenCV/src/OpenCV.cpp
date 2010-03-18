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
#include <ICLOpenCV/OpenCV.h>
namespace icl{

#ifdef HAVE_OPENCV

IplImage *ensureCompatible(IplImage **dst, int depth,const CvSize& size,int channels){
	if(!dst || !*dst){
		*dst = cvCreateImage(cvSize(size.width,size.height),depth,channels);
		return *dst;
	}
	if((*dst)->depth !=  depth || (*dst)->nChannels != channels || size.height != (*dst)->height
			|| size.width != (*dst)->width){
		(*dst)->depth = depth;
		(*dst)->nChannels = channels;
		(*dst)->height = size.height;
		(*dst)->width = size.width;
		(*dst)->origin = 0;
		(*dst)->widthStep = channels*size.width;
		(*dst)->nSize = sizeof(IplImage);
		(*dst)->ID = 0;
		(*dst)->dataOrder = 0;
		(*dst)->roi =0;
		(*dst)->maskROI = 0;
		(*dst)->imageId = 0;
		(*dst)->tileInfo = 0;
		(*dst)->imageSize = size.height*size.width*channels;
	}
	return *dst;
}


template<typename SRC_T,typename DST_T>
void img_to_ipl_srcpref(const ImgBase *src, IplImage **dst){
	switch(src->getDepth()){
	case depth8u:{
		ensureCompatible(dst,IPL_DEPTH_8U,cvSize(src->getWidth(),src->getHeight()),src->getChannels());
		*dst = cvCreateImage(cvSize(src->getWidth(),src->getHeight()),IPL_DEPTH_8U, src->getChannels());
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
	planarToInterleaved(&tmp,(DST_T*)(*dst)->imageData);//,(*dst)->widthStep);
}

template<typename SRC_T>
CvArr *img_to_ipl_dstpref(Img<SRC_T> *src,CvArr *dst){
	ICLASSERT(src);
	ICLASSERT(dst);
	IplImage imageHeader;
	IplImage* image = cvGetImage(dst, &imageHeader);
	for(int i=0;i<src->getChannels()/2;++i){
		src->swapChannels(i,src->getChannels()-1-i);
	}
	switch(image->depth){
	case IPL_DEPTH_8U:{
		planarToInterleaved(src,(icl8u*)image->imageData);
		break;}
	case IPL_DEPTH_8S:{
		//in this case we use icl16s
		planarToInterleaved(src,(signed char*)image->imageData);
		break;}
	case IPL_DEPTH_16S:{
		cout << src->getLineStep() << endl;
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

template<typename DST_T>
Img<DST_T> *ipl_to_img_dstpref(CvArr *src,Img<DST_T> *dst){
	ICLASSERT_RETURN_VAL(dst,dst);
	IplImage imageHeader;
	IplImage* image = cvGetImage(src, &imageHeader);
	ImgBase *ib=dst;
	ensureCompatible(&ib,dst->getDepth(),Size(image->width,image->height),image->nChannels);
	switch(image->depth){
	case IPL_DEPTH_8U:{
		interleavedToPlanar((icl8u*)image->imageData,dst,image->nChannels*image->width);
		break;}
	case IPL_DEPTH_8S:{
		//in this case we use icl16s
		interleavedToPlanar((signed char*)image->imageData,dst,image->nChannels*image->width);
		break;}
	case IPL_DEPTH_16S:{
		interleavedToPlanar((icl16s*)image->imageData,dst,image->nChannels*image->width);
		break;}
	case IPL_DEPTH_32S:{
		interleavedToPlanar((icl32s*)image->imageData,dst,image->nChannels*image->width);
		break;}
	case IPL_DEPTH_32F:{
		interleavedToPlanar((icl32f*)image->imageData,dst,image->nChannels*image->width);
		break;}
	case IPL_DEPTH_64F:{
		interleavedToPlanar((icl64f*)image->imageData,dst,image->nChannels*image->width);
		break;}
	default :{
		//this should not happen
		throw ICLException("Invalid source depth");
	}
	}
	return dst;
}

template<typename SRC_T,typename DST_T>
void ipl_to_img_srcpref(IplImage *src, ImgBase **dst){
	ICLASSERT(src);
	ICLASSERT(dst);
	IplImage imageHeader;
	IplImage* image = cvGetImage(src, &imageHeader);
	switch(image->depth){
	case IPL_DEPTH_8U:{
		ensureCompatible(dst,depth8u,Size(image->width,image->height),image->nChannels);
		break;}
	case IPL_DEPTH_8S:{
		//in this case we use icl16s
		ensureCompatible(dst,depth16s,Size(image->width,image->height),image->nChannels);
		break;}
	case IPL_DEPTH_16S:{
		ensureCompatible(dst,depth16s,Size(image->width,image->height),image->nChannels);
		break;}
	case IPL_DEPTH_32S:{
		ensureCompatible(dst,depth32s,Size(image->width,image->height),image->nChannels);
		break;}
	case IPL_DEPTH_32F:{
		ensureCompatible(dst,depth32f,Size(image->width,image->height),image->nChannels);
		break;}
	case IPL_DEPTH_64F:{
		ensureCompatible(dst,depth64f,Size(image->width,image->height),image->nChannels);
		break;}
	default :{
		//this should not happen
		throw ICLException("Invalid source depth");
	}
	}
	SRC_T *data = (SRC_T*) image->imageData;
	interleavedToPlanar(data,(*dst)->asImg<DST_T>(),image->width*image->nChannels*sizeof(DST_T));
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
			ipl_to_img_dstpref<icl##D>(src,(*dst)->asImg<icl##D>());   \
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
#endif
}

