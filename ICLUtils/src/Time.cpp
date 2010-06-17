/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/Time.cpp                                  **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLUtils/Time.h>
#include <sstream>
// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifdef SYSTEM_WINDOWS
#   include <time.h>
#   include <sys/timeb.h>
#else
#   include <sys/time.h>
#endif

namespace icl {

   const Time Time::null(0);
   
   Time::Time() :
      m_usec(0)
   {
   }

   Time
   Time::now()
   {
#ifdef SYSTEM_WINDOWS
      struct timeb tb;
      ftime(&tb);
      return Time(tb.time * static_cast<value_type>(1000000) + tb.millitm * static_cast<value_type>(1000));
#else
      struct timeval tv;
      gettimeofday(&tv, 0);
      return Time(tv.tv_sec * static_cast<value_type>(1000000) + tv.tv_usec);
#endif
   }

   Time
   Time::seconds(value_type t)
   {
      return Time(t * static_cast<value_type>(1000000));
   }

   Time
   Time::milliSeconds(value_type t)
   {
      return Time(t * static_cast<value_type>(1000));
   }

   Time
   Time::microSeconds(value_type t)
   {
      return Time(t);
   }

   Time::value_type
   Time::toSeconds() const
   {
      return m_usec / 1000000;
   }

   Time::value_type
   Time::toMilliSeconds() const
   {
      return m_usec / 1000;
   }

   Time::value_type
   Time::toMicroSeconds() const
   {
      return m_usec;
   }

   double
   Time::toSecondsDouble() const
   {
      return m_usec / 1000000.0;
   }

   double
   Time::toMilliSecondsDouble() const
   {
      return m_usec / 1000.0;
   }

   double
   Time::toMicroSecondsDouble() const
   {
      return static_cast<double>(m_usec);
   }

   std::string
   Time::toString() const{
     return toStringFormated("%x %H:%M:%S:%#",32);
   }
  /*
      time_t time = static_cast<long>(m_usec / 1000000);

      struct tm* t;
#ifdef SYSTEM_WINDOWS
      t = localtime(&time);
#else
      struct tm tr;
      localtime_r(&time, &tr);
      t = &tr;
#endif

      char buf[32];
      strftime(buf, sizeof(buf), "%x %H:%M:%S", t);

     std::string buf = toStringFormated("%x %H:%M:%S:",32);
     std::ostringstream os;
     os << buf << ":";
     os.fill('0');
     os.width(3);
     os << static_cast<long>(m_usec % 1000000 / 1000);
      return os.str();
      */
  
  
  std::string Time::toStringFormated(const std::string &pattern,unsigned int bufferSize) const{
    
    std::ostringstream os;

    for(unsigned int i=0;i<pattern.length()-1;++i){
      if(pattern[i]=='%'){
        ++i;
        switch(pattern[i]){
          case '*': os << (m_usec % 1000000); break; 
          case '#': os << (m_usec % 1000000 / 1000); break;
          case '-': os << (m_usec % 1000); break;
          default:
            os << pattern[i-1];
            os << pattern[i];
        }
      }else{
        os << pattern[i];
      }
    }
    
    
    time_t time = static_cast<long>(m_usec / 1000000);
    
    struct tm* t;
#ifdef SYSTEM_WINDOWS
    t = localtime(&time);
#else
    struct tm tr;
    localtime_r(&time, &tr);
    t = &tr;
#endif
    
    char *buf = new char[bufferSize];
    strftime(buf,bufferSize,os.str().c_str(),t);
    std::string s(buf);
    delete [] buf;
    return s;
  }
  
  Time::Time(value_type usec) :
    m_usec(usec)
  {}

   std::ostream& operator<<(std::ostream& out, const Time& t){
     return out << t.m_usec;
   }

  std::istream& operator>>(std::istream& in, Time &t){
    return in >> t.m_usec;
  }
}
