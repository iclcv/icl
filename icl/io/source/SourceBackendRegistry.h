// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/plugin/PluginRegistry.h>
#include <icl/io/source/DeviceDescription.h>

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace icl::io {

  class SourceBackend;

  /// Process-wide registry of source-backend factories keyed by `id` (e.g.
  /// "dc", "v4l", "ws", "file", "create", …).
  /** Façade over `utils::PluginRegistry<std::string, CreateFn>` for the
      core factory map, plus three side maps for the orthogonal per-backend
      concerns that don't fit a generic registry's Entry shape:
      device-list listing functions, bus-reset functions, and verbatim
      one-line description strings (for `icl-pipe -i help` etc.).

      Backends self-register at static-init time via the REGISTER_SOURCE_BACKEND
      and REGISTER_SOURCE_BACKEND_BUS_RESET_FUNCTION macros below — see the
      __attribute__((constructor, used)) note in the macro bodies for why
      they're spelled this way (macOS dyld dead-stripping). */
  class ICLIO_API SourceBackendRegistry {
  public:
    SourceBackendRegistry(const SourceBackendRegistry&) = delete;
    SourceBackendRegistry &operator=(const SourceBackendRegistry&) = delete;

    using CreateFn     = std::function<SourceBackend*(const std::string&)>;
    using DeviceListFn = std::function<const std::vector<DeviceDescription>&(std::string, bool)>;
    using BusResetFn   = std::function<void(bool)>;

    /// Underlying primitive for the factory map.
    using Registry = utils::PluginRegistry<std::string, CreateFn>;

    static SourceBackendRegistry* getInstance();

    void registerType(const std::string &id,
                      CreateFn creator,
                      DeviceListFn device_list,
                      const std::string &description);

    void registerBusReset(const std::string &id,
                          BusResetFn reset_function);

    SourceBackend* create(const std::string &id, const std::string &param);

    std::vector<std::string> getRegistered();

    /// (id, "paramHint~explanation") pairs for the `-i list` affordance.
    /** Mirrors the sink side: the description travels in the registry Entry,
        the id is the registry key. */
    std::vector<std::pair<std::string,std::string>> getInfos();

    const std::vector<DeviceDescription>&
    getDeviceList(std::string id, std::string hint="", bool rescan=true);

    void resetBus(const std::string &id, bool verbose);

  private:
    SourceBackendRegistry() : m_factories(utils::OnDuplicate::Throw) {}

    Registry m_factories;                                                     //!< id → {CreateFn, description}
    std::recursive_mutex m_mutex;                                             //!< guards the side maps below
    std::map<std::string, DeviceListFn, std::less<>> m_deviceLists;           //!< id → listing function
    std::map<std::string, BusResetFn,   std::less<>> m_busResets;             //!< id → bus reset function
  };

} // namespace icl::io

/// Self-register a source backend at static-init time.
/** Use exactly once per backend, typically at the bottom of its .cpp:
    The DESCRIPTION is a `"paramHint~explanation"` pair (the `~` separates
    the two columns of the `-i list` table), mirroring REGISTER_SINK_BACKEND.
    \code
      REGISTER_SOURCE_BACKEND(dc, createDCSource, getDCDeviceList,
                       "1394 device id~libdc1394_2 FireWire camera");
    \endcode
    `__attribute__((constructor, used))` is required for macOS — dyld
    will silently dead-strip an anonymous-namespace static-storage ctor
    even when its `__GLOBAL__sub_I_*` symbol survives in `nm`. */
#define REGISTER_SOURCE_BACKEND(NAME,CREATE_FUNC,DEVICE_LIST_FUNC,DESCRIPTION)        \
  extern "C" __attribute__((constructor, used)) void                           \
  iclRegisterSourceBackend_##NAME() {                                          \
    ::icl::io::SourceBackendRegistry::getInstance()                                  \
        ->registerType(#NAME, CREATE_FUNC, DEVICE_LIST_FUNC, DESCRIPTION);     \
  }

#define REGISTER_SOURCE_BACKEND_BUS_RESET_FUNCTION(NAME,BUS_RESET_FUNC)               \
  extern "C" __attribute__((constructor, used)) void                           \
  iclRegisterSourceBackendBusReset_##NAME() {                                  \
    ::icl::io::SourceBackendRegistry::getInstance()                                  \
        ->registerBusReset(#NAME, BUS_RESET_FUNC);                             \
  }
