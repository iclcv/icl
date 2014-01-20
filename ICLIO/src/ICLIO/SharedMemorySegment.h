/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/SharedMemorySegment.h                  **
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

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Time.h>
#include <ICLUtils/Thread.h>
#include <ICLUtils/Exception.h>
#include <set>


namespace icl {
  namespace io {

    /// Implementation of a cross-process shared memory
    /**
      The SharedMemorySegment can be used to share information between
      multiple processes on a single system.

      This class uses in iternal singleton implementation and map allowing
      multiple thereads to read a ShredMemory concurrent. The drawback is
      that external locking is needed for concurrent reading and writing.

      Created SharedMemorySegments are registered via the
      SharedMemorySegmentRegister.

      A signal handler is installed to ensure that all SharedMemorySegments
      are released.
    **/
    class ICLIO_API SharedMemorySegment{
      public:

        /// Creates a SharedMemorySegment instance.
        /**
          No memory allocation is done here. To connect to the corresponding
          SharedMemorySegment call lock().
          @param name The name identifying the segment.
          @param minsize The minimal needed size.
        **/
        SharedMemorySegment(std::string name="", int minsize=1);

        /// The destructor basically calls release
        ~SharedMemorySegment();

        /// Releases the acuired SharedMemorySegment.
        /**
          When the last instance calls release the underlying singleton
          implementation unlocks and detatches from the systems SharedMemory.
        **/
        void release();

        /// Returns the current SharedMemorySegments name
        std::string getName();

        /// Sets name and minsize to the passed values.
        /**
          This function just sets the variables. Potential releasing and
          reattaching happens on the following lock().
        **/
        void reset(std::string name, int minsize=0);

        /// Tells whether this instance is currently attached to a SharedMemory
        /**
          Attachment is performed lock() and detachment release().
        **/
        bool isAttached();

        /// Locks the underlying SharedMemorySegment, aquireing when needed.
        /**
          Here happens all the fun.
          * When this instance is not connected, it will be connected.
          * When minsize > getSize or a resize is requested, this will detach
            and reattach/recreate with the needed size.
          * Every call to lock needs a call to unlock.
          * Multiple threads of the same process cat get the lock
            simultaneously. That way, concurrent access in one process is
            possible but extra locking is necessary when this is not desired.
          * Can return true without having the necessary size when already
            locked by this process.
          @return whether the segment is locked. may sometimes not be reached,
                  when this process already holds the lock.
        **/
        bool lock(int minsize=0);

        /// Unlocks the underlying SharedMemorySegment.
        /**
          @return whether the segment is not locked. true when still locked.
        **/
        bool unlock();

        /// Forces a resize of the SharedMemory to the passed minsize
        /**
          This can shrink the SharedMemorySegment when the current space is
          no longer needed. It will force a recreation of the underlying
          SharedMemory to the passed size. It will not prevend other processes
          from re-enlarging the Segment.
          Caution: Don't use this to ensure that the SharedMemory is large
          enough. Call lock() with the desired size instead.
        **/
        void forceMinSize(int minsize);

        /// Returns the current size of the SharedMemorySegment in bytes
        /**
          @return -1 when this instance is not attached
        **/
        int getSize();

        /// Tells whether the Segment is empty
        /**
          Basically checks whether all data bits are zero. A freshly created
          SharedMemorySegment is always overwritten with zeros.
        **/
        bool isEmpty() const;

        /// getter for the MemorySegment data.
        /**
          @return a pointer to the memory block. Null when not attached or not
                  locked.
        **/
        void* data();

        /// implicit const getter for the MemorySegment data.
        /**
          @return a pointer to the memory block. Null when not attached or not
                  locked.
        **/
        const void* data() const;

        /// explicit const getter for the MemorySegment data.
        /**
          @return a pointer to the memory block. Null when not attached or not
                  locked.
        **/
        const void* constData() const;

        /// internal implementation
        struct Impl;
      private:
        /// instance locking
        mutable utils::Mutex m_Mutex;

        /// pointer to the internal implementation
        Impl* m_Impl;

        /// the current name of the SharedMemorySegment
        std::string m_Name;

        /// local minimal size tf the Segment
        int m_Minsize;
    };

    /// an easy way to lock a SharedMemorySegment, unlocking at destruction.
    struct ICLIO_API SharedMemorySegmentLocker{
        /// local reference to SharedMemorySegment
        SharedMemorySegment &segment;

        /// The constructor calls lock() on the SharedMemorySement.
        SharedMemorySegmentLocker(SharedMemorySegment &seg, int minSize=0);

        /// The constructor calls lock() on the SharedMemorySement.
        SharedMemorySegmentLocker(SharedMemorySegment* seg, int minSize=0);

        /// The destructor calls unlock() on the SharedMemorySement.
        ~SharedMemorySegmentLocker();
    };

    /// This is used to register all created SharedMemorySegments
    /**
      Each SharedMemorySegment::Impl instance adds its segment name to the
      register on creation and removes it on destruction.
    **/
    class ICLIO_API SharedMemorySegmentRegister
    {
      public:
        friend struct SharedMemorySegment::Impl;

        /// Returns a set of the currently registered  SharedMemorySegments
        static std::set<std::string> getSegmentSet();

        /// Linux only. When a SharedMemory was not released correctly this can help.
        /**
          Attaches and detatches the Segment identified by 'name'. When no other
          instances are connected. This will free the system memory segment.
        **/
        static void freeSegment(std::string name);

        /// calls freeSegment() on all registered SharedMemorySegments
        static void freeAllSegments();

        /// Linux only. Releases all SharedMemorySegments and SystemSemaphores
        /**
          All SharedMemorySegments and SystemSemaphores starting with adress
          '0x51' will be freed by force. This is the last resource when
          something realy bad happened.
        **/
        static void resetBus();

      private:
        /// Adds a segment name to the register
        static void addSegment(std::string name);

        /// Removes a segment name to the register
        static void removeSegment(std::string name);
    };

  } // namespace io
}  // namespace icl
