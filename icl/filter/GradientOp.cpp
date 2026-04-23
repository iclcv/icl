// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/GradientOp.h>
#include <icl/filter/ConvolutionOp.h>
#include <icl/core/Img.h>
#include <cmath>
#include <icl/utils/prop/Constraints.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {

  struct GradientOp::Data {
    // Sobel convolutions produce Img16s output from Img8u input (standard
    // ICL behaviour). Kept as members so the bank is allocated once.
    ConvolutionOp convX{ConvolutionKernel(ConvolutionKernel::sobelX3x3)};
    ConvolutionOp convY{ConvolutionKernel(ConvolutionKernel::sobelY3x3)};
    Image gx, gy;
    Image u8Buffer;  // for non-8u sources
  };

  static const char *MODE_MENU = "x,y,intensity,angle";

  static const char *modeName(GradientOp::Mode m){
    switch(m){
      case GradientOp::x:         return "x";
      case GradientOp::y:         return "y";
      case GradientOp::intensity: return "intensity";
      case GradientOp::angle:     return "angle";
    }
    return "intensity";
  }
  static GradientOp::Mode parseMode(const std::string &s){
    if(s == "x")     return GradientOp::x;
    if(s == "y")     return GradientOp::y;
    if(s == "angle") return GradientOp::angle;
    return GradientOp::intensity;
  }

  GradientOp::GradientOp(Mode mode)
    : m_mode(mode), m_normalize(true), m_data(new Data){
    addProperty("mode",utils::prop::menuFromCsv(MODE_MENU), modeName(mode));
    addProperty("normalize",utils::prop::Flag{}, m_normalize);
    registerCallback([this](const Property &p){
      if(p.name == "mode")           m_mode = parseMode(p.as<std::string>());
      else if(p.name == "normalize") m_normalize = p.as<bool>();
    });
  }

  GradientOp::~GradientOp(){ delete m_data; }

  void GradientOp::setMode(Mode m){ setPropertyValue("mode", modeName(m)); }
  void GradientOp::setNormalize(bool n){ prop("normalize").value = n; }

  // ConvolutionOp on an 8u source with forceUnsignedOutput=false produces 16s.
  // For other depths ConvolutionOp preserves the source depth — we coerce to
  // 8u first for consistent behaviour (matches the old GradientImage class).
  static const Image &to8u(const Image &src, Image &buffer){
    if(src.getDepth() == depth8u) return src;
    if(buffer.isNull()
       || buffer.getSize()     != src.getSize()
       || buffer.getChannels() != src.getChannels()
       || buffer.getFormat()   != src.getFormat()){
      buffer = Image(src.getSize(), depth8u, src.getChannels(), src.getFormat());
    }
    src.convertTo(buffer);
    return buffer;
  }

  void GradientOp::apply(const Image &src, Image &dst){
    ICLASSERT_RETURN(!src.isNull());

    const Image &src8u = to8u(src, m_data->u8Buffer);

    // Compute both gradients — cheap, and required for intensity/angle.
    m_data->convX.setClipToROI(getClipToROI());
    m_data->convY.setClipToROI(getClipToROI());
    m_data->convX.apply(src8u, m_data->gx);
    m_data->convY.apply(src8u, m_data->gy);

    const Img16s &gx = m_data->gx.as16s();
    const Img16s &gy = m_data->gy.as16s();
    const Size size  = gx.getSize();
    const int C      = gx.getChannels();

    if(m_mode == x || m_mode == y){
      // Output is the raw int16 gradient — copy the right one.
      const Image &pick = (m_mode == x) ? m_data->gx : m_data->gy;
      if(m_normalize){
        // Map [-1020,1020] (int16 Sobel magnitude) into 32f [0,255] for display.
        if(!prepare(dst, depth32f, size, pick.getFormat(), C,
                    pick.getROI(), pick.getTime())) return;
        const Img16s &g = pick.as16s();
        Img32f &d = dst.as32f();
        for(int c = 0; c < C; ++c){
          const icl16s *s = g.getData(c);
          icl32f *o = d.getData(c);
          for(int i = g.getDim() - 1; i >= 0; --i){
            o[i] = (static_cast<float>(s[i]) + 1020.f) * (255.f / 2040.f);
          }
        }
      }else{
        dst = pick.deepCopy();
      }
      return;
    }

    // intensity / angle — both 32f outputs.
    if(!prepare(dst, depth32f, size, gx.getFormat(), C,
                gx.getROI(), src.getTime())) return;
    Img32f &d = dst.as32f();
    for(int c = 0; c < C; ++c){
      const icl16s *ax = gx.getData(c);
      const icl16s *ay = gy.getData(c);
      icl32f *out = d.getData(c);
      const int dim = gx.getDim();
      if(m_mode == intensity){
        for(int i = 0; i < dim; ++i){
          const float a = ax[i], b = ay[i];
          out[i] = std::sqrt(a*a + b*b);
        }
      }else{ // angle
        for(int i = 0; i < dim; ++i){
          out[i] = std::atan2(static_cast<float>(ay[i]),
                              static_cast<float>(ax[i]));
        }
      }
    }

    if(m_normalize){
      d.normalizeAllChannels(Range<icl32f>(0, 255));
    }
  }

  REGISTER_CONFIGURABLE_DEFAULT(GradientOp);
  } // namespace icl::filter
