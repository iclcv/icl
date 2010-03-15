/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLUtils/Timer.h                               **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter, Robert Haschke                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#include <ICLUtils/Time.h>
#include <vector>
#include <string>
#include <ICLUtils/Macros.h>
/*
  Timer.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICL_TIMER_H
#define ICL_TIMER_H


namespace icl {

class Timer
{
 private:
  int m_iTimerMode;
  std::vector<Time::value_type> m_vecTime;
  std::vector<std::string> m_vecTimerName;
  
  Time::value_type getTime();
  
 public:
  /// @{ @name constructors / destructor
  ///Construct the Timer class.      
  /** @param iTimerMode Select the output format (0 = ms, 1 = ns)
  **/
  Timer(int iTimerMode=0);

   /// @}

/* }}} */

/* {{{ Timer functions */

  //--------------------------------------------------------------------------
  ///Start the time measurement 
  /** 
      @sa stopTimer
  **/
  void startTimer();

  //--------------------------------------------------------------------------
  /// Set a sub timer.
  /** Each sub time is printed at the end of the whole time
      measure process (stopTimer).
      @param sName Set the timer name (only for user information)
      @sa stopTimer
  **/
  void stopSubTimer(std::string sName = "no name");
  
  //--------------------------------------------------------------------------
  /// Stop the time measurement and print the complete working time
  /** 
      @sa stopSubTimer
  **/
    void stopTimer(std::string sName = "no name");

    /// alias for startTimer
    void start(){ startTimer(); }
    
    /// alias for stopTimer
    void stop(std::string sName = "no name" ){ stopTimer(sName) ;}
    
    /// stops the timer and returns the overall working time as long int
    long int stopSilent();
/* }}} */

}; //class Timer

} //namespace ICL


#endif //ICL_TIME_H
