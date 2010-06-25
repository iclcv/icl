/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/UncopiedInstance.h                    **
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

#ifndef ICL_UNCOPIED_INSTANCE_H
#define ICL_UNCOPIED_INSTANCE_H

namespace icl{
  
  /// Utility class for class instances that are created brand new on copy
  /** Consider the following problem:
      You have a class with some mutex'ed interface
      \code
      class Camera{
         float *data;
         Mutex mutex;
         public:
         void lock(){ mutex.lock(); }
         void unlock() { mutex.unlock(); }
         ...
      };
      \endcode
      As the mutex class is an instance of the Uncopyable interfaces, it cannot 
      be copied. Furthermore, all classes X that have a member of type Mutex are Uncopyable too, unless,
      A special copy constructor and assignment operator for X is defined. This can be bypassed using the
      UncopiedInstance interface.\n
      Due to template based inheritance, Uncopied instances can be used as their (templated) child instances.
      One major drawback is, that the wrapped T instance (wrapped by inheritance) is always constructed using
      the ()-empty constructor.\n
      The Camera class from above can simply use the default copy constructor and assignment operator if we
      use an UncopiedInstance<Mutex> instead of the Mutex class itself.
      \code
      class Camera{
         float *data;
         UncopiedInstance<Mutex> mutex;
         public:
         void lock(){ mutex.lock(); }
         void unlock() { mutex.unlock(); }
         ...
      };
      
  */
  template<class T>
  class UncopiedInstance : public T{
    public:
    
    /// default constructor calls T()
    inline UncopiedInstance():T(){}
    
    /// default copy constructor calls T()
    inline UncopiedInstance(const UncopiedInstance &other):T(){}
    
    /// assignment operator (does NOT call T::operator=(other))
    inline UncopiedInstance &operator=(const UncopiedInstance &other){
      return *this;
    }
  };
}

#endif
