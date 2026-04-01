// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#include <ICLIO/OpenCVVideoWriter.h>
#include <ICLUtils/File.h>
#include <ICLCore/OpenCV.h>

using namespace icl::utils;
using namespace icl::core;

#define ICL_FOURCCC(c1,c2,c3,c4)                                        \
  (((c1)&255) + (((c2)&255)<<8) + (((c3)&255)<<16) + (((c4)&255)<<24))

namespace icl{
  namespace io{

    OpenCVVideoWriter::OpenCVVideoWriter(const std::string &filename, const std::string &fourcc,
                                         double fps, Size frame_size, int frame_color){
      if(File(filename).exists()){
        throw ICLException("file already exists");
      }
      if(fps <= 0){
        throw ICLException("Invalid fps value");
      }
      if(frame_size.width < 1 || frame_size.height < 1){
        throw ICLException("frame size invalid");
      }
      /*if(0){
          throw ICLException("frame color invalid");
          }*/

      int FOURCC = -1;
      if(fourcc.length() == 4){
        FOURCC = ICL_FOURCCC(fourcc[0],fourcc[1],fourcc[2],fourcc[3]);
      }

      writer.reset(new cv::VideoWriter(filename.c_str(), FOURCC, fps,
                                       cv::Size(frame_size.width, frame_size.height),
                                       frame_color));

    }

    OpenCVVideoWriter::~OpenCVVideoWriter(){
    }

    void OpenCVVideoWriter::write(const ImgBase *image){
      ICLASSERT_RETURN(image);
      ICLASSERT_RETURN(image->getDim());
      ICLASSERT_RETURN(image->getChannels());
      cv::Mat* im = 0;
      im = core::img_to_mat(image,im);
      writer->write(*im );
    }

    OpenCVVideoWriter &OpenCVVideoWriter::operator<<(const ImgBase *image){
      write(image);
      return *this;
    }
  } // namespace io

}
