/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/ImgBase.cpp                        **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter, Michael Goetting, Robert Haschke  **
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

#include <ICLCore/ImgBase.h>
#include <ICLCore/Img.h>
#include <ICLUtils/StringUtils.h>

using namespace icl::utils;
using namespace icl::math;

namespace icl {
  namespace core{
    
    ImgBase::ImgBase(depth d, const ImgParams &params):m_oParams(params),m_eDepth(d) {
      FUNCTION_LOG("ImgBase(" << getWidth()
                   << "," << getHeight()
                   << "," << translateFormat(getFormat()) 
                   << ", "<< translateDepth(getDepth()) 
                   << "," << getChannels() << ")  this:" << this); 
    }
    
    ImgBase::~ImgBase(){
      FUNCTION_LOG("");
    }
  
    void ImgBase::print(const std::string title) const{
      FUNCTION_LOG(sTitle);
      
      std::cout << " -----------------------------------------" << std::endl
                << "| image     : " << title  << std::endl
                << "| timestamp : " << getTime() << std::endl
                << "| size      : " << getSize() << std::endl
                << "| channels  : " << getChannels() << std::endl
                << "| depth     : " << getDepth() << std::endl
                << "| format    : " << getFormat() << std::endl
                << "| roi       : " << (hasFullROI() ? str("full") : str(getROI())) << std::endl
                << " -----------------------------------------" << std::endl;
      
      for(int i=0;i<getChannels();++i){
        std::cout << "| range channel " << i << " :" << getMinMax(i) << std::endl;
      }
      
      std::cout << " -----------------------------------------" << std::endl;
      
      if(hasMetaData()){
        std::cout << "| no meta data available " << std::endl;
      }else{
        std::cout << "| meta data: " << std::endl;
        std::cout << getMetaData() << std::endl;
      }
      std::cout << " -----------------------------------------" << std::endl;
    }
    ImgBase *ImgBase::convert(depth d) const{
      FUNCTION_LOG("");
      switch(d){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return this->convert((Img<icl##D>*)0); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
        default: ICL_INVALID_FORMAT; break;
      }
    }

    ImgBase *ImgBase::convert(ImgBase *poDst) const{
      FUNCTION_LOG("");
      if(!poDst) return deepCopy();
      switch(poDst->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return this->convert(poDst->asImg<icl##D>()); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
        default: ICL_INVALID_FORMAT; break;
      }
    }
    template<class otherT>
    Img<otherT> *ImgBase::convert(Img<otherT> *poDst) const{ 
      FUNCTION_LOG("ptr:"<<poDst);
      if(!poDst) poDst = new Img<otherT>(getParams());
      else poDst->setParams(getParams());
      poDst->setTime(getTime());
      poDst->setMetaData(getMetaData());
    
#define ICL_INSTANTIATE_DEPTH(D)                                        \
      case depth##D: for(int c=getChannels()-1;c>=0;--c){               \
        icl::core::convert<icl##D,otherT>(asImg<icl##D>()->getData(c),  \
                                          asImg<icl##D>()->getData(c)+getDim(),poDst->getData(c)); \
      } break;
      switch(getDepth()){
        ICL_INSTANTIATE_ALL_DEPTHS;
      }
#undef ICL_INSTANTIATE_DEPTH
      return poDst;    
    }

    ImgBase *ImgBase::convertROI(depth d) const{
      FUNCTION_LOG("");
      switch(d){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return this->convertROI((Img<icl##D>*)0); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
        default: ICL_INVALID_FORMAT; break;
      }
    }

    ImgBase *ImgBase::convertROI(ImgBase *poDst) const{
      FUNCTION_LOG("");
      if(!poDst) return deepCopy();
      switch(poDst->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return this->convertROI(poDst->asImg<icl##D>()); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
        default: ICL_INVALID_FORMAT; break;
      }
    }

    template<class otherT>
    Img<otherT> *ImgBase::convertROI(Img<otherT> *poDst) const{ 
      FUNCTION_LOG("ptr:"<<poDst);
      if(!poDst){
        poDst = new Img<otherT>(getROISize(),getChannels(),getFormat());
      }else{
        ICLASSERT( poDst->getROISize() == getROISize() );
        poDst->setChannels(getChannels());
        poDst->setFormat(getFormat());
      }
      poDst->setTime(getTime());
      poDst->setMetaData(getMetaData());
    
#define ICL_INSTANTIATE_DEPTH(D)                                        \
      case depth##D:                                                    \
        for(int c=getChannels()-1;c>=0;--c){                            \
          convertChannelROI(asImg<icl##D>(),c,getROIOffset(),getROISize(), \
                            poDst,c,poDst->getROIOffset(),poDst->getROISize()); \
        }                                                               \
        break;
    
      switch(getDepth()){
        ICL_INSTANTIATE_ALL_DEPTHS;
      }
#undef ICL_INSTANTIATE_DEPTH
      return poDst;    
    }

    void ImgBase::setFormat(format fmt){
      FUNCTION_LOG("");
      int newcc = getChannelsOfFormat(fmt);
      if(fmt != formatMatrix && newcc != getChannels()){
        setChannels(newcc);
      }
      m_oParams.setFormat(fmt);
    }

    void ImgBase::clear(int iChannel, icl64f val, bool bROIOnly){
      FUNCTION_LOG("");
      switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                        \
        case depth##D:                                                  \
          asImg<icl##D>()->clear(iChannel,                              \
                                 clipped_cast<icl64f,icl##D>(val),      \
                                 bROIOnly); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH   
      }
    }
 
    void ImgBase::normalizeAllChannels(const Range<icl64f> &dstRange){
      FUNCTION_LOG("");
      switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                        \
        case depth##D: asImg<icl##D>()->normalizeAllChannels(           \
                                                             dstRange.castTo<icl##D>()); break;                    
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      }
    }
  
    void ImgBase::normalizeChannel(int iChannel,const Range<icl64f> &srcRange,const Range<icl64f> &dstRange){
      FUNCTION_LOG("");
      switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                        \
        case depth##D: asImg<icl##D>()->normalizeChannel(iChannel,      \
                                                         srcRange.castTo<icl##D>(), \
                                                         dstRange.castTo<icl##D>()); break;                       
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      }
    }

  
    void ImgBase::normalizeChannel(int iChannel,const Range<icl64f> &dstRange){
      FUNCTION_LOG("");
      switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                        \
        case depth##D: asImg<icl##D>()->normalizeChannel(iChannel,      \
                                                         dstRange.castTo<icl##D>()); break;                     
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      }
    }

    void ImgBase::normalizeImg(const Range<icl64f> &srcRange,const Range<icl64f> &dstRange){
      FUNCTION_LOG("");
      switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                        \
        case depth##D: asImg<icl##D>()->normalizeImg(                   \
                                                     srcRange.castTo<icl##D>(), \
                                                     dstRange.castTo<icl##D>()); break;          
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      }
    }
  
    void ImgBase::normalizeImg(const Range<icl64f> &dstRange){
      FUNCTION_LOG("");
      switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                        \
        case depth##D: asImg<icl##D>()->normalizeImg(                   \
                                                     dstRange.castTo<icl##D>()); break;           
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      }
    }
  

    icl64f ImgBase::getMax(int iChannel, Point *coords) const{
      FUNCTION_LOG("");
      switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return asImg<icl##D>()->getMax(iChannel,coords);
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      }
      return 0;
    }
    icl64f ImgBase::getMin(int iChannel, Point *coords) const{
      FUNCTION_LOG("");
      switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return asImg<icl##D>()->getMin(iChannel,coords);
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH   
      }
      return 0;
    }


    icl64f ImgBase::getMin() const{
      FUNCTION_LOG("");
      switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return asImg<icl##D>()->getMin();
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH   
      }
      return 0;
    }

    icl64f ImgBase::getMax() const{
      FUNCTION_LOG("");
      switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return asImg<icl##D>()->getMax();
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH   
      }
      return 0;
    }

    const Range<icl64f> ImgBase::getMinMax(int iChannel, Point *minCoords, Point *maxCoords) const{
      FUNCTION_LOG("");
      switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return asImg<icl##D>()->getMinMax(iChannel,minCoords,maxCoords).castTo<icl64f>();
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH   
      }
      return Range<icl64f>();
    }

    const Range<icl64f> ImgBase::getMinMax() const{
      FUNCTION_LOG("");
      switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return asImg<icl##D>()->getMinMax().castTo<icl64f>();
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH   
      }
      return Range<icl64f>();
    }

    void ImgBase::setParams(const ImgParams &params){
      FUNCTION_LOG("");
      setChannels(params.getChannels());
      setSize(params.getSize());
      setFormat(params.getFormat());
      setROI(params.getROI());
    }
  
    std::ostream &operator<<(std::ostream &s, const ImgBase &image){
      return s << "Img<" << image.getDepth() << ">(Size("<<image.getSize() <<"),"
               << image.getFormat() << ","<<image.getChannels() <<","
               << image.getROI() << "," << image.getTime() << ")";
    }

#define ICL_INSTANTIATE_DEPTH(D)                  \
  template ICL_CORE_API Img<icl##D> *ImgBase::convert<icl##D>(Img<icl##D> *poDst = NULL) const; \
  template ICL_CORE_API Img<icl##D> *ImgBase::convertROI<icl##D>(Img<icl##D> *poDst = NULL) const;
    ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
  } // namespace core

} //namespace icl
