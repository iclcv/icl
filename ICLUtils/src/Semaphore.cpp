/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/Semaphore.cpp                             **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
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

#include <ICLUtils/Semaphore.h>
#include <semaphore.h>
#include <ICLUtils/Macros.h>

namespace icl{
  

  class SemaphoreImpl{
  public:
    inline SemaphoreImpl(int n):n(n){
      ICLASSERT_RETURN(n>0);
	  #ifndef ICL_SYSTEM_WINDOWS
      sem_init(&s,0,(unsigned int)n);
	  #else
	  
	  #endif
    }
    inline ~SemaphoreImpl(){
	  #ifndef ICL_SYSTEM_WINDOWS
      sem_destroy(&s);
	  #else
	  
	  #endif
    }
    inline void inc(int n=1){
      for(int i=0;i<n;i++){
	    #ifndef ICL_SYSTEM_WINDOWS
        sem_post(&s);
		#else
		
		#endif
      }
    }
    inline void dec(int n=1){
      for(int i=0;i<n;i++){
        #ifndef ICL_SYSTEM_WINDOWS
		sem_wait(&s);
		#else
		
		#endif
      }
    }
    inline int getValue(){
      int val = 0;
	  #ifndef ICL_SYSTEM_WINDOWS
      sem_getvalue(&s,&val);
	  #else
	  
	  #endif
      return val;
    }
    inline int getMaxValue(){
      return n;
    }

    inline bool tryAcquire(){
	  #ifndef ICL_SYSTEM_WINDOWS
      int ret = sem_trywait(&s);
	  #else
	  int ret = 0;
	  #endif
      return ret == 0;
    }
    inline bool tryRelease(){
	  #ifndef ICL_SYSTEM_WINDOWS
      int ret = sem_post(&s);
	  #else
	  int ret = 0;
	  #endif
      return ret == 0;
    }
  private:
    sem_t s;
    int n;
  };
  
  void SemaphoreImplDelOp::delete_func(SemaphoreImpl *impl){
    ICL_DELETE(impl);
  }
  
  
  Semaphore::Semaphore(int n):
    ShallowCopyable<SemaphoreImpl,SemaphoreImplDelOp>(new SemaphoreImpl(n)){
    
  }
  
  void Semaphore::operator++(int dummy){
    (void)dummy;
    impl->inc(1);
  }
  void Semaphore::operator--(int dummy){
    (void)dummy;
    impl->dec(1);
  }
  void Semaphore::operator+=(int val){
    impl->inc(val);
  }
  void Semaphore::operator-=(int val){
    impl->dec(val);
  }
  bool Semaphore::tryAcquire(){
    return impl->tryAcquire();
  }
 
  bool Semaphore::tryRelease(){
    return impl->tryRelease();
  }
  
  int Semaphore::getValue(){
    return impl->getValue();
  } 
  int Semaphore::getMaxValue(){
    return impl->getMaxValue();
  }
}
