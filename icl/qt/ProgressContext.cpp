// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/ProgressContext.h>
#include <icl/qt/Application.h>

#include <QApplication>
#include <QProgressDialog>

#include <algorithm>
#include <functional>

namespace icl::qt {

  namespace {
    int clamp100(float p) { return std::max(0, std::min(100, (int)(p + 0.5f))); }

    /// Run \a fn on the GUI thread (inline if already there). No-op without a GUI
    /// ICLApplication, so ProgressContext is inert in headless/console contexts.
    void runOnGui(std::function<void()> fn, bool blocking) {
      ICLApplication *app = ICLApplication::instance();
      if (!app) return;
      if (ICLApplication::isGUIThreadActive()) { fn(); return; }
      app->executeInGUIThread(std::function<void(int)>([fn](int) { fn(); }), 0, blocking);
    }
  }

  struct ProgressContext::Data {
    QProgressDialog *dlg = nullptr;
  };

  ProgressContext::ProgressContext(const std::string &title, float percent,
                                   int minDurationMs, bool modal)
    : m_data(new Data) {
    Data *d = m_data.get();
    const int v = clamp100(percent);
    runOnGui([d, title, v, minDurationMs, modal]() {
      auto *dlg = new QProgressDialog(QString::fromStdString(title), QString(), 0, 100);
      dlg->setWindowTitle(QString::fromStdString(title));
      dlg->setWindowModality(modal ? Qt::ApplicationModal : Qt::NonModal);
      if (!modal) dlg->setAttribute(Qt::WA_ShowWithoutActivating);  // don't steal focus
      dlg->setCancelButton(nullptr);     // progress only, not cancellable
      dlg->setMinimumDuration(minDurationMs < 0 ? 0 : minDurationMs);
      dlg->setAutoClose(false);
      dlg->setAutoReset(false);
      dlg->setValue(v);
      if (minDurationMs <= 0) dlg->show();   // immediate; else Qt auto-shows when slow
      QApplication::processEvents();
      d->dlg = dlg;
    }, /*blocking*/ true);               // dialog exists before the ctor returns
  }

  ProgressContext &ProgressContext::operator=(float percent) {
    Data *d = m_data.get();
    const int v = clamp100(percent);
    runOnGui([d, v]() {
      if (d->dlg) { d->dlg->setValue(v); QApplication::processEvents(); }
    }, /*blocking*/ false);              // updates are fire-and-forget (FIFO before dtor)
    return *this;
  }

  void ProgressContext::setText(const std::string &text) {
    Data *d = m_data.get();
    runOnGui([d, text]() {
      if (d->dlg) { d->dlg->setLabelText(QString::fromStdString(text)); QApplication::processEvents(); }
    }, false);
  }

  void ProgressContext::setTitle(const std::string &title) {
    Data *d = m_data.get();
    runOnGui([d, title]() {
      if (d->dlg) d->dlg->setWindowTitle(QString::fromStdString(title));
    }, false);
  }

  ProgressContext::~ProgressContext() {
    Data *d = m_data.get();
    runOnGui([d]() {
      if (d->dlg) { d->dlg->close(); d->dlg->deleteLater(); d->dlg = nullptr; }
    }, /*blocking*/ true);               // dialog gone before the object is destroyed
  }

} // namespace icl::qt
