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

#include <ICLUtils/Time.h>
#include <ICLUtils/Thread.h>
#include <ICLUtils/Exception.h>
#include <set>


namespace icl {
  namespace io {

    class SharedMemorySegment{
      public:

        enum ErrorCode{
          /// No error occurred.
          NoError = 0x1,
          /// The operation failed because the caller didn't have the required permissions.
          PermissionDenied,
          /// A create operation failed because the requested size was invalid.
          InvalidSize,
          /// The operation failed because of an invalid key.
          KeyError,
          /// A create() operation failed because an underlying shared memory segment with the specified key already existed.
          AlreadyExists,
          /// An attach() failed because a shared memory segment with the specified key could not be found.
          NotFound,
          /// The attempt to lock() the shared memory segment failed because create() or attach() failed and returned false, or because a system error occurred in QSystemSemaphore::acquire().
          LockError,
          /// A create() operation failed because there was not enough memory available to fill the request.
          OutOfResources,
          /// Something else happened and it was bad.
          UnknownError
        };

        enum AcquisitionCode{
          created = 0x101,
          attached,
          existed,
          emptyCreate,
          error
        };

        SharedMemorySegment(std::string name="", int size=0);
        ~SharedMemorySegment();
        AcquisitionCode acquire(std::string name="", int size=0);
        void release();

        bool lock(int minSize=0, bool reduce=false);
        bool unlock();

        const void* constData() const;
        void* data();
        const void* data() const;
        void clear();

        int size() const;
        bool isAttached() const;
        bool isEmpty() const;
        void setMinSize(int size);
        int getMinSize();
        int getObserversCount();
        bool isObservable();
        std::string getName();

        static void resetSegment(std::string name);
        static std::string errorToString(SharedMemorySegment::ErrorCode error);

      private:
        struct Impl;
        mutable utils::Mutex m_Mutex;
        Impl* m_Impl;
        std::string m_Name;
    };


    struct SharedMemorySegmentLocker{
        SharedMemorySegment &segment;
        SharedMemorySegmentLocker(SharedMemorySegment &seg, int minSize=0, bool reduce=false);
        SharedMemorySegmentLocker(SharedMemorySegment* seg, int minSize=0, bool reduce=false);
        ~SharedMemorySegmentLocker();
    };

    class SharedMemorySegmentRegister
    {
      public:

        static std::set<std::string> getSegmentSet();
        static void addSegment(std::string name);
        static void removeSegment(std::string name);

        static void resetSegment(std::string name);
        static void resetAllSegment();
    };

  } // namespace io
}  // namespace icl
