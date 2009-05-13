#ifndef ICL_STACKTIMER_H
#define ICL_STACKTIMER_H


#include <iclTimer.h>
#include <string>
#include <iclMacros.h>
#include <cstdio>

namespace icl{
  /// Tool for benchmarking method calls \ingroup TIME
  /** The stack timer class is an Extension of the Timer class, 
      that provides very convenient function benchmarking 
      functionality.
      The only thing, a programmer needs to do, is to write the
      Macro <b><em> BENCHMARK_THIS_FUNCTION  </em></b> at the
      beginning of the function(s), that should be benchmarked.
      The underlying implementation will then record each function
      call and execution time.
      At the end of the programm, the implicitly crated
      StackTimerNotifier will print the following information 
      about the benchmarked function:
      - execution count
      - overall execution time
      - average execution time
      - minimun execution time
      - maximum execution time
      - the function name (nice, isn't it)

      See also the following example:
      <pre>
      void funcion1(...){
         BENCHMARK_THIS_FUNCTION;
         ...
         ...
      }
      void funcion2(...){
         BENCHMARK_THIS_FUNCTION;
         ...
         ...
      }
      </pre>
      At the end of the programm, the following info could be
      given to you:
      <pre>
      calls[     361]  time[ 77.7 ms]  avg[  215 ns]  min[  183 ns]  max[  5.2 ms] {function1}
      calls[     361]  time[  1.7 ms]  avg[    4 ns]  min[    4 ns]  max[   51 ns] {function2}
      </pre>
  }
  */
  class StackTimer{
  public:
    /// StackTimerNotifier constructor, USE BENCHMARK_THIS_FUNCTION-MACRO instead
    class StackTimerNotifier{
      public:
      StackTimerNotifier(const char* functionname, 
                         bool writeCounts=true, 
                         bool writeAvg=true,
                         bool writeMin=true,
                         bool writeMax=true){
        m_bWriteCounts = writeCounts;
        m_bWriteAvg = writeAvg;
        m_bWriteMin = writeMin;
        m_bWriteMax = writeMax;
        m_liCount = 0;
        m_liTime = 0;
        m_sFunctionName = functionname;
        m_liMaxTime = 0;
        m_liMinTime = 2<<29;
      }
      std::string getTimeStr(long int l){
        static char acBuf[100];
        if(l>1000000){
          sprintf(acBuf,"%3ld.%1ld  s",l/1000000,l/100000-10*(l/1000000));
        }else if(l>1000){
          sprintf(acBuf,"%3ld.%1ld ms",l/1000,l/100-10*(l/1000));
        }else{
          sprintf(acBuf,"  %3ld ns",l);
        }
        return std::string(acBuf);
      }
      /// StackTimerNotifier destructor, USE BENCHMARK_THIS_FUNCTION-MACRO instead
      ~StackTimerNotifier(){
        if(m_bWriteCounts){
          printf("calls[%8ld]  ",m_liCount);
        }
        printf("time[%s]  ",getTimeStr(m_liTime).c_str());
        if(m_bWriteAvg){
          printf("avg[%s]  ",getTimeStr(m_liTime/m_liCount).c_str());
        }
        if(m_bWriteMin){
          printf("min[%s]  ",getTimeStr(m_liMinTime).c_str());
        } 
        if(m_bWriteMax){
          printf("max[%s]  ",getTimeStr(m_liMaxTime).c_str());
        }
        printf("{%s}\n",m_sFunctionName.c_str());
      }
      /// increments the execution counter, USE BENCHMARK_THIS_FUNCTION-MACRO instead
      void incCount(){
        m_liCount++;
      }
      /// adds execution time, USE BENCHMARK_THIS_FUNCTION-MACRO instead
      void incTime(long int dt){
        m_liTime+=dt;
        m_liMinTime = iclMin(dt,m_liMinTime);
        m_liMaxTime = iclMax(dt,m_liMaxTime);
      }
      private:
      long int m_liCount;
      long int m_liTime;
      long int m_liMaxTime;
      long int m_liMinTime;
      std::string m_sFunctionName;

      bool m_bWriteCounts;
      bool m_bWriteAvg;
      bool m_bWriteMin;
      bool m_bWriteMax;
    };
    /// StackTimer constructor, USE BENCHMARK_THIS_FUNCTION-MACRO instead
    StackTimer(StackTimerNotifier *notifier){
      m_poTimer = new Timer(1);//1=ns
      m_poNotifier = notifier;
      notifier->incCount();
      m_poTimer->start();
    }
    /// StackTimerNotifier destructor, USE BENCHMARK_THIS_FUNCTION-MACRO instead
    ~StackTimer(){
      m_poNotifier->incTime(m_poTimer->stopSilent());
      delete m_poTimer;
    }
    private:
    Timer *m_poTimer;
    StackTimerNotifier* m_poNotifier;
   
  };
  
  
#define BENCHMARK_THIS_FUNCTION                                         \
  static StackTimer::StackTimerNotifier __notifier(__FUNCTION__);       \
  StackTimer __stacktimer(&__notifier);
  
#define BENCHMARK_THIS_FUNCTION_LITE                                       \
  static StackTimer::StackTimerNotifier __notifier(__FUNCTION__,0,0,0,0);  \
  StackTimer __stacktimer(&__notifier);
}
#endif
