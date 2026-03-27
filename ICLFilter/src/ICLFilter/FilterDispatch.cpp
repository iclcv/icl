/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/FilterDispatch.cpp             **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
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

#include <ICLFilter/FilterDispatch.h>

namespace icl {
  namespace filter {

    // ================================================================
    // Global Registry
    // ================================================================

    namespace detail {
      std::map<std::string, std::vector<RegistryEntry>>& globalRegistry() {
        static std::map<std::string, std::vector<RegistryEntry>> reg;
        return reg;
      }

      int addToRegistry(const std::string& key, RegistryEntry entry) {
        globalRegistry()[key].push_back(std::move(entry));
        return 0;
      }
    }

    // ================================================================
    // Dispatching
    // ================================================================

    Dispatching::~Dispatching() = default;

    void Dispatching::initDispatching(const std::string& className) {
      m_prefix = className + ".";
    }

    std::string Dispatching::qualifiedName(const std::string& shortName) const {
      return m_prefix + shortName;
    }

    std::vector<BackendSelectorBase*> Dispatching::selectors() {
      std::vector<BackendSelectorBase*> result;
      for(auto& sw : m_switches) result.push_back(sw.get());
      return result;
    }

    BackendSelectorBase* Dispatching::selectorByName(const std::string& shortName) {
      auto it = m_selectorByName.find(shortName);
      return it != m_selectorByName.end() ? it->second : nullptr;
    }

    void Dispatching::forceAll(Backend b) {
      for(auto& sw : m_switches) sw->force(b);
    }

    void Dispatching::unforceAll() {
      for(auto& sw : m_switches) sw->unforce();
    }

    std::vector<std::vector<Backend>>
    Dispatching::allBackendCombinations(const core::Image& src) {
      std::vector<std::vector<Backend>> result;
      for(auto& sw : m_switches)
        result.push_back(sw->applicableBackendsFor(src));
      return result;
    }

  } // namespace filter
} // namespace icl
