// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/Macros.h>
#include <QWidget>
#include <QtCore/QString>

namespace icl::qt {
  /// Base class for Container like GUI handles as Box- or Tab widgets
  class ContainerHandle{
    protected:
    /// must be subclassed
    ContainerHandle(){}

    public:

    /// virtual destructor
    virtual ~ContainerHandle(){}

    /// pure virtual interface
    virtual void add(QWidget *component, const QString &name=""){
      ERROR_LOG("unable to add components to this widget (name was: " << name.toLatin1().data()  << ")");
    }

    /// pure virtual interface
    virtual void insert(int idx, QWidget *component, const QString &name=""){
      ERROR_LOG("unable to insert components into this widget (name was: " << name.toLatin1().data() << ", id was: " << idx << ")");
    }

  };
  } // namespace icl::qt