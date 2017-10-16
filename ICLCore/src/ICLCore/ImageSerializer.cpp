/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/ImageSerializer.cpp                **
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

#include <ICLCore/ImageSerializer.h>
#include <ICLUtils/StringUtils.h>

using namespace icl::utils;

namespace icl{
  namespace core{
    namespace{
      struct BinarySerializer : public std::vector<icl8u>{
        template<class T>
        BinarySerializer &operator<<(const T &t){
          const icl8u *p = (const icl8u*)(&t);
          std::copy(p,p+sizeof(T),std::back_inserter(*this));
          return *this;
        }
      };

      class BinaryUnserializer{
        const icl8u *data;
      public:
        BinaryUnserializer(const void *data) :
          data((const icl8u*)data){ }

        template<class T>
        BinaryUnserializer &operator>>(T &t){
          t = *(const T*)data;
          data += sizeof(T);
          return *this;
        }
      };
    }


    int ImageSerializer::getHeaderSize(){
      return 9*sizeof(icl32s) + sizeof(int64_t);
    }


    ImageSerializer::ImageHeader ImageSerializer::createHeader(const ImgBase *image) throw(ICLException){
      ICLASSERT_THROW(image,ICLException(str(__FUNCTION__)+": image was null"));
      BinarySerializer ser;
      ser << (icl32s)image->getDepth()
          << (icl32s)image->getSize().width
          << (icl32s)image->getSize().height
          << (icl32s)image->getFormat()
          << (icl32s)image->getChannels()
          << (icl32s)image->getROI().x
          << (icl32s)image->getROI().y
          << (icl32s)image->getROI().width
          << (icl32s)image->getROI().height
          << (int64_t)image->getTime().toMilliSeconds();
      return ser;
    }

    int ImageSerializer::estimateImageDataSize(const ImgBase *image) throw (ICLException){
      ICLASSERT_THROW(image,ICLException(str(__FUNCTION__)+": image was null"));
      static const int SIZES[] = { sizeof(icl8u), sizeof(icl16s), sizeof(icl32s), sizeof(icl32f), sizeof(icl64f)};
      return  image->getChannels() * SIZES[image->getDepth()] * image->getDim();
    }

    void ImageSerializer::serialize(const ImgBase *image, icl8u *dst,
                                    const ImageSerializer::ImageHeader &header,
                                    bool skipMetaData) throw (ICLException){
      ICLASSERT_THROW(image,ICLException(str(__FUNCTION__)+": image was null"));

      if(header.size()){
        std::copy(header.begin(),header.end(),dst);
        dst += header.size();
      }else{
        ImageHeader header2 = createHeader(image);
        std::copy(header2.begin(),header2.end(),dst);
        dst += header2.size();
      }
      int dataLength = estimateImageDataSize(image);
      if(dataLength){ // aka if there are channels and a size != null
        int lengthPerChannel = dataLength/image->getChannels();
        for(int i=0;i<image->getChannels();++i){
          memcpy(dst,image->getDataPtr(i),lengthPerChannel);
          dst += lengthPerChannel;
        }
      }
      if(skipMetaData){
        *(icl32s*)dst = 0;
      }else{
        *(icl32s*)dst = (icl32s)image->getMetaData().length();
        dst += sizeof(icl32s);
        std::copy(image->getMetaData().begin(), image->getMetaData().end(), dst);
      }
    }

    void ImageSerializer::serialize(const ImgBase *image, std::vector<icl8u> &data,
                                    const ImageSerializer::ImageHeader &header,
                                    bool skipMetaData) throw (ICLException){
      ICLASSERT_THROW(image,ICLException(str(__FUNCTION__)+": image was null"));

      data.resize(estimateSerializedSize(image,skipMetaData));
      serialize(image,data.data(),header,skipMetaData);
    }

    void ImageSerializer::deserialize(const icl8u *data, ImgBase **dst) throw (ICLException){
      ICLASSERT_THROW(dst,ICLException(str(__FUNCTION__)+": destination ImgBase** was null"));
      ICLASSERT_THROW(dst,ICLException(str(__FUNCTION__)+": source data pinter was null"));

      BinaryUnserializer ser(data);

      icl32s is[9];
      int64_t l;
      for(int i=0;i<9;++i){
        ser >> is[i];
      }
      ser >> l;
      data += getHeaderSize();

      ensureCompatible(dst,depth(is[0]),Size(is[1],is[2]),is[4],(format)is[3],Rect(is[5],is[6],is[7],is[8]));

      ImgBase *image = *dst;
      image->setTime(Time(l));
      int dataLength = estimateImageDataSize(image);
      if(dataLength){
        int lengthPerChannel = dataLength/image->getChannels();
        for(int i=0;i<image->getChannels();++i){
          memcpy(image->getDataPtr(i),data,lengthPerChannel);
          data += lengthPerChannel;
        }
      }

      int metaLen = *(icl32s*)data;
      data+= sizeof(icl32s);
      if(metaLen){
        (*dst)->getMetaData().assign((char*)data, metaLen);
      }else{
        (*dst)->clearMetaData();
      }
    }

    int ImageSerializer::estimateSerializedSize(const ImgBase *image, bool skipMetaData) throw (ICLException){
      ICLASSERT_THROW(image,ICLException(str(__FUNCTION__)+": image was null"));
      return getHeaderSize() + estimateImageDataSize(image) + sizeof(icl32s) + (skipMetaData ? 0 : image->getMetaData().length());
    }

    Time ImageSerializer::deserializeTimeStamp(const icl8u *data) throw (ICLException){
      return Time(*(const int64_t*)(data+9*sizeof(icl32s)));
    }
  } // namespace core
}
