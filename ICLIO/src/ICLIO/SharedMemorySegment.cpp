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

#include <QtCore/QSharedMemory>

#include <map>

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
    //# helper functions #######################################################
    //##########################################################################

    std::vector<std::string> extract_string_vector(unsigned int count, const char* data){
      std::vector<std::string> gs(count);
      for(unsigned int i = 0; i < gs.size(); ++i){
        gs[i] = data;
        data += gs[i].length()+1;
      }
      return gs;
    }

    std::set<std::string> extract_string_set(unsigned int count, const char* data){
      std::set<std::string> gs;
      for(unsigned int i = 0; i < count; ++i){
        std::string value = data;
        gs.insert(value);
        data += value.length()+1;
      }
      return gs;
    }

    void write_string_set(const std::set<std::string> &string_set, char* data){
      std::set<std::string>::iterator it;
      for(it = string_set.begin(); it != string_set.end(); ++it){
        std::string value = *it;
        std::copy(value.begin(),value.end(),data);
        data += value.length();
        *data++ = '\0';
      }
    }

    //##########################################################################
    //# SharedMemoryLocker #####################################################
    //##########################################################################

    struct SharedMemoryLocker{
        QSharedMemory &memory;
        SharedMemoryLocker(QSharedMemory &mem):
          memory(mem){
          memory.lock();
        }
        SharedMemoryLocker(QSharedMemory* mem):
          memory(*mem){
          memory.lock();
        }
        ~SharedMemoryLocker(){
          memory.unlock() ;
        }
    };

    //##########################################################################
    //# SharedMemorySegmentLocker ##############################################
    //##########################################################################

    SharedMemorySegmentLocker::SharedMemorySegmentLocker(SharedMemorySegment &seg):
      segment(seg){
      segment.lock();
    }

    SharedMemorySegmentLocker::SharedMemorySegmentLocker(SharedMemorySegment* seg):
      segment(*seg){
      segment.lock();
    }

    SharedMemorySegmentLocker::~SharedMemorySegmentLocker(){
      segment.unlock() ;
    }

    //##########################################################################
    //# SharedMemorySegmentImpl ################################################
    //##########################################################################

    struct SharedMemorySegment::Impl{
      //friend class SharedMemorySegment;
      private:
        static const int SEGMENT_INFO_SIZE = sizeof(int)
            + sizeof(bool);
        Mutex mutex;
        int segment_locked;
        std::string name;
        bool resize_requested;
        bool resize_own;
        int resize_to;

        static Mutex implMapMutex;
        static std::map<std::string,SharedMemorySegment::Impl*> implmap;
        int localInstances;

        Impl(std::string key) : mutex(Mutex::mutexTypeRecursive),
          segment_locked(0), name(key), resize_requested(false),
          resize_own(false), resize_to(0), localInstances(0)
        {
          info_mem.setKey((str(ICL_SHARED_MEMORY_INFO_PREFIX) + name).c_str());
          data_mem.setKey((name).c_str());
        }

        ~Impl(){
          Mutex::Locker l(mutex);
          DEBUG_LOG("deleting segment: " << name);
          if(info_mem.isAttached()){
            info_mem.lock();
            decObserversCount();
            info_mem.unlock();
          }
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
          SharedMemoryLocker li(info_mem);

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
            return SharedMemorySegment::created;
          } catch (ICLException &e){
            DEBUG_LOG("could not create. must already exist");
            attachSegment();
            return SharedMemorySegment::attached;
          }
        }

        // marks segment to be resized
        bool setResize(int size){
          Mutex::Locker l(mutex);
          if(data_mem.size() != size){
            resize_to = size;
            resize_requested = true;
          }
          return resize_requested;
        }

        // returns true when there is need of further calls to update
        bool update(){
          Mutex::Locker l(mutex);
          // read info segment
          SharedMemoryLocker li(info_mem);
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
            while(!data_mem.isAttached() && !data_mem.create(resize_to, QSharedMemory::ReadWrite)){
              DEBUG_LOG("could not create SharedMemorySegment in update: " << data_mem.errorString().toStdString());
              Thread::msleep(POLLING_SLEEP_MSEC);
            }
            setObservable(true);
            incObserversCount();
            resize_requested = false;
            resize_own = false;
          }
          // can reattach
          if(!data_mem.isAttached() && isObservable()){
            while(!data_mem.isAttached() && !data_mem.attach(QSharedMemory::ReadWrite)){
              DEBUG_LOG("could not attach SharedMemorySegment in update: " << data_mem.errorString().toStdString());
              Thread::msleep(POLLING_SLEEP_MSEC);
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
    };

    //##########################################################################
    //# SharedMemorySegment ####################################################
    //##########################################################################

    SharedMemorySegment::SharedMemorySegment() :
      m_Mutex(Mutex::mutexTypeRecursive), m_Impl(NULL)
    {}

    SharedMemorySegment::~SharedMemorySegment(){
      Mutex::Locker l(m_Mutex);
      release();
    }

    SharedMemorySegment::AcquisitionCode SharedMemorySegment::acquire(std::string name, int size){
      Mutex::Locker l(m_Mutex);
      // when already exists
      if(m_Impl && m_Impl->getName() != name){
        SharedMemorySegment::Impl::free(m_Impl->getName());
        m_Impl = NULL;
      }
      if(!m_Impl){
        m_Impl = SharedMemorySegment::Impl::alloc(name);
      }
      return m_Impl->acquireSegment(size);
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

    bool SharedMemorySegment::setResize(int size){
      Mutex::Locker l(m_Mutex);
      if(!m_Impl) return false;
      return m_Impl->setResize(size);
    }

    bool SharedMemorySegment::update(bool poll){
      if(!poll){
        return m_Impl->update();
      } else {
        while(m_Impl->update()){
          Thread::msleep(POLLING_SLEEP_MSEC);
        }
        return false;
      }
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

    bool SharedMemorySegment::lock(){
      Mutex::Locker l(m_Mutex);
      return m_Impl->lock();
    }

    bool SharedMemorySegment::unlock(){
      Mutex::Locker l(m_Mutex);
      return m_Impl->unlock();
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

      public:
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

        void lock(){
          Mutex::Locker l(mutex);
          reg_segment.lock();
        }

        void unlock(){
          Mutex::Locker l(mutex);
          reg_segment.unlock();
        }

        SharedMemorySegment* getRegisterSegment(){
          return &reg_segment;
        }

        std::set<std::string> getSegmentSet(){
          Mutex::Locker l(mutex);
          const char* data = (const char*) reg_segment.constData();
          int segmentCount = *((int*) data);
          return extract_string_set(segmentCount, data + sizeof(int));
        }

        void setSegmentSet(std::set<std::string> set){
          Mutex::Locker l(mutex);
          // check segment/set size
          int setSize = sizeof(int) + set.size();
          for(std::set<std::string>::iterator it = set.begin();
              it != set.end(); ++it){
            setSize += (*it).size();
          }
          DEBUG_LOG("calculated size for segment: " << setSize);
          int resize = reg_segment.size();
          while(resize < setSize){
            resize += SEGMENT_REGISTER_SIZE;
          }
          DEBUG_LOG("resize to " << resize);
          reg_segment.setResize(resize);
          reg_segment.update(true);

          // write set
          char* data = (char*) reg_segment.data();
          *((int*)data) = (int) set.size();
          write_string_set(set, data + sizeof(int));
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

    static SegmentRegisterData segment_register_data;
    static Mutex segment_register_mutex;

    //##########################################################################
    //# SharedMemorySegmentRegister ############################################
    //##########################################################################

    std::set<std::string> SharedMemorySegmentRegister::getSegmentSet(){
      Mutex::Locker l(segment_register_mutex);
      SharedMemorySegmentLocker(segment_register_data.getRegisterSegment());
      return segment_register_data.getSegmentSet();
    }

    void SharedMemorySegmentRegister::addSegment(std::string name){
      Mutex::Locker l(segment_register_mutex);
      SharedMemorySegmentLocker(segment_register_data.getRegisterSegment());
      segment_register_data.addSegment(name);
    }

    void SharedMemorySegmentRegister::removeSegment(std::string name){
      Mutex::Locker l(segment_register_mutex);
      SharedMemorySegmentLocker(segment_register_data.getRegisterSegment());
      segment_register_data.removeSegment(name);
    }

    void SharedMemorySegmentRegister::resetSegment(std::string name){
      Mutex::Locker l(segment_register_mutex);
      SharedMemorySegmentLocker(segment_register_data.getRegisterSegment());
      SharedMemorySegment::resetSegment(name);
    }

    void SharedMemorySegmentRegister::resetAllSegment(){
      Mutex::Locker l(segment_register_mutex);
      SharedMemorySegmentLocker(segment_register_data.getRegisterSegment());
      std::set<std::string> set = segment_register_data.getSegmentSet();
      std::set<std::string>::iterator it;
      for(it = set.begin(); it != set.end(); ++it){
        SharedMemorySegment::resetSegment(*it);
      }
    }

  } // namespace io
}  // namespace icl
