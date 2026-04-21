// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/ProximityOp.h>
#include <icl/core/CoreFunctions.h>
#include <icl/core/Img.h>
#include <icl/utils/StringUtils.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  // ippiSqrDistanceFull/Same/Valid_Norm_* and ippiCrossCorrFull/Same/Valid_Norm_*
  // were removed from modern IPP (oneAPI 2022+).
  // TODO: implement C++ fallback or use modern ippiSqrDistanceNorm API

  ProximityOp::ProximityOp(optype ot, applymode am):
    m_poImageBuffer(0),m_poTemplateBuffer(0){
    addProperty("operation type","menu","sqrDistance,crossCorr,crossCorrCoeff",ot,0,
                "Proximity measurement type");
    addProperty("apply mode","menu","full,valid,same",am,0,
                "Proximity apply mode");
  }

  ProximityOp::~ProximityOp(){
    delete m_poImageBuffer;
    delete m_poTemplateBuffer;
  }

  void ProximityOp::setOpType(optype ot){
    setPropertyValue("operation type",ot);
  }

  void ProximityOp::setApplyMode(applymode am){
    setPropertyValue("apply mode",am);
  }

  ProximityOp::optype ProximityOp::getOpType() const{
    return const_cast<ProximityOp*>(this)->getPropertyValue("operation type");
  }

  ProximityOp::applymode ProximityOp::getApplyMode() const{
    return const_cast<ProximityOp*>(this)->getPropertyValue("apply mode");
  }

  void ProximityOp::apply([[maybe_unused]] const core::Image &src1, [[maybe_unused]] const core::Image &src2, [[maybe_unused]] core::Image &dst){
    ERROR_LOG("ProximityOp::apply not yet implemented (deprecated IPP API removed, C++ fallback needed)");
  }

  } // namespace icl::filter