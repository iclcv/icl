// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Michael Goetting, Robert

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/plugin/PluginRegistry.h>
#include <icl/utils/SteppingRange.h>
#include <icl/utils/config/Configurable.h>
#include <icl/core/ImgBase.h>
#include <icl/io/grabber/GrabberDeviceDescription.h>

namespace icl::core { class Image; }
namespace icl::utils { class ProgArg; }
namespace icl::filter { class ImageUndistortion; }

#include <functional>
#include <string>
#include <vector>
#include <set>
#include <mutex>

namespace icl::io {
/** \cond */
template <class T> class GrabberHandle;
class GenericGrabber;
/** \endcond */

/// Common interface class for all grabbers \ingroup GRABBER_G
/** The Grabber is ICL's common interface for image acquisition
tools. A large set of Grabbers is available and wrapped
by the GenericGrabber class. We strongly recommend to
use the GenericGrabber class for image acquisition within
applications.

The Grabber itself has a very short interface for the user:
usually, a grabber is instantiated and its grab() method is
called to aquire the next available image.


\section DES Desired parameters

In addition, the Grabber supports a set of so called
'desired-parameters'. These can be set to overwrite the
image parameters that are used by the underlying implementation.
A FileGrabber e.g. will by default return images that have
the same parameter that the grabbed image file provides. However,
in some situations, the user might want to adapt these parameters
E.g. if the image parameters that are provided by the grabber
are not suitable for an algorithm. If this is the case, the
Grabber's desired parameters can be set using the
Grabber::setDesired-template.\n
Currently, the image parameters 'core::depth', 'size' and 'core::format'
can be adapted seperately by setting desired parameters. Once
desired parameters are set, the can be reset to the grabber's
default by calling grabber::ignoreDesired<T> where one of the
types core::depth, core::format or icl::utils::Size is used as type T.


\section UND Image Undistortion

The Grabber does also provide an interface to set up
image undistortion parameters. The can be estimated
with ICL's distortion calibration tool. The undistortion
operation is accelerated using an internal warp-table.
By these means, image undistion is directly applied on the
grabbed images, which lets the user then work with
undistored images.


\section IM Implementing Grabbers

In order to implement a new Grabber class, some steps are necessary.
First, the new Grabber needs to be implemented. This must
implement the Grabber::acquireImage method, that uses an underlying
image source to acquire a single new image. This can have any
parameters and core::depth (usually, the image parameters are somehow
related to the output of the underlying image source).
If the grabber is available, one should think about adapting
the grabber to inherit the icl::GrabberHandle class that adds
the ability of instantiating one Grabber several times without
having to handle double device accesses explicitly.


\section PROP Properties

The Grabber implements the Configurable interface that is used
to implement dynamically settable properties. Each Grabber
must have at least the two properties 'core::format' and 'size'. These
are handled in a special way by the automatically created Grabber-
property-GUIs available in the ICLQt package.


*/
class ICLIO_API Grabber : public utils::Configurable{
/// internal data class
struct Data;

/// hidden data
Data *data;

public:
Grabber(const Grabber&) = delete;
Grabber& operator=(const Grabber&) = delete;

protected:
/// internally set a desired format
virtual void setDesiredFormatInternal(core::format fmt);

/// internally set a desired format
virtual void setDesiredSizeInternal(const utils::Size &size);

/// internally set a desired format
virtual void setDesiredDepthInternal(core::depth d);

/// returns the desired format
virtual core::format getDesiredFormatInternal() const;

/// returns the desired format
virtual core::depth getDesiredDepthInternal() const;

/// returns the desired format
virtual utils::Size getDesiredSizeInternal() const;

public:

/// grant private method access to the grabber handle template
template<class X> friend class GrabberHandle;

/// grant private method access to the GenericGrabber class
friend class GenericGrabber;

///
Grabber();

/// Destructor
virtual ~Grabber();

/// Grabs the next image and returns it as an Image value
core::Image grabImage();

/// @{ @name desired image parameters

/// Override the depth that grab() returns; reset with ignoreDesiredDepth().
void useDesired(core::depth d)             { setDesiredDepthInternal(d); }
/// Override the size that grab() returns; reset with ignoreDesiredSize().
void useDesired(const utils::Size &size)   { setDesiredSizeInternal(size); }
/// Override the format that grab() returns; reset with ignoreDesiredFormat().
void useDesired(core::format fmt)          { setDesiredFormatInternal(fmt); }
/// Override all three params at once.
void useDesired(core::depth d, const utils::Size &size, core::format fmt);

/// Returns the current desired depth (depth(-1) if not overridden).
core::depth  getDesiredDepth()  const { return getDesiredDepthInternal();  }
/// Returns the current desired size (Size::null if not overridden).
utils::Size  getDesiredSize()   const { return getDesiredSizeInternal();   }
/// Returns the current desired format (format(-1) if not overridden).
core::format getDesiredFormat() const { return getDesiredFormatInternal(); }

/// True iff a desired depth has been set (i.e. not the sentinel).
bool desiredDepthUsed()  const { return static_cast<int>(getDesiredDepth())  != -1; }
/// True iff a desired size has been set.
bool desiredSizeUsed()   const { return getDesiredSize() != utils::Size::null; }
/// True iff a desired format has been set.
bool desiredFormatUsed() const { return static_cast<int>(getDesiredFormat()) != -1; }

/// Reset the depth override (subsequent grabs use the backend's native depth).
void ignoreDesiredDepth()  { setDesiredDepthInternal(static_cast<core::depth>(-1)); }
/// Reset the size override.
void ignoreDesiredSize()   { setDesiredSizeInternal(utils::Size::null); }
/// Reset the format override.
void ignoreDesiredFormat() { setDesiredFormatInternal(static_cast<core::format>(-1)); }

/// Reset all three overrides.
void ignoreDesired();

/// @}

/// @{ @name distortion functions

/// enables the undistorion
void enableUndistortion(const std::string &filename);

/// enables the undistortion plugin for the grabber using radial and tangential distortion parameters
void enableUndistortion(const filter::ImageUndistortion &udist);

/// enables undistortion from given programm argument.
/** where first argument is the filename of the xml file and second is the size of picture*/
void enableUndistortion(const utils::ProgArg &pa);

/// enables undistortion for given warp map
void enableUndistortion(const core::Img32f &warpMap);

/// sets how undistortion is interpolated (supported modes are interpolateNN and interpolateLIN)
/** Please note, that this method has no effect if the undistortion was not enabled before
   using one of the Grabber::enableUndistortion methods. Furthermore, the setting is lost
   if the undistortion is deactivated using Grabber::disableUndistortion */
void setUndistortionInterpolationMode(core::scalemode mode);

/// disables distortion
void disableUndistortion();

/// returns whether distortion is currently enabled
bool isUndistortionEnabled() const;

/// returns the internal warp map or NULL if undistortion is not enabled
const core::Img32f *getUndistortionWarpMap() const;
/// @}

/// Same as Configurable::registerCallback, but wraps the callback so it
/// acquires `m_grabMutex` before firing. Lets backends that mutate
/// internal state from a property change (typically via
/// processPropertyChange) rely on the base for the callback-side lock
/// against the grab() reader path. Mirrors UnaryOp::registerCallback —
/// see project_configurable_op_threadsafety.md for rationale.  Returns
/// a token that can be passed to Configurable::removeCallback.
utils::Configurable::CallbackToken registerCallback(utils::Configurable::Callback cb);

protected:


/// Main interface method, implemented by every Grabber backend.
/** Acquires a new image using the backend's image acquisition path.
    Called by grab() under m_grabMutex; backends may return an internal
    buffer whose lifetime extends until the next acquireImage() call. */
virtual const core::ImgBase *acquireImage() = 0;

/// Utility function that allows for much easier implementation of grabUD
/** called by the grabbers grab() method **/
const core::ImgBase *adaptGrabResult(const core::ImgBase *src, core::ImgBase **dst);

/// Serializes the grab() reader against property callbacks that mutate
/// backend state. Recursive so a property change firing during
/// adaptGrabResult / undistortion (which uses internal WarpOp etc.)
/// doesn't deadlock. Acquired at the top of Grabber::grab() and
/// inside the wrapped registerCallback overload above. Mirrors
/// UnaryOp::m_applyMutex.
mutable std::recursive_mutex m_grabMutex;

protected:
/// Internal funnel: locks m_grabMutex, calls acquireImage(), runs the
/// adaptGrabResult + warp pipeline.  Subclasses implement acquireImage()
/// instead of overriding this; FileGrabber uses it from bufferImages()
/// to pre-load into ImgBase* slots (eventually a vector<Image>).
const core::ImgBase *grab(core::ImgBase **dst=0);

private:
/// callback for changed configurable properties
void processPropertyChange(const utils::Configurable::Property &prop);

};

class ICLIO_API GrabberRegistry {
public:
  GrabberRegistry(const GrabberRegistry&) = delete;
  GrabberRegistry &operator=(const GrabberRegistry&) = delete;

  using CreateFn     = std::function<Grabber*(const std::string&)>;
  using DeviceListFn = std::function<const std::vector<GrabberDeviceDescription>&(std::string, bool)>;
  using BusResetFn   = std::function<void(bool)>;

  /// Underlying primitive for the factory map. Device lists, bus resets
  /// and description strings are per-backend side concerns kept on the
  /// class itself.
  using Registry = utils::PluginRegistry<std::string, CreateFn>;

  static GrabberRegistry* getInstance();

  void registerGrabberType(const std::string &grabberid,
                           CreateFn creator,
                           DeviceListFn device_list);

  void registerGrabberBusReset(const std::string &grabberid,
                               BusResetFn reset_function);

  void addGrabberDescription(const std::string &grabber_description);

  Grabber* createGrabber(const std::string &grabberid, const std::string &param);

  std::vector<std::string> getRegisteredGrabbers();

  std::vector<std::string> getGrabberInfos();

  const std::vector<GrabberDeviceDescription>& getDeviceList(std::string id, std::string hint="", bool rescan=true);

  void resetGrabberBus(const std::string &id, bool verbose);

private:
  GrabberRegistry() : m_factories(utils::OnDuplicate::Throw) {}

  Registry m_factories;                                                     //!< id → CreateFn
  std::recursive_mutex m_mutex;                                             //!< guards the side maps below
  std::map<std::string, DeviceListFn, std::less<>> m_deviceLists;           //!< id → listing function
  std::map<std::string, BusResetFn,   std::less<>> m_busResets;             //!< id → bus reset function
  std::set<std::string>                            m_descriptions;          //!< verbatim strings, for getGrabberInfos()
};

/** \endcond */

/// registration macro for grabbers
/** @see \ref REG */
#define REGISTER_GRABBER(NAME,CREATE_FUNC,DEVICE_LIST_FUNC,DESCRIPTION)        \
  extern "C" __attribute__((constructor, used)) void                           \
  iclRegisterGrabber_##NAME() {                                                \
    auto *_inst = ::icl::io::GrabberRegistry::getInstance();                   \
    _inst->registerGrabberType(#NAME, CREATE_FUNC, DEVICE_LIST_FUNC);          \
    _inst->addGrabberDescription(DESCRIPTION);                                 \
  }

#define REGISTER_GRABBER_BUS_RESET_FUNCTION(NAME,BUS_RESET_FUNC)               \
  extern "C" __attribute__((constructor, used)) void                           \
  iclRegisterGrabberBusReset_##NAME() {                                        \
    ::icl::io::GrabberRegistry::getInstance()                                  \
        ->registerGrabberBusReset(#NAME, BUS_RESET_FUNC);                      \
  }

  } // namespace icl::io