// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once
#include <icl/utils/CompatMacros.h>
#ifdef ICL_HAVE_QT
#include <icl/qt/Qt.h>
#include <icl/qt/Application.h>
#include <icl/qt/Quick.h>
#endif

#include <icl/utils/Thread.h>
#include <icl/utils/StringUtils.h>
#include <icl/io/GenericGrabber.h>
#include <icl/utils/ProgArg.h>
#include <mutex>
#include <icl/io/FileWriter.h>
#include <icl/core/Color.h>
#include <icl/core/Image.h>
