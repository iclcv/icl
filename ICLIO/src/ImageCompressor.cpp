/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ImageCompressor.cpp                          **
** Module : ICLIO                                                  **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLIO/ImageCompressor.h>
#include <stdint.h>
#ifdef HAVE_LIBJPEG
#include <ICLIO/JPEGEncoder.h>
#include <ICLIO/JPEGDecoder.h>
#endif
#include <ICLBlob/RegionDetectorTools.h>
#include <ICLIO/File.h>

#include <ICLUtils/StackTimer.h>

namespace icl{

  struct ImageCompressor::Data{
    std::vector<icl8u> encoded_buffer;
    Img8u decoded_buffer;
    CompressionMode mode;
    int jpegQuality;
#ifdef HAVE_LIBJPEG
    SmartPtr<JPEGEncoder> jpegEncoder;
#endif
  };

  ImageCompressor::ImageCompressor(ImageCompressor::CompressionMode mode):m_data(new Data){
    m_data->mode = mode;
    m_data->jpegQuality = 90;
  }

  ImageCompressor::~ImageCompressor(){
    delete m_data;
  }
  
  /// only decodes an image header
  ImageCompressor::Header ImageCompressor::decodeHeader(const icl8u *data,int len, bool decodeMetaData){
    if(data[0] != 'r' || data[1] != 'l' || data[2] != 'e'){
      throw ICLException("given data is RLE");
    }
    const icl32s *p = reinterpret_cast<const icl32s*>(data+3);
    const int metaLen = p[8];
    const int headerLen = 47+metaLen;
    
    Header header = { 
      CompressionMode(p[0]),
      Size(p[1],p[2]),
      Rect(p[3],p[4],p[5],p[6]),
      p[7],
      "",
      Time(*reinterpret_cast<const int64_t*>(data+9)),
      data+headerLen,
      len-headerLen
    };
    if(decodeMetaData) header.metaData.resize(metaLen);

    std::copy(data+47,data+headerLen,header.metaData.begin());
    return header;
  }


  int ImageCompressor::estimateBufferSize(const Img8u &image, int metaDataLength){
    const int headerSize = 47 + metaDataLength;
    const int numPix = image.getDim() * image.getChannels();
    return headerSize + numPix;
  }

  void ImageCompressor::setCompressionMode(CompressionMode mode){
    m_data->mode = mode;
  }
  
  
  /// returns the current image compression mode
  ImageCompressor::CompressionMode ImageCompressor::getCompressionMode() const{
    return m_data->mode;
  }

  static const icl8u *find_first_not_binarized(const icl8u *curr, const icl8u *end, icl8u val){
    if(val){
      for(;curr<end;++curr){
        if(*curr < 127) return curr;
      }
    }else{
      for(;curr<end;++curr){
        if(*curr >= 127) return curr;
      }
    }
    return end;
  }

  static icl8u *compressChannel(const icl8u *imageData, 
                                icl8u *compressedData, 
                                const Size &imageSize, 
                                const Rect &roi, 
                                ImageCompressor::CompressionMode mode){
    switch(mode){
      case ImageCompressor::CompressRLE1Bit:{
        BENCHMARK_THIS_SECTION("compress 1bit");
        const icl8u *imageDataEnd = imageData+imageSize.getDim();
        icl8u currVal = !!*imageData;
        int allLen = 0;
        while(imageData < imageDataEnd){
          const icl8u *other = find_first_not_binarized(imageData,imageDataEnd,currVal); 
          size_t len = (size_t)(other-imageData);
          while(len >= 128){
            *compressedData++ = 0xff >> !currVal;
            len -= 128;
            allLen += 128;
          }
          if(len){
            *compressedData++ = (len-1) | (currVal << 7);
            allLen += len;
          }
          currVal = !currVal;
          imageData = other;
        }
        break;
      }case ImageCompressor::NoCompression:{
         std::copy(imageData,imageData+imageSize.getDim(), compressedData);
         compressedData += imageSize.getDim();
         break;
       }
      case ImageCompressor::CompressRLE4Bit:{
        const icl8u *imageDataEnd = imageData+imageSize.getDim();

        int currVal = *imageData & 0xf0; // use most significant 4 bits
        int currLen = 0;
        while(imageData < imageDataEnd){
          while( (imageData < imageDataEnd) && ((*imageData & 0xf0) == currVal) ){
            ++currLen;
            ++imageData;
          }
          while(currLen >= 16){
            *compressedData++ = currVal | 0xf;
            currLen -= 16;
          }
          if(currLen){
            *compressedData++ = currVal | (currLen-1);
          }
          
          currVal = *imageData & 0xf0;
          currLen = 0;
        }
        break;
      }
      case ImageCompressor::CompressRLE6Bit:{
        const icl8u *imageDataEnd = imageData+imageSize.getDim();
        static const int VAL_MASK=0xFC;
        static const int LEN_MASK=0x3;
        static const int MAX_LEN=4;
        
        int currVal = *imageData & VAL_MASK; // use most significant 6 bits
        int currLen = 0;
        
        while(imageData < imageDataEnd){
          while( (imageData < imageDataEnd) && ((*imageData & VAL_MASK) == currVal) ){
            ++currLen;
            ++imageData;
          }
          while(currLen >= MAX_LEN){
            *compressedData++ = currVal | LEN_MASK;
            currLen -= MAX_LEN;
          }
          if(currLen){
            *compressedData++ = currVal | (currLen-1);
          }
          
          currVal = *imageData & VAL_MASK;
          currLen = 0;
        }
        break;
      }
      default: throw ICLException("ImageCompressor::compressChannel: unsupported compression mode!" );
    }
    return compressedData;
  }

  static const icl8u *uncompressChannel(icl8u *imageData, 
                                        int imageDataLen, 
                                        const icl8u *compressedData, 
                                        ImageCompressor::CompressionMode mode){
    switch(mode){
      case ImageCompressor::CompressRLE1Bit:{
        icl8u *pc = imageData;
        icl8u *pcEnd = imageData + imageDataLen;
        const icl8u *p = compressedData;
        
        for(;pc < pcEnd; ++p){
          const int l = ( (*p) & 127) + 1;
          std::fill(pc,pc+l,((*p)>>7) * 255);
          pc += l;
        }
        compressedData = p;
        break;
      }
      case ImageCompressor::CompressRLE4Bit:{
        icl8u *pc = imageData;
        icl8u *pcEnd = imageData + imageDataLen;
        const icl8u *p = compressedData;
        for(;pc < pcEnd; ++p){
          const int l = ( (*p) & 0xf) + 1;
          std::fill(pc,pc+l,((*p) & 0xf0));
          pc += l;
        }
        compressedData = p;   
        break;
      }
      case ImageCompressor::CompressRLE6Bit:{
        static const int VAL_MASK=0xFC;
        static const int LEN_MASK=0x3;

        icl8u *pc = imageData;
        icl8u *pcEnd = imageData + imageDataLen;
        const icl8u *p = compressedData;
        for(;pc < pcEnd; ++p){
          const int l = ( (*p) & LEN_MASK) + 1;
          std::fill(pc,pc+l,((*p) & VAL_MASK));
          pc += l;
        }
        compressedData = p;   
        break;
      }


      case ImageCompressor::NoCompression:{
        std::copy(compressedData,compressedData+imageDataLen,imageData);
        compressedData += imageDataLen;
        break;
      }
      default: throw ICLException("ImageCompressor::compressChannel: unsupported compression mode!" ); 
    }
    return compressedData;

    
  }


  void ImageCompressor::setJPEGQuality(int quality){
    if(quality <= 0 || quality > 100){
      ERROR_LOG("ImageCompressor::setJPEGQuality: invalid quality value!");
    }else{
      m_data->jpegQuality = quality;
    }
  }


  const ImageCompressor::CompressedData ImageCompressor::encode(const Img8u &image, 
                                                                const std::string &meta){
    if(m_data->mode == CompressJPEG){
#ifdef HAVE_LIBJPEG
      if(meta.length()){
        WARNING_LOG("adding meta data to jpeg compressed images is not yet supported!");
      }
      if(!m_data->jpegEncoder) m_data->jpegEncoder = new JPEGEncoder;
      m_data->jpegEncoder->setQuality(m_data->jpegQuality);
      const JPEGEncoder::EncodedData &encoded = m_data->jpegEncoder->encode(&image);
      
      int rawDataSize = estimateBufferSize(image,0);
      return CompressedData(encoded.bytes,encoded.len, float(encoded.len)/rawDataSize);
#else
      throw ICLException("ImageCompressor:encode jpeg compression is not supported without libjpeg");
#endif
    }
    
    m_data->encoded_buffer.resize(estimateBufferSize(image,(int)meta.length()));
    
    icl8u  *header8u = reinterpret_cast<icl8u*>(m_data->encoded_buffer.data());
    *header8u++ = 'r'; // magick code!
    *header8u++ = 'l';
    *header8u++ = 'e';
    icl32s *header = reinterpret_cast<icl32s*>(header8u);
    *header++ = (int)m_data->mode;
    *header++ = image.getWidth();
    *header++ = image.getHeight();
    *header++ = 0; //roi.x;
    *header++ = 0; //roi.y;
    *header++ = image.getWidth(); //roi.width;
    *header++ = image.getHeight(); //roi.height;
    *header++ = image.getChannels();
    *header++ = (icl32s)meta.length();
    
    int64_t *ts = reinterpret_cast<int64_t*>(header);
    *ts++ = image.getTime().toMicroSeconds();
    
    icl8u *p = reinterpret_cast<icl8u*>(ts);
    std::copy(meta.begin(),meta.end(),p);
    p += meta.length();
    
    for(int c=0;c<image.getChannels();++c){
      p = compressChannel(image.begin(c),p, image.getSize(),image.getImageRect(), m_data->mode);
    }
    
    float len = (float)(p-m_data->encoded_buffer.data());
    return CompressedData(m_data->encoded_buffer.data(), (int)len, len/m_data->encoded_buffer.size());
  }

  Time ImageCompressor::decodeTimeStamp(const icl8u *data){
    ICLASSERT_THROW(data, ICLException("ImageCompressor::decodeTimeStamp: "
                                       "given data pointer is NULL"));
    return *reinterpret_cast<const int64_t*>(data+32);
  }

  void ImageCompressor::Header::setupImage(Img8u &image){
    image.setSize(size);
    image.setROI(roi);
    image.setChannels(channelCount);
    image.setTime(timeStamp);
  }
  
  const Img8u &ImageCompressor::decode(const icl8u *data, int len, std::string *meta){
    ICLASSERT_THROW(data, ICLException("ImageCompressor::decode: "
                                       "given data pointer is NULL"));
    ICLASSERT_THROW(len > 3, ICLException("ImageCompressor::decode: given data pointer is too short"));

    //SHOW(std::string("")+char(data[0])+char(data[1])+char(data[2]));
    bool isRLE = (data[0]=='r' && data[1]=='l' && data[2]=='e');
    if(!isRLE){
#ifdef HAVE_LIBJPEG
      if(meta){
        WARNING_LOG("sending meta data is not supported with jpeg compression");
      }
      JPEGDecoder::decode(data,len,bpp(m_data->decoded_buffer));
#else
      throw ICLException("ImageCompressor::decode: jpeg decoding is not supported without LIBJPEG");
#endif
      return m_data->decoded_buffer;

    }else{
      Header header = decodeHeader(data,len,meta);
      if(meta) *meta = header.metaData;
      header.setupImage(m_data->decoded_buffer);
      
      const icl8u *p = header.dataBegin;
      
      for(int c=0;c<m_data->decoded_buffer.getChannels();++c){
        p = uncompressChannel(m_data->decoded_buffer.begin(c), header.size.getDim(), p, header.mode);
      }
      
      return m_data->decoded_buffer;
    }
  }
}
