/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : include/ICLIO/OpenCVVideoWriter.h                      **
 ** Module : ICLIO                                                  **
 ** Authors: Christian Groszewski                                   **
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

#ifndef ICL_OPENCVVIDEOWRITER_H
#define ICL_OPENCVVIDEOWRITER_H

#include <string>
#ifdef HAVE_OPENCV211
#include <opencv2/highgui/highgui_c.h>
#else
#include <highgui.h>
#include <cxtypes.h>
#endif
#include <ICLIO/ImageOutput.h>
#include <ICLUtils/Uncopyable.h>



namespace icl{

  class OpenCVVideoWriter :public ImageOutput, public Uncopyable{
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
	          double fps, Size frame_size, int frame_color=1) throw (ICLException);

	/// Destructor
	~OpenCVVideoWriter();

	/// writes the next image
	void write(const ImgBase *image);
        
        /// wraps write to implement ImageOutput interface
        virtual void send(const ImgBase *image) { write(image); }

	/// as write but in stream manner
	OpenCVVideoWriter &operator<<(const ImgBase *image);

};

}

#endif
