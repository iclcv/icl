// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <QtCore/QEvent>

namespace icl::qt {
  /// Utility class for threaded updatable sliders
  struct SliderUpdateEvent : public QEvent{
    int value;
    static const QEvent::Type EVENT_ID=static_cast<QEvent::Type>(QEvent::User+1);
    SliderUpdateEvent(int value):
      QEvent(EVENT_ID),value(value){
    }
  };

  } // namespace icl::qt