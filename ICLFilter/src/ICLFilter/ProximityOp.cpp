// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLFilter/ProximityOp.h>
#include <ICLCore/CoreFunctions.h>
#include <ICLCore/Img.h>
#include <ICLUtils/StringUtils.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{

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

    void ProximityOp::apply(const core::Image &src1, const core::Image &src2, core::Image &dst){
      (void)src1; (void)src2; (void)dst;
      ERROR_LOG("ProximityOp::apply not yet implemented (deprecated IPP API removed, C++ fallback needed)");
    }

  } // namespace filter
} // namespace icl
