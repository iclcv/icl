/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/Time.h                                **
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

#include <string>
#include <stdint.h>
// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICL_UTIL_TIME_H
#define ICL_UTIL_TIME_H


namespace icl{

  /// ICL Time class (taken from the Ice lib) \ingroup TIME
  class Time {
    public:

    /// internal data type (64Bit integer)
    typedef int64_t value_type;
    
    // undefined time: 0
    static const Time null;
    
    Time();
    
    Time(value_type);
    
    // No copy constructor and assignment operator necessary. The
      // automatically generated copy constructor and assignment
      // operator do the right thing.
    
      static Time now();
      static Time seconds(value_type);
      static Time milliSeconds(value_type);
      static Time microSeconds(value_type);
    
      value_type toSeconds() const;
      value_type toMilliSeconds() const;
      value_type toMicroSeconds() const;
         
      double toSecondsDouble() const;
      double toMilliSecondsDouble() const;
      double toMicroSecondsDouble() const;

      std::string toString() const;
      
      /// allows to create a formated string using strftime system-functions
      /** Please refer to your system dependent strftime reference:
          <b>please note, that strftime is does not support milli and micro-second accuracy,
          </b>. So this feature is implemented here.
          Please use
          - %* for the remaining usecs less then the last second
          - %# for the remaining millisecond less then the last second
          - %- for the remaining usecs less then the last millisecond

          For example, the default toString() functions uses this time patterns: "%x %H:%M:%S:%#"
          
      */
      std::string toStringFormated(const std::string &fmt,unsigned int bufferSize=32) const;

      //xcf4cis stuff
      Time age() const {
         return Time::microSeconds(Time::now().m_usec - m_usec);
      }

      Time operator-() const
         {
            return Time(-m_usec);
         }

      Time operator-(const Time& rhs) const
         {
            return Time(m_usec - rhs.m_usec);
         }

      Time operator+(const Time& rhs) const
         {
            return Time(m_usec + rhs.m_usec);
         }

      Time& operator+=(const Time& rhs)
         {
            m_usec += rhs.m_usec;
            return *this;
         }

      Time& operator-=(const Time& rhs)
         {
            m_usec -= rhs.m_usec;
            return *this;
         }

      bool operator<(const Time& rhs) const
         {
            return m_usec < rhs.m_usec;
         }

      bool operator<=(const Time& rhs) const
         {
            return m_usec <= rhs.m_usec;
         }

      bool operator>(const Time& rhs) const
         {
            return m_usec > rhs.m_usec;
         }

      bool operator>=(const Time& rhs) const
         {
            return m_usec >= rhs.m_usec;
         }

      bool operator==(const Time& rhs) const
         {
            return m_usec == rhs.m_usec;
         }

      bool operator!=(const Time& rhs) const
         {
            return m_usec != rhs.m_usec;
         }

      Time& operator*=(const Time& rhs)
         {
            m_usec *= rhs.m_usec;
            return *this;
         }

      Time operator*(const Time& rhs) const
         {
            Time t;
            t.m_usec = m_usec * rhs.m_usec;
            return t;
         }

      Time& operator/=(const Time& rhs)
         {
            m_usec /= rhs.m_usec;
            return *this;
         }

      Time operator/(const Time& rhs) const
         {
            Time t;
            t.m_usec = m_usec / rhs.m_usec;
            return t;
         }

      Time& operator*=(int rhs)
         {
            m_usec *= rhs;
            return *this;
         }

      Time operator*(int rhs) const
         {
            Time t;
            t.m_usec = m_usec * rhs;
            return t;
         }

      Time& operator/=(int rhs)
         {
            m_usec /= rhs;
            return *this;
         }

      Time operator/(int rhs) const
         {
            Time t;
            t.m_usec = m_usec / rhs;
            return t;
         }

      Time& operator*=(value_type rhs)
         {
            m_usec *= rhs;
            return *this;
         }

      Time operator*(value_type rhs) const
         {
            Time t;
            t.m_usec = m_usec * rhs;
            return t;
         }

      Time& operator/=(value_type rhs)
         {
            m_usec /= rhs;
            return *this;
         }

      Time operator/(value_type rhs) const
         {
            Time t;
            t.m_usec = m_usec / rhs;
            return t;
         }

      Time& operator*=(double rhs)
         {
            m_usec = static_cast<value_type>(static_cast<double>(m_usec) * rhs);
            return *this;
         }

      Time operator*(double rhs) const
         {
            Time t;
            t.m_usec = static_cast<value_type>(static_cast<double>(m_usec) * rhs);
            return t;
         }

      Time& operator/=(double rhs)
         {
            m_usec = static_cast<value_type>(static_cast<double>(m_usec) / rhs);
            return *this;
         }

      Time operator/(double rhs) const
         {
            Time t;
            t.m_usec = static_cast<value_type>(static_cast<double>(m_usec) / rhs);
            return t;
         }

      friend std::ostream& operator<<(std::ostream&, const Time&);
      
      friend std::istream& operator>>(std::istream&, Time&);

   private:

      value_type m_usec;
   };
  
   /// writes Time instances value type into the stream
   std::ostream& operator<<(std::ostream&, const Time&);

   /// reads Time instances value type from the stream
   std::istream& operator>>(std::istream&, Time&);

} // End namespace icl

#endif
