// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Viktor Richter, Christof Elbrechter

#pragma once

#include <ICLIO/Grabber.h>
#include <ICLUtils/Time.h>
#include <ICLIO/OpenNIUtils.h>
#include <ICLUtils/Thread.h>
#include <ICLIO/OpenNIIncludes.h>
#include <mutex>

namespace icl::io {
  // Forward declaration of OpenNIGrabberImpl
  class OpenNIGrabber;

  /// Internally spawned thread class for continuous grabbing. Only one instance needed.
  class OpenNIGrabberThread : public utils::Thread {
    public:

      /// Constructor set up GrabberThread
      OpenNIGrabberThread();

      /// Destructor stops thread and releases all resources.
      ~OpenNIGrabberThread();

      /// adds a grabber to be updated every frame.
      /**
      * The thread should be stopped beforehand and restarted afterwards.
      */
      void addGrabber(OpenNIGrabber* grabber);

      /// removes a grabber so it no longer will be updated.
      /**
      * The thread should be stopped beforehand and restarted afterwards.
      */
      void removeGrabber(OpenNIGrabber* grabber);

    private:
      /// constantly calls update on OpenNI context and updates image buffers. While grabbers are registered.
      void run();

      /// internally used set of grabber pointers
      std::set<OpenNIGrabber*> m_Grabber;
  };

  /// Grabber implementation for OpenNI based camera access.
  class OpenNIGrabber : public Grabber {
    public:
      friend class OpenNIGrabberThread;

      /// The constructor
      /**
      * @param args NodeInfo of the device to use.
      */
      OpenNIGrabber(std::string args);

      /// Destructor
      ~OpenNIGrabber();

      /// grab function grabs an image (destination image is adapted on demand)
      /** @copydoc icl::io::Grabber::grab(core::ImgBase**) **/
      virtual const core::ImgBase* acquireDisplay();

      /**
          returns the underlying handle of the grabber.
          In this case the corresponding MapGenerator.
      **/
      virtual void* getHandle();

    private:
      /// makes the MapGenerator grab a new image. called repeatedly in thread.
      void grabNextDisplay();

      /**
          switches the current generator to desired. this function works but
          after changing to another Generator the camcfg-properties will not
          be refreshed.
      **/
      void setGeneratorTo(icl_openni::OpenNIMapGenerator::Generators desired);

      /// callback for changed configurable properties
      void processPropertyChange(const utils::Configurable::Property &prop);

      /// Returns the string representation of the currently used device.
      std::string getName();

      /// Mutex used for concurrency issues.
      std::recursive_mutex m_Mutex;
      /// a grabber id
      std::string m_Id;
      /// pointer to the currently used image generator
      icl_openni::OpenNIMapGenerator* m_Generator;
      /// internally used ReadWriteBuffer
      icl_openni::ReadWriteBuffer<core::ImgBase>* m_Buffer;
      /// whether double frames should be omited
      bool m_OmitDoubleFrames;
  };

  } // namespace icl::io