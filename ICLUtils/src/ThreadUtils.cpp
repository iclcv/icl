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

#include <ICLUtils/ThreadUtils.h>
#include <vector>

namespace icl{

  struct ExecThreadImpl : public Thread{
    std::vector<ExecThread::callback> cbs;
    bool looped;
    
    inline void one_cycle(){
      for(unsigned int i=0;i<cbs.size();++i){
        cbs[i]();
      }
      /// allow thread join from outside!
      usleep(0);
    }
    
    virtual void run(){
      if(looped){
        while(true){
          one_cycle();
        }
      }else{
        one_cycle();
      }
    }
  };
  
  ExecThread::ExecThread(ExecThread::callback a, 
                         ExecThread::callback b, 
                         ExecThread::callback c, 
                         ExecThread::callback d, 
                         ExecThread::callback e,
                         ExecThread::callback f) throw (ICLException){
    
    if(!a) throw ICLException("ExecThread called with NULL function!");
    
    impl = new ExecThreadImpl;
    impl->cbs.push_back(a);
    
    if(b) impl->cbs.push_back(b);
    if(c) impl->cbs.push_back(c);
    if(d) impl->cbs.push_back(d);
    if(e) impl->cbs.push_back(e);
    if(f) impl->cbs.push_back(f);
  }
  
 
  ExecThread::~ExecThread(){
    ICL_DELETE(impl);
  }
  
  void ExecThread::run(bool looped){
    impl->looped = looped;
    impl->start();
  }


  // older misc

  namespace{
    static std::vector<Thread*> g_registered_threads_vec;
  }
  
  void register_thread(Thread *t){
    g_registered_threads_vec.push_back(t);
  }
  
  void stop_all_threads(){
    for(unsigned int i=0;i<g_registered_threads_vec.size();++i){
      g_registered_threads_vec[i]->stop();
    }
  }


}

