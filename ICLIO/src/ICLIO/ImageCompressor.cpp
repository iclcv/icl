/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/ImageCompressor.cpp                    **
** Module : ICLIO                                                  **
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

#include <ICLIO/ImageCompressor.h>
#include <ICLIO/Kinect11BitCompressor.h>
#include <ICLFilter/DitheringOp.h>
#include <stdint.h>
#ifdef ICL_HAVE_LIBJPEG
#include <ICLIO/JPEGEncoder.h>
#include <ICLIO/JPEGDecoder.h>
#endif
//#include <ICLCV/RegionDetectorTools.h>
#include <ICLUtils/File.h>
#include <ICLUtils/StringUtils.h>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::filter;

namespace icl{
  namespace io{

    struct ImageCompressor::Data{
      Img8u ditheringBuffer;
      std::vector<icl8u> encoded_buffer;
      ImgBase *decoded_buffer;
      ImageCompressor::CompressionSpec compression;

  #ifdef ICL_HAVE_LIBJPEG
      SmartPtr<JPEGEncoder> jpegEncoder;
  #endif

    };

    ImageCompressor::ImageCompressor(const ImageCompressor::CompressionSpec &spec):m_data(new Data){
      m_data->decoded_buffer = 0;
      setCompression(spec);
    }

    ImageCompressor::~ImageCompressor(){
      ICL_DELETE(m_data->decoded_buffer);
      delete m_data;
    }

    /// only decodes an image header
    ImageCompressor::Header ImageCompressor::uncompressHeader(const icl8u *data,int len){
      ICLASSERT_THROW(len > (int)sizeof(Header::Params), ICLException("ImageCompressor::uncompressHeader: data length too small"));
      Header header;
      header.params = *(reinterpret_cast<const Header::Params*>(data));
      header.data = data;
      return header;
    }

    /// internal utlity function
    int ImageCompressor::estimateRawDataSize(const ImgBase *image, bool skipMetaData){
      return (image->getChannels() * image->getDim() * getSizeOf(image->getDepth()) +
              (skipMetaData ? 0 : image->getMetaData().length()) + sizeof(ImgParams) +
              sizeof(depth) + sizeof(Time) );
    }

    int ImageCompressor::estimateEncodedBufferSize(const ImgBase *image, bool skipMetaData){
      const int metaDataLength = skipMetaData ? 0 : (int)image->getMetaData().length();
      const int headerSize = sizeof(Header::Params) + metaDataLength;
      const int numPix = image->getDim() * image->getChannels() * getSizeOf(image->getDepth());
      if(m_data->compression.mode == "rlen" && m_data->compression.quality == "8"){
        return headerSize + 2*numPix;
      }else{
        return headerSize + numPix;
      }
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
                                  const ImageCompressor::CompressionSpec &spec,
                                  Img8u &ditheringBuffer){
      if(spec.mode == "none"){
        std::copy(imageData,imageData+imageSize.getDim(), compressedData);
        compressedData += imageSize.getDim();
      }else if(spec.mode == "dith"){
        DitheringOp op;
        op.setLevels(round(pow(2,parse<int>(spec.quality))));
        Img8u tmp(imageSize, formatGray, std::vector<icl8u*>(1,(icl8u*)imageData), false);
        op.apply(&tmp,bpp(ditheringBuffer));
        return compressChannel(ditheringBuffer.begin(0), compressedData, imageSize, roi,
                               ImageCompressor::CompressionSpec("rlen",spec.quality),
                               ditheringBuffer);
      }else if(spec.mode == "rlen"){
        switch(parse<int>(spec.quality)){
          case 1:{
            const icl8u *imageDataEnd = imageData+imageSize.getDim();
            icl8u currVal = !!*imageData;
            int allLen = 0;
            while(imageData < imageDataEnd){
              const icl8u *other = find_first_not_binarized(imageData,imageDataEnd,currVal);
              size_t len = (size_t)(other-imageData);
              while(len >= 128){
                *compressedData++ = 0xff >> int(!currVal);
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
          }
          case 4:{
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
          case 6:{
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
          case 8:{
            static const int MAX_LEN=256;
            const icl8u *imageDataEnd = imageData+imageSize.getDim();
            int currVal = *imageData;
            int currLen = 0;
            // std::cout << "################" << std::endl;
            while(imageData < imageDataEnd){
              while( (imageData < imageDataEnd) && (*imageData == currVal) ){
                ++currLen;
                ++imageData;
              }
              //std::cout << currLen << " x" << currVal << " | " <<std::flush;
              while(currLen >= MAX_LEN){
                *compressedData++ = currVal;
                *compressedData++ = MAX_LEN-1;
                currLen -= MAX_LEN;
              }
              if(currLen){
                *compressedData++ = currVal;
                *compressedData++ = currLen-1;
              }
              currVal = *imageData;
              currLen = 0;
            }
            break;
          }
          default: throw ICLException("ImageCompressor::compressChannel: unsupported RLE compression quality (" + spec.quality + ")");
				}
      }else{
        throw ICLException("ImageCompressor::compressChannel: unsupported compression mode (" + spec.mode + ")");
      }
      return compressedData;
    }

    static const icl8u *uncompressChannel(icl8u *imageData,
                                          int imageDataLen,
                                          const icl8u *compressedData,
                                          const ImageCompressor::CompressionSpec &spec){
      if(spec.mode == "none"){
        std::copy(compressedData,compressedData+imageDataLen,imageData);
        compressedData += imageDataLen;
      }else if(spec.mode == "rlen" || spec.mode == "dith"){
        switch(parse<int>(spec.quality)){
          case 1:{
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
          case 4:{
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
          case 6:{
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
          case 8:{
            icl8u *pc = imageData;
            icl8u *pcEnd = imageData + imageDataLen;
            const icl8u *p = compressedData;
            for(;pc < pcEnd; p+=2){
              const int l = p[1]+1;
              std::fill(pc,pc+l,p[0]);
              pc += l;
            }
            compressedData = p;
            break;
          }
          default:
            throw ICLException("ImageCompressor::uncompressChannel: unsupported RLE compression quality (" + spec.quality + ")");
        }
      }else{
        throw ICLException("ImageCompressor::uncompressChannel: unsupported compression mode (" + spec.mode + ")");
      }
      return compressedData;
    }

    inline void set_4(char p[4], const char *s){
      for(int i=0;i<4;++i){
        p[i] = s[i];
      }
    }

    ImageCompressor::Header ImageCompressor::createHeader(const ImgBase *image, bool skipMetaData){
      Header::Params params;
      set_4(params.magick,"!icl");
      set_4(params.compressionMode,m_data->compression.mode.c_str());
      params.compressionQuality = parse<icl32s>(m_data->compression.quality);
      params.width = image->getWidth();
      params.height = image->getHeight();
      params.roiX = image->getROIXOffset();
      params.roiY = image->getROIYOffset();
      params.roiWidth = image->getROIWidth();
      params.roiHeight = image->getROIHeight();
      params.channels = image->getChannels();
      params.colorFormat = (icl32s)image->getFormat();
      params.depth = (icl32s)image->getDepth();
      params.dataLen = 0;
      params.metaLen = skipMetaData ? 0 : image->getMetaData().length();
      params.timeStamp = image->getTime().toMicroSeconds();
      Header header = { params, 0 };
      return header;
    }

    const ImageCompressor::CompressedData ImageCompressor::compress(const ImgBase *image, bool skipMetaData){
      ICLASSERT_THROW(image,ICLException("ImageCompressor::compress: image width null"));

			if( (m_data->compression.mode != "none") && image->getDepth() != depth8u
					&& ( (m_data->compression.mode != "1611") && image->getDepth() != depth16s) ){
        throw ICLException("ImageCompressor::compress: image compression is only supported for Img8u images");
      }

      Header header = createHeader(image,skipMetaData);

      if(m_data->compression.mode == "jpeg"){
  #ifdef ICL_HAVE_LIBJPEG

        if(!m_data->jpegEncoder) m_data->jpegEncoder = new JPEGEncoder;
        m_data->jpegEncoder->setQuality(parse<int>(m_data->compression.quality));
        const JPEGEncoder::EncodedData &jpeg = m_data->jpegEncoder->encode(image->as8u());

        int minLen = sizeof(Header::Params) + header.params.metaLen + jpeg.len;
        if((int)m_data->encoded_buffer.size() < minLen){
          m_data->encoded_buffer.resize(minLen);
        }
        icl8u *dst = m_data->encoded_buffer.data();
        header.data = dst;
        header.params.dataLen = minLen;
        *reinterpret_cast<Header::Params*>(dst) = header.params;
        dst += sizeof(Header::Params);

        if(!skipMetaData){
          std::copy(image->getMetaData().begin(), image->getMetaData().end(),dst);//header.metaBegin(), header.metaBegin()+header.params.metaLen,dst);
          dst+= header.params.metaLen;
        }

        std::copy(jpeg.bytes,jpeg.bytes+jpeg.len, dst);

        return CompressedData(m_data->encoded_buffer.data(),minLen,float(minLen)/estimateRawDataSize(image,skipMetaData));
  #else
        throw ICLException("ImageCompressor:encode jpeg compression is not supported without libjpeg");
  #endif
			} else if(m_data->compression.mode == "1611") {

				const Img16s *img16s_in = image->as16s();
				int len = img16s_in->getSize().getDim(); // we support one channel only (single channel 16-bit-kinect image)

				int encoded_len = Kinect11BitCompressor::estimate_packed_size(len);
				const int metaDataLength = skipMetaData ? 0 : (int)image->getMetaData().length();
				const int headerSize = sizeof(Header::Params) + metaDataLength;
				const int numBytes = encoded_len*sizeof(uint16_t);
				const int finalSize = headerSize+numBytes;

				m_data->encoded_buffer.resize(finalSize);
				icl8u *dst = m_data->encoded_buffer.data();

				header.data = dst;
				header.params.dataLen = m_data->encoded_buffer.size();

				*reinterpret_cast<Header::Params*>(dst) = header.params;
				dst += sizeof(Header::Params);
				if(!skipMetaData){
					std::copy(image->getMetaData().begin(), image->getMetaData().end(),dst);
					dst += header.params.metaLen;
				}

				const uint16_t *src_16 = (const uint16_t*)(img16s_in->getData(0));
				uint16_t *dst_16 = (uint16_t*)(dst);

				int q = parse<int>(m_data->compression.quality);
				if (q == 0)
					Kinect11BitCompressor::pack16to11_2(src_16,dst_16,len);
				else if (q == 1)
					Kinect11BitCompressor::pack16to11(src_16,dst_16,len);

				return CompressedData(m_data->encoded_buffer.data(),finalSize,encoded_len/float(len));
			}

      m_data->encoded_buffer.resize(estimateEncodedBufferSize(image,skipMetaData));

      icl8u *dst = m_data->encoded_buffer.data();
      header.data = dst;
      header.params.dataLen = m_data->encoded_buffer.size();

      *reinterpret_cast<Header::Params*>(dst) = header.params;
      dst += sizeof(Header::Params);
      if(!skipMetaData){
        std::copy(image->getMetaData().begin(), image->getMetaData().end(),dst);
        dst+= header.params.metaLen;
      }

      float len = 0;

      if(image->getDepth() == depth8u){
        const Img8u *image8u = image->as8u();
        for(int c=0;c<image->getChannels();++c){
          dst = compressChannel(image8u->begin(c),dst, image8u->getSize(),
                                image8u->getImageRect(), header.compressionSpec(),
                                m_data->ditheringBuffer);
        }
        len = (float)(dst-m_data->encoded_buffer.data());
      }else{ // only no-compression mode
        int l = image->getDim() * getSizeOf(image->getDepth());
        for(int c=0;c<image->getChannels();++c){
          std::copy((const icl8u*)image->getDataPtr(c),
                    (const icl8u*)image->getDataPtr(c)+l,
                    dst+c*l);
        }

        len = l * image->getChannels();
      }

      return CompressedData(m_data->encoded_buffer.data(), (int)len+sizeof(Header::Params), len/m_data->encoded_buffer.size());
    }

    Time ImageCompressor::pickTimeStamp(const icl8u *data){
      ICLASSERT_THROW(data, ICLException("ImageCompressor::decompressTimeStamp: "
                                         "given data pointer is NULL"));
      return Time(reinterpret_cast<const Header::Params*>(data)->timeStamp);
    }

    void ImageCompressor::Header::setupImage(ImgBase **image){
      ensureCompatible(image, (depth)params.depth, Size(params.width,params.height),
                       params.channels, (format)params.colorFormat,
                       Rect(params.roiX,params.roiY,params.roiWidth,params.roiHeight));
      (*image)->setTime(Time(params.timeStamp));
    }

    ImageCompressor::CompressionSpec ImageCompressor::Header::compressionSpec() const{
      return CompressionSpec(getCompressionMode(), str(params.compressionQuality));
    }

    const ImgBase *ImageCompressor::uncompress(const icl8u *data, int len, ImgBase **dst){
      ICLASSERT_THROW(data, ICLException("ImageCompressor::uncompress: "
                                         "given data pointer is NULL"));
      ICLASSERT_THROW(len > (int)sizeof(Header::Params),
                      ICLException("ImageCompressor::uncompress: given data pointer is too short"));

      Header header = uncompressHeader(data,len);
			ImgBase *&useDst = dst ? *dst : m_data->decoded_buffer;

      if(header.getCompressionMode() == "jpeg"){
  #ifdef ICL_HAVE_LIBJPEG
        JPEGDecoder::decode(header.imageBegin(), header.imageLen(), &useDst);
        useDst->getMetaData().assign(header.metaBegin(), header.metaBegin()+header.params.metaLen);
  #else
        throw ICLException("ImageCompressor::uncompress: jpeg decoding is not supported without LIBJPEG");
  #endif
			}else if(header.getCompressionMode() == "1611") {
				int len = header.params.width*header.params.height;
				header.setupImage(&useDst);
				useDst->getMetaData().assign(header.metaBegin(), header.metaBegin()+header.params.metaLen);

				Img16s *s16s = useDst->as16s();
				uint16_t *dst16 = (uint16_t*)s16s->getData(0);
				const uint16_t *src16 = (const uint16_t*)header.imageBegin();
				if (header.params.compressionQuality == 0)
					Kinect11BitCompressor::unpack11to16_2(src16,dst16,len);
				else if (header.params.compressionQuality == 1)
					Kinect11BitCompressor::unpack11to16(src16,dst16,len);

			}else{

        header.setupImage(&useDst);
        useDst->getMetaData().assign(header.metaBegin(), header.metaBegin()+header.params.metaLen);

        const icl8u *p = header.imageBegin();

        if(useDst->getDepth() == depth8u){
          Img8u *b8u = useDst->as8u();
          for(int c=0;c<b8u->getChannels();++c){
            p = uncompressChannel(b8u->begin(c), header.params.width * header.params.height, p, header.compressionSpec());
          }
        }else{
          int l = useDst->getDim() * getSizeOf(useDst->getDepth());
          for(int c=0;c<useDst->getChannels();++c){
            std::copy(p+c*l, p+(c+1)*l, (icl8u*)useDst->getDataPtr(c));
					}
				}
      }
      return useDst;
		}

    void ImageCompressor::setCompression(const ImageCompressor::CompressionSpec &spec){
      if(spec.mode == "none"){
        if(spec.quality.length() && spec.quality != "none"){
          WARNING_LOG("ImageCompressor::setCompression: quality value for compression 'none' is not used");
        }// good
      }else if(spec.mode == "rlen" || spec.mode == "dith"){
        int q = parse<int>(spec.quality);
        if(q!=1 && q!=4 && q!=6 && q!=8){
          throw ICLException("ImageCompressor::setCompression: invalid rlen compression quality (" + spec.quality + ")");
        }
      }else if(spec.mode == "jpeg"){
        int q = parse<int>(spec.quality);
        if(q<0 || q>100){
          throw ICLException("ImageCompressor::setCompression: invalid jpeg compression quality (" + spec.quality + ")");
        }
			}else if (spec.mode == "1611") {
				int q = parse<int>(spec.quality);
				if(spec.quality.length() && (q != 1 && q != 0)){
					throw ICLException("ImageCompressor::setCompression: invalid 1611 compression quality (" + spec.quality + ")");
				}
			}else{
        throw ICLException("ImageCompressor::setCompression: invalid compression mode (" + spec.mode + ")");
      }
      m_data->compression = spec;
    }

    ImageCompressor::CompressionSpec ImageCompressor::getCompression() const{
      return m_data->compression;
    }

  } // namespace io
}
