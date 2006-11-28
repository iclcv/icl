/*
  Timer.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include <Timer.h>
using namespace std;

//---- ICL in its own namespace ----
namespace icl {

// {{{ Konstruktor/ Destruktor

Timer::Timer(int iTimerMode)
{
  FUNCTION_LOG("Timer mode: " << iTimerMode);
  m_iTimerMode = iTimerMode;

  m_vecTime.clear();
  m_vecTimerName.clear();
}

// }}}

// {{{ Timer functions

void Timer::startTimer()
{
  FUNCTION_LOG("string");
  m_vecTime.push_back(getTime());
  m_vecTimerName.push_back("_START_");
}

void Timer::stopSubTimer(string sName)
{
  FUNCTION_LOG("");
  m_vecTime.push_back(getTime());
  m_vecTimerName.push_back(sName);  
}

void Timer::stopTimer(string sName)
{
  FUNCTION_LOG("string");

  long int lTmpTimeDiff = 0;
  
  m_vecTime.push_back(getTime());
  m_vecTimerName.push_back(sName);  

  cout << endl;
  cout << " ------------------------------------------- " << endl;
  cout << " --             Time measure              -- " << endl;
  
  if (m_vecTime.size() > 2) {
    for (unsigned int i=1;i<m_vecTime.size();i++)
    {
      lTmpTimeDiff = m_vecTime[i] - m_vecTime[i-1];
      
      switch(m_iTimerMode)
      {
        case 0:
          cout << " --  [" << m_vecTimerName[i] << "] -> Time: " 
               << lTmpTimeDiff << " ms" << endl;
          break;
          
        case 1:
          cout << " --  [" << m_vecTimerName[i] << "] -> Time: "
               << lTmpTimeDiff << " ns" << endl;
          break;
      }
    }    
  }
  
  lTmpTimeDiff = m_vecTime[m_vecTime.size()-1] - m_vecTime[0];
  
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

// }}}

// {{{ Misc. functions

long int Timer::getTime()
{
  FUNCTION_LOG("");
  struct timeval tv;
  gettimeofday( &tv, 0 );
  
  switch (m_iTimerMode)
  {
    case 0: //ms
      return tv.tv_sec * 1000 + tv.tv_usec / 1000;
      
    default: //ns
      return tv.tv_sec * 1000000 + tv.tv_usec;
  }
}

// }}}

} //namespace
