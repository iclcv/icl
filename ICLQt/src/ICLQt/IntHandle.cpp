// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/IntHandle.h>
#include <string>
#include <charconv>
#include <QLineEdit>


namespace icl::qt {
  void IntHandle::operator=(int i){
    (**this)->setText(QString::number(i));
  }
  int IntHandle::getValue() const{
    auto bytes = (**this)->text().toLatin1();
    int val = 0;
    std::from_chars(bytes.data(), bytes.data() + bytes.size(), val);
    return val;
  }
  } // namespace icl::qt