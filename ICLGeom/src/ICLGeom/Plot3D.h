// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/GUIComponent.h>
#include <ICLUtils/Range.h>
#include <ICLGeom/PlotWidget3D.h>
#include <ICLGeom/PlotHandle3D.h>

namespace icl::geom {
    /// Specialized 3D visualization component intended for 3D-box plots (needs ICLGeom-library to be linked)
    /** Creates a geom::PlotHandle3D, optimized for 3D-box plots. Internally, the created PlotWidget3D consists
        of an inherited ICLDrawWidget3D, a geom::Scene and a geom::Camera that are autpmatically created and
        linked to the visualization and mouse interaction

        \section INC Including
        Please note, that including this class will automaticall also include the
        dependent classes PlotWidget3D and PlotHandle3D for convenience reasons
        */
    struct Plot3D : public qt::GUIComponent{
      private:
      /// internally used utility method
      static std::string form_args(const utils::Range32f &xrange,
                                   const utils::Range32f &yrange,
                                   const utils::Range32f &zrange){
        std::ostringstream str;
        str << xrange.minVal << ',' << xrange.maxVal << ','
            << yrange.minVal << ',' << yrange.maxVal << ','
            << zrange.minVal << ',' << zrange.maxVal;
        return str.str();
      }

      public:
      /// create Plot3D component with given defaultViewPortsize
      /** The given defaultViewPortsize is to create an OpenGL viewport as long as no
          backgrond image is given. */
      Plot3D(const utils::Range32f &xrange=utils::Range32f(0,0),
             const utils::Range32f &yrange=utils::Range32f(0,0),
             const utils::Range32f &zrange=utils::Range32f(0,0)):
      qt::GUIComponent("plot3D",form_args(xrange,yrange,zrange)){}
    };

  }