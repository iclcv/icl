// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLGeom/PointCloudOutput.h>
#include <ICLUtils/ProgArg.h>

namespace icl{
  namespace geom{

    /// Generic interface for PointCloud sources
    class ICLGeom_API GenericPointCloudOutput : public PointCloudOutput{
      struct Data;
      Data *m_data;

      public:
      GenericPointCloudOutput(const GenericPointCloudOutput&) = delete;
      GenericPointCloudOutput& operator=(const GenericPointCloudOutput&) = delete;


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
