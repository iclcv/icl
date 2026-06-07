// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Michael Goetting, Robert

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/SteppingRange.h>
#include <icl/utils/config/Configurable.h>
#include <icl/core/ImgBase.h>
#include <icl/io/source/DeviceDescription.h>

namespace icl::core { class Image; }
namespace icl::utils { class ProgArg; }
namespace icl::filter { class ImageUndistortion; }

#include <string>
#include <vector>
#include <mutex>

namespace icl::io {
/** \cond */
class ImageSource;
/** \endcond */

/// Common interface class for all image source backends \ingroup SOURCE_G
/** The SourceBackend is ICL's common interface for image acquisition
tools. A large set of source backends is available and wrapped
by the ImageSource class. We strongly recommend to
use the ImageSource class for image acquisition within
applications.

The SourceBackend itself has a very short interface for the user:
usually, a backend is instantiated and its grab() method is
called to aquire the next available image.


\section DES Desired parameters

In addition, the SourceBackend supports a set of so called
'desired-parameters'. These can be set to overwrite the
image parameters that are used by the underlying implementation.
A FileSource e.g. will by default return images that have
the same parameter that the grabbed image file provides. However,
in some situations, the user might want to adapt these parameters
E.g. if the image parameters that are provided by the source
are not suitable for an algorithm. If this is the case, the
SourceBackend's desired parameters can be set using the
SourceBackend::setDesired-template.\n
Currently, the image parameters 'core::depth', 'size' and 'core::format'
can be adapted seperately by setting desired parameters. Once
desired parameters are set, the can be reset to the source's
default by calling SourceBackend::ignoreDesired<T> where one of the
types core::depth, core::format or icl::utils::Size is used as type T.


\section UND Image Undistortion

The SourceBackend does also provide an interface to set up
image undistortion parameters. The can be estimated
with ICL's distortion calibration tool. The undistortion
operation is accelerated using an internal warp-table.
By these means, image undistion is directly applied on the
grabbed images, which lets the user then work with
undistored images.


\section IM Implementing source backends

In order to implement a new SourceBackend class, some steps are necessary.
First, the new SourceBackend needs to be implemented. This must
implement the SourceBackend::acquireImage method, that uses an underlying
image source to acquire a single new image. This can have any
parameters and core::depth (usually, the image parameters are somehow
related to the output of the underlying image source).


\section PROP Properties

The SourceBackend implements the Configurable interface that is used
to implement dynamically settable properties. Each SourceBackend
must have at least the two properties 'core::format' and 'size'. These
are handled in a special way by the automatically created SourceBackend-
property-GUIs available in the ICLQt package.


*/
class ICLIO_API SourceBackend : public utils::Configurable{
/// internal data class
struct Data;

/// hidden data
Data *data;

public:
SourceBackend(const SourceBackend&) = delete;
SourceBackend& operator=(const SourceBackend&) = delete;

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

/// grant private method access to the ImageSource class
friend class ImageSource;

///
SourceBackend();

/// Destructor
virtual ~SourceBackend();

/// Grabs the next image and returns it as an Image value
core::Image grab();

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

/// enables the undistortion plugin for the source using radial and tangential distortion parameters
void enableUndistortion(const filter::ImageUndistortion &udist);

/// enables undistortion from given programm argument.
/** where first argument is the filename of the xml file and second is the size of picture*/
void enableUndistortion(const utils::ProgArg &pa);

/// enables undistortion for given warp map
void enableUndistortion(const core::Img32f &warpMap);

/// sets how undistortion is interpolated (supported modes are interpolateNN and interpolateLIN)
/** Please note, that this method has no effect if the undistortion was not enabled before
   using one of the SourceBackend::enableUndistortion methods. Furthermore, the setting is lost
   if the undistortion is deactivated using SourceBackend::disableUndistortion */
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

/// Main interface method, implemented by every SourceBackend backend.
/** Acquires a new image using the backend's image-acquisition path.
    Called by grab() under m_grabMutex.

    **Lifetime contract**: backends typically return an `Image` that
    shallow-shares a backend-owned internal buffer.  The returned Image
    is valid until the next acquireImage() call on the same SourceBackend —
    callers who need to retain it longer must deep-copy explicitly. */
virtual core::Image acquireImage() = 0;

/// Serializes the grab() reader path against property callbacks
/// that mutate backend state.  Recursive so a property change firing
/// during adaptGrabResult / undistortion doesn't deadlock.  Acquired
/// at the top of grab() and inside the wrapped registerCallback
/// overload above.  Mirrors UnaryOp::m_applyMutex.
mutable std::recursive_mutex m_grabMutex;

private:
/// Converts src to the active "desired" depth/size/format if any are
/// set, using data->converter into data->adaptBuffer.  Returns src
/// unchanged (shallow share) if no conversion is needed.
core::Image adaptGrabResult(const core::Image &src);

/// callback for changed configurable properties
void processPropertyChange(const utils::Configurable::Property &prop);

};

  } // namespace icl::io

// SourceBackendRegistry + REGISTER_SOURCE_BACKEND macros live in their own header;
// included here for backward compatibility so backends don't have to add
// a second include just to use the macro.
#include <icl/io/source/SourceBackendRegistry.h>