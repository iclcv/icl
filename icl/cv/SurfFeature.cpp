// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/cv/SurfFeature.h>

#include <icl/utils/Macros.h>

namespace icl::cv {
    float SurfFeature::operator-(const SurfFeature &other) const{
      float sum=0.f;
      for(int i=0; i < 64; ++i){
        sum += utils::sqr(descriptor[i]-other.descriptor[i]);
      }
      return sum;
    }

    utils::VisualizationDescription SurfFeature::vis(int dx, int dy) const {
      utils::VisualizationDescription d;

      float s = 2 * scale;

      int cx = x + dx;
      int cy = y + dy;

      d.color(0,255,0,255);
      d.linewidth(2);
      int cx2 = cx + cos(orientation) * s;
      int cy2 = cy + sin(orientation) * s;
      d.line(cx,cy,cx2,cy2);

      d.linewidth(1);
      if(laplacian == 1) d.color(0,100,255,255);
      else if(laplacian == 0) d.color(255,0,0,255);
      else d.color(0,255,0,255); //  ??

      d.fill(255,0,0,30);
      d.circle(cx,cy,s);

      return d;
    }
  }