// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke, Andre Justus

#include <icl/filter/ConvolutionOp.h>
#include <icl/core/Img.h>
#include <icl/core/Image.h>
#include <icl/utils/prop/Constraints.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  const char* toString(ConvolutionOp::Op op) {
    switch(op) {
      case ConvolutionOp::Op::apply: return "apply";
    }
    return "?";
  }

  core::ImageBackendDispatching& ConvolutionOp::prototype() {
    static core::ImageBackendDispatching proto;
    [[maybe_unused]] static bool init = [&] {
      proto.addSelector<ConvSig>(Op::apply);
      return true;
    }();
    return proto;
  }

  // Keep these two mappings in lockstep with ConvolutionKernel::fixedType.
  static const char *fixedTypeName(ConvolutionKernel::fixedType t){
    switch(t){
      case ConvolutionKernel::gauss3x3:   return "gauss3x3";
      case ConvolutionKernel::gauss5x5:   return "gauss5x5";
      case ConvolutionKernel::sobelX3x3:  return "sobelX3x3";
      case ConvolutionKernel::sobelX5x5:  return "sobelX5x5";
      case ConvolutionKernel::sobelY3x3:  return "sobelY3x3";
      case ConvolutionKernel::sobelY5x5:  return "sobelY5x5";
      case ConvolutionKernel::laplace3x3: return "laplace3x3";
      case ConvolutionKernel::laplace5x5: return "laplace5x5";
      case ConvolutionKernel::custom:     return "custom";
    }
    return "custom";
  }

  static const char *KERNEL_MENU =
    "gauss3x3,gauss5x5,sobelX3x3,sobelX5x5,"
    "sobelY3x3,sobelY5x5,laplace3x3,laplace5x5,custom";

  static bool parseFixedType(const std::string &s, ConvolutionKernel::fixedType &out){
    if(s == "gauss3x3")   { out = ConvolutionKernel::gauss3x3;   return true; }
    if(s == "gauss5x5")   { out = ConvolutionKernel::gauss5x5;   return true; }
    if(s == "sobelX3x3")  { out = ConvolutionKernel::sobelX3x3;  return true; }
    if(s == "sobelX5x5")  { out = ConvolutionKernel::sobelX5x5;  return true; }
    if(s == "sobelY3x3")  { out = ConvolutionKernel::sobelY3x3;  return true; }
    if(s == "sobelY5x5")  { out = ConvolutionKernel::sobelY5x5;  return true; }
    if(s == "laplace3x3") { out = ConvolutionKernel::laplace3x3; return true; }
    if(s == "laplace5x5") { out = ConvolutionKernel::laplace5x5; return true; }
    return false; // "custom" or unknown — caller leaves kernel untouched
  }

  void ConvolutionOp::addConvProperties(const ConvolutionKernel &kernel, bool forceUnsignedOutput){
    addProperty("kernel",utils::prop::menuFromCsv(KERNEL_MENU), fixedTypeName(kernel.getFixedType()));
    addProperty("force unsigned output",utils::prop::Flag{}, forceUnsignedOutput);
  }

  void ConvolutionOp::property_callback(const Property &p){
    if(p.name == "kernel"){
      ConvolutionKernel::fixedType t;
      if(parseFixedType(p.as<std::string>(), t)){
        // Replace kernel + update NeighborhoodOp's mask size.
        m_kernel = ConvolutionKernel(t);
        setMask(m_kernel.getSize());
      }
      // else: "custom" — leave m_kernel alone (was set explicitly via setKernel)
    }else if(p.name == "force unsigned output"){
      m_forceUnsignedOutput = p.as<bool>();
    }
  }

  ConvolutionOp::ConvolutionOp(const ConvolutionKernel &kernel):
    NeighborhoodOp(kernel.getSize()),
    ImageBackendDispatching(prototype()),
    m_kernel(kernel),
    m_forceUnsignedOutput(false){
    addConvProperties(kernel, false);
    registerCallback([this](const Property &p){ property_callback(p); });
  }
  ConvolutionOp::ConvolutionOp(const ConvolutionKernel &kernel, bool forceUnsignedOutput):
    NeighborhoodOp(kernel.getSize()),
    ImageBackendDispatching(prototype()),
    m_kernel(kernel),
    m_forceUnsignedOutput(forceUnsignedOutput){
    addConvProperties(kernel, forceUnsignedOutput);
    registerCallback([this](const Property &p){ property_callback(p); });
  }

  void ConvolutionOp::setKernel(const ConvolutionKernel &kernel){
    m_kernel = kernel;
    setMask(m_kernel.getSize());
    // Sync the "kernel" property so listeners (GUI etc.) see the change; the
    // callback's own rebuild path is a no-op on identical values / "custom".
    setPropertyValue("kernel", fixedTypeName(kernel.getFixedType()));
  }

  void ConvolutionOp::setForceUnsignedOutput(bool v){ setPropertyValue("force unsigned output", v); }

  bool ConvolutionOp::getForceUnsignedOutput() const {
    return prop("force unsigned output").as<bool>();
  }

  void ConvolutionOp::apply(const core::Image &src, core::Image &dst) {
    ICLASSERT_RETURN(!src.isNull());
    ICLASSERT_RETURN(!m_kernel.isNull());

    depth dstDepth = m_forceUnsignedOutput ? src.getDepth()
                     : (src.getDepth() == depth8u ? depth16s : src.getDepth());
    if(!prepare(dst, src, dstDepth)) return;

    if(src.getDepth() >= depth32f){
      m_kernel.toFloat();
    }else if(m_kernel.isFloat()){
      // Need an int kernel on an int-source path. toInt(true) truncates
      // normalized floats to zeros (e.g. gauss3x3 1/16 → 0), producing an
      // all-zero kernel → black output. If the kernel originated from a
      // known fixed type, rebuild from the lookup table — lossless.
      if(m_kernel.getFixedType() != ConvolutionKernel::custom){
        m_kernel = ConvolutionKernel(m_kernel.getFixedType());
      }else{
        WARNING_LOG("convolution of non-float images with float kernels is not supported\n"
                    "use an int-kernel instead. For now, the kernel is casted to int-type");
        m_kernel.toInt(true);
      }
    }

    getSelector<ConvSig>(Op::apply).resolve(src)->apply(src, dst, *this);
  }

  REGISTER_CONFIGURABLE_DEFAULT(ConvolutionOp);
  } // namespace icl::filter