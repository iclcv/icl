// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Viktor Richter, Christof Elbrechter

#pragma once
// this is done to prevent gcc from throwing warnings caused in these headers
#pragma GCC system_header
#include <pcl/search/octree.h>

// alneuman: bugfix for pcl-1.7 and clang https://github.com/introlab/rtabmap/issues/127
#include <pcl/pcl_config.h>
#if PCL_VERSION_COMPARE(<, 1, 8, 0)
#include <pcl/search/impl/search.hpp>
#endif // PCL_VERSION_COMPARE
// end bugfix
