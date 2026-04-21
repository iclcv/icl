// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Registers the legacy `Event -> H` smuggling rules for every handle
// that `DataStore::Data::render() / install() / link() /
// registerCallback() / enable() / disable()` can reach.  The
// alternative — replacing the smuggling in `DataStore::Data` with
// direct method dispatch over the stored handle type — is a cleaner
// refactor but out of scope here; see the "MultiTypeMap -> AnyMap"
// item on TODO.md, which will rewrite `Data::*` in one pass.

#include <icl/utils/Assign.h>
#include <icl/utils/AssignRegistry.h>
#include <icl/utils/Macros.h>

#include <icl/qt/DataStore.h>  // Event
#include <icl/qt/MouseHandler.h>

// Concept checks below reach widget methods through `handle->enable()` /
// `(*handle)->install()` / etc., which require the wrapped QWidget
// subclasses to be complete types at the point of instantiation.
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>

#include <icl/qt/ButtonGroupHandle.h>
#include <icl/qt/ButtonHandle.h>
#include <icl/qt/CheckBoxHandle.h>
#include <icl/qt/ColorHandle.h>
#include <icl/qt/ComboHandle.h>
#include <icl/qt/DispHandle.h>
#include <icl/qt/DrawHandle.h>
#include <icl/qt/DrawHandle3D.h>
#include <icl/qt/DrawWidget3D.h>

// GLCallback lives inside ICLDrawWidget3D — pull a short alias into
// scope so the concept below can name it without dragging the nested
// scope through the `requires` expression.
namespace icl::qt {
  using GLCallback = ICLDrawWidget3D::GLCallback;
}
#include <icl/qt/FPSHandle.h>
#include <icl/qt/FSliderHandle.h>
#include <icl/qt/FloatHandle.h>
#include <icl/qt/ImageHandle.h>
#include <icl/qt/IntHandle.h>
#include <icl/qt/LabelHandle.h>
#include <icl/qt/PlotHandle.h>
#include <icl/qt/SliderHandle.h>
#include <icl/qt/SpinnerHandle.h>
#include <icl/qt/StateHandle.h>
#include <icl/qt/StringHandle.h>
#include <icl/qt/TabHandle.h>

namespace icl::utils {

  // Detection concepts — each handle implements a different subset of
  // the render/install/link/… surface.  `Assign<H, Event>::apply` uses
  // `if constexpr` to reach only the methods the target handle has.

  template<class H>
  concept HasRender = requires(H h) { h.render(); };

  // Handles that wrap a widget with an .install(MouseHandler*) method
  // (ImageHandle, DrawHandle, DrawHandle3D).  The legacy smuggler
  // reached the method via `(*handle)->install(...)` — derefs the
  // GUIHandle<T> to get the widget pointer.
  template<class H>
  concept HasInstallMouse = requires(H h, icl::qt::MouseHandler *m) {
    (*h)->install(m);
  };

  // Only DrawHandle3D currently supports .link(GLCallback*), also via
  // the widget derefence pattern.
  template<class H>
  concept HasLink = requires(H h, icl::qt::GLCallback *cb) {
    (*h)->link(cb);
  };

  template<class H>
  concept HasRegisterCallback = requires(H h, const std::function<void()> &cb) {
    h.registerCallback(cb);
  };

  template<class H>
  concept HasRegisterComplexCallback = requires(H h,
    const std::function<void(const std::string &)> &cb) {
    h.registerCallback(cb);
  };

  template<class H>
  concept HasEnableDisable = requires(H h) { h.enable(); h.disable(); };

  // Specialization of the Assign trait for `H = Event`.  Any handle
  // that lets at least one of the event verbs through satisfies
  // `value = true`.  The `apply()` body is a message-string dispatch,
  // matching the legacy `AssignSpecial<Event, H>` behaviour.
  template<class H>
    requires (HasRender<H> || HasInstallMouse<H> || HasLink<H>
              || HasRegisterCallback<H> || HasRegisterComplexCallback<H>
              || HasEnableDisable<H>)
  struct Assign<H, icl::qt::DataStore::Data::Event>
      : std::true_type {
    static void apply(H &dst, icl::qt::DataStore::Data::Event &src) {
      const std::string &m = src.message;
      if (m == "register") {
        if constexpr (HasRegisterCallback<H>) dst.registerCallback(src.cb);
      } else if (m == "register-complex") {
        if constexpr (HasRegisterComplexCallback<H>) dst.registerCallback(src.cb2);
      } else if (m == "enable") {
        if constexpr (HasEnableDisable<H>) dst.enable();
      } else if (m == "disable") {
        if constexpr (HasEnableDisable<H>) dst.disable();
      } else if (m == "render") {
        if constexpr (HasRender<H>) dst.render();
      } else if (m == "install") {
        if constexpr (HasInstallMouse<H>)
          (*dst)->install(static_cast<icl::qt::MouseHandler *>(src.data));
      } else if (m == "link") {
        if constexpr (HasLink<H>)
          (*dst)->link(static_cast<icl::qt::GLCallback *>(src.data));
      } else {
        ERROR_LOG("DataStore Event '" << m << "' not supported by handle type "
                  << typeid(H).name());
      }
    }
  };

}  // namespace icl::utils

namespace {
  using icl::utils::AssignRegistry;
  using icl::qt::DataStore;
  using Event = DataStore::Data::Event;

  __attribute__((constructor))
  static void icl_register_event_dispatch_rules() {
    // All handles that plausibly participate in the legacy
    // render/install/register/enable/disable smuggling.
    AssignRegistry::enroll<icl::qt::ImageHandle,       Event>();
    AssignRegistry::enroll<icl::qt::DrawHandle,        Event>();
    AssignRegistry::enroll<icl::qt::DrawHandle3D,      Event>();
    AssignRegistry::enroll<icl::qt::PlotHandle,        Event>();
    AssignRegistry::enroll<icl::qt::FPSHandle,         Event>();
    AssignRegistry::enroll<icl::qt::SliderHandle,      Event>();
    AssignRegistry::enroll<icl::qt::FSliderHandle,     Event>();
    AssignRegistry::enroll<icl::qt::SpinnerHandle,     Event>();
    AssignRegistry::enroll<icl::qt::IntHandle,         Event>();
    AssignRegistry::enroll<icl::qt::FloatHandle,       Event>();
    AssignRegistry::enroll<icl::qt::StringHandle,      Event>();
    AssignRegistry::enroll<icl::qt::CheckBoxHandle,    Event>();
    AssignRegistry::enroll<icl::qt::ButtonHandle,      Event>();
    AssignRegistry::enroll<icl::qt::ButtonGroupHandle, Event>();
    AssignRegistry::enroll<icl::qt::ComboHandle,       Event>();
    AssignRegistry::enroll<icl::qt::LabelHandle,       Event>();
    AssignRegistry::enroll<icl::qt::ColorHandle,       Event>();
    AssignRegistry::enroll<icl::qt::TabHandle,         Event>();
    AssignRegistry::enroll<icl::qt::StateHandle,       Event>();
    AssignRegistry::enroll<icl::qt::DispHandle,        Event>();
  }
}
