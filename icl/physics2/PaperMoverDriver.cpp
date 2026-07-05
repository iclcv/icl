// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics2/PaperMoverDriver.h>
#include <icl/physics2/PaperDriver.h>
#include <icl/viz3d/Node.h>
#include <icl/utils/Macros.h>
#include <icl/utils/prop/Constraints.h>

namespace icl::physics2 {

  PaperMoverDriver::PaperMoverDriver() {
    using namespace utils;
    addProperty("grab radius", prop::Range{.min=0.0f, .max=0.4f, .step=0.01f}, 0.06f,
                "paper-space radius of the grabbed patch (0 = a single point)");
  }

  void PaperMoverDriver::onAttach() {
    m_paper = node() ? node()->getDriver<PaperDriver>() : nullptr;
    if (!m_paper)
      ERROR_LOG("PaperMoverDriver must share its node with a PaperDriver "
                "(add the PaperDriver first)");
  }

  void PaperMoverDriver::beginGrab(const utils::Point32f &paperCoords) {
    if (m_paper) m_paper->beginGrab(paperCoords, (float)prop("grab radius").value);
  }
  void PaperMoverDriver::beginSheetGrab(const utils::Point32f &paperCoords) {
    // a radius bigger than the unit paper square grabs every node -> rigid carry
    if (m_paper) m_paper->beginGrab(paperCoords, 100.f);
  }
  void PaperMoverDriver::updateGrab(const Vec &worldTarget) {
    if (m_paper) m_paper->updateGrab(worldTarget);
  }
  void PaperMoverDriver::endGrab() {
    if (m_paper) m_paper->endGrab();
  }

} // namespace icl::physics2
