// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/LabelHandle.h>
#include <QPainter>
#include <QLabel>

namespace icl{
  namespace qt{
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
  } // namespace qt
}
