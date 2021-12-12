/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Timer.cpp                        **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter, Robert Haschke                    **
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

#include <ICLUtils/Timer.h>
#include <ICLUtils/Time.h>

using namespace std;

namespace icl {
  namespace utils{

    Timer::Timer(int iTimerMode){
      m_iTimerMode = iTimerMode;
    }

    void Timer::startTimer(){
      m_vecTime.push_back(getTime());
      m_vecTimerName.push_back("_START_");
    }

    void Timer::stopSubTimer(string sName){
      m_vecTime.push_back(getTime());
      m_vecTimerName.push_back(sName);
    }

    void Timer::stopTimer(string sName){
      long int lTmpTimeDiff = 0;

      m_vecTime.push_back(getTime());
      m_vecTimerName.push_back(sName);

      cout << endl;
      cout << " ------------------------------------------- " << endl;
      cout << " --             Time measure              -- " << endl;

      if (m_vecTime.size() > 2) {
        for (unsigned int i=1;i<m_vecTime.size();i++)
          {
            lTmpTimeDiff = (long int)(m_vecTime[i] - m_vecTime[i-1]);

            switch(m_iTimerMode)
              {
                case 0:
                  cout << " --  [" << m_vecTimerName[i] << "] -> Time: "
                       << lTmpTimeDiff << " ms" << endl;
                  break;

                case 1:
                  cout << " --  [" << m_vecTimerName[i] << "] -> Time: "
                       << lTmpTimeDiff << " micro sec" << endl;
                  break;
              }
          }
      }

      lTmpTimeDiff = (long int)(m_vecTime[m_vecTime.size()-1] - m_vecTime[0]);

      switch(m_iTimerMode)
        {
          case 0:
            cout << " ------------------------------------------- " << endl;
            cout << " [ --- ] -> Complete time: " << lTmpTimeDiff << " ms" << endl;
            break;

          case 1:
            cout << " ------------------------------------------- " << endl;
            cout << " [ --- ] -> Complete time: " << lTmpTimeDiff << " ns" << endl;
            break;
        }
      cout << endl;
    }

    long int Timer::stopSilent(){
      return  (long int)(getTime() - m_vecTime[0]);
    }

    Time::value_type Timer::getTime(){
      switch (m_iTimerMode){
        case 0: //ms
          return Time::now().toMilliSeconds();

        default: //µs
          return Time::now().toMicroSeconds();
      }
    }

  } // utils

} // icl
