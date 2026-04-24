// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Lukas Twardon, Christof Elbrechter

#include <icl/geom/Primitive3DFilter.h>
#include <icl/utils/Xml.h>
#include <icl/utils/StringUtils.h>

namespace icl::geom {
      const char *Primitive3DFilter::KERNEL_CODE = (
      "float4 conj(float4 q) {                                                                                                                                                      \n"
      "    float4 res;                                                                                                                                                              \n"
      "    res.x = -q.x;                                                                                                                                                            \n"
      "    res.y = -q.y;                                                                                                                                                            \n"
      "    res.z = -q.z;                                                                                                                                                            \n"
      "    res.w = q.w;                                                                                                                                                             \n"
      "    return res;                                                                                                                                                              \n"
      "}                                                                                                                                                                            \n"
      "                                                                                                                                                                             \n"
      "float4 multiply(float4 q1, float4 q2) {                                                                                                                                      \n"
      "    float4 res;                                                                                                                                                              \n"
      "    res.x = q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y;                                                                                                                   \n"
      "    res.y = q1.w*q2.y - q1.x*q2.z + q1.y*q2.w + q1.z*q2.x;                                                                                                                   \n"
      "    res.z = q1.w*q2.z + q1.x*q2.y - q1.y*q2.x + q1.z*q2.w;                                                                                                                   \n"
      "    res.w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z;                                                                                                                   \n"
      "    return res;                                                                                                                                                              \n"
      "}                                                                                                                                                                            \n"
      "                                                                                                                                                                             \n"
      "float4 rotateVector(float4 v, float4 q) {                                                                                                                                    \n"
      "    float4 res;                                                                                                                                                              \n"
      "    v.w = 0;                                                                                                                                                                 \n"
      "    res = multiply(v, conj(q));                                                                                                                                              \n"
      "    res = multiply(q, res);                                                                                                                                                  \n"
      "    return res;                                                                                                                                                              \n"
      "}                                                                                                                                                                            \n"
      "                                                                                                                                                                             \n"
      "__kernel void createGroupMap(const __global float *pcbuffer,                                                                                                                 \n"
      "                             __global unsigned char *groupmapbuffer,                                                                                                         \n"
      "                             const float a, const float b, const float c,                                                                                                    \n"
      "                             const float4 orientation,                                                                                                                       \n"
      "                             const float4 position,                                                                                                                          \n"
      "                             const int type, const int groupBit) {                                                                                                           \n"
      "    const int i = get_global_id(0);                                                                                                                                          \n"
      "    float4 relpoint;                                                                                                                                                         \n"
      "    relpoint.x = pcbuffer[4*i] - position.x;                                                                                                                                 \n"
      "    relpoint.y = pcbuffer[4*i+1] - position.y;                                                                                                                               \n"
      "    relpoint.z = pcbuffer[4*i+2] - position.z;                                                                                                                               \n"
      "    relpoint = rotateVector(relpoint, conj(orientation));                                                                                                                    \n"
      "    if(a == 0 || b == 0 || c == 0)                                                                                                                                           \n"
      "        return;                                                                                                                                                              \n"
      "    if(type == 0)                                                                                                                                                            \n"
      "        groupmapbuffer[i] |= ( (relpoint.x <= a) && (relpoint.x >= -a) && (relpoint.y <= b) && (relpoint.y >= -b) &&(relpoint.z <= c) && (relpoint.z >= -c) ) << groupBit;   \n"
      "    else if(type == 1)                                                                                                                                                       \n"
      "        groupmapbuffer[i] |= ( (relpoint.x*relpoint.x)/(a*a) + (relpoint.y*relpoint.y)/(b*b) + (relpoint.z*relpoint.z)/(c*c) <= 1 ) << groupBit;                             \n"
      "    else if(type == 2)                                                                                                                                                       \n"
      "        groupmapbuffer[i] |= ( ((relpoint.x*relpoint.x)/(a*a) + (relpoint.y*relpoint.y)/(b*b) <= 1) && (relpoint.z <= c) && (relpoint.z >= -c) ) << groupBit;                \n"
      "}                                                                                                                                                                            \n"
      "                                                                                                                                                                             \n"
      "__kernel void createActionMap(const __global unsigned char *groupmapbuffer,                                                                                                  \n"
      "                              __global unsigned char *actionmapbuffer,                                                                                                       \n"
      "                              const __global unsigned char *formulabuffer,                                                                                                   \n"
      "                              const unsigned int formulaSize) {                                                                                                              \n"
      "    const int i = get_global_id(0);                                                                                                                                          \n"
      "    bool intersectionValue = false;                                                                                                                                          \n"
      "    for(unsigned int g = 0; g < formulaSize/3; g++) {                                                                                                                        \n"
      "        if(formulabuffer[3*g]) {                                                                                                                                             \n"
      "            actionmapbuffer[i] = actionmapbuffer[i] || intersectionValue;                                                                                                    \n"
      "            intersectionValue = true;                                                                                                                                        \n"
      "        }                                                                                                                                                                    \n"
      "        if(formulabuffer[3*g+1])                                                                                                                                             \n"
      "            intersectionValue = intersectionValue && (groupmapbuffer[i] & (1 << formulabuffer[3*g+2]));                                                                      \n"
      "        else                                                                                                                                                                 \n"
      "            intersectionValue = intersectionValue && !(groupmapbuffer[i] & (1 << formulabuffer[3*g+2]));                                                                     \n"
      "    }                                                                                                                                                                        \n"
      "    actionmapbuffer[i] = actionmapbuffer[i] || intersectionValue;                                                                                                            \n"
      "}                                                                                                                                                                            \n"
      );

      Primitive3DFilter::Primitive3DFilter(const FilterConfig &config) : config(config) {

          #ifdef ICL_HAVE_OPENCL
          program = utils::CLProgram("gpu", KERNEL_CODE);
          kernelCreateGroupMap = program.createKernel("createGroupMap");
          kernelCreateActionMap = program.createKernel("createActionMap");
          #endif

      }

      void Primitive3DFilter::apply(const std::vector<Primitive3D> &primitives, PointCloudObjectBase &pcObj, core::Img32f *depthImage) {

          pcObj.lock();

          // dimension of the point cloud
          int dim = pcObj.getDim();

          // the group map (8 bits per point with 1 = point is inside the primitive group)
          std::vector<unsigned char> groupMap(dim, 0);

          // the xyz(h) data segment
          core::DataSegment<float,4> xyzh;
          core::DataSegment<float,3> xyz;
          if(pcObj.supports(PointCloudObjectBase::XYZH)) {
              xyzh = pcObj.selectXYZH();
          } else if(pcObj.supports(PointCloudObjectBase::XYZ)) {
              xyz = pcObj.selectXYZ();
          } else {
              DEBUG_LOG("Filter error: Point cloud has no XYZ or XYZH data.");
              return;
          }

          #ifdef ICL_HAVE_OPENCL
          pcbuffer = program.createBuffer("r", dim*4*sizeof(float));
          if(pcObj.supports(PointCloudObjectBase::XYZH)) {
              if(xyzh.isPacked()) {
                  pcbuffer.write(xyzh.getDataPointer(), dim*4*sizeof(float));
              } else {
                  std::vector<math::Vec4> xyzhPacked;
                  for(int i = 0; i < dim; ++i) {
                      xyzhPacked.push_back(xyzh[i]);
                  }
                  pcbuffer.write(xyzhPacked.data(), dim*4*sizeof(float));
              }
          } else if(pcObj.supports(PointCloudObjectBase::XYZ)) {
              if(xyz.isPacked()) {
                  pcbuffer.write(xyz.getDataPointer(), dim*4*sizeof(float));
              } else {
                  std::vector<math::Vec4> xyzPacked;
                  for(int i = 0; i < dim; ++i) {
                      xyzPacked.push_back(math::Vec4(xyz[i][0], xyz[i][1], xyz[i][2], 1));
                  }
                  pcbuffer.write(xyzPacked.data(), dim*4*sizeof(float));
              }
          }
          groupmapbuffer = program.createBuffer("rw", dim);
          groupmapbuffer.write(groupMap.data(), dim);
          #endif

          // for all primitives
          for(std::vector<Primitive3D>::const_iterator it = primitives.begin(); it != primitives.end(); ++it) {

              // match regular expression with description to find primitive group
              int groupBit = -1;
              for(std::map<std::string, unsigned char, std::less<>>::const_iterator mapit = config.mapRegexToBit.begin(); mapit != config.mapRegexToBit.end(); ++mapit) {
                  std::string regex = mapit->first;
                  if(utils::match(it->description, regex)) {
                      if(groupBit == -1) {
                          groupBit = config.mapRegexToBit[regex];
                      } else {
                          DEBUG_LOG("Filter error: Primitive groups are not disjoint.");
                          return;
                      }
                  }
              }

              // padding
              float padding = config.mapGroupBitToPadding[groupBit];

              // scale
              float a = it->scale[0]/2.0 + padding;
              float b = it->scale[1]/2.0 + padding;
              float c = it->scale[2]/2.0 + padding;
              if(a <= 0 || b <= 0 || c <= 0)
                  continue;

              #ifdef ICL_HAVE_OPENCL

              int opencltype = 0;
              if(it->type == CUBE)
                  opencltype = 0;
              else if(it->type == SPHERE)
                  opencltype = 1;
              else if(it->type == CYLINDER)
                  opencltype = 2;
              kernelCreateGroupMap.setArgs(pcbuffer, groupmapbuffer, a, b, c, math::Vec4(it->orientation.v[0], it->orientation.v[1], it->orientation.v[2], it->orientation.w),
                             it->position, opencltype, groupBit);
              kernelCreateGroupMap.apply(dim);

              groupmapbuffer.read(groupMap.data(), dim);

              #else

              // for all points
              for(int i = 0; i < pcObj.getDim(); ++i) {
                  // translation
                  math::Vec3 relpoint(xyzh[i][0] - it->position[0], xyzh[i][1] - it->position[1], xyzh[i][2] - it->position[2]);
                  // rotation
                  relpoint = it->orientation.conj().rotateVector(relpoint);
                  // filtering
                  if(it->type == CUBE)
                      groupMap[i] |= ( (relpoint[0] <= a) && (relpoint[0] >= -a) && (relpoint[1] <= b) && (relpoint[1] >= -b) &&(relpoint[2] <= c) && (relpoint[2] >= -c) ) << groupBit;
                  else if(it->type == SPHERE)
                      groupMap[i] |= ( (relpoint[0]*relpoint[0])/(a*a) + (relpoint[1]*relpoint[1])/(b*b) + (relpoint[2]*relpoint[2])/(c*c) <= 1 ) << groupBit;
                  else if(it->type == CYLINDER)
                      groupMap[i] |= ( ((relpoint[0]*relpoint[0])/(a*a) + (relpoint[1]*relpoint[1])/(b*b) <= 1) && (relpoint[2] <= c) && (relpoint[2] >= -c) ) << groupBit;
              }

              #endif

          }

          // for all filter actions
          for(std::vector<std::shared_ptr<FilterAction> >::iterator actionit = config.filterActions.begin(); actionit != config.filterActions.end(); ++actionit) {
              // the action map (1 byte per point with 1 = point should be filtered)
              std::vector<unsigned char> actionMap(dim, 0);

              #ifdef ICL_HAVE_OPENCL

              groupmapbuffer = program.createBuffer("rw", dim);
              actionmapbuffer = program.createBuffer("rw", dim);
              formulabuffer = program.createBuffer("r", (*actionit)->formula.size());
              groupmapbuffer.write(groupMap.data(), dim);
              actionmapbuffer.write(actionMap.data(), dim);
              formulabuffer.write((*actionit)->formula.data(), (*actionit)->formula.size());

              kernelCreateActionMap.setArgs(groupmapbuffer, actionmapbuffer, formulabuffer, static_cast<unsigned int>(((*actionit)->formula.size())));
              kernelCreateActionMap.apply(dim);

              actionmapbuffer.read(actionMap.data(), dim);

              #else

              // for all points
              for(int i = 0; i < pcObj.getDim(); ++i) {
                  bool intersectionValue = false;
                  // for all atomic primitive groups in the filter formula
                  for(unsigned int g = 0; g < (*actionit)->formula.size()/3; g++) {
                      // oring
                      if((*actionit)->formula[3*g]) {
                          actionMap[i] = actionMap[i] || intersectionValue;
                          intersectionValue = true;
                      }
                      // possibly negating and anding
                      if((*actionit)->formula[3*g+1])
                          intersectionValue = intersectionValue && (groupMap[i] & (1 << (*actionit)->formula[3*g+2]));
                      else
                          intersectionValue = intersectionValue && !(groupMap[i] & (1 << (*actionit)->formula[3*g+2]));
                  }
                  // the last oring
                  actionMap[i] = actionMap[i] || intersectionValue;
              }

              #endif

              // perform actual filter action
              (*actionit)->performAction(pcObj, actionMap, groupMap, depthImage);

              // update point cloud dim
              dim = pcObj.getDim();

          }

          pcObj.unlock();

      }

      Primitive3DFilter::FilterConfig::FilterConfig(const std::string &filename) {

          namespace xml = ::icl::utils::xml;

          // parse XML document
          xml::Document doc;
          try {
              doc = xml::Document::parseFile(filename);
          } catch(const xml::ParseError &) {
              throw utils::ParseException("Could not parse XML config.");
          } catch(const utils::ICLException &) {
              throw utils::ParseException("Could not parse XML config.");
          }
          xml::Element rootNode = doc.root();
          if(rootNode.empty() || rootNode.name() != "pointcloudfilter")
              throw utils::ParseException("No pointcloudfilter root node.");

          // parse primitivegroups
          unsigned char currentBit = 0;
          for(xml::Element primitivegroupNode = rootNode.child("primitivegroup"); primitivegroupNode; primitivegroupNode = primitivegroupNode.nextSibling("primitivegroup")) {
              if(currentBit >= 8) throw utils::ParseException("Too many (more than 8) primitive groups.");
              xml::Attribute idAttr = primitivegroupNode.attribute("id");
              if(idAttr.empty()) throw utils::ParseException("Primitivegroup has no id attribute.");
              std::string id(idAttr.value());
              xml::Attribute regexAttr = primitivegroupNode.attribute("regex");
              if(regexAttr.empty()) throw utils::ParseException("Primitivegroup has no regex attribute.");
              std::string regex(regexAttr.value());
              xml::Attribute paddingAttr = primitivegroupNode.attribute("padding");
              float padding = 0.0;
              if(!paddingAttr.empty()) {
                  padding = paddingAttr.asFloat();
              }
              mapGroupIdToBit[id] = currentBit;
              mapRegexToBit[regex] = currentBit;
              mapGroupBitToPadding[currentBit] = padding;
              currentBit++;
          }

          // parse unions — single XPath matches the six action tags directly
          xml::NodeSet actionNodes = doc.root().selectAll(
              "/pointcloudfilter/*[self::remove or self::setpos or self::color or self::label or self::intensity or self::filterdepthimg]");
          for(xml::Element actionNode : actionNodes) {

              std::vector<unsigned char> formula;

              // parse groups (one element intersections)
              for(xml::Element groupNode = actionNode.child("group"); groupNode; groupNode = groupNode.nextSibling("group")) {
                  unsigned char newIntersection = 1;
                  unsigned char inner;
                  unsigned char groupBit;
                  xml::Attribute partAttr = groupNode.attribute("part");
                  if(partAttr.empty()) throw utils::ParseException("Group has no part attribute.");
                  std::string part(partAttr.value());
                  xml::Attribute idAttr = groupNode.attribute("id");
                  if(idAttr.empty()) throw utils::ParseException("Group has no id attribute.");
                  std::string id(idAttr.value());
                  if(part == "inner")
                      inner = 1;
                  else if(part == "outer")
                      inner = 0;
                  else
                      throw utils::ParseException("Part attribute must be inner or outer.");
                  try{
                      groupBit = mapGroupIdToBit.at(id);
                  } catch(const std::out_of_range&) {
                      throw utils::ParseException("No primitive group with id " + id + " has been defined.");
                  }
                  formula.push_back(newIntersection);
                  formula.push_back(inner);
                  formula.push_back(groupBit);
              }

              // parse intersections of groups
              for(xml::Element intersectionNode = actionNode.child("intersection"); intersectionNode; intersectionNode = intersectionNode.nextSibling("intersection")) {
                  unsigned char newIntersection = 1;
                  for(xml::Element groupNode = intersectionNode.child("group"); groupNode; groupNode = groupNode.nextSibling("group")) {
                      unsigned char inner;
                      unsigned char groupBit;
                      xml::Attribute partAttr = groupNode.attribute("part");
                      if(partAttr.empty()) throw utils::ParseException("Group has no part attribute.");
                      std::string part(partAttr.value());
                      xml::Attribute idAttr = groupNode.attribute("id");
                      if(idAttr.empty()) throw utils::ParseException("Group has no id attribute.");
                      std::string id(idAttr.value());
                      if(part == "inner")
                          inner = true;
                      else if(part == "outer")
                          inner = false;
                      else
                          throw utils::ParseException("Part attribute must be inner or outer.");
                      try{
                          groupBit = mapGroupIdToBit.at(id);
                      } catch(const std::out_of_range&) {
                          throw utils::ParseException("No primitive group with id " + id + " has been defined.");
                      }
                      formula.push_back(newIntersection);
                      formula.push_back(inner);
                      formula.push_back(groupBit);
                      newIntersection = 0;
                  }
              }

              // parse remove/setpos/color/label/intensity parameters
              if(formula.size() > 0) {
                  std::string_view actionName = actionNode.name();
                  if(actionName == "remove") {
                      std::shared_ptr<RemoveAction> removeActionPtr(new RemoveAction(formula));
                      filterActions.push_back(removeActionPtr);
                  } else if(actionName == "setpos") {
                      xml::Attribute xAttr = actionNode.attribute("x");
                      if(xAttr.empty()) throw utils::ParseException("Setpos has no x attribute.");
                      xml::Attribute yAttr = actionNode.attribute("y");
                      if(yAttr.empty()) throw utils::ParseException("Setpos has no y attribute.");
                      xml::Attribute zAttr = actionNode.attribute("z");
                      if(zAttr.empty()) throw utils::ParseException("Setpos has no z attribute.");
                      std::shared_ptr<SetposAction> setposActionPtr(new SetposAction(formula, xAttr.asFloat(), yAttr.asFloat(), zAttr.asFloat()));
                      filterActions.push_back(setposActionPtr);
                  } else if(actionName == "color") {
                      xml::Attribute rAttr = actionNode.attribute("r");
                      if(rAttr.empty()) throw utils::ParseException("Color has no r attribute.");
                      xml::Attribute gAttr = actionNode.attribute("g");
                      if(gAttr.empty()) throw utils::ParseException("Color has no g attribute.");
                      xml::Attribute bAttr = actionNode.attribute("b");
                      if(bAttr.empty()) throw utils::ParseException("Color has no b attribute.");
                      xml::Attribute aAttr = actionNode.attribute("a");
                      if(aAttr.empty()) throw utils::ParseException("Color has no a attribute.");
                      std::shared_ptr<ColorAction> colorActionPtr(new ColorAction(formula, rAttr.asFloat(), gAttr.asFloat(), bAttr.asFloat(), aAttr.asFloat()));
                      filterActions.push_back(colorActionPtr);
                  } else if(actionName == "label") {
                      xml::Attribute valueAttr = actionNode.attribute("value");
                      if(valueAttr.empty()) throw utils::ParseException("Label has no value attribute.");
                      std::shared_ptr<LabelAction> labelActionPtr(new LabelAction(formula, valueAttr.asInt()));
                      filterActions.push_back(labelActionPtr);
                  } else if(actionName == "intensity") {
                      xml::Attribute valueAttr = actionNode.attribute("value");
                      if(valueAttr.empty()) throw utils::ParseException("Intensity has no value attribute.");
                      std::shared_ptr<IntensityAction> intensityActionPtr(new IntensityAction(formula, valueAttr.asFloat()));
                      filterActions.push_back(intensityActionPtr);
                  } else if(actionName == "filterdepthimg") {
                      xml::Attribute valueAttr = actionNode.attribute("value");
                      if(valueAttr.empty()) throw utils::ParseException("Filterdepthimg has no value attribute.");
                      std::shared_ptr<FilterDepthImgAction> filterDepthImgActionPtr(new FilterDepthImgAction(formula, valueAttr.asFloat()));
                      filterActions.push_back(filterDepthImgActionPtr);
                  }
              }

          }

      }

      void Primitive3DFilter::RemoveAction::performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                                          std::vector<unsigned char> &groupMap, core::Img32f *depthImage) {

          int new_i = 0;

          std::vector<unsigned char> oldActionMap(actionMap);

          bool removedColor = false;
          bool removedPos = false;
          bool changedMaps = false;

          #define ICL_IMPLEMENT_FUNCTION(T,N,TYPE,CATEGORY)                                                                          \
          if(pcObj.supports(PointCloudObjectBase::TYPE)) {                                                                \
              if(!(std::strcmp(#CATEGORY,"COLOR") == 0 && removedColor) && !(std::strcmp(#CATEGORY,"POS") == 0 && removedPos)) {     \
                  core::DataSegment<T,N> segment = pcObj.select##TYPE();                                                        \
                  new_i = 0;                                                                                                         \
                  for(int i = 0; i < pcObj.getDim(); ++i) {                                                                          \
                      segment[new_i] = segment[i];                                                                                   \
                      if(!changedMaps) {                                                                                             \
                          actionMap[new_i] = actionMap[i];                                                                           \
                          groupMap[new_i] = groupMap[i];                                                                             \
                      }                                                                                                              \
                      if(!oldActionMap[i]) {                                                                                         \
                          new_i++;                                                                                                   \
                      }                                                                                                              \
                  }                                                                                                                  \
              }                                                                                                                      \
          }                                                                                                                          \
          if(std::strcmp(#CATEGORY,"COLOR") == 0) removedColor = true;                                                                              \
          if(std::strcmp(#CATEGORY,"POS") == 0) removedPos = true;                                                                                  \
          changedMaps = true;

          ICL_IMPLEMENT_FUNCTION(float,4,RGBA32f,COLOR)
          ICL_IMPLEMENT_FUNCTION(icl8u,4,BGRA,COLOR)
          ICL_IMPLEMENT_FUNCTION(icl8u,3,BGR,COLOR)
          ICL_IMPLEMENT_FUNCTION(icl32s,1,BGRA32s,COLOR)
          ICL_IMPLEMENT_FUNCTION(float,4,XYZH,POS)
          ICL_IMPLEMENT_FUNCTION(float,3,XYZ,POS)
          ICL_IMPLEMENT_FUNCTION(float,4,Normal,OTHER)
          ICL_IMPLEMENT_FUNCTION(float,1,Intensity,OTHER)
          ICL_IMPLEMENT_FUNCTION(icl32s,1,Label,OTHER)

          #undef ICL_IMPLEMENT_FUNCTION

          actionMap.resize(new_i);
          groupMap.resize(new_i);
          pcObj.setDim(new_i);

      }

      void Primitive3DFilter::SetposAction::performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                                          std::vector<unsigned char> &groupMap, core::Img32f *depthImage) {

          if(pcObj.supports(PointCloudObjectBase::XYZ)) {
              math::FixedColVector<float,3> actionPos(x, y, z);
              core::DataSegment<float,3> posSegment = pcObj.selectXYZ();
              for(int i = 0; i < pcObj.getDim(); ++i) {
                  if(actionMap[i]) {
                      posSegment[i] = actionPos;
                  }
              }
          } else if(pcObj.supports(PointCloudObjectBase::XYZH) || pcObj.canAddFeature(PointCloudObjectBase::XYZH)) {
              if(!pcObj.supports(PointCloudObjectBase::XYZH))
                  pcObj.addFeature(PointCloudObjectBase::XYZH);
              math::FixedColVector<float,4> actionPos(x, y, z, 1);
              core::DataSegment<float,4> posSegment = pcObj.selectXYZH();
              for(int i = 0; i < pcObj.getDim(); ++i) {
                  if(actionMap[i]) {
                      posSegment[i] = actionPos;
                  }
              }
          } else {
              DEBUG_LOG("Filter error: Position feature is not supported and can not be added.");
          }

      }

      void Primitive3DFilter::ColorAction::performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                                         std::vector<unsigned char> &groupMap, core::Img32f *depthImage) {

          if(pcObj.supports(PointCloudObjectBase::BGRA)) {
              math::FixedColVector<unsigned char,4> actionColor(b, g, r, a);
              core::DataSegment<unsigned char,4> colorSegment = pcObj.selectBGRA();
              for(int i = 0; i < pcObj.getDim(); ++i) {
                  if(actionMap[i]) {
                      colorSegment[i] = actionColor;
                  }
              }
          } else if(pcObj.supports(PointCloudObjectBase::BGR)) {
              math::FixedColVector<unsigned char,3> actionColor(b, g, r);
              core::DataSegment<unsigned char,3> colorSegment = pcObj.selectBGR();
              for(int i = 0; i < pcObj.getDim(); ++i) {
                  if(actionMap[i]) {
                      colorSegment[i] = actionColor;
                  }
              }
          } else if(pcObj.supports(PointCloudObjectBase::BGRA32s)) {
              icl32s actionColor = ((unsigned char)b) | (((unsigned char)g) << 8) | (((unsigned char)r) << 16) | (((unsigned char)a) << 24);
              core::DataSegment<icl32s,1> colorSegment = pcObj.selectBGRA32s();
              for(int i = 0; i < pcObj.getDim(); ++i) {
                  if(actionMap[i]) {
                      colorSegment[i] = actionColor;
                  }
              }
          } else if(pcObj.supports(PointCloudObjectBase::RGBA32f) || pcObj.canAddFeature(PointCloudObjectBase::RGBA32f)) {
              if(!pcObj.supports(PointCloudObjectBase::RGBA32f))
                  pcObj.addFeature(PointCloudObjectBase::RGBA32f);
              math::FixedColVector<float,4> actionColor(r, g, b, a);
              core::DataSegment<float,4> colorSegment = pcObj.selectRGBA32f();
              for(int i = 0; i < pcObj.getDim(); ++i) {
                  if(actionMap[i]) {
                      colorSegment[i] = actionColor;
                  }
              }
          } else {
              DEBUG_LOG("Filter error: Color feature is not supported and can not be added.");
          }

      }

      void Primitive3DFilter::LabelAction::performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                                         std::vector<unsigned char> &groupMap, core::Img32f *depthImage) {

          if(pcObj.supports(PointCloudObjectBase::Label) || pcObj.canAddFeature(PointCloudObjectBase::Label)) {
              if(!pcObj.supports(PointCloudObjectBase::Label))
                  pcObj.addFeature(PointCloudObjectBase::Label);
              icl32s actionLabel = value;
              core::DataSegment<icl32s,1> labelSegment = pcObj.selectLabel();
              for(int i = 0; i < pcObj.getDim(); ++i) {
                  if(actionMap[i]) {
                      labelSegment[i] = actionLabel;
                  }
              }
          } else {
              DEBUG_LOG("Filter error: Label feature is not supported and can not be added.");
          }

      }

      void Primitive3DFilter::IntensityAction::performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                                             std::vector<unsigned char> &groupMap, core::Img32f *depthImage) {

          if(pcObj.supports(PointCloudObjectBase::Intensity) || pcObj.canAddFeature(PointCloudObjectBase::Intensity)) {
              if(!pcObj.supports(PointCloudObjectBase::Intensity))
                  pcObj.addFeature(PointCloudObjectBase::Intensity);
              float actionIntensity = value;
              core::DataSegment<float,1> intensitySegment = pcObj.selectIntensity();
              for(int i = 0; i < pcObj.getDim(); ++i) {
                  if(actionMap[i]) {
                      intensitySegment[i] = actionIntensity;
                  }
              }
          } else {
              DEBUG_LOG("Filter error: Intensity feature is not supported and can not be added.");
          }

      }

      void Primitive3DFilter::FilterDepthImgAction::performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                                                  std::vector<unsigned char> &groupMap, core::Img32f *depthImage) {

          if(depthImage == 0) {
              DEBUG_LOG("Filter error: There is no depth image to be filtered.");
              return;
          }

          if(pcObj.getDim() != depthImage->getDim()) {
              DEBUG_LOG("Filter error: Dimensions of point cloud and depth image must be the same.");
              return;
          }

          core::Channel32f cDepth = (*depthImage)[0];

          for(int i = 0; i < depthImage->getDim(); ++i) {
              if(actionMap[i]) {
                  cDepth[i] = value;
              }
          }

      }

      void Primitive3DFilter::Primitive3D::toSceneObject(icl::geom::SceneObject *object, uint32_t slices,
                                                         icl::geom::GeomColor const &color) {

          math::Mat4 mat = orientation.getTransformationMatrix();
          mat(0, 3) = position[0];
          mat(1, 3) = position[1];
          mat(2, 3) = position[2];
          SceneObject *added_obj = 0;
          if (object) {
              switch (type) {
              case(CUBE): {
                  added_obj = object->addCuboid(0,0,0,
                                                scale[0],scale[1],scale[2]);
                  break;
              }
              case(SPHERE): {
                  added_obj = object->addSpheroid(0,0,0,
                                                  scale[0]/2.0,scale[1]/2.0,scale[2]/2.0,
                                                  slices,slices);
                  break;
              }
              case(CYLINDER): {
                  added_obj = object->addCylinder(0,0,0,
                                                  scale[0],scale[1],scale[2],
                                                  slices);
                  break;
              }
              default: {
                  std::stringstream sstream;
                  sstream << type;
                  WARNING_LOG("No primitive type for id: "+sstream.str());
                  return;//break;
              }
              }
          }
          if(added_obj){
            added_obj->setMaterial(Material::fromColor(color));
            added_obj->setTransformation(mat);
          }

      }

    } // namespace icl::geom