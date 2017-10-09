/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/Dragger.h                              **
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
#include <ICLUtils/Point32f.h>
#include <ICLUtils/Rect32f.h>
#include <ICLCore/Types.h>
#include <ICLCore/CoreFunctions.h>

namespace icl{
  namespace qt{

    /** \cond */
    class ICLDrawWidget;
    /** \endcond */

    /// Utility class that implements draggable rects on the ChromaWidget surface \ingroup UNCOMMON
    struct ICLQt_API Dragger{

      /// Utility class which helps to convert rgb to RG-Chroma and back
      struct Color{

        /// conversion function calculates r and g given b R and G
        /** @param r return value for r
            @param g return value for g
            @param b blue value
            @param x R value (it is called x because R complains the x axis of the ChromaWidget)
            @param y G value (it is called y because G complains the y axis of the ChromaWidget)
        **/
        static void xyb_to_rg(icl8u &r, icl8u &g, float b, float x, float y){
          float k = x/(1.0-x);
          g = utils::clipped_cast<icl32f,icl8u>((b*(k+1.0))/(1.0/y-1.0-k));
          r = utils::clipped_cast<icl32f,icl8u>(g/y-g-b);
        }
        /// Create a new color with given red, green, blue and alpha value
        Color(icl8u r=0, icl8u g=0, icl8u b=0, icl8u a=255):
          r(r),g(g),b(b),a(a){}

        icl8u r; /**!< red value */
        icl8u g; /**!< green value */
        icl8u b; /**!< blue value */
        icl8u a; /**!< alpha value */
      };

      /// creates a new Dragger
      /** @param p initial position in relative coordinates [0,1]
          @param d relative dimension in each direction (0.1 -> 10% of widget size)
          @param c color of the dragger
      **/
      Dragger(const utils::Point32f &p=utils::Point32f::null, float d=0.02,const Color &c=Color(255,0,0)):
        p(p),d(d),r(utils::Rect32f(p.x-d,p.y-d,2*d,2*d)),dr(false),c(c),ov(false){}

      /// returns whether a given relative position is inside the dragger
      bool hit(const utils::Point32f &x) const{ return r.contains(x.x,x.y); }

      /// returns the draggers relative rect
      const utils::Rect32f &rect() const { return r; }

      /// returns the current dim variable
      float dim() const { return d; }

      /// returns the current center position
      const utils::Point32f &pos() const { return p; }

      /// returns the current color
      const Color &col() const { return c; }

      /// sets the current color
      /** @param r red value
          @param g green value
          @param b blue value
      **/
      void setColor(float r,float g, float b){
        c = Color(utils::clipped_cast<icl32f,icl8u>(r),
                  utils::clipped_cast<icl32f,icl8u>(g),
                  utils::clipped_cast<icl32f,icl8u>(b));
      }

      /// sets the current dim
      /** @param d new dim value (relative) */
      void setDim(float d){
        this->d=d;
        r.x = p.x-d;
        r.y = p.y-d;
      }

      /// sets the current position (which is clipped to [0,1])
      /** @param x new center position of the dragger */
      void setPos(const utils::Point32f &x){
        p = x;
        p.x = utils::clip(p.x,float(0.0),float(1.0));
        p.y = utils::clip(p.y,float(0.0),float(1.0));
        r.x = p.x-d;
        r.y = p.y-d;
      }
      /// moves the dragger by a given distance
      /** The resulting position is clipped to [0,1]
          @param dist distance to move
      **/
      void move(const utils::Point32f &dist){
        setPos(p+dist);
      }

      /// makes this dragger "dragged" by the mouse
      /** @param x mouse grip point which is used to calculate the
                   offset to the draggers center called "dragOffs"
      **/
      void drag(const utils::Point32f &x) {
        dr = true;
        dragOffs = p - x;
      }

      /// once dragged, this function will move the dragger
      /** The new position is clip( x+dragOffs, [0,1])
          @param x new position (result is clipped to [0,1]
      **/
      void dragTo(const utils::Point32f &x){
        setPos(x+dragOffs);
      }

      /// drops this dragger, to it is no longer moved by the mouse
      void drop() { dr = false; }

      /// draws this dragger into the given ICLDrawWidget
      /** The ICLDrawWidget must be set up before using
          \code
          w->setImage( chromaSpaceImage );
          w->lock();
          w->reset();
          w->rel(); // !!

          MyDragger.draw(w);

          w->unlock();
          w->update();
          \endcode
          @param w drawing context
      **/
      void draw( ICLDrawWidget *w) const;

      /// returns whether the mouse is currently over this dragger
      bool over() const { return ov; }

      /// sets the internal "over"-variable to indicate, that the mouse is over the dragger
      /** The "over"-variable is used in the draw(..)-function for a nice mouse-over effect
      **/
      void setOver(bool val=true) { ov = val; }

      /// returns whether this dragger is currently dragged
      bool dragged() const { return dr; }

      private:

      utils::Point32f p; /**!< current center position (relative to the parent widgets size) */
      float d;    /**!< relative dimension of this dragger in each direction */
      utils::Rect32f r;  /**!< current relative rect of this dragger (p.x-d,p.y-d,2*d,2*d) */
      bool dr;    /**!< flag to indicate whether this dragger is currently dragged */
      Color c;    /**!< current color of this dragger */
      bool ov;    /**!< flag to indicate whether this mouse is currently over this dragger */
      utils::Point32f dragOffs; /** offset from the draggers center to the location where it was dragged by the mouse */
    };

  } // namespace qt
}

