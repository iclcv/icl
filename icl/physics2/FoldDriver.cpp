// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics2/FoldDriver.h>
#include <icl/physics2/PaperDriver.h>
#include <icl/viz3d/Node.h>
#include <icl/utils/Macros.h>
#include <icl/utils/prop/Constraints.h>

namespace icl::physics2 {

  FoldDriver::FoldDriver() {
    using namespace utils;
    addProperty("auto extend", prop::Flag{}, true,
                "extend the drawn fold line to the sheet edges (crease all the way across)");
  }

  void FoldDriver::onAttach() {
    m_paper = node() ? node()->getDriver<PaperDriver>() : nullptr;
    if (!m_paper)
      ERROR_LOG("FoldDriver must share its node with a PaperDriver "
                "(add the PaperDriver first)");
  }

  void FoldDriver::foldAlongLine(const utils::Point32f &a, const utils::Point32f &b) {
    if (!m_paper) return;
    m_paper->foldAlongLine(a, b, (bool)prop("auto extend").value);
  }

  void FoldDriver::setPreview(const Vec &a, const Vec &b) {
    m_previewA = a; m_previewB = b; m_previewActive = true;
  }
  void FoldDriver::clearPreview() { m_previewActive = false; }
  bool FoldDriver::getPreview(Vec &a, Vec &b) const {
    if (!m_previewActive) return false;
    a = m_previewA; b = m_previewB; return true;
  }

} // namespace icl::physics2
