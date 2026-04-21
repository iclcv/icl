/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLFilter/src/ICLFilter/MotionSensitiveTemporalSmoothi **
 **          ng.cpp                                                 **
 ** Module : ICLFilter                                              **
 ** Authors: Andre Ueckermann, Christof Elbrechter                  **
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

#include <icl/filter/MotionSensitiveTemporalSmoothing.h>
#include <algorithm>
#include <cstring>
#include <limits>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
namespace {

/// Per-channel temporal smoothing: ring buffer + motion-aware averaging.
/// Comparison of pixel values against nullValue uses C++ promotion rules
/// (unsigned char promoted to int, int promoted to float) to match the
/// original behavior where nullValue=-1 means "no null values".
template<class T>
void smoothChannel(const T* srcData, T* dstData, float* motionData,
                 std::vector<Img<T>>& history, int& imgCount,
                 int w, int h, int filterSize, int difference, int nullValue) {
const int n = w * h;

// Store current frame in ring buffer
const int slot = imgCount % filterSize;
std::memcpy(history[slot].getData(0), srcData, n * sizeof(T));
imgCount++;

const int activeCount = std::min(imgCount, filterSize);

// Pre-collect data pointers for efficient inner loop
const int maxPtrs = activeCount;
std::vector<const T*> histPtrs(maxPtrs);
for (int t = 0; t < maxPtrs; t++) {
  histPtrs[t] = history[t].getData(0);
}

for (int i = 0; i < n; i++) {
  int count = 0;
  double sum = 0;
  T minVal = std::numeric_limits<T>::max();
  T maxVal = std::numeric_limits<T>::lowest();

  for (int t = 0; t < activeCount; t++) {
    T v = histPtrs[t][i];
    // v != nullValue: uses C++ implicit promotion
    //   icl8u:  (int)v != (int)nullValue  — -1 never matches 0..255
    //   icl32f: (float)v != (float)nullValue
    if (v != nullValue) {
      count++;
      sum += v;
      if (v < minVal) minVal = v;
      if (v > maxVal) maxVal = v;
    }
  }

  if (count == 0) {
    dstData[i] = static_cast<T>(nullValue);
    motionData[i] = 0.f;
  } else if ((maxVal - minVal) > difference) {
    dstData[i] = srcData[i];
    motionData[i] = 255.f;
  } else {
    dstData[i] = static_cast<T>(sum / count);
    motionData[i] = 0.f;
  }
}
}

} // anonymous namespace


MotionSensitiveTemporalSmoothing::MotionSensitiveTemporalSmoothing(
  int nullValue, int maxFilterSize)
  : m_nullValue(nullValue)
  , m_maxFilterSize(maxFilterSize)
  , m_filterSize(std::max(1, maxFilterSize / 2))
  , m_difference(10)
  , m_useCL(true)
{
}

MotionSensitiveTemporalSmoothing::~MotionSensitiveTemporalSmoothing() = default;

void MotionSensitiveTemporalSmoothing::reinit(int channels, depth d, Size size) {
m_numChannels = channels;
m_depth = d;
m_size = size;
m_channels.clear();
m_channels.resize(channels);

for (int c = 0; c < channels; c++) {
  auto& ch = m_channels[c];
  ch.imgCount = 0;
  ch.motionImage = Img32f(size, 1);
  ch.motionImage.clear(-1, 0.f);

  if (d == depth32f) {
    ch.historyF.resize(m_maxFilterSize);
    for (auto& img : ch.historyF) {
      img = Img32f(size, 1);
      img.clear(-1, 0.f);
    }
  } else {
    ch.historyC.resize(m_maxFilterSize);
    for (auto& img : ch.historyC) {
      img = Img8u(size, 1);
      img.clear(-1, (icl8u)0);
    }
  }
}
}

void MotionSensitiveTemporalSmoothing::apply(const Image &src, Image &dst) {
if (!src.hasFullROI())
  throw ICLException("MotionSensitiveTemporalSmoothing::apply: no ROI support");

const depth d = src.getDepth();
if (d != depth8u && d != depth32f)
  throw ICLException("MotionSensitiveTemporalSmoothing::apply: depth 32f and 8u only");

if (!prepare(dst, src)) return;

if (src.getChannels() != m_numChannels ||
    src.getDepth() != m_depth ||
    src.getSize() != m_size) {
  reinit(src.getChannels(), src.getDepth(), src.getSize());
}

const int w = m_size.width;
const int h = m_size.height;

if (d == depth32f) {
  const Img32f& s = src.as32f();
  Img32f& ds = dst.as32f();
  for (int c = 0; c < m_numChannels; c++) {
    auto& ch = m_channels[c];
    smoothChannel<icl32f>(s.getData(c), ds.getData(c),
                          ch.motionImage.getData(0),
                          ch.historyF, ch.imgCount,
                          w, h, m_filterSize, m_difference, m_nullValue);
  }
} else {
  const Img8u& s = src.as8u();
  Img8u& ds = dst.as8u();
  for (int c = 0; c < m_numChannels; c++) {
    auto& ch = m_channels[c];
    smoothChannel<icl8u>(s.getData(c), ds.getData(c),
                         ch.motionImage.getData(0),
                         ch.historyC, ch.imgCount,
                         w, h, m_filterSize, m_difference, m_nullValue);
  }
}
}

void MotionSensitiveTemporalSmoothing::setUseCL(bool use) {
m_useCL = use;
}

bool MotionSensitiveTemporalSmoothing::isCLActive() const {
return m_useCL;
}

void MotionSensitiveTemporalSmoothing::setFilterSize(int filterSize) {
filterSize = std::clamp(filterSize, 1, m_maxFilterSize);
if (filterSize != m_filterSize) {
  m_filterSize = filterSize;
  for (auto& ch : m_channels) {
    ch.imgCount = 0;
  }
}
}

void MotionSensitiveTemporalSmoothing::setDifference(int difference) {
m_difference = difference;
}

Img32f MotionSensitiveTemporalSmoothing::getMotionImage() const {
if (m_channels.empty()) return Img32f();
return m_channels[0].motionImage;
}

} // namespace icl::filter