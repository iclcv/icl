// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/File.h>
#include <icl/utils/plugin/PluginRegistry.h>
#include <icl/utils/Rect.h>
#include <icl/utils/Size.h>
#include <icl/utils/time/Time.h>
#include <icl/core/Img.h>
#include <icl/io/Grabber.h>

#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace icl::io {

  /// Collection of image parameters decoded from a file header. Used by
  /// file-grabber plugins (CSV, PNM, JPEG decoder) to communicate
  /// per-file metadata to their callers. Formerly nested as
  /// `FileGrabberPlugin::HeaderInfo`; hoisted to namespace scope in the
  /// Phase-4c function-plugin conversion.
  struct HeaderInfo {
    core::format imageFormat;
    core::depth  imageDepth;
    utils::Rect  roi;
    utils::Time  time;
    utils::Size  size;
    int          channelCount;
    int          imageCount;
  };

  /// Callable type stored in the file-grabber registry: `(file, dest)`
  /// → void. Allocates / updates `*dest`.
  using FileGrabberFn = std::function<void(utils::File&, core::ImgBase**)>;

  /// Process-wide registry of file-extension → grab callable.
  /** Uses `OnDuplicate::KeepHighestPriority`: strictly-higher priority
      wins, ties fall back to first-wins. Used so libpng (prio 0) beats
      ImageMagick (prio -10) for `.png` deterministically regardless of
      dyld static-init order. */
  using FileGrabberRegistry =
      utils::FunctionPluginRegistry<void(utils::File&, core::ImgBase**)>;

  /// Singleton accessor for the process-wide file-grabber registry.
  ICLIO_API FileGrabberRegistry& fileGrabberRegistry();

  /// Grabber implementation to grab from files \ingroup FILEIO_G \ingroup GRABBER_G
  class ICLIO_API FileGrabber : public Grabber {
    public:

      /// Create a NULL FileGrabber
      FileGrabber();

      /// Create a file grabber with given pattern and parameters
      FileGrabber(const std::string &pattern, bool buffer=false, bool ignoreDesiredParams=false);

      /// Destructor
      virtual ~FileGrabber();

      /// grab implementation
      virtual const core::ImgBase *acquireDisplay();

      /// returns the count of files that are available
      unsigned int getFileCount() const;

      /// returns the next to-be-grabbed file name
      const std::string &getNextFileName() const;

      /// pre-buffers all images internally
      void bufferImages(bool omitExceptions=true);

      /// internally skips the next to-be-grabbed image
      void next();

      /// internally sets the next-image pointer back
      void prev();

      /// forces the filegrabber to use a plugin for the given suffix
      void forcePluginType(const std::string &suffix);

    private:
      const core::ImgBase *grabDisplay();
      void addProperties();
      void processPropertyChange(const utils::Configurable::Property &p);
      void updateProperties(const core::ImgBase* img);

      struct Data;
      Data *m_data;
      std::recursive_mutex m_propertyMutex;
      bool m_updatingProperties;
  };

  } // namespace icl::io

/// Self-register a grab callable for a given file extension.
/** Mirror of `REGISTER_FILE_WRITER_PLUGIN`. FACTORY is a callable of
    signature `void(utils::File&, core::ImgBase**)` — typically a lambda
    with a function-local static impl instance. */
#define REGISTER_FILE_GRABBER_PLUGIN(TAG, EXTENSION, ...)                       \
  ICL_REGISTER_PLUGIN(::icl::io::fileGrabberRegistry(), TAG, EXTENSION, __VA_ARGS__)
