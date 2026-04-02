// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLUtils/Thread.h>
#include <ICLUtils/Macros.h>
#include <chrono>

namespace icl::utils {
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

  } // namespace icl::utils