/*
  Timer.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICL_TIMER_H
#define ICL_TIMER_H

#include <time.h>
#include <sys/time.h>
#include <vector>
#include <string>
#include <Macros.h>

using namespace std;

namespace icl {

class Timer
{
 private:
  int m_iTimerMode;
  vector<long int> m_vecTime;
  vector<string> m_vecTimerName;
  
  long int getTime();
  
 public:
/* {{{ Konstruktor/ Destruktor */
  //@{ @name Constructors
  ///Construct the Timerr class.      
  /** @param iTimerMode Select the output format (0 = ms, 1 = ns)
  **/
  Timer(int iTimerMode=0);

/* }}} */

/* {{{ Timer functions */

  //--------------------------------------------------------------------------
  ///Start the time measurement 
  /** 
      @param timerName Set the timer name (only for user information)
      @sa stopTimer
  **/
  void startTimer();

  //--------------------------------------------------------------------------
  /// Set a sub timer.
  /** Each sub time is printed at the end of the whole time
      measure process (stopTimer).
      @param timerName Set the timer name (only for user information)
      @sa stopTimer
  **/
  void stopSubTimer(string sName);
  
  //--------------------------------------------------------------------------
  /// Stop the time measurement and print the complete working time
  /** 
      @sa stopSubTimer
  **/
    void stopTimer(string sName);

/* }}} */

}; //class Timer

} //namespace ICL


#endif //ICL_TIME_H
