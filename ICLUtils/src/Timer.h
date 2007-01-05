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

namespace icl {

class Timer
{
 private:
  int m_iTimerMode;
  std::vector<long int> m_vecTime;
  std::vector<std::string> m_vecTimerName;
  
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
