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
