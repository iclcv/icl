/*
  Timer.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "Timer.h"

//---- ICL in its own namespace ----
namespace icl {

// {{{ Konstruktor/ Destruktor

Timer::Timer(string timerName, int iTimerMode)
{
  FUNCTION_LOG("name: " << timerName << "mode: " << iTimerMode);
  m_sTimerName = timerName;
  m_iTimerMode = iTimerMode;
}

// }}}

// {{{ Timer functions

void Timer::startTimer()
{
  FUNCTION_LOG("");
  m_vecTime.clear();
  m_vecTime.push_back(getTime());
}

void Timer::stopSubTimer()
{
  FUNCTION_LOG("");
  m_vecTime.push_back(getTime());
}

void Timer::stopTimer()
{
  FUNCTION_LOG("");

  long lTmpTimeDiff = 0;
  
  m_vecTime.push_back(getTime());
  
  for (unsigned int i=1;i<m_vecTime.size();i++)
  {
    lTmpTimeDiff = m_vecTime[i] - m_vecTime[i-1];
  
    switch(m_iTimerMode)
    {
      case 0:
        cout << " [" << m_sTimerName << "]" << " Computing time (Part " << i 
             << ") = " << lTmpTimeDiff << " ms" << endl;
        break;
        
      case 1:
        cout << " [" << m_sTimerName << "]" << " Computing time (Part " << i 
             << ") = " << lTmpTimeDiff << " ns" << endl;
        break;
    }
  }    
  
  lTmpTimeDiff = m_vecTime[m_vecTime.size()-1] - m_vecTime[0];
  
  switch(m_iTimerMode)
  {
    case 0:
      cout << " [" << m_sTimerName << "]" << " Computing time (Complete) = " 
           << lTmpTimeDiff << " ms" << endl;
      break;
      
    case 1:
      cout << " [" << m_sTimerName << "]" << " Computing time (Complete) = "
           << lTmpTimeDiff << " ns" << endl;
      break;
  }
}

// }}}

// {{{ Misc. functions

long Timer::getTime()
{
  FUNCTION_LOG("");
  long currTime = 0;
  
  struct timeval tv;
  
  gettimeofday( &tv, 0 );
  time_t a = time(0);
  tm *t=localtime(&a);
  
  switch (m_iTimerMode)
  {
    case 0:
      currTime  = 3600000 * t->tm_hour + 60000 * t->tm_min +
        1000 * t->tm_sec + tv.tv_usec / 1000;
      break;

    case 1:
      currTime = 60000000 * t->tm_min + 1000000 * t->tm_sec + tv.tv_usec;
      break;
  }
  
  return currTime;
}

// }}}

} //namespace
