/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/RSBPointCloudSender.h              **
** Module : ICLGeom                                                **
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

#include <ICLGeom/PointCloudOutput.h>
#include <ICLUtils/Uncopyable.h>

namespace icl{
  namespace geom{

    /// RSB-based sendig of point cloud data
    class RSBPointCloudSender : public PointCloudOutput, public utils::Uncopyable{
      struct Data;
      Data *m_data;

      public:

      /// create RSBPointCloudSender sending to given scope via given (comma-sep.) transport List
      /** If the given scope is empty, now initialization is performed and a "null" instance is
          created, that must be initalized afterwards using init */
      RSBPointCloudSender(const std::string &scope="", const std::string &transportList="spread");

      /// Destructor
      ~RSBPointCloudSender();

      /// deferred intialization
      void init(const std::string &scope, const std::string &transportList="spread");

      /// returns initialization state
      bool isNull() const;

      /// actual implementation of the PointCloudOutput interface
      virtual void send(const PointCloudObjectBase &dst);
    };
  } // namespace geom
}

