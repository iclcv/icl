/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/GenericPointCloudOutput.h          **
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
#include <ICLUtils/ProgArg.h>

namespace icl{
  namespace geom{

    /// Generic interface for PointCloud sources
    class ICLGeom_API GenericPointCloudOutput : public PointCloudOutput, public utils::Uncopyable{
      struct Data;
      Data *m_data;

      public:

      /// Empty constructor (creates a null instance)
      GenericPointCloudOutput();

      /// Constructor with initialization
      /** Possible plugins:
          * <b>rsb</b> rsb-transport-list: rsb-scope-list
      */
      GenericPointCloudOutput(const std::string &sourceType, const std::string &srcDescription);

      /// direct initialization from program argument
      /** Prog-arg is assumed to have 2 sub-args */
      GenericPointCloudOutput(const utils::ProgArg &pa);

      /// destructor
      ~GenericPointCloudOutput();

      /// deferred intialization
      void init(const std::string &sourceType, const std::string &srcDescription);

      /// deferred initialization from ProgArg (most common perhaps)
      /** Prog-arg is assumed to have 2 sub-args */
      void init(const utils::ProgArg &pa);

      /// not initialized yet?
      bool isNull() const;

      /// fills the given point cloud with grabbed information
      virtual void send(const PointCloudObjectBase &src);

    };
  } // namespace geom
}

