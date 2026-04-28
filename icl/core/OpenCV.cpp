// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Christian Groszewski

#include <icl/core/OpenCV.h>
#include <icl/core/CoreFunctions.h>

using namespace icl::utils;

namespace icl::core {

  static int estimate_mat_type(depth d, int channels){
    switch(d){
      case depth8u : return CV_MAKETYPE(CV_8U,channels);
      case depth16s : return CV_MAKETYPE(CV_16S,channels);
      case depth32s : return CV_MAKETYPE(CV_32S,channels);
      case depth32f : return CV_MAKETYPE(CV_32F,channels);
      case depth64f : return CV_MAKETYPE(CV_64F,channels);
      default: ICL_INVALID_DEPTH;
    }
    return -1;
  }

  static depth extract_depth(const ::cv::Mat *src){
    using namespace ::cv;
    switch(src->depth()){
      case CV_8U: return depth8u;
      case CV_16S: return depth16s;
      case CV_32S: return depth32s;
      case CV_32F: return depth32f;
      case CV_64F: return depth64f;
      default: throw ICLException("unsupported opencv mat depth");
    }
    return depth8u;
  }

  static Size extract_size(const ::cv::Mat *src){
    return Size(src->cols, src->rows);
  }

  static int extract_channels(const ::cv::Mat *src){
    return src->channels();
  }


  MatWrapper::MatWrapper(){}
  MatWrapper::MatWrapper(depth d, const Size &size, int channels){
    adapt(d,size,channels);
  }

  void MatWrapper::adapt(depth d, const Size &size, int channels){
    if(d != getDepth() || size != getSize() || channels != getChannels()){
      mat.create(size.height, size.width,
                 estimate_mat_type(d,channels));
    }
  }

  MatWrapper &MatWrapper::operator=(const ImgBase &image){
    img_to_mat(&image, &mat);
    return *this;
  }
  void MatWrapper::copyTo(ImgBase **dst){
    mat_to_img(&mat, dst);
  }
  void MatWrapper::convertTo(ImgBase &dst){
    mat_to_img(&mat, &dst);
  }

  Size MatWrapper::getSize() const{
    return extract_size(&mat);
  }
  int MatWrapper::getChannels() const{
    return extract_channels(&mat);
  }
  depth MatWrapper::getDepth() const{
    return extract_depth(&mat);
  }

  template<class T>
  T* MatWrapper::getInterleavedData(){
    return reinterpret_cast<T*>(mat.data);
  }

  template<class T>
  const T* MatWrapper::getInterleavedData() const{
    return reinterpret_cast<const T*>(mat.data);
  }

  MatWrapper::MatWrapper(const MatWrapper &other){
    *this = other;
  }

  MatWrapper::MatWrapper(const ::cv::Mat &other){
    *this = other;
  }

  MatWrapper &MatWrapper::operator=(const ::cv::Mat &other){
    other.copyTo(mat);
    return *this;
  }

  MatWrapper &MatWrapper::operator=(const MatWrapper &m){
    return *this = m.mat;
  }



#define ICL_INSTANTIATE_DEPTH(D)                                        \
  template ICLCore_API icl##D*MatWrapper::getInterleavedData<icl##D>(); \
  template ICLCore_API const icl##D* MatWrapper::getInterleavedData<icl##D>() const;
ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH


  ::cv::Mat *img_to_mat(const ImgBase *src, ::cv::Mat *dstIn){
    ICLASSERT_THROW(src && src->getChannels() > 0 && src->getChannels() <=4, ICLException("img_to_mat: invalid source image"));
    ::cv::Mat *dst = dstIn ? dstIn : new ::cv::Mat;
    dst->create(src->getHeight(), src->getWidth(), estimate_mat_type(src->getDepth(), src->getChannels()));
    ICLASSERT_THROW(dst->isContinuous(), ICLException("created cv::Mat is not continuous"));

    switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: planarToInterleaved(src->as##D(), reinterpret_cast<icl##D*>(dst->data)); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: ICL_INVALID_DEPTH;
    }
    return dst;
  }

  void mat_to_img(const ::cv::Mat *src, ImgBase **dstIn){
    ensureCompatible(dstIn, extract_depth(src), extract_size(src), extract_channels(src));
    mat_to_img(src, *dstIn);
  }

  ImgBase *mat_to_img(const ::cv::Mat *src, ImgBase *dstIn){
    ICLASSERT_THROW(src, ICLException("mat_to_img: input is null"));
    ICLASSERT_THROW(src->isContinuous(), "mat_to_img: input image must be continoues");

    ImgBase *dst = dstIn ? dstIn : imgNew(extract_depth(src),
                                          extract_size(src),
                                          extract_channels(src));

    switch(dst->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                        \
      case depth##D:{                                                 \
        switch(src->depth()){                                         \
          case CV_8U: interleavedToPlanar(reinterpret_cast<const icl8u*>(src->data),dst->as##D()); break; \
          case CV_16S: interleavedToPlanar(reinterpret_cast<const icl16s*>(src->data),dst->as##D()); break; \
          case CV_32S: interleavedToPlanar(reinterpret_cast<const icl32s*>(src->data),dst->as##D()); break; \
          case CV_32F: interleavedToPlanar(reinterpret_cast<const icl32f*>(src->data),dst->as##D()); break; \
          case CV_64F: interleavedToPlanar(reinterpret_cast<const icl64f*>(src->data),dst->as##D()); break; \
          default: break;                                             \
        }                                                             \
        break;                                                        \
      }
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: ICL_INVALID_DEPTH;
    }
    return dst;
  }


  } // namespace icl::core
