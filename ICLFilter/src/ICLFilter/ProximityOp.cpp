/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/ProximityOp.cpp                **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

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
