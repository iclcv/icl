/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PointCloudSerializer.h             **
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

#include <ICLGeom/PointCloudObjectBase.h>
#include <ICLCore/DataSegmentBase.h>

#include <map>

namespace icl{

  namespace geom{

    /// Utility class used as intermediate layer for point cloud serialization
    struct ICLGeom_API PointCloudSerializer{

      /// Mandatory information provided by each point cloud
      struct MandatoryInfo{
        int width;
        int height;
        bool organized;
        icl64s timestamp;
        bool padding[15];
      };

      struct SerializationDevice{
        virtual void initializeSerialization(const MandatoryInfo &info) = 0;
        virtual icl8u *targetFor(const std::string &featureName, int bytes) = 0;
      };
      struct DeserializationDevice{
        virtual MandatoryInfo getDeserializationInfo() = 0;
        virtual std::vector<std::string> getFeatures() = 0;
        virtual const icl8u *sourceFor(const std::string &featureName, int &bytes) = 0;
      };

      /// Utilitty class used for
      struct DefaultSerializationDevice : public SerializationDevice{
        virtual void initializeSerialization(const MandatoryInfo &info);
        virtual icl8u *targetFor(const std::string &featureName, int bytes);

        /// saved info
        MandatoryInfo info;

        /// interall collected data
        std::map<std::string, std::vector<icl8u> > data;

        /// returns number of bytes when all data is concatenated
        int getFullSerializationSize() const;

        /// data needs to be at least getFullSerializationSize bytes long
        void copyData(icl8u *dst);

        void clear();
      };

      struct DefaultDeserializationDevice : public DeserializationDevice {
        MandatoryInfo info;
        std::map<std::string, std::vector<icl8u> > data;
        std::vector<std::string> features;

        DefaultDeserializationDevice(const icl8u *data);

        virtual MandatoryInfo getDeserializationInfo() { return info; }
        virtual std::vector<std::string> getFeatures() { return features; }

        virtual const icl8u *sourceFor(const std::string &featureName, int &bytes);
      };

      static void serialize(const PointCloudObjectBase &o, SerializationDevice &d);

      static void deserialize(PointCloudObjectBase &o, DeserializationDevice &d);
    };

  }
}
