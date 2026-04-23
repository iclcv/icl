// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke, Andre Justus

#include <icl/filter/WeightedSumOp.h>
#include <icl/core/Img.h>
#include <icl/core/Image.h>
#include <icl/utils/prop/Constraints.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  // Same "weights as comma-separated string" pattern as WeightChannelsOp.
  static const char *WEIGHTED_SUM_DEFAULT = "0.299,0.587,0.114";

  static std::string joinW(const std::vector<icl64f> &w){
    std::string s;
    for(size_t i = 0; i < w.size(); ++i){ if(i) s += ","; s += str(w[i]); }
    return s.empty() ? std::string(WEIGHTED_SUM_DEFAULT) : s;
  }
  static std::vector<icl64f> parseW(const std::string &s){
    std::vector<icl64f> out;
    for(const std::string &t : tok(s, ",")){
      try{ out.push_back(parse<icl64f>(t)); }catch(...){}
    }
    return out;
  }

  WeightedSumOp::WeightedSumOp(){
    addProperty("weights",utils::prop::Text{.maxLength=128}, WEIGHTED_SUM_DEFAULT);
    registerCallback([this](const Property &p){
      if(p.name == "weights") m_vecWeights = parseW(p.value);
    });
    m_vecWeights = parseW(WEIGHTED_SUM_DEFAULT);
  }
  WeightedSumOp::WeightedSumOp(const std::vector<icl64f> &weights)
    : m_vecWeights(weights){
    addProperty("weights",utils::prop::Text{.maxLength=128}, joinW(weights));
    registerCallback([this](const Property &p){
      if(p.name == "weights") m_vecWeights = parseW(p.value);
    });
  }
  void WeightedSumOp::setWeights(const std::vector<icl64f> &weights){
    setPropertyValue("weights", joinW(weights));
  }

  template <class T, class D>
  void apply_ws(const Img<T> &src, Img<D> &dst, const std::vector<D> &weights) {
    const ImgIterator<T> itSrc = src.beginROI(0);
    const ImgIterator<T> itSrcEnd = src.endROI(0);
    ImgIterator<D> itDst = dst.beginROI(0);

    D w = weights[0];
    for(;itSrc != itSrcEnd; ++itSrc, ++itDst){
      *itDst = *itSrc * w;
    }

    for (int c=src.getChannels()-1; c > 0; c--) {
      w = weights[c];
      const ImgIterator<T> itSrc = src.beginROI(c);
      const ImgIterator<T> itSrcEnd = src.endROI(c);
      ImgIterator<D> itDst = dst.beginROI(0);
      for(;itSrc!=itSrcEnd; ++itSrc, ++itDst){
        *itDst += *itSrc * w;
      }
    }
  }

  void WeightedSumOp::apply(const Image &src, Image &dst) {
    ICLASSERT_RETURN( static_cast<int>(m_vecWeights.size()) == src.getChannels() );

    depth dstDepth = src.getDepth() == depth64f ? depth64f : depth32f;
    if(!prepare(dst, dstDepth,
                getClipToROI() ? src.getROISize() : src.getSize(),
                formatMatrix, 1,
                getClipToROI() ? Rect(Point::null, src.getROISize()) : src.getROI(),
                src.getTime())) return;

    if(dstDepth == depth64f){
      Img64f &d = dst.as64f();
      src.visit([&](const auto &s) {
        apply_ws(s, d, m_vecWeights);
      });
    }else{
      std::vector<icl32f> v(m_vecWeights.size());
      for(unsigned int i=0;i<m_vecWeights.size();++i){
        v[i] = static_cast<float>(m_vecWeights[i]);
      }
      Img32f &d = dst.as32f();
      src.visit([&](const auto &s) {
        apply_ws(s, d, v);
      });
    }
  }

  REGISTER_CONFIGURABLE_DEFAULT(WeightedSumOp);
  } // namespace icl::filter