// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke

#include <ICLUtils/Timer.h>
#include <ICLUtils/Time.h>

namespace icl::utils {
  Timer::Timer(int iTimerMode){
    m_iTimerMode = iTimerMode;
  }

  void Timer::startTimer(){
    m_vecTime.push_back(getTime());
    m_vecTimerName.push_back("_START_");
  }

  void Timer::stopSubTimer(std::string sName){
    m_vecTime.push_back(getTime());
    m_vecTimerName.push_back(sName);
  }

  void Timer::stopTimer(std::string sName){
    long int lTmpTimeDiff = 0;

    m_vecTime.push_back(getTime());
    m_vecTimerName.push_back(sName);

    std::cout << std::endl;
    std::cout << " ------------------------------------------- " << std::endl;
    std::cout << " --             Time measure              -- " << std::endl;

    if (m_vecTime.size() > 2) {
      for (unsigned int i=1;i<m_vecTime.size();i++)
        {
          lTmpTimeDiff = static_cast<long int>(m_vecTime[i] - m_vecTime[i-1]);

          switch(m_iTimerMode)
            {
              case 0:
                std::cout << " --  [" << m_vecTimerName[i] << "] -> Time: "
                     << lTmpTimeDiff << " ms" << std::endl;
                break;

              case 1:
                std::cout << " --  [" << m_vecTimerName[i] << "] -> Time: "
                     << lTmpTimeDiff << " micro sec" << std::endl;
                break;
            }
        }
    }

    lTmpTimeDiff = static_cast<long int>(m_vecTime[m_vecTime.size()-1] - m_vecTime[0]);

    switch(m_iTimerMode)
      {
        case 0:
          std::cout << " ------------------------------------------- " << std::endl;
          std::cout << " [ --- ] -> Complete time: " << lTmpTimeDiff << " ms" << std::endl;
          break;

        case 1:
          std::cout << " ------------------------------------------- " << std::endl;
          std::cout << " [ --- ] -> Complete time: " << lTmpTimeDiff << " ns" << std::endl;
          break;
      }
    std::cout << std::endl;
  }

  long int Timer::stopSilent(){
    return  static_cast<long int>(getTime() - m_vecTime[0]);
  }

  Time::value_type Timer::getTime(){
    switch (m_iTimerMode){
      case 0: //ms
        return Time::now().toMilliSeconds();

      default: //�s
        return Time::now().toMicroSeconds();
    }
  }

  } // namespace icl::utils