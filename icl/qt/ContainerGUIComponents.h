// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

// The container components (HBox, VBox, HScroll, VScroll, HSplit, VSplit, Tab,
// StatusBar) are the designated-init structs in <icl/qt/ui.h>; they inherit
// ContainerGUIComponent directly and carry a polymorphic ContainerComponent.
// This header survives only as the include alias other code expects.
#include <icl/qt/ContainerGUIComponent.h>
