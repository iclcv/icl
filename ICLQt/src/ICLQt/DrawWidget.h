/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/DrawWidget.h                           **
** Module : ICLQt                                                  **
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

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/VisualizationDescription.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/Rect32f.h>
#include <ICLMath/FixedMatrix.h>
#include <ICLQt/Widget.h>
#include <QtCore/QMutex>

namespace icl{
  /** \cond */
  namespace core{ class ImgBase; }
  /** \endcond */

  namespace qt{
    /** \cond */
    class GLPaintEngine;
    /** \endcond */

  /// Extended Image visualization widget, with a drawing state machine interface \ingroup COMMON
  /** The ICLDrawWidget can be used to draw annotation on images in real time.
      It provides the ability for translating draw command given in image coordinations
      with respect to the currently used image scaling type (hold-ar, no-scaling or
      fit to widget) and to the currently used widget size.

      <h2>Drawing-State machine</h2>
      Like other drawing state machines, like the QPainter or OpenGL, the ICLDrawWidget
      can be used for drawing 2D-primitives step by step into the frame-buffer using
      OpenGL hardware acceleration. Each implementation of drawing function
      should contain the following steps.
      \code

      drawWidget->setImage(..);  /// sets up a new background image

      drawWidget.lock();   /// (is deprecated) locks the draw widget against the drawing loop
      drawWidget.reset();  /// (is deprecated) deletes all further draw commands
      ... draw commands ...

      drawWidget->unlock();   /// (is deprecated) enable the widget to be drawed

      \endcode

      \section DRAWING_EXAMPLE Sample Application for Image Segmentation

      <TABLE border=0><TR><TD>
      \code

  \#include <ICLQt/Common.h>
  \#include <ICLQt/Quick.h>

icl::qt::GUI gui;
std::vector<double> c(3,255); // ref color

void click(const MouseEvent &e){
if(e.isLeft() && !gui["vis"].as<int>()){
  c = e.getColor();
}
}

void init(){
  gui << Draw().handle("draw").minSize(32,24)
      << (HBox().maxSize(100,3)
         << Combo("image,levelmap").handle("vis")
         << Slider(2,10,5).out("levels").label("levels"));

  gui.show();
  gui["draw"].install(new MouseHandler(click));
}

void run(){
  static GenericGrabber g(pa("-input"));
  g.setDesiredSizeInternal(utils::Size::VGA);

  // DrawHandle object draw provides direct access to the underlying
  // ICLDrawWidget by the 'operator->' i.e., it behaves like
  // an ICLDrawWidget-pointer
  DrawHandle draw = gui["draw"];

  // do some image processing (pretty slow here)
  ImgQ im = cvt(g.grab());

  // re-quantize grabbed image to reduce levels
  ImgQ lm = levels(im,gui["levels"].as<int>());

  vector<vector<utils::Point> > pxs;
  pxs.push_back(vector<utils::Point>(1, Point(lm.getWidth()/2, lm.getHeight()/2)));

  // visualize selected image
  ImgQ *ims[2] = {&im, &lm};
  draw = ims[gui["vis"].as<int>()];


  // use drawing state-machine to post draw commands
  draw->pointsize(2);

  draw->color(255,0,0,60);
  draw->points(pxs[0]);

}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"-input(2)",init,run).exec();
}

    \endcode

</TD><TD valign="top">
    \image html drawing-example.png "Screenshot of sample application"
</TD></TR></TABLE>
*/
    class ICLQt_API ICLDrawWidget : public ICLWidget {
      template<class T>
      static inline void icl_given_type_has_no_int_index_operator(const T &t){
        t[0];
      }

      public:
      /// enum used for specification of predefined symbols
      enum Sym {symRect,symCross,symPlus,symTriangle,symCircle};

      /// creates a new ICLDrawWidget embedded into the parent component
      ICLDrawWidget(QWidget *parent=0);

      /// destructor2
      ~ICLDrawWidget();

      /// sets whether the draw commands are accumulative (default is on=true)
      /** If the "auto reset queue" flag is set to false, the internal draw command
          queues are not cleared when ICLWidget::render() is called. In this case,
          ICLDrawWidget::resetQueue can be called manually. */
      void setAutoResetQueue(bool on);

      /// clears the current draw queue
      /** This is done automatically by default. Only if the user wants accumulative
          draw commands, resetQueue can be called manually to clear the queues */
      void resetQueue();

#if 0
      /// locks the state machine
      /** The state machine collects all draw commands in a command queue internally.
          The access to this command queue must be locked, as push operations on this
          queue are implemented not thread safe. The command queue is also used by the
          internal drawing mechanism, which translates the stored commands into appropriate
          OpenGL commands. Do not forget to call unlock after performing all drawing
          commands.
          */
      ICL_DEPRECATED void lock(){}//m_oCommandMutex.lock();}

      /// unlocks the draw command queue ( so it can be drawn by OpenGL )
      ICL_DEPRECATED void unlock(){}//{m_oCommandMutex.unlock();}
#endif

      /// sets up the state machine to treat coordinates in the image pixel coordinate system
      /** the visualization space is x={0..w-1} and y={0..h-1}*/
      void abs();

      /// sets up the state machine to receive relative coordinates in range [0,1]
      void rel();

      /// draws an image into the given rectangle
      /** The image is copied by the state machine to ensure, that the data is persistent
          when the draw command is passed to the underlying PaintEngine. Otherwise, it would
          not be possible to ensure, that the set image data that is drawn is not changed
          elsewhere
          <b>note:</b> If image has four channels, the last channel is used as alpha channel
          where a pixel value of 0 means invisible=100%transparent and 255 means no transparency
          */
      void image(core::ImgBase *image, float x, float y, float w, float h);

      /// convenience function for image using an image rect
      inline void image(core::ImgBase *image, const utils::Rect &r){
        this->image(image,r.x,r.y,r.width,r.height);
      }

      /// draws an image into given quadrangle
      /** node order is
          <pre>
          a--------b
          |         \
          |          \
          d---___     \
          ---___c
          </pre>
          */
      void image(const core::ImgBase *image, const float a[2], const float b[2],
                 const float c[2], const float d[2]);

      /// draws a string into the given rect
      /** if w=-1 and h=-1, fontsize is used to determine the bitmap size.
          The given fontsize paramter defines the font-size in screen pixels.
          <b>Important: if the given fontsize is negative, its absolute value is used
          but the font size unit is image pixels instead of screen-pixels </b>
          */
      void text(std::string text, float x, float y, float w, float h, float fontsize=10);

      /// draws text at given x, y location with given fontsize
      /*    The given fontsize paramter defines the font-size in screen pixels.
          <b>Important: if the given fontsize is negative, its absolute value is used
          but the font size unit is image pixels instead of screen-pixels </b>
          */
      void text(const std::string &text, float x, float y, float fontsize=10){
        this->text(text,x,y,-1,-1,fontsize);
      }

      /// draws the text at given position p
      void text(const std::string &text, const utils::Point32f &p, float fontsize=10){
        this->text(text,p.x,p.y,-1,-1,fontsize);
      }

      /// draws a point at the given location
      void point(float x, float y);

      /// convenience wrapper for utils::Point types
      void point(const utils::Point &p){
        this->point(p.x,p.y);
      }

      /// convenience wrapper for utils::Point32f types
      void point(const utils::Point32f &p){
        this->point(p.x,p.y);
      }

      /// convenience wrapper for arbitrary types, that provide an index operator [int]
      template<class VectorType>
      void point(const VectorType &p){
        icl_given_type_has_no_int_index_operator(p);
        this->point(p[0],p[1]);
      }

      /// draws a set of points
      /** for relative utils::Point coordinates the factors can be set
          point i is drawn at pts[i].x/xfac and pts[i].y/yfac
          **/
      void points(const std::vector<utils::Point> &pts, int xfac=1, int yfac=1);

      /// draws a set of points
      void points(const std::vector<utils::Point32f> &pts);

      /// convenience wrapper for arbitrary types, that provide an index operator [int]
      template<class VectorType>
      void point(const std::vector<VectorType> &points){
        icl_given_type_has_no_int_index_operator(points[0]);
        std::vector<utils::Point32f> tmp(points.size());
        for(unsigned int i=0;i<points.size();++i) tmp[i] = utils::Point32f(points[i][0],points[i][1]);
        this->points(tmp);
      }


      /// draws a set of connected points
      /** for relative utils::Point coordinates the factors can be set
          point i is drawn at pts[i].x/xfac and pts[i].y/yfac
          **/
      void linestrip(const std::vector<utils::Point> &pts, bool closeLoop=true, int xfac=1, int yfac=1);

      /// draws a set of connected points
      void linestrip(const std::vector<utils::Point32f> &pts, bool closeLoop=true);


      /// draws a line from point (x1,y1) to point (x2,y2)
      void line(float x1, float y1, float x2, float y2);

      /// convenience function for drawing lines between two points
      void line(const utils::Point32f &a, const utils::Point32f &b);

      /// convenience wrapper for arbitrary types, that provide an index operator [int]
      template<class VectorTypeA, class VectorTypeB>
      void line(const VectorTypeA &a, const VectorTypeB &b){
        icl_given_type_has_no_int_index_operator(a);
        icl_given_type_has_no_int_index_operator(b);
        this->line(a[0],a[1],b[0],b[1]);
      }

      /// draws an arrow from a to b (arrow cap is at b)
      void arrow(float ax, float ay, float bx, float by, float capsize=10);

      /// draws an arrow from a to b (arrow cap is at b)
      void arrow(const utils::Point32f &a, const utils::Point32f &b, float capsize=10);

      /// draws a rect with given parameters
      void rect(float x, float y, float w, float h);

      /// convenience function for drawing float rects
      void rect(const utils::Rect32f &r);

      /// draws a rect from a icl utils::Rect structure
      void rect(const utils::Rect &r);

      /// draws a triangle defined by 3 points
      void triangle(float x1, float y1, float x2, float y2, float x3, float y3);

      /// draws a triangle defined by 3 points
      void triangle(const utils::Point32f &a, const utils::Point32f &b, const utils::Point32f &c);

      /// draws a quad with given 4 points
      void quad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);

      /// draws a quad with given 4 points
      void quad(const utils::Point32f &a, const utils::Point32f &b, const utils::Point32f &c, const utils::Point32f &d);

      /// draws an ellipse with given parameters (w==H --> circle)
      void ellipse(float x, float y, float w, float h);

      /// draws an ellipse into given rectangle
      void ellipse(const utils::Rect &r);

      /// draws an ellipse into given rectangle
      void ellipse(const utils::Rect32f &r);

      /// draws a circle with given center and radius
      void circle(float cx, float cy, float r);

      /// draws a circle with given center and radius
      void circle(const utils::Point32f &center, float radius);

      /// draws a convex polygon
      void polygon(const std::vector<utils::Point32f> &ps);

      /// draws a convex polygon (int-points)
      void polygon(const std::vector<utils::Point> &ps);

      /// draws a regular grid between given points
      void grid(const utils::Point32f *points, int nx, int ny, bool rowMajor=true);

      /// draws a predefined symbol at the given location
      /** The symbols size can be set using symsize */
      void sym(float x, float y, Sym s);

      /// convenience wrapper for sym(float,float,Sym)
      void sym(const utils::Point32f &p, Sym s){
        sym(p.x,p.y,s);
      }

      /// this is a convenience function for sym(float,float,Sym)
      /** possible values for sym are:
          - 'r' for rectangle
          - '+' for plus
          - 'x' for cross
          - 't' for triangle
          - 'o' for circle

          instead of writing
          \code draw->sym(x,y,ICLDrawWidget::symPlus) \endcode
          you can simply write
          \code draw->sym(x,y,'+') \endcode

          for all invalid chars, 'x' is used
          */
      void sym(float x, float y, char sym){
        if(sym == 'r') this->sym(x,y,symRect);
        else if(sym == '+') this->sym(x,y,symPlus);
        else if(sym == 't') this->sym(x,y,symTriangle);
        else if(sym == 'o') this->sym(x,y,symCircle);
        else this->sym(x,y,symCross);
      }

      /// convenicence wrapper for sym(float,flota,char)
      void sym(const utils::Point32f &p, char sym){
        this->sym(p.x,p.y,sym);
      }

      /// sets the size for following "sym" draw commands
      void symsize(float w, float h=-1); // if h==-1, h = w;

      /// sets current linewidth (default is 1);
      void linewidth(float w);

      /// sets current pointsize (default is 1)
      void pointsize(float s);

      /// sets an angle for text that is rendered
      /** The angle is specified in degrees and in clock-wise direction */
      void textangle(float angleDeg);

      /// sets the internal default font size
      /** The default font size is used if the font-size given to text is set to 0
          the 'default'-default size is 10 */
      void fontsize(float size);

      /// sets the draw state machines "edge"-color buffer to a given value
      /** Primitives except images are drawn with the currently set "color"
          and filled with the currently set "fill"
          alpha values of 0 disables the edge drawing at all
          */
      void color(float r, float g, float b, float alpha = 255);

      /// set the draw state machines "fill"-color buffer to a given value
      /** Primitives except images are drawn with the currently set "color"
          and filled with the currently set "fill"
          alpha values of 0 disables the edge drawing at all
          */
      void fill(float r, float g, float b, float alpha = 255);

      /// utility template method that allows to pass 3D vectors as colors
      template<class T, unsigned int COLS>
      inline void color(const math::FixedMatrix<T,COLS,3/COLS> &v){
        color((float)v[0],(float)v[1],(float)v[2]);
      }

      /// utility template method that allows to pass 4D vectors as colors
      template<class T, unsigned int COLS>
      inline void color(const math::FixedMatrix<T,COLS,4/COLS> &v){
        color((float)v[0],(float)v[1],(float)v[2],(float)v[3]);
      }

      /// utility template method that allows to pass 3D vectors as fill color
      template<class T, unsigned int COLS>
      inline void fill(const math::FixedMatrix<T,COLS,3/COLS> &v){
        fill((float)v[0],(float)v[1],(float)v[2]);
      }

      /// utility template method that allows to pass 4D vectors as fill color
      template<class T, unsigned int COLS>
      inline void fill(const math::FixedMatrix<T,COLS,4/COLS> &v){
        fill((float)v[0],(float)v[1],(float)v[2],(float)v[3]);
      }

      /// disables drawing edges
      void nocolor();

      /// disables filling primitives
      void nofill();

      /// draws a VisualizationDescription instance
      /** Internally, the description is decomposed to normal function calls*/
      void draw(const utils::VisualizationDescription &d);

      /// this function can be reimplemented in derived classes to perform some custom drawing operations
      virtual void customPaintEvent(PaintEngine *e);

      /// this function can be reimplemented perform some custom initialization before the actual draw call
      virtual void initializeCustomPaintEvent(PaintEngine *e);

      /// this function can be reimplemented perform some custom initialization after the actual draw call
      virtual void finishCustomPaintEvent(PaintEngine *e);


      /// forward declaration of an internally used state class
      class State;

      /// forward declaration of the internally used DrawCommandClass
      class DrawCommand;

      protected:

      /// swaps the draw queues
      virtual void swapQueues();

      /// two lists of draw commands
      /** queues[0] is filled, queues[1] is drawn*/
      std::vector<DrawCommand*> *m_queues[2];

      /// Data of the "State Machine"
      State *m_poState;

      /// utils::Mutex for a thread save event queue
      QMutex m_oCommandMutex;

      /// internal flag
      bool m_autoResetQueue;
    };
  } // namespace qt
}

