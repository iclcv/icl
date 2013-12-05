/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/OpenCVVideoGrabber.cpp                 **
** Module : ICLIO                                                  **
** Authors: Christian Groszewski, Viktor Richter                   **
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

#include <ICLIO/OpenCVVideoGrabber.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{

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

    std::string fourCCStringFromDouble(double value){
      int fourInt = (int) value;
      std::string ret((char*) (&fourInt), 4);
      return ret;
    }

    const ImgBase *OpenCVVideoGrabber::acquireImage(){
      utils::Mutex::Locker l(mutex);
      ICLASSERT_RETURN_VAL( !(data->cvc==0), 0);
      core::ipl_to_img(cvQueryFrame(data->cvc),&data->m_buffer);
      if(data->use_video_fps){
        data->fpslimiter->wait();
      }
      updating = true;
      setPropertyValue("pos_msec_current", cvGetCaptureProperty(data->cvc,CV_CAP_PROP_POS_MSEC));
      setPropertyValue("pos_frames_current", cvGetCaptureProperty(data->cvc,CV_CAP_PROP_POS_FRAMES));
      setPropertyValue("pos_avi_ratio", cvGetCaptureProperty(data->cvc,CV_CAP_PROP_POS_AVI_RATIO));
      updating = false;
      return data->m_buffer;
    }

    OpenCVVideoGrabber::OpenCVVideoGrabber(const std::string &fileName)
    throw (FileNotFoundException) : data(new Data), mutex(Mutex::mutexTypeRecursive), updating(false){
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

      // Configurable
      addProperty("pos_msec_current", "info", "", cvGetCaptureProperty(data->cvc,CV_CAP_PROP_POS_MSEC), 0, "");
      addProperty("pos_msec", "range", "[0," + str(1000*((cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FRAME_COUNT) / cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FPS))) ) + "]:1", cvGetCaptureProperty(data->cvc,CV_CAP_PROP_POS_MSEC), 0, "");
      addProperty("pos_frames_current", "info", "", cvGetCaptureProperty(data->cvc,CV_CAP_PROP_POS_FRAMES), 0, "");
      addProperty("pos_frames", "range", "[0,"+str(cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FRAME_COUNT))+"]:1", cvGetCaptureProperty(data->cvc,CV_CAP_PROP_POS_FRAMES), 0, "");
      addProperty("pos_avi_ratio", "info", "[0,1]:"+str(cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FRAME_COUNT) / cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FPS)), cvGetCaptureProperty(data->cvc,CV_CAP_PROP_POS_AVI_RATIO), 100, "");
      addProperty("size", "info", "", str(data->size), 0, "");
      addProperty("format", "info", "", "RGB", 0, "");
      addProperty("fourcc", "info", "", fourCCStringFromDouble(cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FOURCC)), 0, "");
      addProperty("frame_count", "info", "", str(cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FRAME_COUNT)), 0, "");
      addProperty("use_video_fps", "flag", "", data->use_video_fps, 0, "");
      addProperty("video_fps", "info", "", str(cvGetCaptureProperty(data->cvc,CV_CAP_PROP_FPS)), 0, "");

      Configurable::registerCallback(utils::function(this,&OpenCVVideoGrabber::processPropertyChange));
    }

    OpenCVVideoGrabber::~OpenCVVideoGrabber(){
      cvReleaseCapture(&data->cvc);
      delete data->fpslimiter;
      ICL_DELETE(data->m_buffer);
      delete data;
    }

    // callback for changed configurable properties
    void OpenCVVideoGrabber::processPropertyChange(const utils::Configurable::Property &prop){
      utils::Mutex::Locker l(mutex);
      if(updating) return;
      if(prop.name == "pos_msec"){
        cvSetCaptureProperty(data->cvc,CV_CAP_PROP_POS_MSEC,parse<double>(prop.value));
      }else if(prop.name == "pos_frames"){
        cvSetCaptureProperty(data->cvc,CV_CAP_PROP_POS_FRAMES,parse<double>(prop.value));
      }else if(prop.name == "pos_avi_ratio"){
        cvSetCaptureProperty(data->cvc,CV_CAP_PROP_POS_AVI_RATIO,parse<double>(prop.value));
      }else if(prop.name  == "use_video_fps"){
        data->use_video_fps = parse<bool>(prop.value);
      }
    }

    REGISTER_CONFIGURABLE(OpenCVVideoGrabber, return new OpenCVVideoGrabber(""));

    Grabber* createCVVGrabber(const std::string &param){
      return new OpenCVVideoGrabber(param);
    }

    const std::vector<GrabberDeviceDescription>& getOCVVDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(!rescan) return deviceList;

      deviceList.clear();
      // if filter exists, add grabber with filter
      if(hint.size()) deviceList.push_back(
        GrabberDeviceDescription("cvvideo", hint, "A grabber for opencv-videos.")
        );
      return deviceList;
    }

    REGISTER_GRABBER(cvvideo,utils::function(createCVVGrabber), utils::function(getOCVVDeviceList), "cvvideo:video filename:OpenCV based video file source");

  } // namespace io
}
