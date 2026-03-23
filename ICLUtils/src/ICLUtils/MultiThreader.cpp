/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/MultiThreader.cpp                **
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

#include <ICLUtils/MultiThreader.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Thread.h>
#include <ICLUtils/Semaphore.h>
#include <mutex>


namespace icl{
  namespace utils{
    class MultiThreader::MTWorkThread : public Thread{
    public:

      MTWorkThread(Semaphore *sem1, Semaphore *sem2, Semaphore *semDone):work(0),m_bCurr(0){

        m_apoSems[0] = sem1;
        m_apoSems[1] = sem2;
        m_poSemDone = semDone;
        m_bEnd = false;
      }


      ~MTWorkThread(){

        m_oWorkMutex.lock();
        work = 0;
        m_oWorkMutex.unlock();
        m_bEnd = true;
        m_apoSems[m_bCurr]->release(1);
      }


      virtual void run(){

        while(!m_bEnd){
          m_apoSems[m_bCurr]->acquire(1);
          Thread::usleep(0);
          m_oWorkMutex.lock();
          Thread::usleep(0);
          if(work){

            work->perform();
          }
          Thread::usleep(0);
          m_oWorkMutex.unlock();
          Thread::usleep(0);
          m_poSemDone->release(1);
          Thread::usleep(0);
          m_bCurr = !m_bCurr;

          Thread::usleep(0);
        }
      }


      void setWork(MultiThreader::Work *work){

        m_oWorkMutex.lock();
        this->work = work;
        m_oWorkMutex.unlock();
      }


    private:
      MultiThreader::Work *work;
      Semaphore *m_apoSems[2];
      Semaphore *m_poSemDone;
      std::recursive_mutex m_oWorkMutex;
      bool m_bCurr;
      bool m_bEnd;
    };


    class MultiThreaderImpl{

    public:
      MultiThreaderImpl(int nThreads):

        m_iNThreads(nThreads),m_bCurr(0){
        m_vecThreads.resize(nThreads);

        m_apoSems[0] = new Semaphore(nThreads);
        m_apoSems[1] = new Semaphore(nThreads);
        m_poDoneSem = new Semaphore(nThreads);


        m_apoSems[0]->acquire(nThreads);
        m_apoSems[1]->acquire(nThreads);
        m_poDoneSem->acquire(nThreads);

        for(int i=0;i<nThreads;++i){
          m_vecThreads[i] = new MultiThreader::MTWorkThread(m_apoSems[0],m_apoSems[1],m_poDoneSem);
          m_vecThreads[i]->start();
        }
      }

      ~MultiThreaderImpl(){

        for(int i=0;i<m_iNThreads;++i){
          delete m_vecThreads[i];
        }
        delete m_apoSems[0];
        delete m_apoSems[1];
      }


      inline void apply(MultiThreader::WorkSet &ws){

        ICLASSERT_RETURN(static_cast<int>(ws.size()) == m_iNThreads);

        /// arm the threads
        for(unsigned int i=0;i<ws.size();i++){
          m_vecThreads[i]->setWork(ws[i]);
        }

        m_apoSems[m_bCurr]->release(m_iNThreads);
        m_poDoneSem->acquire(m_iNThreads);

        /// defuse the threads !
        for(unsigned int i=0;i<ws.size();i++){
          m_vecThreads[i]->setWork(0);
        }

        m_bCurr =! m_bCurr;
      }


      inline int getNumThreads() const {

        return m_iNThreads;
      }


    private:
      int m_iNThreads;
      std::vector<MultiThreader::MTWorkThread*> m_vecThreads;
      Semaphore *m_apoSems[2];
      Semaphore *m_poDoneSem;
      std::recursive_mutex m_oSemMutex;
      bool m_bCurr;

    };


    MultiThreader::MultiThreader() = default;

    MultiThreader::MultiThreader(int nThreads):
      impl(std::make_shared<MultiThreaderImpl>(nThreads)){
    }

    void MultiThreader::operator()(MultiThreader::WorkSet &ws){
      ICLASSERT_RETURN(impl);
      impl->apply(ws);
    }

    int MultiThreader::getNumThreads() const{
      ICLASSERT_RETURN_VAL(impl,0);
      return impl->getNumThreads();
    }

  } // namespace utils
}
