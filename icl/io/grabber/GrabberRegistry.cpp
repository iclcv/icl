// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/grabber/GrabberRegistry.h>
#include <icl/io/grabber/Grabber.h>
#include <icl/utils/Exception.h>
#include <icl/utils/Macros.h>

#include <sstream>

namespace icl::io {

  GrabberRegistry* GrabberRegistry::getInstance(){
    static GrabberRegistry inst;
    return &inst;
  }

  void GrabberRegistry::registerGrabberType(const std::string &grabberid,
                                            CreateFn creator,
                                            DeviceListFn device_list)
  {
    m_factories.registerPlugin(grabberid, std::move(creator));
    std::scoped_lock l(m_mutex);
    m_deviceLists[grabberid] = std::move(device_list);
  }

  void GrabberRegistry::registerGrabberBusReset(const std::string &grabberid,
                                                BusResetFn reset_function)
  {
    std::scoped_lock l(m_mutex);
    if(auto it = m_busResets.find(grabberid); it != m_busResets.end())
      throw utils::ICLException("unable to register grabber bus reset function for "
          + grabberid + ": only one per grabber can be registered");
    m_busResets[grabberid] = std::move(reset_function);
  }

  void GrabberRegistry::addGrabberDescription(const std::string &grabber_description)
  {
    std::scoped_lock l(m_mutex);
    if(auto it = m_descriptions.find(grabber_description); it != m_descriptions.end())
      throw utils::ICLException("unable to add grabber description: \n"
          + grabber_description + "\n description already exists");
    m_descriptions.insert(grabber_description);
  }

  Grabber* GrabberRegistry::createGrabber(const std::string &grabberid, const std::string &param){
    const auto *e = m_factories.get(grabberid);
    if(!e) throw utils::ICLException("unknown grabber id '"
        + grabberid + "'. can not create unknown grabber");
    return e->payload(param);  // can throw too
  }

  std::vector<std::string> GrabberRegistry::getRegisteredGrabbers(){
    return m_factories.keys();
  }

  std::vector<std::string> GrabberRegistry::getGrabberInfos(){
    std::scoped_lock l(m_mutex);
    return std::vector<std::string>(m_descriptions.begin(), m_descriptions.end());
  }

  const std::vector<GrabberDeviceDescription>&
  GrabberRegistry::getDeviceList(std::string id, std::string hint, bool rescan){
    std::scoped_lock l(m_mutex);
    if(auto it = m_deviceLists.find(id); it != m_deviceLists.end()){
      return (it->second)(hint, rescan);
    }
    throw utils::ICLException("unknown grabber id '"
        + id + "'. can not std::list devices of unknown grabber");
  }

  void GrabberRegistry::resetGrabberBus(const std::string &id, bool verbose){
    std::scoped_lock l(m_mutex);
    if(auto it = m_busResets.find(id); it != m_busResets.end()){
      return (it->second)(verbose);
    }
    std::ostringstream error;
    error << "Can not reset bus of grabber '" << id << "'. No bus-reset function registered.";
    throw utils::ICLException(error.str());
  }

} // namespace icl::io
