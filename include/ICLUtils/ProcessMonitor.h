/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/ProcessMonitor.h                      **
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

#ifndef ICL_PROCESS_MONITOR_H
#define ICL_PROCESS_MONITOR_H

#include <ICLUtils/Thread.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Function.h>
#include <iostream>

namespace icl{
  
  /// The ProcessMonitor class grants access to processes memory and CPU usage
  /** Since querying process specific information is not standardized in C++,
      Only a Linux solution is provided yet. 

      \section SRC Source of the Information
      
      * The pid is obtained via getpid() 
      * numThreads and memoryUsage is obtained by parsing /proc/self/stats 
      * memoryUsage is obtained by reading a piped 'ps' processes output
      * since querying information an in particular waiting for 'ps' output
        takes some time, all the information is collected in a dedicated 
        thread. 
      * numCPUs is queried once at construction time by parsing /proc/processor
      
      \section CB Callbacks
      Since sometimes, an application wants to do something, when new information
      is available, a callback mechanism is provided as well. Simply register
      an icl::Function instance that is then called whenever a new set of 
      data is available.
  */
  class ProcessMonitor : protected Thread , public Uncopyable{
    /// internal data structure (pimpl)
    struct Data;
    
    /// internal data pointer
    Data *m_data;
    
    public:
    
    /// Utility struct, that is used to pass available information at once
    struct Info{
      int pid;            // current process ID
      int numThreads;     // number of threads of the current process
      float cpuUsage;     // percent
      float memoryUsage;  // used memory in MB
      int numCPUs;        // somethimes, this helps for visualization
    };
    
    /// Create a new instance
    ProcessMonitor();
    
    /// Destructor
    ~ProcessMonitor();
    
    /// implements the Thread::run method
    virtual void run();
    
    /// synchronized access to last queried data
    Info getInfo() const;
    
    /// callback function/functor type 
    typedef Function<void,const Info&> Callback;

    /// registers a callback instance that is automatically called when new data is available
    void registerCallback(Callback cb);
    
    /// removes all registered callbacks
    void removeAllCallbacks();
  };

  /// overloaded ostream operator for the ProcessMonitor's Info data type
  std::ostream &operator<<(std::ostream &s, const ProcessMonitor::Info &info);
}


#endif
