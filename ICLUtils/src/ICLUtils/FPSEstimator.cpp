// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLUtils/FPSEstimator.h>
#include <ICLUtils/Macros.h>
#include <cstdio>
#include <cstdarg>
#include <cstring>
namespace icl::utils {
  FPSEstimator::FPSEstimator(int n){
    reset(n);
  }

  void FPSEstimator::reset(int n){
    if(n<2){
      n = 2;
    }
    m_iN = n;
    m_qTimes.clear();
    for(int i=0;i<n;i++){
      m_qTimes.push_back(Time());
    }
  }

  void FPSEstimator::tic() const{
    m_qTimes.push_back(Time::now());
    m_qTimes.pop_front();
  }

  float FPSEstimator::getFPSVal() const{
    tic();
    if(m_qTimes.front()==Time::null){
      return -1;
    }else{
      double avgDt = 0;

      Time t = m_qTimes.front();
      for(std::deque<Time>::iterator it=++m_qTimes.begin();it!=m_qTimes.end();++it){
        avgDt += ((*it)-t).toMicroSecondsDouble();
        t = *it;
      }
      return 1.0/(avgDt/(1000000.0*(m_iN-1)));
    }
  }

  std::string FPSEstimator::getFPSString(const std::string &fmt, int bufferSize) const{
    char *buf = new char[bufferSize];
    snprintf(buf,bufferSize,fmt.c_str(),getFPSVal());
    std::string s(buf);
    delete [] buf;
    return s;

  }

  void FPSEstimator::showFPS(const std::string &text) const{
    printf("%s:%s\n",text.c_str(),getFPSString().c_str());
  }

  std::string FPSEstimator::formatted(const char *fmt, ...) const {
    // Replace #fps with the current FPS value
    float fps = getFPSVal();
    char fpsStr[16];
    snprintf(fpsStr, sizeof(fpsStr), "%.0f", fps);

    // Build format string with #fps replaced
    std::string resolved;
    const char *p = fmt;
    while (*p) {
      if (strncmp(p, "#fps", 4) == 0) {
        resolved += fpsStr;
        p += 4;
      } else {
        resolved += *p++;
      }
    }

    // Apply remaining printf args
    va_list args;
    va_start(args, fmt);
    char buf[256];
    vsnprintf(buf, sizeof(buf), resolved.c_str(), args);
    va_end(args);
    return std::string(buf);
  }

  } // namespace icl::utils