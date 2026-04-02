// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/FloatHandle.h>
#include <string>
#include <QLineEdit>


namespace icl::qt {
    void FloatHandle::operator=(float f){
      (**this)->setText(QString::number(f));
    }
    float FloatHandle::getValue() const{
      return static_cast<float>(atof((**this)->text().toLatin1().data()));
    }
  } // namespace icl::qt