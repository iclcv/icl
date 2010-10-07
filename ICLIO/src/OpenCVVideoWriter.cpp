/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLIO/src/OpenCVVideoWriter.cpp                        **
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

#include <ICLIO/OpenCVVideoWriter.h>
#include <ICLIO/File.h>
#include <ICLOpenCV/OpenCV.h>


#define ICL_FOURCCC(c1,c2,c3,c4)  \
    (((c1)&255) + (((c2)&255)<<8) + (((c3)&255)<<16) + (((c4)&255)<<24))

namespace icl{

  OpenCVVideoWriter::OpenCVVideoWriter(const std::string &filename, const std::string &fourcc,
		double fps, Size frame_size, int frame_color) throw (ICLException){
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

	writer = cvCreateVideoWriter(filename.c_str(), FOURCC, fps,
			cvSize(frame_size.width,frame_size.height)
			, frame_color);

}

OpenCVVideoWriter::~OpenCVVideoWriter(){
	cvReleaseVideoWriter(&writer);
}

void OpenCVVideoWriter::write(const ImgBase *image){
	ICLASSERT_RETURN(image);
	ICLASSERT_RETURN(image->getDim());
	ICLASSERT_RETURN(image->getChannels());
	IplImage *im = 0;
	im = icl::img_to_ipl(image,&im);
	cvWriteFrame(writer,im );
	cvReleaseImage(&im);
}

OpenCVVideoWriter &OpenCVVideoWriter::operator<<(const ImgBase *image){
	write(image);
	return *this;
}
}
