// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Viktor Richter, Christof Elbrechter

#include <ICLIO/SharedMemorySegment.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/SignalHandler.h>

#include <QtCore/QSharedMemory>
#ifdef ICL_SYSTEM_LINUX
#include <QtCore/QStringList>
#include <QtCore/QProcess>
#endif

#include <map>
#include <stdio.h>
#include <mutex>

using namespace icl;
using namespace icl::utils;

//TODO: QSharedMemory::create recreates the QSystemSemaphore of a QSharedMemory, rendering it useless when it was locked.

namespace icl::io {
  //##########################################################################
  //# constant fields ########################################################
  //##########################################################################

  int SEGMENT_REGISTER_SIZE  = 10000;
  int POLLING_SLEEP_MSEC  = 10;
  std::string ICL_SHARED_MEMORY_REGISTER_NAME = "icl-shared-memory-segment-register";
  std::string ICL_SHARED_MEMORY_INFO_PREFIX = "icl-shred-memory-segment_info.";

  //##########################################################################
  //# SharedMemoryLocker #####################################################
  //##########################################################################

  struct QSharedMemoryLocker{
      QSharedMemory &memory;
      bool locked;
      QSharedMemoryLocker(QSharedMemory &mem):
        memory(mem){
        locked = memory.lock();
      }
      QSharedMemoryLocker(QSharedMemory* mem):
        memory(*mem){
        locked = memory.lock();
      }
      ~QSharedMemoryLocker(){
        if(locked) memory.unlock();
      }
  };

  //##########################################################################
  //# SharedMemorySegmentLocker ##############################################
  //##########################################################################

  SharedMemorySegmentLocker::SharedMemorySegmentLocker(SharedMemorySegment &seg, int minSize):
    segment(seg){
    segment.lock(minSize);
  }

  SharedMemorySegmentLocker::SharedMemorySegmentLocker(SharedMemorySegment* seg, int minSize):
    segment(*seg){
    segment.lock(minSize);
  }

  SharedMemorySegmentLocker::~SharedMemorySegmentLocker(){
    segment.unlock() ;
  }

  //##########################################################################
  //# SharedMemorySegmentImpl ################################################
  //##########################################################################

  struct SharedMemorySegment::Impl{
      friend class SharedMemorySegment;
    private:
      static const int SEGMENT_INFO_SIZE = sizeof(int)
          + sizeof(bool);
      std::recursive_mutex mutex;
      QSharedMemory info_mem;
      QSharedMemory data_mem;
      int segment_locked;
      std::string name;
      int minsize;

      static std::recursive_mutex implMapMutex;
      static std::map<std::string,SharedMemorySegment::Impl*, std::less<>> implmap;
      int localInstances;

      static void registerSegment(std::string name){
        if(name != ICL_SHARED_MEMORY_REGISTER_NAME){
          SharedMemorySegmentRegister::addSegment(name);
        }
      }

      static void unregisterSegment(std::string name){
        if(name != ICL_SHARED_MEMORY_REGISTER_NAME){
          SharedMemorySegmentRegister::removeSegment(name);
        }
      }

      Impl(std::string key) :
        mutex(), segment_locked(0), name(key),
        minsize(0), localInstances(0)
      {
        SharedMemorySegmentRegister::freeSegment(name);
        info_mem.setKey((str(ICL_SHARED_MEMORY_INFO_PREFIX) + name).c_str());
        data_mem.setKey((name).c_str());
        registerSegment(name);
      }

      ~Impl(){
        std::scoped_lock<std::recursive_mutex> l(mutex);
        while(segment_locked){
          unlock();
        }
        unregisterSegment(name);
      }

      void clearSegment(QSharedMemory* seg){
        char* data = static_cast<char*>(seg->data());
        std::fill(data, data + seg->size() , '\0');
      }

      // sets the segments value to 0
      void clearDataSegment(){
        clearSegment(&data_mem);
      }

      // sets the segments value to 0
      void clearInfoSegment(){
        clearSegment(&info_mem);
      }

      void createSegment(QSharedMemory* seg, int size){
        int corrected_size = (size > 0) ? size : 1;
        // create segment
        seg->create(corrected_size);
        if(seg->error() == QSharedMemory::NoError){
          // created
          seg->lock();
          clearSegment(seg);
          seg->unlock();
        }
      }

      void attachSegment(QSharedMemory* seg){
        if(!seg->isAttached()){
          // try to attach
          seg->attach();
        }
      }

      void acquireSegment(QSharedMemory* seg, int size){
        while(!seg->isAttached()){
          // create info segment
          createSegment(seg, size);
          if(!seg->isAttached()){
            // try to attach
            seg->attach();
          }
          if(seg->error() != QSharedMemory::NoError && seg->error() != QSharedMemory::NotFound){
            std::ostringstream error;
            error << "SharedMemorySegment: Could not get segment for '"
                  << seg->key().toStdString() << "'. Error: " << seg->errorString().toStdString();
            ERROR_LOG(error.str());
            //throw ICLException(error.str());
          }
        }
      }

      void detachSegment(QSharedMemory* seg){
        seg->detach();
        if(seg->error() != QSharedMemory::NoError){
          std::ostringstream error;
          error << "SharedMemorySegment: Could not attach segment for '"
                << seg->key().toStdString() << "'. Error: " << seg->errorString().toStdString();
          ERROR_LOG(error.str());
          //throw ICLException(error.str());
        }
      }

      int getMinSize(){
        const int* data = (const int*) info_mem.constData();
        return *data;
      }

      void setMinSize(int minsize){
        this->minsize = minsize;
        int* data = static_cast<int*>(info_mem.data());
        *(data) = minsize;
      }

      bool needResize(){
        const bool* data = reinterpret_cast<const bool*>(static_cast<const char*>(info_mem.constData()) + sizeof(int));
        return *data;
      }

      void setResize(bool resize){
        bool* data = reinterpret_cast<bool*>(static_cast<char*>(info_mem.data()) + sizeof(int));
        *data = resize;
      }

      int getSize(){
        return data_mem.size();
      }

      // may need multiple calls until data_mem is attached
      void update(){
        std::scoped_lock<std::recursive_mutex> l(mutex);
        // acquire info segment
        acquireSegment(&info_mem, SEGMENT_INFO_SIZE);
        QSharedMemoryLocker li(info_mem);
        if(!li.locked) throw ICLException(str("[")+str(__FILE__)+str(":")+str(__LINE__)+str(": Could not lock info segment"));
        // ensure size
        if(getMinSize() < minsize) setMinSize(minsize);
        if(isAttached() && getSize() < getMinSize()) setResize(true);
        // resize
        if(needResize()){
          if(data_mem.isAttached()){
            detachSegment(&data_mem);
          } else {
            createSegment(&data_mem, getMinSize());
            if(data_mem.isAttached()){
              setResize(false);
            } else {
            }
          }
        }
        // reattach
        if(!needResize() && !data_mem.isAttached()){
          acquireSegment(&data_mem, getMinSize());
        }
      }

      void attach(){
        std::scoped_lock<std::recursive_mutex> l(mutex);
        do{
          update();
        } while (!isAttached());
      }

      bool isAttached(){
        std::scoped_lock<std::recursive_mutex> l(mutex);
        return data_mem.isAttached();
      }

    public:

      std::string getName(){
        const QByteArray asc = data_mem.key().toLatin1();
        return std::string(asc.constData(), asc.length());
        //return data_mem.key().toStdString();
      }

      void forceSetMinSize(int minsize){
        std::scoped_lock<std::recursive_mutex> l(mutex);
        lock(minsize);
        if(needResize()) {
          setMinSize(minsize);
          setResize(true);
          unlock();
        } else {
          setMinSize(minsize);
          setResize(true);
          unlock();
        }
      }

      bool lock(int minsize){
        if(segment_locked < 0){
          ERROR_LOG("this should not become  < 0");
        }
        if(!segment_locked){
          if(minsize > this->minsize) this->minsize = minsize;
          attach();
          if(info_mem.lock()){
            if(data_mem.lock()){
              ++segment_locked;
            } else {
              // could not get data lock
              info_mem.unlock();
            }
          }
        } else {
          ++segment_locked;
        }
        return segment_locked;
      }

      bool unlock(){
        std::scoped_lock<std::recursive_mutex> l(mutex);
        if(!--segment_locked){
          // unlock both segments
          data_mem.unlock();
          info_mem.unlock();
        }
        if(segment_locked < 0){
          DEBUG_LOG("this should not become  < 0");
        }
        return !segment_locked;
      }

      bool isLocked(){
        std::scoped_lock<std::recursive_mutex> l(mutex);
        return segment_locked;
      }

      static SharedMemorySegment::Impl* alloc(std::string name){
        std::scoped_lock<std::recursive_mutex> l(implMapMutex);
        if(!implmap.contains(name)){
          implmap[name] = new Impl(name);
        }
        implmap[name] -> localInstances++;
        return implmap[name];
      }

      static void free(std::string name){
        std::scoped_lock<std::recursive_mutex> l(implMapMutex);
        if(!implmap.contains(name)) return;
        if(--(implmap[name] -> localInstances) == 0){
          ICL_DELETE(implmap[name]);
          implmap.erase(name);
          if(name != ICL_SHARED_MEMORY_REGISTER_NAME){
            SharedMemorySegmentRegister::removeSegment(name);
          }
        }
      }

      static std::map<std::string,SharedMemorySegment::Impl*, std::less<>>* getImplMap(){
        return &implmap;
      }

      static void handleSignal(){
      std::scoped_lock<std::recursive_mutex> l(implMapMutex);
        std::map<std::string,SharedMemorySegment::Impl*, std::less<>>* map =
            SharedMemorySegment::Impl::getImplMap();
        std::map<std::string,SharedMemorySegment::Impl*, std::less<>>::iterator it;
        for(it = map->begin(); it != map->end(); ++it){
          if(it->first != ICL_SHARED_MEMORY_REGISTER_NAME){
            printf("[Deleting SharedMemorySegment \"%s\"]\n",(it->first).c_str());
            ICL_DELETE(it->second);
          }
        }
      }

  };

  std::recursive_mutex SharedMemorySegment::Impl::implMapMutex;
  std::map<std::string,SharedMemorySegment::Impl*, std::less<>> SharedMemorySegment::Impl::implmap;

  //##########################################################################
  //# SharedMemorySegment ####################################################
  //##########################################################################

  SharedMemorySegment::SharedMemorySegment(std::string name, int minsize) :
    m_Mutex(), m_Impl(nullptr), m_Name(name), m_Minsize(minsize)
  {}

  SharedMemorySegment::~SharedMemorySegment(){
    std::scoped_lock<std::recursive_mutex> l(m_Mutex);
    release();
  }

  void SharedMemorySegment::release(){
    std::scoped_lock<std::recursive_mutex> l(m_Mutex);
    if(m_Impl) SharedMemorySegment::Impl::free(m_Impl->getName());
    m_Impl = nullptr;
  }

  std::string SharedMemorySegment::getName(){
    std::scoped_lock<std::recursive_mutex> l(m_Mutex);
    return m_Name;
  }

  void SharedMemorySegment::reset(std::string name, int minsize){
    std::scoped_lock<std::recursive_mutex> l(m_Mutex);
    m_Name = name;
    m_Minsize = minsize;
  }

  bool SharedMemorySegment::isAttached(){
    std::scoped_lock<std::recursive_mutex> l(m_Mutex);
    return m_Impl && m_Impl->isAttached();
  }

  void SharedMemorySegment::forceMinSize(int size){
    std::scoped_lock<std::recursive_mutex> l(m_Mutex);
    //if(m_Impl) m_Impl->setMinSize(size);
    if(m_Impl && size) m_Impl->forceSetMinSize(size);
  }

  int SharedMemorySegment::getSize(){
    std::scoped_lock<std::recursive_mutex> l(m_Mutex);
    if(!m_Impl) return -1;
    return m_Impl->getSize();
  }

  bool SharedMemorySegment::isEmpty() const{
    std::scoped_lock<std::recursive_mutex> l(m_Mutex);
    if(m_Impl && m_Impl->isAttached()){
      const char* data = static_cast<const char*>(m_Impl->data_mem.constData());
      for(int i = 0; i < m_Impl->data_mem.size(); ++i){
        if(*(data + i) != '\0'){
          return false;
        }
      }
    }
    return true;
  }

  void* SharedMemorySegment::data(){
    std::scoped_lock<std::recursive_mutex> l(m_Mutex);
    if(!m_Impl) return nullptr;
    if(m_Impl->isAttached() && m_Impl->isLocked()){
      return m_Impl->data_mem.data();
    } else {
      return nullptr;
    }
  }

  const void* SharedMemorySegment::data() const{
    std::scoped_lock<std::recursive_mutex> l(m_Mutex);
    if(!m_Impl) return nullptr;
    if(m_Impl->isAttached() && m_Impl->isLocked()){
      return m_Impl->data_mem.data();
    } else {
      return nullptr;
    }
  }

  const void* SharedMemorySegment::constData() const{
    std::scoped_lock<std::recursive_mutex> l(m_Mutex);
    if(!m_Impl) return nullptr;
    if(m_Impl->isAttached() && m_Impl->isLocked()){
      return m_Impl->data_mem.constData();
    } else {
      return nullptr;
    }
  }

  bool SharedMemorySegment::lock(int minsize){
    std::scoped_lock<std::recursive_mutex> l(m_Mutex);
    // check impl
    if(m_Impl && (m_Impl->getName() != m_Name)){
      release();
    }
    // create impl
    if(!m_Impl){
      if(!m_Name.length()){
        throw ICLException("Can't lock SharedMemorySegment without name.");
        return false;
      } else {
        m_Impl = SharedMemorySegment::Impl::alloc(m_Name);
      }
    }
    // lock impl
    return m_Impl->lock(minsize);
  }

  bool SharedMemorySegment::unlock(){
    std::scoped_lock<std::recursive_mutex> l(m_Mutex);
    return m_Impl->unlock();
  }

  //##########################################################################
  //# SegmentRegisterData ####################################################
  //##########################################################################

  class SegmentRegisterData{
    private:
      std::recursive_mutex mutex;
      SharedMemorySegment reg_segment;

      // acquires special register segment
      SegmentRegisterData() : mutex()
      {
        // reset segment (neccessary when not correctly destroyed)
        SharedMemorySegmentRegister::freeSegment(ICL_SHARED_MEMORY_REGISTER_NAME);
        // acquire segment
        reg_segment.reset(ICL_SHARED_MEMORY_REGISTER_NAME,
                          SEGMENT_REGISTER_SIZE);
        reg_segment.lock();
        if(reg_segment.isEmpty()){
          std::multiset<std::string> empty;
          setSegmentSet(empty);
        }
        reg_segment.unlock();
      }

    public:

      static SegmentRegisterData* inst(){
        static bool first = true;
        if(first){
          first = false;
          SignalHandler::install("SharedMemorySegment", signal_handler, "SIGINT,SIGTERM,SIGSEGV");
        }
        static SegmentRegisterData segment_register_data;
        return &segment_register_data;
      }

      static std::recursive_mutex* regMutex(){
        static std::recursive_mutex segment_register_mutex;
        return &segment_register_mutex;
      }

      void lock(){
        std::scoped_lock<std::recursive_mutex> l(mutex);
        regMutex()->lock();
        reg_segment.lock();
      }

      void unlock(){
        std::scoped_lock<std::recursive_mutex> l(mutex);
        reg_segment.unlock();
        regMutex()->unlock();
      }

      SharedMemorySegment* getRegisterSegment(){
        return &reg_segment;
      }

      std::multiset<std::string> getSegmentSet(){
        std::scoped_lock<std::recursive_mutex> l(mutex);
        std::multiset<std::string> gs;
        const char* data = static_cast<const char*>(reg_segment.constData());
        int count = *reinterpret_cast<const int*>(data);
        data += sizeof(int);
        for(int i = 0; i < count; ++i){
          std::string value = data;
          gs.insert(value);
          data += value.length()+1;
        }
        return gs;
      }

      void setSegmentSet(std::multiset<std::string> set){
        std::scoped_lock<std::recursive_mutex> l(mutex);
        char* data = static_cast<char*>(reg_segment.data());
        int* setSize = reinterpret_cast<int*>(data);
        data += sizeof(int);
        *setSize = 0;
#ifdef WIN32
        std::multiset<std::string>::iterator it;
#else
        // TODO: is this really necessary?
        std::set<std::string>::iterator it;
#endif
        unsigned int size = 0;
        for(it = set.begin(); it != set.end(); ++it){
          std::string value = *it;
          size += value.length() + 1;
          if(size < reg_segment.getSize()-sizeof(int)){
            std::copy(value.begin(),value.end(),data);
            data += value.length();
            *data++ = '\0';
            (*setSize)++;
          } else {
            ERROR_LOG("\n Can't add '" << value << "' to full SharedMemorySeg"
                      "mentRegister. \nThe size of SharedMemorySegmentRegiste"
                      "r can only be changed in the source ccode. \n Please r"
                      "efer to the developers if you experience this error.");
          }
        }
      }

      void addSegment(std::string name){
        std::scoped_lock<std::recursive_mutex> l(mutex);
        std::multiset<std::string> set = getSegmentSet();
        set.insert(name);
        setSegmentSet(set);
      }

      void removeSegment(std::string name){
        std::scoped_lock<std::recursive_mutex> l(mutex);
        std::multiset<std::string> set = getSegmentSet();
        if(set.contains(name)){
          set.erase(set.find(name));
          setSegmentSet(set);
        }
      }

      ~SegmentRegisterData(){
        // reset all segments
      }

      void release(){
        std::scoped_lock<std::recursive_mutex> l(mutex);
        regMutex()->lock();
        reg_segment.release();
      }
    static void signal_handler(const std::string &signal){
      std::cout << "[SharedMemorySegment signal handling called. Signal " << signal << "]",
      SharedMemorySegment::Impl::handleSignal();
      inst()->release();
    }
  };

  //##########################################################################
  //# SharedMemorySegmentRegister ############################################
  //##########################################################################

  std::set<std::string> SharedMemorySegmentRegister::getSegmentSet(){
    SegmentRegisterData* dat = SegmentRegisterData::inst();
    std::set<std::string> ret;
    dat->lock();
    std::multiset<std::string> set = dat->getSegmentSet();
    ret.insert(set.begin(),set.end());
    dat->unlock();
    return ret;
  }

  void SharedMemorySegmentRegister::addSegment(std::string name){
    SegmentRegisterData* dat = SegmentRegisterData::inst();
    dat->lock();
    dat->addSegment(name);
    dat->unlock();
  }

  void SharedMemorySegmentRegister::removeSegment(std::string name){
    SegmentRegisterData* dat = SegmentRegisterData::inst();
    dat->lock();
    dat->removeSegment(name);
    dat->unlock();
  }

  void SharedMemorySegmentRegister::freeSegment(std::string name){
    QSharedMemory* info = new QSharedMemory();
    QSharedMemory* data = new QSharedMemory();
    info->setKey((str(ICL_SHARED_MEMORY_INFO_PREFIX) + name).c_str());
    data->setKey(name.c_str());
    info->attach(QSharedMemory::ReadWrite);
    data->attach(QSharedMemory::ReadWrite);
    info->detach();
    data->detach();
    ICL_DELETE(info);
    ICL_DELETE(data);
  }

  void SharedMemorySegmentRegister::freeAllSegments(){
    SegmentRegisterData* dat = SegmentRegisterData::inst();
    dat->lock();
    std::multiset<std::string> multiset = dat->getSegmentSet();
    std::set<std::string> set;
    set.insert(multiset.begin(), multiset.end());
    std::set<std::string>::iterator it;
    for(it = set.begin(); it != set.end(); ++it){
      freeSegment(*it);
    }
    dat->unlock();
  }

  void resetQtSystemResource(std::string s){
#ifdef ICL_SYSTEM_LINUX
    static const std::string QT_SHARED_MEM_PREFIX = "0x51";

    QStringList l; l << s.c_str();
    QProcess ipcs;
    ipcs.start("ipcs",l);
    bool ok = ipcs.waitForFinished();
    if(!ok) throw ICLException("unable to call ipcm " + s);
    QString stdout = ipcs.readAllStandardOutput();

    std::vector<std::string> lines = tok(stdout.toLatin1().data(),"\n");
    for(unsigned int i=0;i<lines.size();++i){
      if(lines[i].substr(0,2) != "0x") continue;

      std::vector<std::string> ts = tok(lines[i]," ");
      if(ts.size() > 3 && ts[3][0] == '6' && ts[0].substr(0,4) == QT_SHARED_MEM_PREFIX){
        QProcess ipcrm;
        QStringList l2; l2 << s.c_str() << ts[1].c_str();
        std::cout << "releasing shared qt resource key:" << ts[0] << " shmid:" << ts[1] << std::endl;
        ipcrm.start("ipcrm",l2);
        bool ok = ipcrm.waitForFinished();
        if(!ok) throw ICLException("unable to call ipcrm " + s);
      }
    }
#endif
  }

  void SharedMemorySegmentRegister::resetBus(){
    resetQtSystemResource("-m");
    resetQtSystemResource("-s");
  }

  } // namespace icl::io