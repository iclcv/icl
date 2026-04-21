// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Implementation of `DataStore::Data::render() / install() / link() /
// registerCallback() / enable() / disable() / removeCallbacks()`.
//
// These methods used to smuggle a `DataStore::Data::Event` payload
// through `AssignRegistry::dispatch` (resolving to an
// `Assign<H, Event>::apply` specialization per handle type).  With
// `DataStore` now holding entries as `std::any`, there's no reason
// to route verb dispatch through the assign machinery at all — we
// can type-cascade directly over the stored object.  This TU owns
// that cascade.  The `Event` struct + `Assign<H, Event>` trait +
// `HandleEventEnrollments.cpp` are deleted in the same commit.

#include <icl/qt/DataStore.h>

#include <icl/qt/MouseHandler.h>

#include <icl/qt/BorderHandle.h>
#include <icl/qt/BoxHandle.h>
#include <icl/qt/ButtonGroupHandle.h>
#include <icl/qt/ButtonHandle.h>
#include <icl/qt/CheckBoxHandle.h>
#include <icl/qt/ColorHandle.h>
#include <icl/qt/ComboHandle.h>
#include <icl/qt/DispHandle.h>
#include <icl/qt/DrawHandle.h>
#include <icl/qt/DrawHandle3D.h>
#include <icl/qt/DrawWidget3D.h>  // for GLCallback
#include <icl/qt/FPSHandle.h>
#include <icl/qt/FSliderHandle.h>
#include <icl/qt/FloatHandle.h>
#include <icl/qt/ImageHandle.h>
#include <icl/qt/IntHandle.h>
#include <icl/qt/LabelHandle.h>
#include <icl/qt/MultiDrawHandle.h>
#include <icl/qt/PlotHandle.h>
#include <icl/qt/SliderHandle.h>
#include <icl/qt/SpinnerHandle.h>
#include <icl/qt/SplitterHandle.h>
#include <icl/qt/StateHandle.h>
#include <icl/qt/StringHandle.h>
#include <icl/qt/TabHandle.h>

// Widget headers are required to complete the `(*handle)->method()`
// paths reached by the `HasInstallMouse` / `HasLink` concepts.
#include <icl/qt/Widget.h>
#include <icl/qt/DrawWidget.h>

#include <icl/utils/Macros.h>

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>

#include <any>
#include <functional>
#include <tuple>

namespace icl::qt {

  namespace {

    // List of every handle type the DataStore may store.  Extended when
    // a new handle is added.  Order is irrelevant — visitHandle short-
    // circuits on the first type_info match.
    //
    // `MultiDrawHandle` is intentionally *included* here even though
    // its QObject base means `Assign<H, H>` isn't enrolled — the
    // visitor only needs `std::any_cast<H>(&any)`, which doesn't
    // require copy-assignment.
    using AllHandles = std::tuple<
      SliderHandle, FSliderHandle, SpinnerHandle,
      IntHandle, FloatHandle, StringHandle,
      CheckBoxHandle, ButtonHandle, ButtonGroupHandle,
      ComboHandle, LabelHandle, ColorHandle,
      ImageHandle, DrawHandle, DrawHandle3D,
      PlotHandle, FPSHandle, TabHandle, StateHandle,
      DispHandle, BorderHandle, BoxHandle, SplitterHandle,
      MultiDrawHandle
    >;

    // Apply `f` to the concrete handle stored in `entry`.  Returns
    // true iff the entry was one of the types in `AllHandles`.  On
    // match, `f` is invoked with a `H &` for the stored type.
    template<typename F>
    bool visitHandle(std::any &entry, F f) {
      bool matched = false;
      auto try_cast = [&]<typename H> {
        if (matched) return;
        if (auto *h = std::any_cast<H>(&entry)) {
          f(*h);
          matched = true;
        }
      };
      std::apply([&](auto... hs) {
        (try_cast.template operator()<decltype(hs)>(), ...);
      }, AllHandles{});
      return matched;
    }

    // Capability concepts.  Mirror the ones previously living in
    // HandleEventEnrollments.cpp.
    template<class H> concept HasRender = requires(H h) { h.render(); };
    template<class H> concept HasInstallMouse = requires(H h, MouseHandler *m) {
      (*h)->install(m);
    };
    template<class H> concept HasLink = requires(H h, GLCallback *cb) {
      (*h)->link(cb);
    };
    template<class H> concept HasRegisterCallback = requires(H h, const std::function<void()> &cb) {
      h.registerCallback(cb);
    };
    template<class H> concept HasRegisterComplexCallback = requires(H h,
      const std::function<void(const std::string &)> &cb) {
      h.registerCallback(cb);
    };
    template<class H> concept HasEnableDisable = requires(H h) { h.enable(); h.disable(); };
    template<class H> concept HasRemoveCallbacks = requires(H h) { h.removeCallbacks(); };

    // Utility — shorthand for "dispatch over handle with error log on
    // miss / unknown type".
    template<typename F>
    void dispatchHandle(std::any &entry, const char *verb, F f) {
      if (!visitHandle(entry, f)) {
        ERROR_LOG("DataStore::Data::" << verb
                  << "() not supported for type " << entry.type().name());
      }
    }

  }  // namespace

  // --- Verb implementations --------------------------------------------

  void DataStore::Data::render() {
    dispatchHandle(*m_entry, "render", [](auto &h) {
      using H = std::decay_t<decltype(h)>;
      if constexpr (HasRender<H>) h.render();
      else ERROR_LOG("render() not supported by this handle type");
    });
  }

  void DataStore::Data::link(GLCallback *cb) {
    dispatchHandle(*m_entry, "link", [cb](auto &h) {
      using H = std::decay_t<decltype(h)>;
      if constexpr (HasLink<H>) (*h)->link(cb);
      else ERROR_LOG("link() not supported by this handle type");
    });
  }

  void DataStore::Data::install(MouseHandler *data) {
    dispatchHandle(*m_entry, "install", [data](auto &h) {
      using H = std::decay_t<decltype(h)>;
      if constexpr (HasInstallMouse<H>) (*h)->install(data);
      else ERROR_LOG("install() not supported by this handle type");
    });
  }

  void DataStore::Data::install(std::function<void(const MouseEvent &)> f) {
    // Adapter — keeps the function alive inside a MouseHandler
    // subclass so the handle's existing install(MouseHandler*) path
    // works.
    struct FunctionMouseHandler : public MouseHandler {
      std::function<void(const MouseEvent &)> f;
      FunctionMouseHandler(std::function<void(const MouseEvent &)> f) : f(f) {}
      void process(const MouseEvent &e) { f(e); }
    };

    install(new FunctionMouseHandler(f));
  }

  void DataStore::Data::registerCallback(const std::function<void()> &cb) {
    dispatchHandle(*m_entry, "registerCallback", [&cb](auto &h) {
      using H = std::decay_t<decltype(h)>;
      if constexpr (HasRegisterCallback<H>) h.registerCallback(cb);
      else ERROR_LOG("registerCallback() not supported by this handle type");
    });
  }

  void DataStore::Data::registerCallback(const std::function<void(const std::string &)> &cb) {
    dispatchHandle(*m_entry, "registerCallback(complex)", [&cb](auto &h) {
      using H = std::decay_t<decltype(h)>;
      if constexpr (HasRegisterComplexCallback<H>) h.registerCallback(cb);
      else ERROR_LOG("registerCallback(complex) not supported by this handle type");
    });
  }

  void DataStore::Data::enable() {
    dispatchHandle(*m_entry, "enable", [](auto &h) {
      using H = std::decay_t<decltype(h)>;
      if constexpr (HasEnableDisable<H>) h.enable();
      else ERROR_LOG("enable() not supported by this handle type");
    });
  }

  void DataStore::Data::disable() {
    dispatchHandle(*m_entry, "disable", [](auto &h) {
      using H = std::decay_t<decltype(h)>;
      if constexpr (HasEnableDisable<H>) h.disable();
      else ERROR_LOG("disable() not supported by this handle type");
    });
  }

  void DataStore::Data::removeCallbacks() {
    dispatchHandle(*m_entry, "removeCallbacks", [](auto &h) {
      using H = std::decay_t<decltype(h)>;
      if constexpr (HasRemoveCallbacks<H>) h.removeCallbacks();
      else ERROR_LOG("removeCallbacks() not supported by this handle type");
    });
  }

}  // namespace icl::qt
