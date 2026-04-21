// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/core/ImageSerializer.h>
#include <icl/core/CoreFunctions.h>
#include <icl/utils/StringUtils.h>
#include <cstring>

using namespace icl::utils;

namespace icl::core {
  namespace{
    struct BinarySerializer : public std::vector<icl8u>{
      template<class T>
      BinarySerializer &operator<<(const T &t){
        const icl8u *p = reinterpret_cast<const icl8u*>(&t);
        std::copy(p,p+sizeof(T),std::back_inserter(*this));
        return *this;
      }
    };

    class BinaryUnserializer{
      const icl8u *data;
    public:
      BinaryUnserializer(const void *data) :
        data(static_cast<const icl8u*>(data)){ }

      template<class T>
      BinaryUnserializer &operator>>(T &t){
        t = *reinterpret_cast<const T*>(data);
        data += sizeof(T);
        return *this;
      }
    };
  }


  int ImageSerializer::getHeaderSize(){
    return 9*sizeof(icl32s) + sizeof(int64_t);
  }


  ImageSerializer::ImageHeader ImageSerializer::createHeader(const ImgBase *image){
    ICLASSERT_THROW(image,ICLException(str(__FUNCTION__)+": image was null"));
    BinarySerializer ser;
    ser << static_cast<icl32s>(image->getDepth())
        << static_cast<icl32s>(image->getSize().width)
        << static_cast<icl32s>(image->getSize().height)
        << static_cast<icl32s>(image->getFormat())
        << static_cast<icl32s>(image->getChannels())
        << static_cast<icl32s>(image->getROI().x)
        << static_cast<icl32s>(image->getROI().y)
        << static_cast<icl32s>(image->getROI().width)
        << static_cast<icl32s>(image->getROI().height)
        << static_cast<int64_t>(image->getTime().toMilliSeconds());
    return ser;
  }

  int ImageSerializer::estimateImageDataSize(const ImgBase *image){
    ICLASSERT_THROW(image,ICLException(str(__FUNCTION__)+": image was null"));
    static const int SIZES[] = { sizeof(icl8u), sizeof(icl16s), sizeof(icl32s), sizeof(icl32f), sizeof(icl64f)};
    return  image->getChannels() * SIZES[image->getDepth()] * image->getDim();
  }

  void ImageSerializer::serialize(const ImgBase *image, icl8u *dst,
                                  const ImageSerializer::ImageHeader &header,
                                  bool skipMetaData){
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
      *reinterpret_cast<icl32s*>(dst) = 0;
    }else{
      *reinterpret_cast<icl32s*>(dst) = static_cast<icl32s>(image->getMetaData().length());
      dst += sizeof(icl32s);
      std::copy(image->getMetaData().begin(), image->getMetaData().end(), dst);
    }
  }

  void ImageSerializer::serialize(const ImgBase *image, std::vector<icl8u> &data,
                                  const ImageSerializer::ImageHeader &header,
                                  bool skipMetaData){
    ICLASSERT_THROW(image,ICLException(str(__FUNCTION__)+": image was null"));

    data.resize(estimateSerializedSize(image,skipMetaData));
    serialize(image,data.data(),header,skipMetaData);
  }

  void ImageSerializer::deserialize(const icl8u *data, ImgBase **dst){
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

    ensureCompatible(dst,depth(is[0]),Size(is[1],is[2]),is[4],static_cast<format>(is[3]),Rect(is[5],is[6],is[7],is[8]));

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

    int metaLen = *reinterpret_cast<const icl32s*>(data);
    data+= sizeof(icl32s);
    if(metaLen){
      (*dst)->getMetaData().assign(reinterpret_cast<const char*>(data), metaLen);
    }else{
      (*dst)->clearMetaData();
    }
  }

  int ImageSerializer::estimateSerializedSize(const ImgBase *image, bool skipMetaData){
    ICLASSERT_THROW(image,ICLException(str(__FUNCTION__)+": image was null"));
    return getHeaderSize() + estimateImageDataSize(image) + sizeof(icl32s) + (skipMetaData ? 0 : image->getMetaData().length());
  }

  Time ImageSerializer::deserializeTimeStamp(const icl8u *data){
    return Time(*reinterpret_cast<const int64_t*>(data+9*sizeof(icl32s)));
  }
  } // namespace icl::core