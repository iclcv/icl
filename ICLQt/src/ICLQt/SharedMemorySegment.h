/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/SharedMemorySegmentRegister.h            **
** Module : ICLIO                                                  **
** Authors: Viktor Richter                                         **
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
          NoError = 0,
          /// The operation failed because the caller didn't have the required permissions.
          PermissionDenied = 1,
          /// A create operation failed because the requested size was invalid.
          InvalidSize = 2,
          /// The operation failed because of an invalid key.
          KeyError = 3,
          /// A create() operation failed because an underlying shared memory segment with the specified key already existed.
          AlreadyExists = 4,
          /// An attach() failed because a shared memory segment with the specified key could not be found.
          NotFound = 5,
          /// The attempt to lock() the shared memory segment failed because create() or attach() failed and returned false, or because a system error occurred in QSystemSemaphore::acquire().
          LockError = 6,
          /// A create() operation failed because there was not enough memory available to fill the request.
          OutOfResources = 7,
          /// Something else happened and it was bad.
          UnknownError = 8
        };

        enum AcquisitionCode{
          created = 101,
          attached = 102,
          existed = 103
        };

        SharedMemorySegment();
        ~SharedMemorySegment();
        AcquisitionCode acquire(std::string name, int size);
        void release();

        bool lock();
        bool unlock();

        const void* constData() const;
        void* data();
        const void* data() const;

        int size() const;
        bool isAttached() const;
        bool setResize(int size);
        bool update(bool poll = false);
        int getObserversCount();
        bool isObservable();

        static void resetSegment(std::string name);

      private:
        struct Impl;
        mutable utils::Mutex m_Mutex;
        Impl* m_Impl;
    };


    struct SharedMemorySegmentLocker{
        SharedMemorySegment &segment;
        SharedMemorySegmentLocker(SharedMemorySegment &seg);
        SharedMemorySegmentLocker(SharedMemorySegment* seg);
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
