// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/WeightChannelsOp.h>
#include <icl/core/Img.h>
#include <icl/core/Image.h>
#include <icl/core/Visitors.h>
#include <icl/utils/prop/Constraints.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  // Shared weight-string helpers used by both WeightChannelsOp and WeightedSumOp.
  // Weights live in a "string" property (comma-separated doubles) because the
  // vector length depends on the input image's channel count — no single
  // slider widget fits. Default: ITU-R BT.601 luminance for 3-channel RGB.
  static const char *WEIGHT_STRING_DEFAULT = "0.299,0.587,0.114";

  static std::string joinWeights(const std::vector<icl64f> &w){
    std::string s;
    for(size_t i = 0; i < w.size(); ++i){
      if(i) s += ",";
      s += str(w[i]);
    }
    return s.empty() ? std::string(WEIGHT_STRING_DEFAULT) : s;
  }

  static std::vector<icl64f> parseWeights(const std::string &s){
    std::vector<icl64f> out;
    for(const std::string &tok : tok(s, ",")){
      try{ out.push_back(parse<icl64f>(tok)); }catch(...){}
    }
    return out;
  }

  WeightChannelsOp::WeightChannelsOp(){
    addProperty("weights",utils::prop::Text{.maxLength=128}, WEIGHT_STRING_DEFAULT);
    registerCallback([this](const Property &p){
      if(p.name == "weights") m_vecWeights = parseWeights(p.value);
    });
    m_vecWeights = parseWeights(WEIGHT_STRING_DEFAULT);
  }

  WeightChannelsOp::WeightChannelsOp(const std::vector<icl64f> &weights)
    : m_vecWeights(weights){
    addProperty("weights",utils::prop::Text{.maxLength=128}, joinWeights(weights));
    registerCallback([this](const Property &p){
      if(p.name == "weights") m_vecWeights = parseWeights(p.value);
    });
  }

  void WeightChannelsOp::setWeights(const std::vector<icl64f> &weights){
    setPropertyValue("weights", joinWeights(weights));
  }

  void WeightChannelsOp::apply(const Image &src, Image &dst) {
    ICLASSERT_RETURN( static_cast<int>(m_vecWeights.size()) == src.getChannels() );
    if(!prepare(dst, src)) return;
    src.visitWith(dst, [this](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      visitROILinesPerChannelWith(s, d, [this](const T *sp, T *dp, int ch, int w) {
        icl64f wt = m_vecWeights[ch];
        for(int i = 0; i < w; ++i) {
          dp[i] = clipped_cast<icl64f, T>(static_cast<icl64f>(sp[i]) * wt);
        }
      });
    });
  }

  REGISTER_CONFIGURABLE_DEFAULT(WeightChannelsOp);
  } // namespace icl::filter