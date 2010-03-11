/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
** University of Bielefeld                                                **
** Contact: nivision@techfak.uni-bielefeld.de                             **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_SEMAPHORE_H
#define ICL_SEMAPHORE_H

#include <ICLUtils/ShallowCopyable.h>


namespace icl{
  
  /** \cond */
  class SemaphoreImpl;
  class SemaphoreImplDelOp{
    public: static void delete_func(SemaphoreImpl *impl);
  };
  /** \endcond */
  
  
  /// Simple Semaphore implementation wrapping the standard linux "sem_t"-struct \ingroup THREAD
  class Semaphore : public ShallowCopyable<SemaphoreImpl,SemaphoreImplDelOp>{
    public:
    /// create a semaphore initialized with n resources
    Semaphore(int n=1);
    
    /// releases a resource
    void operator++(int dummy);
    
    /// acquires a new resource
    void operator--(int dummy);
    
    /// releases val resources
    void operator+=(int val);
    
    /// acquires val resources
    void operator-=(int val);
    
    /// releases val resources
    void acquire(int val=1){ (*this)-=val; }

    /// acquires val resources
    void release(int val=1){ (*this)+=val; }
    
    /// trys to acquire one resource if successfull it returns true else false
    bool tryAcquire();
    
    /// releases one resource only if resources are aqcuired
    bool tryRelease();
    
    /// returns the current value
    int getValue();
    
    /// returns the semaphores max-value
    int getMaxValue();
  };
  
}

#endif
