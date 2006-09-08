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

  long int lTmpTimeDiff = 0;
  
  m_vecTime.push_back(getTime());
  
  if (m_vecTime.size() > 2) {
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

long int Timer::getTime()
{
  FUNCTION_LOG("");
  struct timeval tv;
  gettimeofday( &tv, 0 );
  
  switch (m_iTimerMode)
  {
    case 0:
      return tv.tv_sec * 1000 + tv.tv_usec / 1000;

    default:
      return tv.tv_sec * 1000000 + tv.tv_usec;
  }
}

// }}}

} //namespace
