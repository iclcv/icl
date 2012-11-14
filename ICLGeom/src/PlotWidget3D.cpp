/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/PlotWidget3D.cpp                           **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLGeom/PlotWidget3D.h>
#include <ICLGeom/PlotHandle3D.h>
#include <ICLQt/GUIDefinition.h>

namespace icl{
  namespace geom{

    using namespace utils;
    using namespace math;
    using namespace core;
    using namespace qt;


    struct PlotWidget3D::Data{
      Scene scene;
      Range32f viewport[3];
    };
    
    PlotWidget3D::PlotWidget3D(QWidget *parent):ICLDrawWidget3D(parent),m_data(new Data){
      std::fill(m_data->viewport,m_data->viewport+3,Range32f(0,1));
      m_data->scene.setBounds(1);

      m_data->scene.addCamera(Camera()); // todo:find good default 
      
      install(m_data->scene.getMouseHandler(0));
      link(m_data->scene.getGLCallback(0));
    }


    PlotWidget3D::~PlotWidget3D(){
      delete m_data;
    }
    
    void PlotWidget3D::setViewPort(const Range32f &xrange,
                                   const Range32f &yrange,
                                   const Range32f &zrange){
      m_data->viewport[0] = xrange;
      m_data->viewport[1] = yrange;
      m_data->viewport[2] = zrange;
    }
      
    Scene &PlotWidget3D::getScene(){
      return m_data->scene;
    }
    const Scene &PlotWidget3D::getScene() const{
      return m_data->scene;
    }
    










    

    
    namespace{ // only for registering this class as a GUI component!

      struct Plot3DGUIWidget : public GUIWidget{
        PlotWidget3D *draw;
        Plot3DGUIWidget(const GUIDefinition &def):GUIWidget(def,0,0){
          draw = new PlotWidget3D(this);
          
          addToGrid(draw);
          
          if(def.handle() != ""){
            getGUI()->lockData();
            getGUI()->allocValue<PlotHandle3D>(def.handle(),PlotHandle3D(draw,this));
            getGUI()->unlockData();
          }
        }
      };

      qt::GUIWidget *create_plot_3D_widget_instance(const qt::GUIDefinition &def){
        Plot3DGUIWidget *w = 0;
        try{
          w = new  Plot3DGUIWidget(def);
        }catch(ICLException &ex){
          std::cout << ex.what() << std::endl
                    << "syntax is:" 
                    << "draw3D() (no parameters supported)" 
                    << std::endl;
          return 0;
        }
        return w;
      }

      struct Plot3DGUIWidgetRegisterer{
        Plot3DGUIWidgetRegisterer(){ 
          qt::GUI::register_widget_type("plot3D",create_plot_3D_widget_instance);
        }
      } plot3DWidgetRegisterer; 
    }



  }
}
