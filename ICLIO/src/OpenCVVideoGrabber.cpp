/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
 **                         University of Bielefeld                 **
 **                Contact: nivision@techfak.uni-bielefeld.de       **
 **                Website: www.iclcv.org                           **
 **                                                                 **
 ** File   : ICLIO/src/OpenCVVideoGrabber.cpp                       **
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
 *********************************************************************/
#include <ICLIO/OpenCVVideoGrabber.h>

namespace icl{

std::vector<std::string> OpenCVVideoGrabber::getPropertyList(){
	static const std::string ps="pos_msec pos_frames pos_avi_ratio size format fps fourcc frame_count use_video_fps";
	return tok(ps," ");
}

std::string OpenCVVideoGrabber::getType(const std::string &name){
	if( name == "pos_avi_ratio" || name == "fps" || name == "size"
			|| name == "fourcc"|| name == "frame_count"){
		return "info";
	} else if(name == "format" || name == "use_video_fps"){
		return "menu";
	}else if(name == "pos_msec" || name == "pos_frames"){
		return "range";
	}
	return "undefined";
}

std::string OpenCVVideoGrabber::getInfo(const std::string &name){
	if(name == "pos_msec"){
		return "[0,"+ str( 1000*((cvGetCaptureProperty(cvc,CV_CAP_PROP_FRAME_COUNT)/cvGetCaptureProperty(cvc,CV_CAP_PROP_FPS))) )+"]:0.1";
	}else if(name == "pos_frames"){
		return "[0,"+str(cvGetCaptureProperty(cvc,CV_CAP_PROP_FRAME_COUNT))+"]:1";
	}else if(name == "pos_avi_ratio"){
		return "[0,1]:"+str(cvGetCaptureProperty(cvc,CV_CAP_PROP_FRAME_COUNT)/cvGetCaptureProperty(cvc,CV_CAP_PROP_FPS));
	}else if(name == "use_video_fps"){
		return "{\""+str(cvGetCaptureProperty(cvc,CV_CAP_PROP_FPS))+"\"}";
	}else if(name == "size"){
		return "something ...";
	}else if(name == "fps"){
		return "something ...";
	}else if(name == "fourcc"){
		return "something ...";
	}else if(name == "frame_count"){
		return "something ...";
	}else if(name == "format"){
		return "something ...";
	}
	return "undefined";
}

std::string OpenCVVideoGrabber::getValue(const std::string &name){
	if(name == "pos_msec"){
		return str(cvGetCaptureProperty(cvc,CV_CAP_PROP_POS_MSEC));
	}else if(name == "pos_frames"){
		return str(cvGetCaptureProperty(cvc,CV_CAP_PROP_POS_FRAMES));
	}else if(name == "pos_avi_ratio"){
		return str(cvGetCaptureProperty(cvc,CV_CAP_PROP_POS_AVI_RATIO));
	}else if(name == "size"){
		std::ostringstream s;
		    s << cvGetCaptureProperty(cvc,CV_CAP_PROP_FRAME_WIDTH)
		      << "x" << cvGetCaptureProperty(cvc,CV_CAP_PROP_FRAME_HEIGHT);
		    return s.str();
	}else if(name == "fps"){
		return str(cvGetCaptureProperty(cvc,CV_CAP_PROP_FPS));
	}else if(name == "fourcc"){
		return str(cvGetCaptureProperty(cvc,CV_CAP_PROP_FOURCC));
	}else if(name == "frame_count"){
		return str(cvGetCaptureProperty(cvc,CV_CAP_PROP_FRAME_COUNT));
	} else if(name == "format"){
		return "{\"RGB\"}";
	}
	return "undefined";
}

const ImgBase *OpenCVVideoGrabber::grabUD (ImgBase **ppoDst){
	if(!ppoDst){
        ppoDst = &m_poImage;
	}
	ICLASSERT_RETURN_VAL( !(cvc==0), 0);
	IplImage *img = cvQueryFrame(cvc);

	if(!m_bIgnoreDesiredParams){
		Size iplSize(img->width,img->height);
		if(getDesiredSize() == iplSize && getDesiredFormat() == formatRGB){
			ensureCompatible(ppoDst,getDesiredDepth(),getDesiredParams());
			icl::ipl_to_img(img,ppoDst,PREFERE_DST_DEPTH);
		}else{
			ensureCompatible(&scalebuffer,m_eDesiredDepth,iplSize,formatRGB);
			icl::ipl_to_img(img,&scalebuffer,PREFERE_DST_DEPTH);
			ensureCompatible(ppoDst,getDesiredDepth(),getDesiredParams());
			scalebuffer->scaledCopy(ppoDst,interpolateLIN);
		}
	} else {
		//this function takes care if ppoDst is NULL
		icl::ipl_to_img(img,ppoDst);
	}
	if(use_video_fps)
		fpslimiter->wait();
	return *ppoDst;
}

OpenCVVideoGrabber::OpenCVVideoGrabber(const std::string &fileName)
	throw (FileNotFoundException):scalebuffer(0),use_video_fps(false){
	if(!File(fileName).exists()){
		throw FileNotFoundException(filename);
	}
	filename = fileName;
	cvc = cvCaptureFromFile(filename.c_str());
	fpslimiter = new FPSLimiter(cvGetCaptureProperty(cvc,CV_CAP_PROP_FPS));
}

OpenCVVideoGrabber::~OpenCVVideoGrabber(){
	cvReleaseCapture(&cvc);
	delete fpslimiter;
	ICL_DELETE(scalebuffer);
}

int OpenCVVideoGrabber::isVolatile(const std::string &propertyName){
	if(propertyName == "pos_msec"){
		return 1;
	} else if(propertyName == "pos_frames"){
		return 1;
	} else if(propertyName == "pos_avi_ratio"){
		return 1;
	} else {
		return 0;
	}
}

void OpenCVVideoGrabber::setProperty(const std::string &name, const std::string &value){
	int i = 0;
	if(name == "pos_msec"){
		i = cvSetCaptureProperty(cvc,CV_CAP_PROP_POS_MSEC,parse<double>(value));
	}else if(name == "pos_frames"){
		i = cvSetCaptureProperty(cvc,CV_CAP_PROP_POS_FRAMES,parse<double>(value));
	}else if(name == "pos_avi_ratio"){
		//depends on file
		i = cvSetCaptureProperty(cvc,CV_CAP_PROP_POS_AVI_RATIO,parse<double>(value));
	}else if(name == "use_video_fps"){
		use_video_fps = true;
		fpslimiter->setMaxFPS(parse<float>(value));
	}
}
}
