/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/SharedMemorySegment.cpp                **
** Module : ICLIO                                                  **
** Authors: Viktor Richter                                         **
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

#include <ICLIO/SharedMemorySegment.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/SignalHandler.h>

#include <QtCore/QSharedMemory>

#include <map>
#include <stdio.h>

using namespace icl;
using namespace icl::utils;

namespace icl {
  namespace io {

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

    struct SharedMemoryLocker{
        QSharedMemory &memory;
        bool locked;
        SharedMemoryLocker(QSharedMemory &mem):
          memory(mem){
          locked = memory.lock();
        }
        SharedMemoryLocker(QSharedMemory* mem):
          memory(*mem){
          locked = memory.lock();
        }
        ~SharedMemoryLocker(){
          if(locked) memory.unlock();
        }
    };

    //##########################################################################
    //# SharedMemorySegmentLocker ##############################################
    //##########################################################################

    SharedMemorySegmentLocker::SharedMemorySegmentLocker(SharedMemorySegment &seg, int minSize, bool reduce):
      segment(seg){
      segment.lock(minSize, reduce);
    }

    SharedMemorySegmentLocker::SharedMemorySegmentLocker(SharedMemorySegment* seg, int minSize, bool reduce):
      segment(*seg){
      segment.lock(minSize, reduce);
    }

    SharedMemorySegmentLocker::~SharedMemorySegmentLocker(){
      segment.unlock() ;
    }

    //##########################################################################
    //# SharedMemorySegmentImpl ################################################
    //##########################################################################

    struct SharedMemorySegment::Impl{
        class SharedMemorySignalHandler;
        friend class SharedMemorySegment;
      private:
        static const int SEGMENT_INFO_SIZE = sizeof(int)
            + sizeof(bool);
        Mutex mutex;
        int segment_locked;
        std::string name;
        bool resize_requested;
        bool resize_own;
        int min_segment_size;

        static Mutex implMapMutex;
        static std::map<std::string,SharedMemorySegment::Impl*> implmap;
        int localInstances;

        Impl(std::string key) :
          mutex(Mutex::mutexTypeRecursive), segment_locked(0), name(key),
          resize_requested(false), resize_own(false), min_segment_size(0),
          localInstances(0)
        {
          static SharedMemorySignalHandler handler;
          info_mem.setKey((str(ICL_SHARED_MEMORY_INFO_PREFIX) + name).c_str());
          data_mem.setKey((name).c_str());
        }

        ~Impl(){
          Mutex::Locker l(mutex);
          DEBUG_LOG("Segment locked: " << segment_locked);
          while(segment_locked){
            DEBUG_LOG("unlocking: " << segment_locked);
            unlock();
            DEBUG_LOG("unlockced: " << segment_locked);
          }
          DEBUG_LOG("deleting segment: " << name);
          if(info_mem.isAttached() && data_mem.isAttached()){
            info_mem.lock();
            decObserversCount();
            info_mem.unlock();
          }
          if(data_mem.isAttached()) data_mem.detach();
          if(info_mem.isAttached()) info_mem.detach();
        }

        void createSegment(int size){
          Mutex::Locker l(mutex);
          // create info segment
          info_mem.create(SEGMENT_INFO_SIZE);
          if(info_mem.error() != QSharedMemory::NoError){
            // could not create info segment
            std::ostringstream error;
            error << "SharedMemorySegment: Could not create info segment for '"
                  << name << "'. Error: " << info_mem.errorString().toStdString();
            DEBUG_LOG(error.str());
            throw ICLException(error.str());
          }
          SharedMemoryLocker li(info_mem);

          // create data segment
          data_mem.create(size);
          if(data_mem.error() != QSharedMemory::NoError){
            // could not create data segment
            std::ostringstream error;
            error << "SharedMemorySegment: Could not create data segment for '"
                  << name << "'. Error: " << data_mem.errorString().toStdString();
            DEBUG_LOG(error.str());
            info_mem.detach();
            throw ICLException(error.str());
          }

          // successfully created new segment
          initInfoBlock();
        }

        void attachSegment(){
          Mutex::Locker l(mutex);
          // attach info to segment
          info_mem.attach(QSharedMemory::ReadWrite);
          if(info_mem.error() != QSharedMemory::NoError){
            // could not attach info segment
            std::ostringstream error;
            error << "SharedMemorySegment: Could not attach info segment for '"
                  << name << "'. Error: " << info_mem.errorString().toStdString();
            DEBUG_LOG(error.str());
            throw ICLException(error.str());
          }
          /*SharedMemoryLocker li(info_mem);
          // attach data segment
          data_mem.attach(QSharedMemory::ReadWrite);
          if(data_mem.error() != QSharedMemory::NoError){
            // could not attach data segment
            std::ostringstream error;
            error << "SharedMemorySegment: Could not attach data segment for '"
                  << name << "'. Error: " << data_mem.errorString().toStdString();
            DEBUG_LOG(error.str());
            throw ICLException(error.str());
          }
          // successfully attached new segment
          incObserversCount();
          */
          while(update()) {DEBUG_LOG("needs update in attach"); Thread::msleep(1);}
        }


        // sets the segments value to 0
        void clearSegment(){
          char* data = (char*) data_mem.data();
          std::fill(data, data + data_mem.size() ,(char) 0);
        }

      public:
        QSharedMemory info_mem;
        QSharedMemory data_mem;

        int getObserversCount(){
          const int* data = (const int*) info_mem.constData();
          return *data;
        }

        bool isObservable(){
          const char* data = (const char*) info_mem.constData() + sizeof(int);
          return *((bool*) data);
        }

        void setObserversCount(int count){
          char* data = (char*) info_mem.data();
          *((int*)data) = count;
        }

        void incObserversCount(){
          setObserversCount(getObserversCount()+1);
        }

        void decObserversCount(){
          setObserversCount(getObserversCount()-1);
        }

        void setObservable(bool observable){
          char* data = (char*) info_mem.data() + sizeof(int);
          *((bool*)data) = observable;
        }

        void initInfoBlock(){
          char* data = (char*) info_mem.data();
          *((int*)data) = 1;
          data += sizeof(int);
          *((bool*)data) = true;
        }

        std::string getName(){
          Mutex::Locker l(mutex);
          return name;
        }

        SharedMemorySegment::AcquisitionCode acquireSegment(int size){
          Mutex::Locker l(mutex);
          if(isAttached()) return SharedMemorySegment::existed;
          try{
            createSegment(size);
            if(name != ICL_SHARED_MEMORY_REGISTER_NAME){
              SharedMemorySegmentRegister::addSegment(name);
            }
            clearSegment();
            return SharedMemorySegment::created;
          } catch (ICLException &e){
            ERROR_LOG("Cant create segment '" << name << "' Error: " << e.what());
          }
          try{
            attachSegment();
            DEBUG_LOG("attached segment '" << name);
            return SharedMemorySegment::attached;
          } catch (ICLException &e){
            ERROR_LOG("Cant attach segment '" << name << "' Error: " << e.what());
          }
          if(!size){
            return SharedMemorySegment::emptyCreate;
          }
          return SharedMemorySegment::error;
        }

        // sets the minimum size of the segment
        void setMinSize(int size){
          Mutex::Locker l(mutex);
          min_segment_size = size;
        }

        // sets the size of the segment
        void setSize(int size){
          Mutex::Locker l(mutex);
          if(data_mem.size() != size){
            min_segment_size = size;
            resize_requested = true;
          }
        }

        // returns the minimum segment size
        int getMinSize(){
          Mutex::Locker l(mutex);
          return min_segment_size;
        }

        // returns true when there is need of further calls to update
        bool update(){
          Mutex::Locker l(mutex);
          // ensure min size
          if(data_mem.size() < min_segment_size) resize_requested = true;
          // read info segment
          SharedMemoryLocker li(info_mem);
          if(!li.locked){
            DEBUG_LOG("could not lock info segment: " << info_mem.errorString().toStdString());
            return true;
          }
          if(resize_requested && isObservable()){
            // resize was requested and segment is observable. get it
            setObservable(false);
            resize_own = true;
          }
          // segment is attached but not observable. need to detatch.
          if(data_mem.isAttached() && !isObservable()){
            data_mem.detach();
            decObserversCount();
          }
          // owning reset and no observers
          if(!data_mem.isAttached() && resize_own && !getObserversCount()){
            // create with new size
            int error = 100;
            while(!data_mem.isAttached() && !data_mem.create(min_segment_size, QSharedMemory::ReadWrite)){
              Thread::msleep(POLLING_SLEEP_MSEC);
              if(!--error){
                DEBUG_LOG("Could not create SharedMemorySegment in update: " << data_mem.errorString().toStdString());
                throw ICLException(str("Could not create SharedMemorySegment in update: ") + data_mem.errorString().toStdString());
              }
            }
            clearSegment();
            setObservable(true);
            incObserversCount();
            resize_requested = false;
            resize_own = false;
          }
          // can reattach
          if(!data_mem.isAttached() && isObservable()){
            int error = 100;//TODO: does this realy loop non stop
            while(!data_mem.isAttached() && !data_mem.attach(QSharedMemory::ReadWrite)){
              DEBUG_LOG("could not attach SharedMemorySegment in update: " << data_mem.errorString().toStdString());
              Thread::msleep(POLLING_SLEEP_MSEC);
              if(!--error){
                DEBUG_LOG("Could not attach SharedMemorySegment in update: " << data_mem.errorString().toStdString());
                throw ICLException(str("Could not attach SharedMemorySegment in update: ") + data_mem.errorString().toStdString());
              }
            }
            incObserversCount();
          }
          return !data_mem.isAttached();
        }

        bool lock(){
          Mutex::Locker l(mutex);
          if(!segment_locked){
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
          Mutex::Locker l(mutex);
          if(!--segment_locked){
            // unlock both segments
            data_mem.unlock();
            info_mem.unlock();
          }
          return !segment_locked;
        }

        bool isAttached(){
          Mutex::Locker l(mutex);
          return data_mem.isAttached();
        }

        int size(){
          Mutex::Locker l(mutex);
          return data_mem.size();
        }

        bool isLocked(){
          Mutex::Locker l(mutex);
          return segment_locked;
        }

        static SharedMemorySegment::Impl* alloc(std::string name){
          Mutex::Locker l(implMapMutex);
          if(implmap.find(name) == implmap.end()){
            implmap[name] = new Impl(name);
          }
          implmap[name] -> localInstances++;
          return implmap[name];
        }

        static void free(std::string name){
          Mutex::Locker l(implMapMutex);
          if(implmap.find(name) == implmap.end()) return;
          if(--(implmap[name] -> localInstances) == 0){
            ICL_DELETE(implmap[name]);
            implmap.erase(name);
            if(name != ICL_SHARED_MEMORY_REGISTER_NAME){
              SharedMemorySegmentRegister::removeSegment(name);
            }
          }
        }

        static std::map<std::string,SharedMemorySegment::Impl*>* getImplMap(){
          return &implmap;
        }

        class SharedMemorySignalHandler : public SignalHandler{
        public:
          SharedMemorySignalHandler() : SignalHandler("SIGINT,SIGTERM,SIGSEGV"){
            printf("[created signal handler for SharedMemory]\n");
          }
          virtual void handleSignals(const std::string &signal){
            printf("[Unclean break detected. Signal \"%s\"]\n",signal.c_str());
            m_oMutex.lock();
            std::map<std::string,SharedMemorySegment::Impl*>* map =
                SharedMemorySegment::Impl::getImplMap();
            std::map<std::string,SharedMemorySegment::Impl*>::iterator it;
            for(it = map->begin(); it != map->end(); ++it){
              printf("[Deleting SharedMemorySegment \"%s\"]",(it->first).c_str());
              ICL_DELETE(it->second);
            }
            m_oMutex.unlock();
            killCurrentProcess();
          }

        private:
          Mutex m_oMutex;
        };
    };

    //##########################################################################
    //# SharedMemorySegment ####################################################
    //##########################################################################

    SharedMemorySegment::SharedMemorySegment(std::string name, int size) :
      m_Mutex(Mutex::mutexTypeRecursive), m_Impl(NULL), m_Name(name)
    {
      if(m_Name.length()){
        acquire(m_Name, size);
      }
    }

    SharedMemorySegment::~SharedMemorySegment(){
      Mutex::Locker l(m_Mutex);
      release();
    }

    SharedMemorySegment::AcquisitionCode SharedMemorySegment::acquire(std::string name, int size){
      Mutex::Locker l(m_Mutex);
      if(name.length()){
        m_Name = name;
      }
      if(!m_Name.length()){
        throw ICLException("Can't acquire SharedMemorySegment with empty name.");
      }
      DEBUG_LOG("reset segment " << m_Name);
      SharedMemorySegment::resetSegment(m_Name);
      DEBUG_LOG("reset segment done" << m_Name);
      // when already exists
      if(m_Impl && m_Impl->getName() != m_Name){
        SharedMemorySegment::Impl::free(m_Impl->getName());
        m_Impl = NULL;
      }
      if(!m_Impl){
        m_Impl = SharedMemorySegment::Impl::alloc(m_Name);
      }
      SharedMemorySegment::AcquisitionCode code = m_Impl->acquireSegment(size);
      switch(code){
        case created:
        case attached:
        case existed:
          return code;
        case emptyCreate:
          SharedMemorySegment::Impl::free(m_Impl->getName());
          throw ICLException("Attaching SharedMemorySegment failed and can't create empty Segment.");
        case error:
          SharedMemorySegment::Impl::free(m_Impl->getName());
          throw ICLException("Acquiring SharedMemorySegment failed.");
        default:
          return error;
      }
    }

    void SharedMemorySegment::release(){
      Mutex::Locker l(m_Mutex);
      SharedMemorySegment::Impl::free(m_Impl->getName());
      m_Impl = NULL;
    }

    const void* SharedMemorySegment::constData() const{
      Mutex::Locker l(m_Mutex);
      if(!m_Impl) return NULL;
      if(m_Impl->isAttached()){
        return m_Impl->data_mem.constData();
      } else {
        return NULL;
      }
    }

    void* SharedMemorySegment::data(){
      Mutex::Locker l(m_Mutex);
      if(!m_Impl) return NULL;
      if(m_Impl->isAttached()){
        return m_Impl->data_mem.data();
      } else {
        return NULL;
      }
    }

    const void* SharedMemorySegment::data() const{
      Mutex::Locker l(m_Mutex);
      if(!m_Impl) return NULL;
      if(m_Impl->isAttached()){
        return m_Impl->data_mem.data();
      } else {
        return NULL;
      }
    }

    bool SharedMemorySegment::isAttached() const{
      Mutex::Locker l(m_Mutex);
      return m_Impl && m_Impl->isAttached();
    }

    bool SharedMemorySegment::isEmpty() const{
      Mutex::Locker l(m_Mutex);
      if(m_Impl && m_Impl->isAttached()){
        const char* data = (const char*) m_Impl->data_mem.constData();
        for(int i = 0; i < m_Impl->data_mem.size(); ++i){
          if(*(data + i) != (char) 0){
            return false;
          }
        }
      }
      return true;
    }

    int SharedMemorySegment::size() const{
      Mutex::Locker l(m_Mutex);
      return (m_Impl) ? m_Impl->size() : 0;
    }

    void SharedMemorySegment::resetSegment(std::string name){
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

    void SharedMemorySegment::setMinSize(int size){
      Mutex::Locker l(m_Mutex);
      if(m_Impl) m_Impl->setMinSize(size);
    }

    int SharedMemorySegment::getMinSize(){
      Mutex::Locker l(m_Mutex);
      if(!m_Impl) return 0;
      return m_Impl->getMinSize();
    }

    int SharedMemorySegment::getObserversCount(){
      Mutex::Locker l(m_Mutex);
      SharedMemorySegmentLocker m(this);
      return m_Impl->getObserversCount();
    }

    bool SharedMemorySegment::isObservable(){
      Mutex::Locker l(m_Mutex);
      SharedMemorySegmentLocker m(this);
      return m_Impl->isObservable();
    }

    bool SharedMemorySegment::lock(int minSize, bool reduce){
      Mutex::Locker l(m_Mutex);
      Mutex::Locker limpl(m_Impl->mutex);
      if(reduce){
        DEBUG_LOG2("enforce size " << getName() << " - " << m_Impl->size() << " -> " << minSize);
        m_Impl->setSize(minSize);
      } else {
        DEBUG_LOG2("ensure min size " << getName() << " - " << m_Impl->size() << " -> " << getMinSize());
        m_Impl->setMinSize(minSize);
      }
      while (true){
        // update till segment is ready
        while(true){
          if(m_Impl->update()){
            Thread::msleep(POLLING_SLEEP_MSEC);
          } else {
            break;
          }
        }
        if(m_Impl->lock()){
          if(reduce && (m_Impl->size() != minSize)){
            DEBUG_LOG2("enforce size " << getName() << " - " << m_Impl->size() << " -> " << minSize);
            m_Impl->unlock();
          } else if(!reduce && (m_Impl->size() < minSize)){
            DEBUG_LOG2("ensure min size " << getName() << " - " << m_Impl->size() << " -> " << getMinSize());
            m_Impl->unlock();
          } else {
            break;
          }
        } else {
          DEBUG_LOG2("could not get lock "<< getName());
        }
      }
      DEBUG_LOG2("Segment info: \n\tObservable:\t" << m_Impl->isObservable() << "\n\tObservers:\t" << m_Impl->getObserversCount() << "\n");
      return true;
    }

    bool SharedMemorySegment::unlock(){
      Mutex::Locker l(m_Mutex);
      return m_Impl->unlock();
    }

    std::string SharedMemorySegment::getName(){
      return m_Name;
    }

    std::string SharedMemorySegment::errorToString(SharedMemorySegment::ErrorCode error){
      switch(error){
#define CASE(X) case X: return #X
        CASE(SharedMemorySegment::NoError);
        CASE(SharedMemorySegment::PermissionDenied);
        CASE(SharedMemorySegment::InvalidSize);
        CASE(SharedMemorySegment::KeyError);
        CASE(SharedMemorySegment::AlreadyExists);
        CASE(SharedMemorySegment::NotFound);
        CASE(SharedMemorySegment::LockError);
        CASE(SharedMemorySegment::OutOfResources);
        CASE(SharedMemorySegment::UnknownError);
#undef CASE
        default: return "UnknownError";
      }
      return "";
    }

    Mutex SharedMemorySegment::Impl::implMapMutex(Mutex::mutexTypeRecursive);
    std::map<std::string,SharedMemorySegment::Impl*> SharedMemorySegment::Impl::implmap;


    //##########################################################################
    //# SegmentRegisterData ####################################################
    //##########################################################################

    class SegmentRegisterData {
      private:
        Mutex mutex;
        SharedMemorySegment reg_segment;

        // acquires special register segment
        SegmentRegisterData() : mutex(Mutex::mutexTypeRecursive) {
          DEBUG_LOG("register constr");
          // reset segment (neccessary when not correctly destroyed)
          SharedMemorySegment::resetSegment(ICL_SHARED_MEMORY_REGISTER_NAME);
          // acquire segment
          if(SharedMemorySegment::created == reg_segment.acquire(
               ICL_SHARED_MEMORY_REGISTER_NAME,
               SEGMENT_REGISTER_SIZE))
          {
            reg_segment.lock();
            std::set<std::string> empty;
            setSegmentSet(empty);
            reg_segment.unlock();
          }
        }

      public:

        static SegmentRegisterData* inst(){
          static SegmentRegisterData segment_register_data;
          return &segment_register_data;
        }

        static Mutex* regMutex(){
          static Mutex segment_register_mutex;
          return &segment_register_mutex;
        }

        void lock(){
          Mutex::Locker l(mutex);
          regMutex()->lock();
          reg_segment.lock();
        }

        void unlock(){
          Mutex::Locker l(mutex);
          reg_segment.unlock();
          regMutex()->unlock();
        }

        SharedMemorySegment* getRegisterSegment(){
          return &reg_segment;
        }

        std::set<std::string> getSegmentSet(){
          Mutex::Locker l(mutex);
          std::set<std::string> gs;
          const char* data = (const char*) reg_segment.constData();
          int count = *((int*) data);
          data += sizeof(int);
          for(int i = 0; i < count; ++i){
            std::string value = data;
            gs.insert(value);
            data += value.length()+1;
          }
          return gs;
        }

        void setSegmentSet(std::set<std::string> set){
          Mutex::Locker l(mutex);
          char* data = (char*) reg_segment.data();
          int* setSize = (int*) data;
          data += sizeof(int);
          *setSize = 0;
          std::set<std::string>::iterator it;
          unsigned int size = 0;
          for(it = set.begin(); it != set.end(); ++it){
            std::string value = *it;
            size += value.length() + 1;
            if(size < reg_segment.size()-sizeof(int)){
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
          Mutex::Locker l(mutex);
          std::set<std::string> set = getSegmentSet();
          set.insert(name);
          setSegmentSet(set);
        }

        void removeSegment(std::string name){
          Mutex::Locker l(mutex);
          std::set<std::string> set = getSegmentSet();
          if(set.find(name) != set.end()){
            set.erase(name);
            setSegmentSet(set);
          }
        }

        ~SegmentRegisterData(){
          DEBUG_LOG("TODO: reset all segments");
        }
    };

    //##########################################################################
    //# SharedMemorySegmentRegister ############################################
    //##########################################################################

    std::set<std::string> SharedMemorySegmentRegister::getSegmentSet(){
      SegmentRegisterData* dat = SegmentRegisterData::inst();
      dat->lock();
      std::set<std::string> ret = dat->getSegmentSet();
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

    void SharedMemorySegmentRegister::resetSegment(std::string name){
      SharedMemorySegment::resetSegment(name);
    }

    void SharedMemorySegmentRegister::resetAllSegment(){
      SegmentRegisterData* dat = SegmentRegisterData::inst();
      dat->lock();
      std::set<std::string> set = dat->getSegmentSet();
      std::set<std::string>::iterator it;
      for(it = set.begin(); it != set.end(); ++it){
        SharedMemorySegment::resetSegment(*it);
      }
      dat->unlock();
    }

  } // namespace io
}  // namespace icl
