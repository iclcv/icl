/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/Plot3D.h                           **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#pragma once

#include <ICLQt/GUIComponent.h>
#include <ICLUtils/Range.h>
#include <ICLGeom/PlotWidget3D.h>
#include <ICLGeom/PlotHandle3D.h>

namespace icl{
  namespace geom{
  
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
}
