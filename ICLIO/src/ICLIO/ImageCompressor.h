/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/ImageCompressor.h                      **
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

#pragma once

#include <ICLCore/Img.h>
#include <ICLUtils/Uncopyable.h>

namespace icl{
  namespace io{
    /// Encoder class for optimized encoding of images
    /** The ImageCompressor class allows for simple and for optimized serialization and
        desirealization of images
        
        \section MODES Serialization Modes
        
        Right now, 3 different serialization modes are supported. 
        - "rlen" Run Length Encoding: here, the image is scanned line by line
          and instead of encoding pixel data [ pix1, pix2, pix3, ...], it is encoded
          by [value|length] pairs. Which means, that the <em>value</em> was found
          <em>length+1</em> times in the image data. This type of compression is very
          well suited for images with low structure and in particular for
          low noise binary images. However, it fails completely for natual camera images.
          The "rlen" plugin supports four different quality settings, where each setting
          determines the number of bits, that are used for the <em>value</em> and 
          for <em>lenght</em> tokens.
          - Quality 1: value: 1Bit, length 7Bit. This core::format is most appropriate for
            binary images with low noise. Here, a compression to 1% of the original size
            can easily be reached. Please note, that the binarization threshold is explicitly
            set to 127 for this quality level. For the other quality levels, only the
            most significant bits are used.
          - Quality 4: value: 4Bit, length 4Bit. Can be used to compress with remaining 4Bit
            value resolution (usually, this is only needed for very special cases)
            The compression is usually also worse since the maximum reachable compression
            ration is 1/(2^4).
          - Quality 6: value 6Bit, length 2Bit. Bad compression ratio, but an visibly almost
            lossless compression. 
          - Quality 8: value 8Bit, length 8Bit. Best for non-binary images with lots of 
            homogeneous regions. Lossless, but not neccessarily better than uncompressed
            serialization. Best case: 1/(2^8) compression, Worst case: 2/1. 
        - "jpeg" uses jpeg compression. Comparable to motion jpeg and only available if
          ICL has been build with LIBJPEG support. The compression quality can be varied
          from 0 to 100. Which is the default JPEG compression quality value. Usually values
          around 70 provide sufficient results with a very good compression ration (about 2%).
          However, please note that even a quality setting of 100 does not lead to lossless 
          compression.
        - "none" Uncompressed image serialization. This is the only compression mode, that
          can be used for non-core::Img8u images.
        
        \section DEPTH Depth Support
        Only core::Img8u-images can be compressed. For all other formats, the compression 
        mode "none" (with quality setting "none") has to be chosen.
  
        \section SERIALIZATION Serialization Structure
        The serialized image consists of 3 parts: The Header information, which is
        exactly the binarized version of ImageCompressor::Header::Params. Optionally
        meta data, whose length can also be 0. And the actual compressed image data.
        The lengths of the meta data and the image data segements are defined by the
        header.
        
        \section META Meta Data
        The ImageCompressor preserves an images meta data, however meta data is always binarized
        in an uncompressed manner.
        
        \section ROI ROI Support
        The image ROI is also compressed and later uncompressed. So if you
        compress an image with a non-full ROI, the uncompressed version will
        also be the whole image with a non-full ROI.
        
        \section BENCH Benchmark
        There are several factors, that have to be regarded: 
        * image size
        * image fragmentation (b/w noise)
        
        Both, the encoding /decoding steps should scale linearily with 
        the image pixel count, while extremely vertical images provide
        in slower results. Also the image fragmentation is important for
        the en-/de-coding speed. Benchmarks were take on an 
        Intel (R) Xeon E5530  \@2.40GHz CPU, only single channel images were
        used.      
        
        - VGA image, high noise: compression to 20%: encoding 3msec, decoding 2msec
        - VGA image, low noise: compression to 5%: encoding 1msec, decoding 0.8msec
        - VGA image, very low noise: compression to 1.7%: encoding 0.4msec, decoding 0.37msec
        - SXVGA (4-times VGA), high noise, compression to 22%: encoding 8.4msec, decoding 7.8msec
        - SXVGA (4-times VGA), low noise, compression to 4.1%: encoding 2.0msec, decoding 1.8msec
        - SXVGA (4-times VGA), very low noise, compression to 1.4%: encoding 0.8msec, decoding 0.76mse
        
        
        
        \section MEM Memory Consumption 
        In order to avoid run-time memory allocation, the
        run-length coder always ensures that enough bytes
        are available: I.e. 1 byte per image pixel 
        + header size of 37 + N bytes (N is meta data size)\n
        for JPEG data, 2x the raw image data is allocated
    */
    class ICL_IO_API ImageCompressor : public utils::Uncopyable{
      struct Data;  //!< pimpl type
      Data *m_data; //!< pimpl pointer
  
      /// internal utlity function
      int estimateEncodedBufferSize(const core::ImgBase *image, bool skipMetaData);
      
      /// internal utlity function
      int estimateRawDataSize(const core::ImgBase *image, bool skipMetaData);
      
      public:
      /// compression specification
      struct CompressionSpec{
        explicit CompressionSpec(const std::string &mode=std::string(),
                        const std::string &quality=std::string()):mode(mode),quality(quality){}
        std::string mode;    //!< mode
        std::string quality; //!< quality
      };
  
      protected:
      /// Compressed image header
      struct Header{
        struct Params{
          char magick[4];            //!< magick code
          char compressionMode[4];   //!< compression mode
          icl32s compressionQuality; //!< compression quality
          icl32s width;              //!< image width
          icl32s height;             //!< image height
          icl32s roiX;               //!< image roi x offset
          icl32s roiY;               //!< image roi y offset
          icl32s roiWidth;           //!< image roi width
          icl32s roiHeight;          //!< image roi width
          icl32s channels;           //!< image channel count
          icl32s colorFormat;        //!< image color format
          icl32s depth;              //!< image depth
          icl32s dataLen;            //!< length of data segment
          icl32s metaLen;            //!< length of meta data segemnt
          int64_t timeStamp;         //!< timestamp
        } params;
        
        const icl8u *data;           //!< compressed data begin (data[0] = magick[0])
        inline const char *metaBegin() const { return (char*)(data + sizeof(Params)); }
        inline const icl8u *imageBegin() const { return (icl8u*)(metaBegin() + params.metaLen); }
        inline int imageLen() const { return params.dataLen - sizeof(Params) - params.metaLen; }
        inline std::string getMagickCode() const { return std::string(params.magick,params.magick+4); }
        inline std::string getCompressionMode() const { return std::string(params.compressionMode,params.compressionMode+4); }
        CompressionSpec compressionSpec() const;
        void setupImage(core::ImgBase **image);  //!< sets up a given image for beeing compatible the the header
      };
      
      
  
      /// only decodes an image header
      /** not supported for jpeg data */
      Header uncompressHeader(const icl8u *compressedData, int len);
      
      /// creates a header for a given image (not data will be null)
      Header createHeader(const core::ImgBase *image, bool skipMetaData);
  
      public:
  
      
      /// most simple dyn-size array with given data and length
      /** This class does no memory management at all */
      struct CompressedData{
        /// Constructor
        CompressedData(icl8u *bytes=0,int len=0, float compressionRatio=0, 
                       const CompressionSpec &compression=CompressionSpec()):
        bytes(bytes),len(len),compressionRatio(compressionRatio),
          compression(compression){}
        
        /// data pointer
        icl8u *bytes;
        
        /// num elements
        int len;
      
        /// compression rate (1: no compression, 0.5: 50% compression , ...)
        float compressionRatio;
        
        /// underlying compression mode
        CompressionSpec compression;
      };
     
      /// Creates an image Compressor with given compression mode
      ImageCompressor(const CompressionSpec &spec=CompressionSpec("none"));
  
      /// Destructor
      ~ImageCompressor();
      
      /// string based interface for setting the compression mode
      /** @param spec (e.g. CompressionSpec("mode") with mode = rle, jpg, or none)
          the compression quality, which might depend on the set type 
                 (e.g. 1 Bit for binary rle, or 90 for 90% jpeg compression) 
          Current valid compression modes are
          - "none" no compression (default, possible for all data types)
          - "rlen" run lenght encoding (quality can be 1, 4 or 6, it defines 
            the number of bits, that are used for the value domain. 
            Default value is 1, which is used for binary images)
          - "jpeg" jpeg encoding (quality is default jpeg quality 1% - 100%)
      */
      virtual void setCompression(const CompressionSpec &spec);
      
      /// can be implemented for returning the current compression mode
      virtual CompressionSpec getCompression() const;
        
    
  
      /// this can help to find out wheter the data is new and must be encoded
      /** not supported for jpeg data */
      utils::Time pickTimeStamp(const icl8u *compressedData); 
      
      /// encodes a given image into the compressed code
      /** JPEG compression does not yet meta data */
      const CompressedData compress(const core::ImgBase *image, bool skipMetaData=false);
  
      /// decodes the given byte segment
      const core::ImgBase *uncompress(const icl8u *compressedData, int len, core::ImgBase **dst=0);
    };
  
  } // namespace io
}
