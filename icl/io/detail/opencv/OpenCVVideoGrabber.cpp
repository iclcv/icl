// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Viktor Richter, Christof Elbrechter

#include <icl/io/detail/opencv/OpenCVVideoGrabber.h>
#include <icl/utils/prop/Constraints.h>
#include <opencv2/videoio/videoio_c.h>

#include <memory>
#include <mutex>

using namespace icl::utils;
using namespace icl::core;

namespace icl::io {
  struct OpenCVVideoGrabber::Data{
      /// Device struct
      std::unique_ptr<cv::VideoCapture> cvc;
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
    int fourInt = static_cast<int>(value);
    std::string ret(reinterpret_cast<char*>(&fourInt), 4);
    return ret;
  }

  const ImgBase *OpenCVVideoGrabber::acquireDisplay(){
    std::scoped_lock<std::recursive_mutex> l(mutex);
    ICLASSERT_RETURN_VAL( !(data->cvc==nullptr), 0);
    cv::Mat frame;
    data->cvc->read(frame);
    core::mat_to_img(&frame, &data->m_buffer);
    if(data->m_buffer->getChannels() == 3) data->m_buffer->setFormat(formatRGB);
    if (data->use_video_fps){
      data->fpslimiter->wait();
    }

    updating = true;
    setPropertyValue("pos_msec_current", data->cvc->get(cv::CAP_PROP_POS_MSEC));
    setPropertyValue("pos_frames_current", data->cvc->get(cv::CAP_PROP_POS_FRAMES));
    setPropertyValue("pos_avi_ratio", data->cvc->get(cv::CAP_PROP_POS_AVI_RATIO));
    updating = false;
    return data->m_buffer;
  }

  OpenCVVideoGrabber::OpenCVVideoGrabber(const std::string &fileName) : data(new Data), mutex(), updating(false){
    data->m_buffer = 0;
    data->use_video_fps = true;
    data->filename = fileName;

    if(!File(fileName).exists()){
      throw FileNotFoundException(fileName);
    }

    data->cvc.reset(new cv::VideoCapture(fileName.c_str()));
    int fps = data->cvc->get(cv::CAP_PROP_FPS);
    data->fpslimiter = new FPSLimiter(fps);

    data->size.width = data->cvc->get(cv::CAP_PROP_FRAME_WIDTH);
    data->size.height =  data->cvc->get(cv::CAP_PROP_FRAME_HEIGHT);

    // Configurable
    addProperty("pos_msec_current", prop::Info{}, str(data->cvc->get(cv::CAP_PROP_POS_MSEC)), 0, "");
    addProperty("pos_msec",
                prop::Range{.min=0.f,
                                   .max=(float)(1000*(data->cvc->get(cv::CAP_PROP_FRAME_COUNT) / data->cvc->get(cv::CAP_PROP_FPS))),
                                   .step=1.f},
                (float)data->cvc->get(cv::CAP_PROP_POS_MSEC), 0, "");
    addProperty("pos_frames_current", prop::Info{}, str(data->cvc->get(cv::CAP_PROP_POS_FRAMES)), 0, "");
    addProperty("pos_frames",
                prop::Range{.min=0.f,
                                   .max=(float)data->cvc->get(cv::CAP_PROP_FRAME_COUNT),
                                   .step=1.f},
                (float)data->cvc->get(cv::CAP_PROP_POS_FRAMES), 0, "");
    // NB: legacy call had a non-empty "info" string ("[0,1]:<float>") —
    // that extra info grammar isn't representable with prop::Info{} (which
    // carries no info payload).  The info text was not meaningfully read
    // by qt::Prop for "info" type anyway, so drop it.
    addProperty("pos_avi_ratio", prop::Info{},
                str(data->cvc->get(cv::CAP_PROP_POS_AVI_RATIO)), 100, "");
    addProperty("size", prop::Info{}, str(data->size), 0, "");
    addProperty("format", prop::Info{}, "RGB", 0, "");
    addProperty("fourcc", prop::Info{}, fourCCStringFromDouble(data->cvc->get(cv::CAP_PROP_FOURCC)), 0, "");
    addProperty("frame_count", prop::Info{}, str(data->cvc->get(cv::CAP_PROP_FRAME_COUNT)), 0, "");
    addProperty("use_video_fps", prop::Flag{}, data->use_video_fps, 0, "");
    addProperty("video_fps", prop::Info{}, str(data->cvc->get(cv::CAP_PROP_FPS)), 0, "");

    registerCallback([this](const utils::Configurable::Property &p){ processPropertyChange(p); });
  }

  OpenCVVideoGrabber::~OpenCVVideoGrabber(){
    delete data->fpslimiter;
    ICL_DELETE(data->m_buffer);
    delete data;
  }

  // callback for changed configurable properties
  void OpenCVVideoGrabber::processPropertyChange(const utils::Configurable::Property &prop){
    std::scoped_lock<std::recursive_mutex> l(mutex);
    if(updating) return;
    if(prop.name == "pos_msec"){
      data->cvc->set(cv::CAP_PROP_POS_MSEC,parse<double>(prop.value));
    }else if(prop.name == "pos_frames"){
      data->cvc->set(cv::CAP_PROP_POS_FRAMES,parse<double>(prop.value));
    }else if(prop.name == "pos_avi_ratio"){
      data->cvc->set(cv::CAP_PROP_POS_AVI_RATIO,parse<double>(prop.value));
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

  REGISTER_GRABBER(cvvideo,createCVVGrabber, getOCVVDeviceList, "cvvideo:video filename:OpenCV based video file source");

  } // namespace icl::io