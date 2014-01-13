/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/OpenCVVideoWriter.h                    **
** Module : ICLIO                                                  **
** Authors: Christian Groszewski                                   **
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

#include <string>

#ifdef HAVE_OPENCV
#include <opencv/highgui.h>
#endif

#include <ICLIO/ImageOutput.h>
#include <ICLUtils/Uncopyable.h>



namespace icl{
  namespace io{
  
    class ICL_IO_API OpenCVVideoWriter :public ImageOutput{
      private:
      ///OpenCV VideoWriter struct
  	CvVideoWriter *writer;
      public:
        
  	/// Creates a new videowriter with given filename
  	/** @param filename the filename to write to
  	    @param fourcc this is translated into an instance of FOURCC
            possible is:
            * PIM1 (for mpeg 1)
            * MJPG (for motion jepg)
              * MP42 (for mpeg 4.2)
            * DIV3 (for mpeg 4.3)
            * DIVX (for mpeg 4)
            * U263 (for H263 codec)
            * I263 (for H263I codec)
            * FLV1 (for FLV1 code)
            * on linux: IYUV for IYUV codec ??
            * on windows: "" for open dialog
            
  	    @param fps frames per second
  	    @param frame_size size of the frames to be written out
  	    @param frame_color currently only supported on windows 0 for greyscale else color
            **/
  	OpenCVVideoWriter(const std::string &filename, const std::string &fourcc,
                          double fps, utils::Size frame_size, int frame_color=1) throw (utils::ICLException);
        
  	/// Destructor
  	~OpenCVVideoWriter();
        
  	/// writes the next image
  	void write(const core::ImgBase *image);
        
        /// wraps write to implement ImageOutput interface
        virtual void send(const core::ImgBase *image) { write(image); }
        
  	/// as write but in stream manner
  	OpenCVVideoWriter &operator<<(const core::ImgBase *image);
    };
  } // namespace io
}

