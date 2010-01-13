#ifndef EXTRAPOLATOR_H
#define EXTRAPOLATOR_H

#include <ICLCore/Types.h>

namespace icl{
  
  /// class for linear and quadatic extrapolation \ingroup G_PT
  /** The Extrapolator class is designed as template and abstracts from
      the type of the values as well as from the type that is used to
      indicate time stamps for values.

      The template is explicitly intantiated for value type icl32f
      and the time types long int, int and icl32f
  */
  template<class valueType, class timeType>
  class Extrapolator{
      public:
    /// extrapolates the next value (time t) using linear interpolation
    /** The time series is x_2(t-2),x_1(t-1),result(t) \; with \; dt=1
        @param x2 first value at time t-2
        @param x1 second value at time t-1
        @return extrapolated value at time t
    */
    static  valueType predict(valueType x2, valueType x1);
    
    /// extrapolates the next value using quadratic interpolation
    /** The time series is x3\@t-3 x2\@t-2 x1\@t-1 result\@t with dt=1
        @param x3 second value at time t-3
        @param x2 first value at time t-2
        @param x1 second value at time t-1
        @return extrapolated value at time t
    */

    static  valueType predict(valueType x3, valueType x2, valueType x1);

    /// linear interpolation, but with <em>timestamped</em> value
    /** The time series is ... x2(t2) x1(t1) result(t)
        @param x2 third value at time t2
        @param t2 time stamp for value x2
        @param x1 third value at time t1
        @param t1 time stamp for value x1
        @param t  time stamp for the returned value
        @return extrapolated value at time t
    */
    static  valueType predict(valueType x2, timeType t2, valueType x1, timeType t1, timeType t);

    /// quadratic interpolation, but with <em>timestamped</em> value
    /** The time series is ... x3(t3) x2(t2) x1(t1) result(t)
        @param x3 third value at time t3
        @param t3 time stamp for value x3
        @param x2 third value at time t2
        @param t2 time stamp for value x2
        @param x1 third value at time t1
        @param t1 time stamp for value x1
        @param t  time stamp for the returned value
        @return extrapolated value at time t
    */
    static  valueType predict(valueType x3, timeType t3,valueType x2, timeType t2, valueType x1, timeType t1, timeType t);

    /// generic interpolation function abstracting from number of mesh points availability of timestamps
    /** In valueTypetime environments, not every time 3 mesh points for the extrapolation are available. To 
        avoid the necessity of switching between the cases of a different mesh point counts, this function
        provides a generic interface.
      
        <b>Please considere, that n must be one of {1,2,3}</b> and <b>t must not be 0 if ts is given</b>
     
        If n is 1, the prediction returns the current value *xs.

        If n is 2, the prediction returns the first order extrapolated next value:
        <pre>
        dt1 = t1 - t2              for ts=NULL: return x1 + x1 - p2
        dt0 = t - t1
        v1 = (x1-x2)/dt1
        return x1 + dt0*v1
        </pre>
    
        If n is 3, the prediction returns the second order extraploated next value:
        <pre>
        dt2 = t2 - t3              for ts=NULL: v1 = p1-p2
        dt1 = t1 - t2                           a = v1-( p2-p3 )
        dt0 = t - t1                            return p1 + v1 +  a/2.0
        v2 = (x2-x3)/dt2  
        v1 = (x1-x2)/dt1
        a = (v1-v2)/((dt1+dt2)/2)
        return x1 + dt0*v1 + (dt0*dt0)/2.0 * a
        </pre>
    */
    static  valueType predict(int n, valueType *xs, timeType *ts=0, timeType t=timeType(0));
  };
}

#endif
