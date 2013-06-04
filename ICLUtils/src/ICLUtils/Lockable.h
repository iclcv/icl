/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Lockable.h                       **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/Mutex.h>
#include <ICLUtils/UncopiedInstance.h>

namespace icl{
  namespace utils{
  
    /// Interface for objects, that can be locked using an internal mutex
    class Lockable{
      
      /// wrapped mutex variable
      Mutex *m_mutex;

      public:
      
      /// Default constructor
      Lockable(bool recursive=false):
      m_mutex(new Mutex(recursive ? Mutex::mutexTypeRecursive : Mutex::mutexTypeNormal)){
      }
      
      /// copy constructor (does not copy the source mutex)
      Lockable(const Lockable &l) : m_mutex(new Mutex(l.m_mutex->type)){
        
      }

      /// assignment operator (does not copy the source mutex)
      Lockable &operator=(const Lockable &l){
        delete m_mutex;
        m_mutex = new Mutex(l.m_mutex->type);
        return *this;
      }
      
      /// Destructor
      ~Lockable(){
        delete m_mutex;
      }
      
      /// lock object
      void lock() const { m_mutex->lock(); }
      
      /// unlock object
      void unlock() const { m_mutex->unlock(); }
      
      /// returns mutex of this object
      Mutex &getMutex() const { return *m_mutex; }
    };
    
  } // namespace utils
}

