// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once
#include <ICLUtils/CompatMacros.h>
#ifdef ICL_HAVE_QT
#include <ICLQt/Qt.h>
#include <ICLQt/Application.h>
#include <ICLQt/Quick.h>
#endif

#include <ICLUtils/Thread.h>
#include <ICLUtils/StringUtils.h>
#include <ICLIO/GenericGrabber.h>
#include <ICLUtils/ProgArg.h>
#include <mutex>
#include <ICLIO/FileWriter.h>
#include <ICLCore/Color.h>
#include <ICLCore/Image.h>
