/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
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
 ** The development of this software was supported by the           **
 ** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
 ** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
 ** Forschungsgemeinschaft (DFG) in the context of the German       **
 ** Excellence Initiative.                                          **
 **                                                                 **
 *********************************************************************/
#include <ICLIO/OpenCVVideoGrabber.h>

namespace icl{

  struct OpenCVVideoGrabber::Data{
    /// Device struct
    CvCapture *cvc;
    ///Filename of the file
    std::string filename;
    ///Ensures the desired framerate
    FPSLimiter *fpslimiter;
    ///Buffer for scaling if necessary
    ImgBase *m_buffer;
    ///
    bool use_video_fps;
    
    Size size;
  };


  std::vector<std::string> OpenCVVideoGrabber::getPropertyList(){
    static const std::string ps="pos_msec pos_frames pos_avi_ratio size format fourcc frame_count use_video_fps video_fps";
    return tok(ps," ");
  }

  std::string OpenCVVideoGrabber::getType(const std::string &name){
    if( name == "pos_avi_ratio" || name == "fps" || name == "size"
        || name == "fourcc"|| name == "frame_count" || name == "video_fps"){
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
      return "[0,"+ str( 1000*((cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FRAME_COUNT)/
                                cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FPS))) )+"]:0.1";
    }else if(name == "pos_frames"){
      return "[0,"+str(cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FRAME_COUNT))+"]:1";
    }else if(name == "pos_avi_ratio"){
      return "[0,1]:"+str(cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FRAME_COUNT)/
                          cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FPS));
    }else if(name == "use_video_fps"){
      return "{\"on\",\"off\"}";
    }else if(name == "format"){
      return "{\"RGB\"}";
    }else if(name == "size"){
      return "{\"" + str(data->size) + "\"}";
    }
    return "undefined";
  }

  std::string OpenCVVideoGrabber::getValue(const std::string &name){
    if(name == "pos_msec"){
      return str(cvGetCaptureProperty(data->cvc,CV_CAP_PROP_POS_MSEC));
    }else if(name == "pos_frames"){
      return str(cvGetCaptureProperty(data->cvc,CV_CAP_PROP_POS_FRAMES));
    }else if(name == "pos_avi_ratio"){
      return str(cvGetCaptureProperty(data->cvc,CV_CAP_PROP_POS_AVI_RATIO));
    }else if(name == "size"){
      return str(data->size);
    }else if(name == "video_fps"){
      return str(cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FPS));
    }else if(name == "fourcc"){
      return str(cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FOURCC));
    }else if(name == "frame_count"){
      return str(cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FRAME_COUNT));
    } else if(name == "format"){
      return "RGB";
    }
    return "undefined";
  }

  const ImgBase *OpenCVVideoGrabber::acquireImage(){
    ICLASSERT_RETURN_VAL( !(data->cvc==0), 0);
    icl::ipl_to_img(cvQueryFrame(data->cvc),&data->m_buffer);
    if(data->use_video_fps){
      data->fpslimiter->wait();
    }
    return data->m_buffer;
  }

  OpenCVVideoGrabber::OpenCVVideoGrabber(const std::string &fileName)
    throw (FileNotFoundException):data(new Data){
    data->m_buffer = 0;
    data->use_video_fps = true;
    data->filename = fileName;
    
    if(!File(fileName).exists()){
      throw FileNotFoundException(fileName);
    }

    data->cvc = cvCaptureFromFile(fileName.c_str());
    int fps = cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FPS);
    data->fpslimiter = new FPSLimiter(fps);

    data->size.width = cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FRAME_WIDTH);
    data->size.height = cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FRAME_HEIGHT);
  }

  OpenCVVideoGrabber::~OpenCVVideoGrabber(){
    cvReleaseCapture(&data->cvc);
    delete data->fpslimiter;
    ICL_DELETE(data->m_buffer);
    delete data;
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
      i = cvSetCaptureProperty(data->cvc,CV_CAP_PROP_POS_MSEC,parse<double>(value));
    }else if(name == "pos_frames"){
      i = cvSetCaptureProperty(data->cvc,CV_CAP_PROP_POS_FRAMES,parse<double>(value));
    }else if(name == "pos_avi_ratio"){
      i = cvSetCaptureProperty(data->cvc,CV_CAP_PROP_POS_AVI_RATIO,parse<double>(value));
    }else if(name  == "use_video_fps"){
      data->use_video_fps = (value == "on");
    }
    (void)i;
  }
}
