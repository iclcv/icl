// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/StringSignalButton.h>

namespace icl::qt {
  StringSignalButton::StringSignalButton(const QString &text,QWidget *parent):
    QPushButton(text,parent)
  {
    connect(this,SIGNAL(clicked(bool)), this, SLOT(receiveClick(bool)));
  }

  void StringSignalButton::receiveClick(bool enabled){
    static_cast<void>(enabled);
    emit clicked(text());
  }

  } // namespace icl::qt