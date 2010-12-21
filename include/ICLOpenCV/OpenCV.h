/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLOpenCV/OpenCV.h                             **
** Module : ICLOpenCV                                              **
** Authors: Christof Elbrechter, Christian Groszewski              **
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

#ifndef ICL_OPEN_CV_H
#define ICL_OPEN_CV_H

#ifdef HAVE_OPENCV211
#include <opencv2/core/types_c.h>
#include <opencv2/core/core_c.h>
#else
#ifdef HAVE_OPENCV
#include <cxcore.h>
#endif
#endif

#include <ICLCC/CCFunctions.h>
#include <ICLCore/Img.h>
#include <ICLCore/ImgBase.h>
/** CvMat type

struct CvMat{
    int type;
    int step;
    int* refcount;

    uchar* ptr;

    int rows;
    int cols;
};
*/

namespace icl{

  // TODO: everything !

  // p = 7
  // size is 1
  /*template<class T>
  T* icl_allign(T *p, int n=4){
    return reinterpret_cast<T*>( ((size_t)p+n-1) & -n);
  }

  template<class T>
  Img<T> *deep_copy_to_img_base_t(const void *data, int w, int h){
    Img<T> * image = new Img<T>(Size(w,h),1);
    // rows a 4-byte alligned
    const T* src = reinterpret_cast<const T*>(data);
    T* dst = image->begin(0);
    for(int y=0;y<h;++y){
      std::copy(src,src+w,dst+w*y);
      src = icl_allign(src);
    }
    return image;
  }

  ImgBase *deepCopyToImgBase(const CvMat *m){
    ICLASSERT_RETURN_VAL(m,0);
    switch(CV_MAT_DEPTH(m->type)){
      case CV_8S:
        WARNING_LOG("using icl8u to handle CV_8S data type!");
	return 0;
      case CV_8U:
        return deep_copy_to_img_base_t<icl8u>(m->data.ptr,m->cols,m->rows);
      case CV_16U:
        WARNING_LOG("using icl16s to handle CV_16U data type!");
	return 0;
      case CV_16S:
        return deep_copy_to_img_base_t<icl16s>(m->data.ptr,m->cols,m->rows);
      case CV_32S:
        return deep_copy_to_img_base_t<icl32s>(m->data.ptr,m->cols,m->rows);
      case CV_32F:
        return deep_copy_to_img_base_t<icl32f>(m->data.ptr,m->cols,m->rows);
      case CV_64F:
        return deep_copy_to_img_base_t<icl64f>(m->data.ptr,m->cols,m->rows);
    }
    return 0;
  }*/

/// Modes which wether to prefer sourcedepth or destinationdepth
enum DepthPreference{
	PREFERE_SRC_DEPTH,//!< prefere sourcedepth
	PREFERE_DST_DEPTH//!< prefere destinationdepth
};

///Convert OpenCV IplImage to ICLimage
/**Converts IplImage to ICLimage. If dst is NULL, the sourceimagedepth
will be used, else the destinationimagedepth will be used.
@param *src pointer to sourceimage (IplImage)
@param **dst pointer to pointer to destinationimage (ICLimage)
@param e depthpreference*/
ImgBase *ipl_to_img(CvArr *src,ImgBase **dst=0,DepthPreference e=PREFERE_SRC_DEPTH) throw (icl::ICLException);

///Convert ICLimage to OpenCV IplImage
/**Converts ICLimage to IplImage. If dst is NULL, the sourceimagedepth
will be used, else the destinationimagedepth will be used.
@param *src pointer to sourceimage
@param **dst pointer to pointer to destinationimage (IplImage)
@param e depthpreference*/
IplImage *img_to_ipl(const ImgBase *src, IplImage **dst=0,DepthPreference e=PREFERE_SRC_DEPTH)throw (icl::ICLException);

///Copy single ICLimage channel to OpenCV single channel CvMat
/**Copy single ICLimage channel to single channel CvMat. If dst is NULL, the sourceimagedepth
will be used, else the destinationmatrixdepth will be used.
@param *src pointer to sourceimage
@param **dst pointer to pointer to destinationmatrix
@param channel channel to use*/
CvMat* img_to_cvmat(const ImgBase *src, CvMat *dst=0,int channel=0) throw (icl::ICLException);

///Convert single channel ICLimage to OpenCV IplImage
/**Converts single channel ICLimage to IplImage. Data is shared by source and destination.
Using icl8u or icl16s the imagedata is not aligned, but OpenCV expects aligned data.
In this case be careful using further OpenCV functions.
Be careful when releasig (data)pointers.
@param *src pointer to sourceimage
@param *dst pointer to destinationimage (IplImage)*/
IplImage *img_to_ipl_shallow(ImgBase *src,IplImage *dst=0)throw (ICLException);

///Convert single channel ICLimage to OpenCV CvMat
/**Converts single channel ICLimage to a single channel CvMat. Data is shared by
source and destination.
Be careful when releasig (data)pointers.
@param *src pointer to sourceimage
@param *dst pointer to destinationmatrix (IplImage)*/
CvMat *img_to_cvmat_shallow(const ImgBase *src,CvMat *dst=0) throw (ICLException);

}
#endif

