// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/LabelHandle.h>

#include <icl/utils/AssignRegistry.h>

#include <QPainter>
#include <QLabel>

namespace icl::qt {
  void LabelHandle::operator=(const std::string &text){
    lab()->setText(text.c_str());
    lab()->updateFromOtherThread();
  }
  void LabelHandle::operator=(const QString &text){
    lab()->setText(text);
    lab()->updateFromOtherThread();
  }
  void LabelHandle::operator=(const char *text){
    lab()->setText(QString(text));
    lab()->updateFromOtherThread();
  }
  void LabelHandle::operator=(int num){
    lab()->setNum(num);
    lab()->updateFromOtherThread();
  }
  void LabelHandle::operator=(double num){
    lab()->setNum(float(num));
    lab()->updateFromOtherThread();
  }
  void LabelHandle::operator+=(const std::string &text){
    lab()->setText(lab()->text() + text.c_str());
    lab()->updateFromOtherThread();
  }

  template<typename T>
    requires std::is_same_v<T, std::string>
  T LabelHandle::as() const {
    return lab()->text().toLatin1().data();
  }
  template std::string LabelHandle::as<std::string>() const;

  }  // namespace icl::qt

namespace {
  using icl::utils::AssignRegistry;
  using icl::qt::LabelHandle;
  __attribute__((constructor))
  static void icl_register_label_handle_assignments() {
    // Label accepts strings + arithmetic (for display); only string
    // is readable (the displayed text).
    AssignRegistry::enroll_receiver<LabelHandle, int, float, double, std::string>();
    AssignRegistry::enroll<std::string, LabelHandle>();
  }
}