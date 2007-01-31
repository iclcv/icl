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

#include <string>

namespace icl
{

   class Time {
   public:
//
// Definitions of 64-bit integer type used for timestamp
//
#if defined(_MSC_VER)
      typedef __int64       value_type;
#elif defined(__SUNPRO_CC)
      typedef long long     value_type;
#else
      typedef int64_t       value_type;
#endif

	  // undefined time: 0
      static const Time null;
      
      Time();

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

      //xcf4cis stuff
      Time age() const {
         return Time::microSeconds(Time::now()._usec - _usec);
      }

      Time operator-() const
         {
            return Time(-_usec);
         }

      Time operator-(const Time& rhs) const
         {
            return Time(_usec - rhs._usec);
         }

      Time operator+(const Time& rhs) const
         {
            return Time(_usec + rhs._usec);
         }

      Time& operator+=(const Time& rhs)
         {
            _usec += rhs._usec;
            return *this;
         }

      Time& operator-=(const Time& rhs)
         {
            _usec -= rhs._usec;
            return *this;
         }

      bool operator<(const Time& rhs) const
         {
            return _usec < rhs._usec;
         }

      bool operator<=(const Time& rhs) const
         {
            return _usec <= rhs._usec;
         }

      bool operator>(const Time& rhs) const
         {
            return _usec > rhs._usec;
         }

      bool operator>=(const Time& rhs) const
         {
            return _usec >= rhs._usec;
         }

      bool operator==(const Time& rhs) const
         {
            return _usec == rhs._usec;
         }

      bool operator!=(const Time& rhs) const
         {
            return _usec != rhs._usec;
         }

      Time& operator*=(const Time& rhs)
         {
            _usec *= rhs._usec;
            return *this;
         }

      Time operator*(const Time& rhs) const
         {
            Time t;
            t._usec = _usec * rhs._usec;
            return t;
         }

      Time& operator/=(const Time& rhs)
         {
            _usec /= rhs._usec;
            return *this;
         }

      Time operator/(const Time& rhs) const
         {
            Time t;
            t._usec = _usec / rhs._usec;
            return t;
         }

      Time& operator*=(int rhs)
         {
            _usec *= rhs;
            return *this;
         }

      Time operator*(int rhs) const
         {
            Time t;
            t._usec = _usec * rhs;
            return t;
         }

      Time& operator/=(int rhs)
         {
            _usec /= rhs;
            return *this;
         }

      Time operator/(int rhs) const
         {
            Time t;
            t._usec = _usec / rhs;
            return t;
         }

      Time& operator*=(value_type rhs)
         {
            _usec *= rhs;
            return *this;
         }

      Time operator*(value_type rhs) const
         {
            Time t;
            t._usec = _usec * rhs;
            return t;
         }

      Time& operator/=(value_type rhs)
         {
            _usec /= rhs;
            return *this;
         }

      Time operator/(value_type rhs) const
         {
            Time t;
            t._usec = _usec / rhs;
            return t;
         }

      Time& operator*=(double rhs)
         {
            _usec = static_cast<value_type>(static_cast<double>(_usec) * rhs);
            return *this;
         }

      Time operator*(double rhs) const
         {
            Time t;
            t._usec = static_cast<value_type>(static_cast<double>(_usec) * rhs);
            return t;
         }

      Time& operator/=(double rhs)
         {
            _usec = static_cast<value_type>(static_cast<double>(_usec) / rhs);
            return *this;
         }

      Time operator/(double rhs) const
         {
            Time t;
            t._usec = static_cast<value_type>(static_cast<double>(_usec) / rhs);
            return t;
         }

   private:

      Time(value_type);

      value_type _usec;
   };

   std::ostream& operator<<(std::ostream&, const Time&);

} // End namespace icl

#endif
