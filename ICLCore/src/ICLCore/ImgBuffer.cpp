/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/ImgBuffer.cpp                      **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
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

#include <ICLCore/ImgBuffer.h>

using namespace icl::utils;
using namespace icl::math;

namespace icl{
  namespace core{
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
      static SmartPtr<ImgBuffer> i = new ImgBuffer;
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
  template ICL_CORE_API Img<icl##D> *ImgBuffer::get<icl##D>(); \
  template ICL_CORE_API Img<icl##D> *ImgBuffer::get<icl##D>(const ImgParams&); \
  template ICL_CORE_API Img<icl##D> *ImgBuffer::get<icl##D>(const Size&, int);
    ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
  } // namespace core
}
