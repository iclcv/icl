// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLUtils/ProcessMonitor.h>
#include <ICLUtils/StringUtils.h>

#include <fstream>
#include <cstdio>

//#include <sys/time.h>
//#include <sys/resource.h>
#include <sys/types.h>
#ifdef ICL_SYSTEM_WINDOWS
  #include <process.h>
#else
  #include <unistd.h>
#include <mutex>
#endif

namespace icl::utils {
  struct ProcessMonitor::Data{
    std::recursive_mutex mutex;
    ProcessMonitor::Info info;
    FILE *pipe;
    std::vector<std::pair<int,ProcessMonitor::Callback> > callbacks;
    int nextCallbackID;

    Data():pipe(0),nextCallbackID(0){}

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

    void parse_top_line_cpus(const std::string &top){
      std::vector<std::string> ts = tok(top," \t");
      try{
        info.allCpuUsage = 100.0 - parse<float>(ts.at(4));
      }catch(...){}
    }

    void call_callbacks(){
      for(unsigned int i=0;i<callbacks.size();++i){
        callbacks[i].second(info);
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
    size_t fread_result = fread(res, 1, sizeof(res)-1, fp);
    static_cast<void>(fread_result);
    fclose(fp);
    m_data->info.numCPUs = parse<int>(res);
#endif
    start();
  }

  ProcessMonitor::~ProcessMonitor(){
    stop();
#ifdef ICL_SYSTEM_WINDOWS
    if(m_data->pipe) _pclose(m_data->pipe);
#else
    if (m_data->pipe) pclose(m_data->pipe);
#endif
    delete m_data;
  }

  static std::string strip_line(const std::string &s){
    int i=0;
    for(;s.length() && s[i]==' '; ++i);
    return s.substr(i);
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
        if(line.substr(0,7) == "Cpu(s):"){
          std::lock_guard<std::recursive_mutex> l(m_data->mutex);
          m_data->parse_top_line_cpus(line);
        }
        if(strip_line(line).substr(0,pid.length()) == pid){
          std::lock_guard<std::recursive_mutex> l(m_data->mutex);
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
    std::lock_guard<std::recursive_mutex> l(m_data->mutex);
    return m_data->info;
  }

  int ProcessMonitor::registerCallback(ProcessMonitor::Callback cb){
    std::lock_guard<std::recursive_mutex> l(m_data->mutex);
    m_data->callbacks.push_back(std::make_pair(m_data->nextCallbackID++,cb));
    return m_data->callbacks.back().first;
  }

  void ProcessMonitor::removeCallback(int id){
    std::lock_guard<std::recursive_mutex> l(m_data->mutex);
    for(unsigned int i=0;i<m_data->callbacks.size();++i){
      if(m_data->callbacks[i].first == id){
        m_data->callbacks.erase(m_data->callbacks.begin()+i);
        return;
      }
    }
    WARNING_LOG("tryed to removed callback with id " << id << " which was not registered before!");
  }

  void ProcessMonitor::removeAllCallbacks(){
    std::lock_guard<std::recursive_mutex> l(m_data->mutex);
    m_data->callbacks.clear();
  }

  std::ostream &operator<<(std::ostream &s, const ProcessMonitor::Info &info){
#ifdef ICL_SYSTEM_WINDOWS
    return s << "The ProcessMonitor class is not available for Windows";
#else
    return s << "Current Process Information (pid: " << info.pid << ")\n"
             << "thread count : " << info.numThreads << std::endl
             << "num cores    : " << info.numCPUs << std::endl
             << "cpu usage    : " << info.cpuUsage << " %" << std::endl
             << "all cpu usage: " << info.allCpuUsage << " %" << std::endl
             << "memory usage : " << info.memoryUsage << " MB" << std::endl;
#endif
  }

  ProcessMonitor *ProcessMonitor::getInstance(){
    static ProcessMonitor pm;
    return &pm;
  }
  } // namespace icl::utils