/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Thread.cpp                       **
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

#include <ICLUtils/Thread.h>
#include <ICLUtils/Macros.h>
#include <chrono>

namespace icl{
  namespace utils{

    Thread::Thread() = default;

    Thread::~Thread(){
      stop();
    }

    void Thread::start(){
      std::lock_guard<std::mutex> l(m_lifecycle);
      if(m_running){
        ERROR_LOG("unable to start thread (it's still running)");
        return;
      }
      m_running = true;
      m_thread = std::thread([this]{ run(); });
    }

    void Thread::stop(){
      m_running = false;
      std::lock_guard<std::mutex> l(m_lifecycle);
      if(m_thread.joinable()){
        m_thread.join();
        finalize();
      }
    }

    void Thread::wait(){
      std::lock_guard<std::mutex> l(m_lifecycle);
      if(m_thread.joinable()){
        m_thread.join();
        m_running = false;
        finalize();
      }
    }

    void Thread::lock(){ m_mutex.lock(); }
    int Thread::trylock(){ return m_mutex.try_lock() ? 0 : 1; }
    void Thread::unlock(){ m_mutex.unlock(); }

    void Thread::join(){
      std::lock_guard<std::mutex> l(m_lifecycle);
      if(m_thread.joinable()) m_thread.join();
    }

    void Thread::exit(){ m_running = false; }

    bool Thread::running() const { return m_running.load(); }
    bool Thread::runningNoLock() const { return m_running.load(); }

    void Thread::usleep(unsigned int usec){
      std::this_thread::sleep_for(std::chrono::microseconds(usec));
    }

    void Thread::msleep(unsigned int msecs){
      std::this_thread::sleep_for(std::chrono::milliseconds(msecs));
    }

    void Thread::sleep(float secs){
      std::this_thread::sleep_for(std::chrono::duration<float>(secs));
    }

  } // namespace utils
}
