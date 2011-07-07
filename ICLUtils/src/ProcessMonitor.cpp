/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ProcessMonitor.cpp                        **
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

#include <ICLUtils/ProcessMonitor.h>
#include <ICLUtils/StringUtils.h>

#include <fstream>
#include <cstdio>

//#include <sys/time.h>
//#include <sys/resource.h>
//#include <sys/types.h>
//#include <unistd.h>

namespace icl{
  
  struct ProcessMonitor::Data{
    Mutex mutex;
    ProcessMonitor::Info info;
    FILE *pipe;
    std::vector<ProcessMonitor::Callback> callbacks;
    
    void evaluate_proc(){
      std::ifstream proc("/proc/self/status");
      std::string s;
      while(getline(proc, s), !proc.fail()) {
        if(s.substr(0,8) == "Threads:"){
          info.numThreads = parse<int>(s.substr(9));
        }else if(s.substr(0, 7) == "VmSize:") {
          info.memoryUsage = parse<float>(s.substr(8)) / 1000.0f;
        }
      }
    }
    void parse_top_line(const std::string &top){
      std::vector<std::string> ts = tok(top," \t");
      try{
        info.cpuUsage = parse<float>(ts.at(8));
      }catch(...){}
    }
    
    void call_callbacks(){
      for(unsigned int i=0;i<callbacks.size();++i){
        callbacks[i](info);
      }
    }
    
  };
    
  ProcessMonitor::ProcessMonitor(){
    m_data = new Data;
    m_data->info.pid = getpid();
#ifndef ICL_SYSTEM_WINDOWS
    FILE * fp;
    char res[128] = {0};
    fp = popen("/bin/cat /proc/cpuinfo |grep -c '^processor'","r");
    fread(res, 1, sizeof(res)-1, fp);
    fclose(fp);
    std::cout << "number of cores: " << res[0] << std::endl;
    std::cout << "number of cores: " << res << std::endl;
#endif
    start();
  }

  ProcessMonitor::~ProcessMonitor(){
    stop();
    if(m_data->pipe) pclose(m_data->pipe);
    delete m_data;
  }
  
  void ProcessMonitor::run(){
#ifndef ICL_SYSTEM_WINDOWS
    const static std::string pid = str(getpid());
    const static std::string &command = "top -bd 0.1 -p "+pid;

    m_data->pipe = popen(command.c_str(), "r");
    if (!m_data->pipe) {
      ERROR_LOG("ProcessMonitor::run: unable to open pipe to 'top'-command");
      Thread::exit();
    }
    std::string line;
    while(!feof(m_data->pipe)) {
      char c = getc(m_data->pipe);
      line += c;
      if (c == '\n'){
        if(line.substr(0,pid.length()) == pid){
          Mutex::Locker l(m_data->mutex);
          m_data->parse_top_line(line);
          m_data->evaluate_proc();
          m_data->call_callbacks();
        }
        line.clear();
      }
    }
    pclose(m_data->pipe);
    m_data->pipe = 0;
#else
    WARNING_LOG("this tool is not avaialble for Windows");
#endif
  }
  
  ProcessMonitor::Info ProcessMonitor::getInfo() const {
    Mutex::Locker l(m_data->mutex);
    return m_data->info;
  }

  void ProcessMonitor::registerCallback(ProcessMonitor::Callback cb){
    Mutex::Locker l(m_data->mutex);
    m_data->callbacks.push_back(cb);
  }
  
  void ProcessMonitor::removeAllCallbacks(){
    Mutex::Locker l(m_data->mutex);
    m_data->callbacks.clear();
  }

  std::ostream &operator<<(std::ostream &s, const ProcessMonitor::Info &info){
#ifdef ICL_SYSTEM_WINDOWS
    return s << "The ProcessMonitor class is not available for Windows";
#else
    return s << "Current Process Information (pid: " << info.pid << ")\n"
             << "thread count : " << info.numThreads << std::endl
             << "cpu usage    : " << info.cpuUsage << " %" << std::endl
             << "memory usage : " << info.memoryUsage << " MB" << std::endl;
#endif
  }
}
