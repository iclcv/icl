/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/JPEGEncoder.cpp                              **
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

#include <ICLIO/JPEGEncoder.h>
#include <ICLIO/JPEGHandle.h>
#include <ICLCore/Img.h>
#include <ICLUtils/StringUtils.h>
#include <ICLIO/File.h>

namespace icl{

  namespace jpeg_encoder{
    struct MemDst {
      struct jpeg_destination_mgr mgr;
      JOCTET* buffer;
      int bufsize; 
      size_t datasize;
      int* outsize; 
    };
    void init_destination(j_compress_ptr compress){
      MemDst *md = (MemDst*)compress->dest;
      md->mgr.next_output_byte = md->buffer; // target buffer
      md->mgr.free_in_buffer = md->bufsize;  // buffer size
      md->datasize = 0;                      
    }
    int empty_output_buffer (j_compress_ptr compress){
      MemDst *md = (MemDst*)compress->dest;
      md->mgr.next_output_byte = md->buffer;
      md->mgr.free_in_buffer = md->bufsize;
      return true;
    }
    void term_destination (j_compress_ptr compress){
      MemDst *md = (MemDst*)compress->dest;
      md->datasize = md->bufsize - md->mgr.free_in_buffer;
      if (md->outsize) *md->outsize += (int)md->datasize;
    }

    void install_MemDst(j_compress_ptr compress, JOCTET* buffer, int bufsize, int* outsize){
      if(!compress->dest){
        compress->dest = ( (jpeg_destination_mgr*)(*compress->mem->alloc_small) 
                           ((j_common_ptr) compress, JPOOL_PERMANENT, sizeof(MemDst)));
      }
      MemDst *md = (MemDst*)compress->dest;
      md->bufsize = bufsize;
      md->buffer = buffer;
      md->outsize = outsize;
      md->mgr.init_destination = init_destination;
      md->mgr.empty_output_buffer = empty_output_buffer;
      md->mgr.term_destination = term_destination;
    }
  }
  using namespace jpeg_encoder;

  struct JPEGEncoder::Data{
    int quality;
    JPEGEncoder::EncodedData encoded;
    Img8u buffer8u;
    std::vector<icl8u> dataBuffer;
  };

  
  JPEGEncoder::JPEGEncoder(int quality):m_data(new Data){
    m_data->quality = quality;
    m_data->encoded.bytes = 0;
    m_data->encoded.len = 0;
  }
  
  JPEGEncoder::~JPEGEncoder(){
    delete m_data;
  }

  void JPEGEncoder::setQuality(int quality){
    m_data->quality = quality;
  }
    
    
  const JPEGEncoder::EncodedData &JPEGEncoder::encode(const ImgBase *image){
    if(!image){
      m_data->encoded.bytes = 0;
      m_data->encoded.len = 0;
      ERROR_LOG("JPEGEncoder::encode: given image is NULL");
      return m_data->encoded;
    }

    format fmt = image->getFormat();
    int channels = image->getChannels();
    
    if(channels != 1 && channels != 3){
      throw ICLException("JEPGEncoder:encode: jpeg does only support 1 or 3 channels");
    }
    const Img8u *psrc = 0;
    if(image->getDepth()!= depth8u){
      static bool first = true;
      if(first){
        first = false;
        WARNING_LOG("JPEGEncoder:encode: given image depth was not 8u so it had to be converted internally\n"
                    "this might lead to loss of precision (this message is only shown once)");
      }
      image->convert(&m_data->buffer8u);
      psrc = &m_data->buffer8u;
    }else{
      psrc = image->as8u();
    }
    const Img8u &src = *psrc;
    
    //////////////////////////////////////////////////////////////////////
    /// WRITE HEADER DATA ////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    J_COLOR_SPACE jCS;
    switch (fmt) {
      case formatGray: jCS = JCS_GRAYSCALE; break;
      case formatYUV:  jCS = JCS_YCbCr; break;
      case formatRGB:  jCS = JCS_RGB; break;
      case formatMatrix:{
        if(channels == 1){
          jCS = JCS_GRAYSCALE; 
        }else if(channels == 3){
          jCS = JCS_RGB;
        }else{
          throw ICLException(str(__FUNCTION__)+": matrix format with " + str(channels) + " channels is not supported");
        }
      }

      default: 
        throw ICLException(str(__FUNCTION__)+":"+str(fmt) + " not supported by jpeg");
    }
    
    ICLException err(str(__FUNCTION__)+": Error in JPEG compression");

    struct jpeg_compress_struct jpgCinfo;
    struct icl_jpeg_error_mgr   jpgErr;
    
    // Step 1: Set up the error handler first, in case initialization fails
    jpgCinfo.err = jpeg_std_error(&jpgErr);
    if (setjmp(jpgErr.setjmp_buffer)) {
      /* If we get here, the JPEG code has signaled an error.
          * We need to clean up the JPEG object and signal the error to the caller */
      jpeg_destroy_compress(&jpgCinfo);
      throw err;
    }
    
    /* Now we can initialize the JPEG compression object. */
    jpeg_create_compress(&jpgCinfo);
    
    // Step 2: specify data destination
    int bytesWritten = 0;
    m_data->dataBuffer.resize(4000 + src.getWidth() * src.getHeight() * src.getChannels() * 2);
    install_MemDst(&jpgCinfo,(JOCTET*)m_data->dataBuffer.data(),m_data->dataBuffer.size(),&bytesWritten);
    
    /* Step 3: set parameters for compression */
    jpgCinfo.image_width  = src.getSize().width;
    jpgCinfo.image_height = src.getSize().height;
    jpgCinfo.input_components = src.getChannels(); // # of color components 
    jpgCinfo.in_color_space = jCS; 	/* colorspace of input image */
    
    /* Now use the library's routine to set default compression parameters.
        * (You must set at least jpgCinfo.in_color_space before calling this,
        * since the defaults depend on the source color space.) */
    jpeg_set_defaults(&jpgCinfo);
    
    /* Now you can set any non-default parameters you wish to.
        * Here we just illustrate the use of quality (quantization table) scaling: */
    jpeg_set_quality(&jpgCinfo, m_data->quality, TRUE /* limit to baseline-JPEG values */);
    
    /* Step 4: Start compressor */
    /* TRUE ensures that we will write a complete interchange-JPEG file.
        * Pass TRUE unless you are very sure of what you're doing. */
    jpeg_start_compress(&jpgCinfo, TRUE);

#ifdef HAVE_JPEG_MARKERS    
    // this leads to errors when loading the encoded stuff from data segment
    /* Step 5: Write comments */
    char acBuf[1024];
    // timestamp
#if __WORDSIZE == 64
    sprintf (acBuf, "TimeStamp %ld", src.getTime().toMicroSeconds());
#else
    sprintf (acBuf, "TimeStamp %lld", src.getTime().toMicroSeconds());
#endif

    jpeg_write_marker(&jpgCinfo, JPEG_COM, (JOCTET*) acBuf, strlen(acBuf));
    
    // ROI
    Rect roi = src.getROI ();
    sprintf (acBuf, "ROI %d %d %d %d", roi.x, roi.y, roi.width, roi.height);
    jpeg_write_marker(&jpgCinfo, JPEG_COM, (JOCTET*) acBuf, strlen(acBuf));
#endif

    //////////////////////////////////////////////////////////////////////
    /// WRITE IMAGE DATA  ////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    
    /* Step 6: while (scan lines remain to be written) */
    if (src.getChannels () == 1) {
      int iLineStep = src.getSize().width;
      // grayscale image, can handover image channels directly
      while (jpgCinfo.next_scanline < jpgCinfo.image_height) {
        icl8u *pcBuf = const_cast<icl8u*>(src.getData (0)) + jpgCinfo.next_scanline*iLineStep;
        (void) jpeg_write_scanlines(&jpgCinfo, &pcBuf, 1);
      }
    } else {
      // file format is interleaved, i.e. RGB or something similar
      const Size& size = src.getSize();
      std::vector<icl8u> buf(3*size.width);
      const icl8u *pcR = src.begin(0);
      const icl8u *pcG = src.begin(1);
      const icl8u *pcB = src.begin(2);
      for (int l=0; l<size.height; l++) {
        icl8u *pc=buf.data();
        for (int c=0; c<size.width; ++c){
          *pc++ = pcR[c];
          *pc++ = pcG[c];
          *pc++ = pcB[c];
        } 
        pcR += size.width;
        pcG += size.width;
        pcB += size.width;
        icl8u *pcBuf = buf.data();
        (void) jpeg_write_scanlines(&jpgCinfo, &pcBuf, 1);
      } 
    }

    
    /* Step 7: Finish compression */
    jpeg_finish_compress(&jpgCinfo);
    
    /* Step 8: release JPEG compression object */
    jpeg_destroy_compress(&jpgCinfo);

    m_data->encoded.bytes = m_data->dataBuffer.data();
    m_data->encoded.len = bytesWritten;
    SHOW(m_data->dataBuffer.size());
    SHOW(bytesWritten);
    return m_data->encoded;

  }

  void JPEGEncoder::writeToFile(const ImgBase *image, const std::string &filename){
    const EncodedData &encoded = encode(image);
    File file(filename);
    file.open(File::writeBinary);
    file.write(encoded.bytes,encoded.len);
    file.close();
  }
}
