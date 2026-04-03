// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/BorderHandle.h>

#include <QGroupBox>

namespace icl::qt {
  std::string BorderHandle::getTitle() const{
    const BorderHandle &bh = *this;//const_cast<BorderHandle*>(this);
    const QGroupBox *gb = *bh;
    if(gb){
      return gb->title().toLatin1().data();
    }
    return "undefined!";
  }
  void BorderHandle::operator=(const std::string &title){
    if(**this) (**this)->setTitle(title.c_str());
    (**this)->update();
  }


  } // namespace icl::qt