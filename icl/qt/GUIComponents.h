// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

// The concrete GUI components (Slider, Button, Label, Display, ...) are the
// designated-init structs in <icl/qt/ui.h>; each is a polymorphic GUIComponent
// that builds its own widget via createWidget(). This header survives only as
// the include alias other framework code expects.
#include <icl/qt/GUIComponent.h>

/** \cond */
namespace icl::utils { class Configurable; }
/** \endcond */
