// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/IntHandle.h>
#include <string>
#include <QLineEdit>


namespace icl{
  namespace qt{

    void IntHandle::operator=(int i){
      (**this)->setText(QString::number(i));
    }
    int IntHandle::getValue() const{
      return atoi((**this)->text().toLatin1().data());
    }
  } // namespace qt
}
