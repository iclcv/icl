/*
  ICLTimer.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICL_TIMER_H
#define ICL_TIMER_H

#include <time.h>
#include <sys/time.h>
#include "ICL.h"

using namespace std;

namespace icl {

class ICLTimer
{
 private:
  int m_iTimerMode;
  vector<long> m_vecTime;
  string m_sTimerName;
  
  long getTime();
  
 public:
/* {{{ Konstruktor/ Destruktor */  //@{ @name Constructors  ///Construct the ICLTimerr class.        /** @param timerName Set the timer name (only for info)      @param iTimerMode Select the output format (0 = ms, 1 = ns)  **/  ICLTimer(string timerName="icl timer", int iTimerMode=0);/* }}} */

/* {{{ Timer functions */  //--------------------------------------------------------------------------  ///Start the time measurement   /**       @sa stopTimer  **/  void startTimer();  //--------------------------------------------------------------------------  /// Set a sub timer.  /** Each sub time is printed at the end of the whole time      measure process (stopTimer).      @sa stopTimer  **/  void stopSubTimer();    //--------------------------------------------------------------------------  /// Stop the time measurement and print the complete working time  /**       @sa stopSubTimer  **/    void stopTimer();/* }}} */

}; //class ICLTimer

} //namespace ICL


#endif //ICL_TIME_H
