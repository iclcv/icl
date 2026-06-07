// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/config/Configurable.h>
#include <icl/core/Image.h>

#include <string>
#include <vector>

namespace icl::utils { class ProgArg; }
namespace icl::filter { class ImageUndistortion; }

namespace icl::io {
  class SourceBackend;       // detail contract (io/source/SourceBackend.h)
  struct DeviceDescription;  // io/source/DeviceDescription.h

  /// User-facing image source: string-configurable acquisition front-end. \ingroup GRABBER_G
  /** ImageSource is the recommended entry point for image acquisition in
      application code.  It selects a backend (file, camera, network, …)
      from a string device spec, owns it behind a PIMPL, and forwards
      grab() plus the "desired params" / undistortion controls to it.  The
      wrapped backend's properties (camera controls + the "desired size" /
      "undistortion.*" pseudo-properties ImageSource installs) surface as
      siblings on this ImageSource via the Configurable child mechanism.

      The set of available backends is **not** documented here — it depends
      on which optional dependencies were compiled in.  Query it at runtime:
      \code
        icl-viewer -i list      # prints every registered backend + its
                                # parameter syntax and description
      \endcode
      (programmatically: construct with device order "list", or read
      SourceBackendRegistry).  Backends self-register a description string
      via REGISTER_SOURCE_BACKEND, so the list is always in sync. */
  class ICLIO_API ImageSource : public utils::Configurable {
      struct Data;     //!< PIMPL
      Data *m_data;    //!< owned backend + device description + lock

    public:
      ImageSource(const ImageSource&) = delete;
      ImageSource& operator=(const ImageSource&) = delete;

      /// Empty default constructor — creates a null instance (adapt via init()).
      ImageSource();

      /// Construct from a program argument (two sub-parameters: device + spec).
      explicit ImageSource(const utils::ProgArg &pa);

      /// Construct with a device priority list + params (calls init()).
      ImageSource(const std::string &devicePriorityList,
                  const std::string &params,
                  bool notifyErrors = true);

      /// Destructor
      virtual ~ImageSource();

      /// (Re)initialize the backend.
      /** @param devicePriorityList comma-separated device tokens, tried in
                 order (e.g. "dc,file"); the first that yields a device wins.
                 The special token "list" prints the available-backend table
                 (see class doc) and terminates.
          @param params comma-separated per-device params, each optionally
                 extended with `\@prop=value` settings applied right after
                 instantiation, plus the special `\@info`, `\@load=file`,
                 `\@udist=file` tokens (e.g. "dc=0\@size=VGA").
          @param notifyErrors if false, no exception is thrown when no
                 suitable device is found. */
      void init(const std::string &devicePriorityList,
                const std::string &params,
                bool notifyErrors = true);

      /// init() from a program argument
      void init(const utils::ProgArg &pa);

      /// init() from a DeviceDescription (calls init(dev.type, dev.type+"="+dev.id, false))
      void init(const DeviceDescription &dev);

      /// resets resources on given devices (e.g. firewire bus)
      static void resetBus(const std::string &deviceList="dc", bool verbose=false);

      /// the active backend type string (empty if null)
      std::string getType() const;

      /// the wrapped backend (nullptr if null).  Prefer the forwarded
      /// properties (setPropertyValue) over reaching in through this.
      SourceBackend *getBackend() const;

      /// grabs the next image
      core::Image grab();

      /// whether an underlying backend could be created
      bool isNull() const;

      /// shorthand for !isNull()
      operator bool() const;

      /// @{ @name desired image parameters (forward to the wrapped backend)
      void setDesiredFormatInternal(core::format fmt);
      void setDesiredSizeInternal(const utils::Size &size);
      void setDesiredDepthInternal(core::depth d);
      core::format getDesiredFormatInternal() const;
      core::depth  getDesiredDepthInternal() const;
      utils::Size  getDesiredSizeInternal() const;

      void useDesired(core::depth d);
      void useDesired(const utils::Size &size);
      void useDesired(core::format fmt);
      void useDesired(core::depth d, const utils::Size &size, core::format fmt);

      core::depth  getDesiredDepth()  const;
      utils::Size  getDesiredSize()   const;
      core::format getDesiredFormat() const;

      bool desiredDepthUsed()  const;
      bool desiredSizeUsed()   const;
      bool desiredFormatUsed() const;

      void ignoreDesiredDepth();
      void ignoreDesiredSize();
      void ignoreDesiredFormat();
      void ignoreDesired();
      /// @}

      /// @{ @name undistortion (forward to the wrapped backend)
      void enableUndistortion(const std::string &filename);
      void enableUndistortion(const filter::ImageUndistortion &udist);
      void enableUndistortion(const utils::ProgArg &pa);
      void enableUndistortion(const core::Img32f &warpMap);
      void setUndistortionInterpolationMode(core::scalemode mode);
      void disableUndistortion();
      bool isUndistortionEnabled() const;
      const core::Img32f *getUndistortionWarpMap() const;
      /// @}

      /// list of currently available devices matching the filter string
      /** Filter is a comma-separated list of `deviceType` or
          `deviceType=deviceID` tokens. */
      static const std::vector<DeviceDescription> &getDeviceList(const std::string &filter, bool rescan=true);
  };

  } // namespace icl::io
