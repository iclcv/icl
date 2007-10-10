#ifndef ICL_MULTI_THREADER_H
#define ICL_MULTI_THREADER_H

#include <iclSemaphore.h>
#include <iclMutex.h>
#include <iclShallowCopyable.h>
#include <vector>

namespace icl{

  /** \cond */
  class MultiThreaderImpl;
  class MultiThreaderImplDelOp{
    public: static void delete_func(MultiThreaderImpl *impl);
  };
  /** \endcond */
  
  /// Utility class for parallelizing algorithms \ingroup THREAD
  /** \section OV Overview
      The Multithreader class provides a simple interface to parallelize algorithms.
      To achieve a multi-threaded implementation of an algorithm, you have to split
      the computation loop into N parts (Work-Packages or short Work). Each of this
      Works will be computed in a single thread internally when given to the apply
      operator of the MultiThreader. \n
      
      \section __EX Example
      The following example explains how to parallelize a simple function-call
      \code
      #include <iclQuick.h>
      #include <math.h>
      #include <iclMultiThreader.h>
      
      // a simple function calculating the l2 norm on a data array
      void l2norm_vec(float *dataStart,float *dataEnd, int dimPerElem, float *dst){
        for(;dataStart<dataEnd;dst++){
          float accu = 0; 
          for(int i=0;i<dimPerElem;i++,dataStart++){
            accu += (*dataStart) * (*dataStart);
          }
          *dst = sqrt(accu);
        }
      }
      
      // Wrapper for this function (implementing the MultiThreader::Work interface)
      struct L2NormWork : public MultiThreader::Work{
        L2NormWork(float *dataStart,float *dataEnd, int dimPerElem, float *dst):
          dataStart(dataStart),dataEnd(dataEnd),dimPerElem(dimPerElem),dst(dst){
        }
        float *dataStart,*dataEnd;
        int dimPerElem;
        float *dst;
      
        // interface function -> calls the wrapped function
        virtual void perform(){
          l2norm_vec(dataStart,dataEnd,dimPerElem,dst);
        }
      };
      

      int main(){
        // creating some really BIG data array
        const int DIM = 10000000;
        // dimension of each data element
        int dim = 100;
        float *data = new float[DIM];
        float *dst = new float[DIM/dim];
      
        // apply single threaded 100 times:
        tic();
        for(int i=0;i<100;i++){
          l2norm_vec(data,data+DIM,dim,dst);
        }
        toc();  
      
        //--- multi threaded part --------------------------
      
        // create a MultiThreader with 2 Threads
        MultiThreader mt(2);
      
        // create a WorkSet with 2 work packages
        MultiThreader::WorkSet ws(2);
      
        // 1st package: first half of the array
        ws[0] = new L2NormWork(data,data+DIM/2,dim,dst);
      
        // 2nd package: second half of the array
        ws[1] = new L2NormWork(data+DIM/2,data+DIM,dim,dst+DIM/2);
      
        // apply 100 times multi-threaded in 2 Threads
        tic();
        for(int i=0;i<100;i++){
          mt(ws);
        }
        toc();
      
        // release the work instances
        delete ws[0];
        delete ws[1];
        delete [] data;
        delete [] dst;
        return 0;
      }
      \endcode
      
      \section RES Result
      The following benchmark results were obtained:
      - <b>pentium M 1.6 GHz</b>
        - Single Threaded: <b>2672 ms</b>
        - Multi Threaded: <b>2689 ms</b> (rarely slower)
      - <b>Core 2 Duo E6600 2.4 GHz</b>
        - Single Threaded: <b>1348 ms</b>
        - Multi Threaded: <b>692 ms</b> (about 94 percent faster)

      \section USAB Usability
      However, the MultiThreader provides a powerful interface for
      parallelizing code, it is still a bit inconvenient to use. The
      Example above has shown, that the programmer has to write about 
      10 additional line for the function wrapper and another 4 lines 
      to create the WorkSet and to fill the MultiThreader instance 
      with it.\n
      For more convenience some additional high level classes and functions
      should be implemented. For instance the SplittedUnaryop class of the
      ICLFilter package, which provides a top level interface for parallelizing
      unary operators (class interface UnaryOp)
  */
  class MultiThreader : public ShallowCopyable<MultiThreaderImpl,MultiThreaderImplDelOp>{
    public:
    
    /// plugin class for work packages performed parallel
    class Work{
      public:
      /// virtual destructor doing nothing
      virtual ~Work(){}
      /// abstract working function
      virtual void perform()=0;
    };

    /** \cond */
    /// internally used working Thread class
    class MTWorkThread;
    /** \endcond */

    /// set of work packages, that should be performed parallel
    typedef std::vector<Work*> WorkSet;
    
    /// Empty (null) constructor
    MultiThreader();
    
    /// Default constructor with defined set of working threads
    MultiThreader(int nThreads);
    
    /// applying operator (performs each Work* element of ws parallel)
    void operator()(WorkSet &ws);
    
    /// returns the number of WorkThreads
    int getNumThreads() const;
  };
}
#endif
