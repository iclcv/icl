// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#include <ICLIO/CreateGrabber.h>
#include <ICLIO/TestImages.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{

    const ImgBase* CreateGrabber::acquireDisplay(){
      if(m_updateTimeStamp){
        m_image -> setTime();
      }
      return m_image;
    }

    CreateGrabber::CreateGrabber(const std::string &what){
      m_updateTimeStamp = true;
      m_image = TestImages::create(what);
      if(!m_image) throw ICLException("unable to create a 'CreateGrabber' from given description '"+what+"'");
      addProperty("format", "info", "", "RGB", 0, "");
      addProperty("size", "info", "", "512x512", 0, "");
      addProperty("update timestamp", "flag", "", m_updateTimeStamp, 0, "Whether the timestamp of the image should be set everytime an the image is grabbed.");
      Configurable::registerCallback([this](const utils::Configurable::Property &p){ processPropertyChange(p); });
    }

    CreateGrabber::~CreateGrabber(){
      ICL_DELETE(m_image);
    }

    void CreateGrabber::processPropertyChange(const utils::Configurable::Property &prop){
      if(prop.name == "update timestamp") {
        m_updateTimeStamp = parse<bool>(prop.value);
      }
    }

    REGISTER_CONFIGURABLE(CreateGrabber, return new CreateGrabber("parrot"));

    Grabber* createCreateGrabber(const std::string &param){
      return new CreateGrabber(param);
    }

    const std::vector<GrabberDeviceDescription>& getCreateDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(deviceList.empty()){
          deviceList.push_back(GrabberDeviceDescription("create","parrot", "everywhere available test images source parrot"));
          deviceList.push_back(GrabberDeviceDescription("create","lena", "everywhere available test images source lena"));
          deviceList.push_back(GrabberDeviceDescription("create","flowers", "everywhere available test images source flowers"));
          deviceList.push_back(GrabberDeviceDescription("create","mandril", "everywhere available test images source mandril"));
          deviceList.push_back(GrabberDeviceDescription("create","cameraman", "everywhere available test images source cameraman"));
          deviceList.push_back(GrabberDeviceDescription("create","women", "everywhere available test images source women"));
          deviceList.push_back(GrabberDeviceDescription("create","tree", "everywhere available test images source tree"));
          deviceList.push_back(GrabberDeviceDescription("create","house", "everywhere available test images source house"));
      }
      return deviceList;
    }

    REGISTER_GRABBER(create,createCreateGrabber, getCreateDeviceList,"create:parrot|lena|cameraman|mandril:everywhere available test images source");

  } // namespace io
}
