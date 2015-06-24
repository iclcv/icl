/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/ConfigurableRemoteServer.h             **
** Module : ICLIO                                                  **
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

#pragma once

#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Configurable.h>

namespace icl{
  namespace io{

    class ConfigurableRemoteServer : public utils::Uncopyable{
      struct Client;
      struct Data;
      Data *m_data;

      public:
      ConfigurableRemoteServer();
      ConfigurableRemoteServer(utils::Configurable *configurable, 
                               const std::string &scope);
      ConfigurableRemoteServer(const std::string &configurableID,
                               const std::string &scope);

      void init(utils::Configurable *configurable, 
                const std::string &scope);

      void init(const std::string &configurableID,
                const std::string &scope);
      
      ~ConfigurableRemoteServer();

      static utils::Configurable *create_client(const std::string &remoteServerScope);
    };
  }
}
