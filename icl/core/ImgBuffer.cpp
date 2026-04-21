// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/core/ImgBuffer.h>

using namespace icl::utils;
using namespace icl::math;

namespace icl::core {
  struct ImgBuffer::Data{
    std::vector<ImgBase*> bufs[depthLast+1];
  };

  ImgBuffer::ImgBuffer(){
    data = new ImgBuffer::Data;
  }
  ImgBuffer::~ImgBuffer(){
    delete data;
  }

  ImgBuffer *ImgBuffer::instance(){
    static std::shared_ptr<ImgBuffer> i(new ImgBuffer);
    return i.get();
  }

  ImgBase *ImgBuffer::get(depth d){
    switch(d){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return get<icl##D>();
      ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
    }
    return 0;
  }

  ImgBase *ImgBuffer::get(depth d, const Size &size, int channels){
    switch(d){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return get<icl##D>(size,channels);
      ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
    }
    return 0;
  }

  ImgBase *ImgBuffer::get(depth d, const ImgParams &params){
    switch(d){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return get<icl##D>(params);
      ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
    }
    return 0;
  }

  template<class T>
  Img<T> *ImgBuffer::get(const Size &size, int channels){
    return get<T>(ImgParams(size,channels));
  }

  template<class T>
  Img<T> *ImgBuffer::get(const ImgParams &params){
    std::vector<ImgBase*> &buf = data->bufs[getDepth<T>()];
    Img<T> * independentOne = 0;
    std::vector<ImgBase*>::iterator begin = buf.begin();
    std::vector<ImgBase*>::iterator end = buf.end();
    for(std::vector<ImgBase*>::iterator it = begin; it != end; ++it){
      if((*it)->isIndependent()){
        if((*it)->getParams() == params){
          return (*it)->asImg<T>();
        }else{
          if(!independentOne){
            independentOne = (*it)->asImg<T>();
          }
        }
      }
    }
    if(independentOne){
      independentOne->setParams(params);
      return independentOne;
    }else{
      buf.push_back(new Img<T>(params));
      return buf.back()->asImg<T>();
    }
  }

  template<class T>
  Img<T> *ImgBuffer::get(){
    std::vector<ImgBase*> &buf = data->bufs[getDepth<T>()];
    std::vector<ImgBase*>::iterator begin = buf.begin();
    std::vector<ImgBase*>::iterator end = buf.end();
    for(std::vector<ImgBase*>::iterator it = begin; it != end; ++it){
      if((*it)->isIndependent()){
        return (*it)->asImg<T>();
      }
    }
    buf.push_back(new Img<T>);
    return buf.back()->asImg<T>();
  }

#define ICL_INSTANTIATE_DEPTH(D)                  \
template ICLCore_API Img<icl##D> *ImgBuffer::get<icl##D>(); \
template ICLCore_API Img<icl##D> *ImgBuffer::get<icl##D>(const ImgParams&); \
template ICLCore_API Img<icl##D> *ImgBuffer::get<icl##D>(const Size&, int);
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
  } // namespace icl::core