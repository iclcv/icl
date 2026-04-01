// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Time.h>
#include <deque>
#include <string>

namespace icl{
  namespace utils{
    ///Utility clas for online FPS estimation \ingroup TIME
    class ICLUtils_API FPSEstimator{
      public:

      ///Constructor
      /** Current FPS values are averaged over given intervall n*/
      FPSEstimator(int n=2);

      /// virtual destructor
      virtual ~FPSEstimator() {}

      /// Sets a new averaging interval
      void reset(int n);

      /// pushes current time into the time averaging queue and pop oldest time from the queue
      virtual void tic() const;

      /// applies tic() and returns current FPS estimate
      virtual float getFPSVal() const;

      /// applies tic() and returns current fps estimate as formated string
      virtual std::string getFPSString(const std::string &fmt="%3.4f fps", int bufferSize=30) const;

      /// applies tic() and shows current FPS estimate on std::out
      virtual void showFPS(const std::string &text="") const;

      private:
      /// internal time queue
      mutable std::deque<Time> m_qTimes;

      /// time-queues size
      int m_iN;
    };

  #define FPS_LOG_THIS_FUNCTION(N) static FPSEstimator __FPSEstimator__((N)); \
                                   __FPSEstimator__.showFps(__FUNCTION__);

  } // namespace utils
}
