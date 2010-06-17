/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ThreadUtils.cpp                           **
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

