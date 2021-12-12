/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/CreateGrabber.cpp                      **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Viktor Richter                    **
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

#include <ICLIO/CreateGrabber.h>
#include <ICLIO/TestImages.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{

    const ImgBase* CreateGrabber::acquireImage(){
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
      Configurable::registerCallback(utils::function(this,&CreateGrabber::processPropertyChange));
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

    REGISTER_GRABBER(create,utils::function(createCreateGrabber), utils::function(getCreateDeviceList),"create:parrot|lena|cameraman|mandril:everywhere available test images source");

  } // namespace io
}
