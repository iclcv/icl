// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Lukas Twardon, Christof Elbrechter

#pragma once

#include <icl/geom/PointCloudObject.h>
#include <icl/cv3d/Primitive3D.h>
#include <functional>

#ifdef ICL_HAVE_OPENCL
#include "icl/utils/cl/CLProgram.h"
#endif

namespace icl::geom {
      /// Class for filtering point clouds according to a set of primitives and a filter configuration
      class Primitive3DFilter {

      public:

          /// The geometric primitive descriptor now lives standalone in
          /// icl/cv3d/Primitive3D.h (Primitive3D::PrimitiveType / ::Quaternion).
          using Primitive3D = icl::geom::Primitive3D;

          /// a general filter action
          struct FilterAction {

              /// Default constructor
              FilterAction() {}

              /// Constructor
              FilterAction(std::vector<unsigned char> formula) : formula(formula) {}

              virtual ~FilterAction() {}

              /// perform the actual filter action
              /**
                  @param pcObj the point cloud to be filtered
                  @param actionMap the action map specifying which points to filter out (may change in case points are actually removed)
                  @param groupMap may change in case points are actually removed
                  @param depthImage pointer to the depth image or 0 if no depth image
              */
              virtual void performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                         std::vector<unsigned char> &groupMap, core::Img32f *depthImage) = 0;

              /// the filter action formula in disjunctive normal form (three bytes per atomic primitive group)
              /// first byte: 1 if this is the beginning of a new intersection, 0 otherwise
              /// second byte: 1 if the inner part of the primitive group should be filtered out, 0 for the inner part (negation)
              /// third byte: the group bit (internal group id which is set according to the id in filter config)
              std::vector<unsigned char> formula;

          };

          /// remove action
          struct RemoveAction : FilterAction {

              /// Constructor
              RemoveAction(std::vector<unsigned char> formula) : FilterAction(formula) {}

              virtual ~RemoveAction() {}

              void performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                 std::vector<unsigned char> &groupMap, core::Img32f *depthImage);

          };

          /// setpos action
          struct SetposAction : FilterAction {

              /// Constructor
              SetposAction(std::vector<unsigned char> formula, float x, float y, float z) : FilterAction(formula), x(x), y(y), z(z) {}

              virtual ~SetposAction() {}

              void performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                 std::vector<unsigned char> &groupMap, core::Img32f *depthImage);

              /// parameters
              float x;
              float y;
              float z;

          };

          /// color action
          struct ColorAction : FilterAction {

              /// Constructor
              ColorAction(std::vector<unsigned char> formula, float r, float g, float b, float a) : FilterAction(formula), r(r), g(g), b(b), a(a) {}

              virtual ~ColorAction() {}

              void performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                 std::vector<unsigned char> &groupMap, core::Img32f *depthImage);

              /// parameters
              float r;
              float g;
              float b;
              float a;

          };

          /// label action
          struct LabelAction : FilterAction {

              /// Constructor
              LabelAction(std::vector<unsigned char> formula, icl32s value) : FilterAction(formula), value(value) {}

              virtual ~LabelAction() {}

              void performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                 std::vector<unsigned char> &groupMap, core::Img32f *depthImage);

              /// value
              icl32s value;

          };

          /// intensity action
          struct IntensityAction : FilterAction {

              /// Constructor
              IntensityAction(std::vector<unsigned char> formula, float value) : FilterAction(formula), value(value) {}

              virtual ~IntensityAction() {}

              void performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                 std::vector<unsigned char> &groupMap, core::Img32f *depthImage);

              /// value
              float value;

          };

          /// filterdepthimg action
          struct FilterDepthImgAction : FilterAction {

              /// Constructor
              FilterDepthImgAction(std::vector<unsigned char> formula, float value) : FilterAction(formula), value(value) {}

              virtual ~FilterDepthImgAction() {}

              void performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                 std::vector<unsigned char> &groupMap, core::Img32f *depthImage);

              /// value
              float value;

          };

          /// filter config
          struct FilterConfig {

              /// Constructor
              /** Parses the XML config.
                  @param filename the XML config file
              */
              FilterConfig(const std::string &filename);

              /// map primitive group id to internal groupBit
              std::map<std::string, unsigned char, std::less<>> mapGroupIdToBit;

              /// map regular expression (matching the primitive description) to internal groupBit
              std::map<std::string, unsigned char, std::less<>> mapRegexToBit;

              /// map groupBit to padding
              std::map<unsigned char, float> mapGroupBitToPadding;

              /// all filter actions to be performed
              std::vector<std::shared_ptr<FilterAction> > filterActions;

          };

          /// Constructor
          Primitive3DFilter(const FilterConfig &config);

          /// Destructor
          virtual ~Primitive3DFilter() {}

          /// set filter config
          void setConfig(const FilterConfig &config) {
              this->config = config;
          }

          /// applies the filter operation to the given point cloud
          /**
             @param primitives a vector of geometric primitives
             @param pcObj the point cloud to be filtered
             @param depthImage the optional depth image (only needed if there is a filterdepthimg tag in the config)
          */
          void apply(const std::vector<Primitive3D> &primitives, PointCloudObjectBase &pcObj, core::Img32f *depthImage = 0);

      private:

          #ifdef ICL_HAVE_OPENCL
          utils::CLProgram program;
          utils::CLBuffer pcbuffer, groupmapbuffer, actionmapbuffer, formulabuffer;
          utils::CLKernel kernelCreateGroupMap;
          utils::CLKernel kernelCreateActionMap;
          #endif

          static const char *KERNEL_CODE;
          FilterConfig config;

      };

    } // namespace icl::geom