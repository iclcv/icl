// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLMarkers/FiducialImpl.h>

namespace icl::markers {
  FiducialImpl::FiducialImpl(const FiducialImpl &o):
    parent(o.parent),supported(o.supported),computed(o.computed),
    id(o.id),index(o.index){

    info2D = o.info2D ? new Info2D(*o.info2D) : 0;
    info3D = o.info3D ? new Info3D(*o.info3D) : 0;
  }

  FiducialImpl &FiducialImpl::operator=(const FiducialImpl &o){
    parent = o.parent;
    supported = o.supported;
    computed = o.computed;
    id = o.id;
    index = o.index;
    if(o.info2D){
      if(!info2D) info2D = new Info2D(*o.info2D);
      else *info2D = *o.info2D;
    }else{
      ICL_DELETE(info2D);
    }
    if(o.info3D){
      if(!info3D) info3D = new Info3D(*o.info3D);
      else *info3D = *o.info3D;
    }else{
      ICL_DELETE(info3D);
    }
    return *this;
  }

  } // namespace icl::markers