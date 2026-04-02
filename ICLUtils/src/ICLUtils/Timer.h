// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke

#pragma once

#include <ICLUtils/Macros.h>
#include <ICLUtils/Time.h>
#include <vector>
#include <string>

namespace icl::utils {
  class ICLUtils_API Timer{
    private:
    int m_iTimerMode;
    std::vector<Time::value_type> m_vecTime;
    std::vector<std::string> m_vecTimerName;

    Time::value_type getTime();

    public:
    /// Constructor
    /** @param iTimerMode Select the output format (0 = ms, 1 = ns)
        **/
    Timer(int iTimerMode=0);

    ///Start the time measurement
    void startTimer();

    /// Set a sub timer.
    /** Each sub time is printed at the end of the whole time
        measure process (stopTimer).
        @param sName Set the timer name (only for user information)
        @sa stopTimer
        **/
    void stopSubTimer(std::string sName = "no name");
    /// Stop the time measurement and print the complete working time

    void stopTimer(std::string sName = "no name");

    /// alias for startTimer
    void start(){ startTimer(); }

    /// alias for stopTimer
    void stop(std::string sName = "no name" ){ stopTimer(sName) ;}

    /// stops the timer and returns the overall working time as long int
    long int stopSilent();
  }; //class Timer
  } // namespace icl::utils