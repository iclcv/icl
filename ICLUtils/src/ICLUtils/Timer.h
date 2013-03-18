/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Timer.h                          **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter, Robert Haschke                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#include <ICLUtils/Time.h>
#include <vector>
#include <string>
#include <ICLUtils/Macros.h>
#pragma once

namespace icl {
  namespace utils{
    class Timer{
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
  } // namespace utils
  
} //namespace ICL

