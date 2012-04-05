/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/ImageCompressor.h                        **
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

#ifndef ICL_IMAGE_COMPRESSOR_H
#define ICL_IMAGE_COMPRESSOR_H

#include <ICLCore/Img.h>
#include <ICLUtils/Uncopyable.h>

namespace icl{
  /// Encoder class for optimized encoding of images
  /** The image encoder class allows for simple and for optimized serialization and
      desirealization of images
      
      \section MODES Serialization Modes

      \section HEAD RLE Header Information
      This is the structure of the header (XB means X bytes) for run length encoded data
      - 3B: magic bytes "rle"
      - 4B: compression-mode
      - 4B: width
      - 4B: height
      - 4B: ROI x
      - 4B: ROI y
      - 4B: ROI width
      - 4B: ROI height
      - 4B: byte channel-count
      - 4B: size of meta data (content: N)
      - 8B: image timestamp in microseconds
      - NB: meta data
      
      \section ET Encoding Technique
      Right after the header, the image data is encoded. This is a list
      of bytes [b]. Each byte b encodes a run of length 1-128. The most
      significant by defines the color (1: white, 0: black) of that run.
      The remaining 7 bits directly encode the run length. Please note,
      that there are not runs of length 0, which is why the length that 
      is encoded by the 7 least significant bits of b (b & 127) is always
      increased by 1 internally. 

      \section CR Compression Rates
      The Resuling data structure BinaryImageCompressor::Data does also
      provide information about the compression rate. For very noise
      binare images, this might be something like 0.3. For common binary
      images with low noise (e.g. an image showing several fiducial markers
      filtered with a local threshold operation), the compression rate
      usually gets down to something like 0.03 (i.e. to 3% of the original
      memory usage). Due to the fact, that in a worst case scenerio each
      pixel has to be represented by one run (i.e. also one byte), The
      compression ratio can never get much higher than 1.0. Only the
      additional image header can make the compression rate become higher 
      than 1.0
      
      \section ROI ROI Support
      Right now, the BinaryImageCompressor doese not support image ROIs
      in the decoding step. Please do not use images with ROI yet.
      
      \section BENCH Benchmark
      There are several factors, that have to be regarded: 
      * image size
      * image fragmentation (b/w noise)
      
      Both, the encoding /decoding steps should scale linearily with 
      the image pixel count, while extremely vertical images provide
      in slower results. Also the image fragmentation is important for
      the en-/de-coding speed. Benchmarks were take on an 
      Intel (R) Xeon E5530  @2.40GHz CPU, only single channel images were
      used.      
      
      - VGA image, high noise: compression to 20%: encoding 3msec, decoding 2msec
      - VGA image, low noise: compression to 5%: encoding 1msec, decoding 0.8msec
      - VGA image, very low noise: compression to 1.7%: encoding 0.4msec, decoding 0.37msec
      - SXVGA (4-times VGA), high noise, compression to 22%: encoding 8.4msec, decoding 7.8msec
      - SXVGA (4-times VGA), low noise, compression to 4.1%: encoding 2.0msec, decoding 1.8msec
      - SXVGA (4-times VGA), very low noise, compression to 1.4%: encoding 0.8msec, decoding 0.76msec
      
      
      \section MEM Memory Consumption 
      In order to avoid run-time memory allocation, the
      run-length coder always ensures that enough bytes
      are available: I.e. 1 byte per image pixel 
      + header size of 32 + N bytes (N is meta data size)\n
      for JPEG data, 2x the raw image data is allocated
  */
  class ImageCompressor : public Uncopyable{
    struct Data;  //!< pimpl type
    Data *m_data; //!< pimpl pointer
    public:

    enum CompressionMode{
      CompressRLE1Bit,
      CompressRLE4Bit,
      CompressRLE6Bit,
      CompressJPEG,   //!< needs libjpeg
      NoCompression   
    };

    private:

    int estimateBufferSize(const Img8u &image, int metaDataLength);
    public:
    
    /// Creates an image Compressor with given compression mode
    ImageCompressor(CompressionMode mode=CompressRLE1Bit);

    /// Destructor
    ~ImageCompressor();
    
    /// sets the image compression mode
    void setCompressionMode(CompressionMode mode);

    /// sets the jpeg compression quality
    void setJPEGQuality(int percent);

    /// returns the current image compression mode
    CompressionMode getCompressionMode() const;
      
    /// most simple dyn-size array with given data and length
    /** This class does no memory management at all */
    struct CompressedData{
      /// Constructor
      CompressedData(icl8u *bytes=0,int len=0, float compression=0):
      bytes(bytes),len(len),compression(compression){}
    
      /// data pointer
      icl8u *bytes;
    
      /// num elements
      int len;
    
      /// compression rate (1: no compression, 0.5: 50% compression , ...)
      float compression;
      
      /// underlying compression mode
      CompressionMode mode;
    };
    
    /// Compressed image header only used for RunLength encoded data
    struct Header{
      CompressionMode mode;    //!< compression type
      Size size;               //!< the image's size
      Rect roi;                //!< the image's region of interest
      int channelCount;        //!< number of channels
      std::string metaData;    //!< metaData associated with the image
      Time timeStamp;          //!< the image's timestamp
      const icl8u *dataBegin;  //!< data pointer that points to the first non-header-byte
      int dataLen;             //!< length of the data without the header-size
      void setupImage(Img8u &image);
    };
    
    /// encodes a given image into the compressed code
    /** JPEG compression does not yet meta data */
    const CompressedData encode(const Img8u &image, const std::string &meta="");
    
    /// this can help to find out wheter the data is new and must be encoded
    /** not supported for jpeg data */
    Time decodeTimeStamp(const icl8u *data); 
    
    /// only decodes an image header
    /** not supported for jpeg data */
    Header decodeHeader(const icl8u *data, int len, bool decodeMetaData);

    /// decodes the given byte segment
    const Img8u &decode(const icl8u *data, int len, std::string *meta=0);
  };

}
#endif
