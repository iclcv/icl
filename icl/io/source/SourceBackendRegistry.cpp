// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/source/SourceBackendRegistry.h>
#include <icl/io/detail/SourceBackend.h>
#include <icl/utils/Exception.h>
#include <icl/utils/Macros.h>

#include <sstream>

namespace icl::io {

  SourceBackendRegistry* SourceBackendRegistry::getInstance(){
    static SourceBackendRegistry inst;
    return &inst;
  }

  void SourceBackendRegistry::registerType(const std::string &id,
                                           CreateFn creator,
                                           DeviceListFn device_list,
                                           const std::string &description)
  {
    m_factories.registerPlugin(id, std::move(creator), description);
    std::scoped_lock l(m_mutex);
    m_deviceLists[id] = std::move(device_list);
  }

  void SourceBackendRegistry::registerBusReset(const std::string &id,
                                               BusResetFn reset_function)
  {
    std::scoped_lock l(m_mutex);
    if(auto it = m_busResets.find(id); it != m_busResets.end())
      throw utils::ICLException("unable to register source-backend bus reset function for "
          + id + ": only one per backend can be registered");
    m_busResets[id] = std::move(reset_function);
  }

  SourceBackend* SourceBackendRegistry::create(const std::string &id, const std::string &param){
    const auto *e = m_factories.get(id);
    if(!e) throw utils::ICLException("unknown source-backend id '"
        + id + "'. can not create unknown source backend");
    return e->payload(param);  // can throw too
  }

  std::vector<std::string> SourceBackendRegistry::getRegistered(){
    return m_factories.keys();
  }

  std::vector<std::pair<std::string,std::string>> SourceBackendRegistry::getInfos(){
    std::vector<std::pair<std::string,std::string>> out;
    for(const auto &e : m_factories.entries()){
      out.emplace_back(e.key, e.description);
    }
    return out;
  }

  const std::vector<DeviceDescription>&
  SourceBackendRegistry::getDeviceList(std::string id, std::string hint, bool rescan){
    std::scoped_lock l(m_mutex);
    if(auto it = m_deviceLists.find(id); it != m_deviceLists.end()){
      return (it->second)(hint, rescan);
    }
    throw utils::ICLException("unknown source-backend id '"
        + id + "'. can not std::list devices of unknown source backend");
  }

  void SourceBackendRegistry::resetBus(const std::string &id, bool verbose){
    std::scoped_lock l(m_mutex);
    if(auto it = m_busResets.find(id); it != m_busResets.end()){
      return (it->second)(verbose);
    }
    std::ostringstream error;
    error << "Can not reset bus of source backend '" << id << "'. No bus-reset function registered.";
    throw utils::ICLException(error.str());
  }

} // namespace icl::io
