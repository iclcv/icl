/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/DefineRectanglesMouseHandler.h         **
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
#include <ICLUtils/Lockable.h>
#include <ICLUtils/Any.h>
#include <ICLUtils/Function.h>
#include <ICLCore/Color.h>
#include <ICLQt/MouseHandler.h>

namespace icl{
  namespace qt{

    /** \cond */
    class ICLDrawWidget;
    /** \endcond */


    /// Utility mouse handler implementation that allows to define rectangles via drag and drop
    /** The DefineRectanglesMouseHandler can easily be installed on an ICLDrawWidget instance.
        It's current state can be visualized by using it's visualize-method in the applications
        working loop.

        \section MI Mouse Interaction

        - Left mouse nutton: define new rectangles and modify/move existing ones
        - Rigtht mouse button: delete existing rectangles
        - Middle mouse button: put rectangle to the back-layer

        \subsection MID Detailed Description
        The left mouse button is used for most interaction stuff. It is used to define new
        rectangles: Simply use a drag and drop gesture to define the rectangle. The left mouse
        button is also used to modify already existing rectangles: the user can either drag
        edges of existing rectangles or corners (i.e. two edges) or the whole rect by dragging
        any point in the center of the rectangle.
        The right mouse button is used to delete existing rectangles. The middle mousebutton
        can be used to chage a rectangles layer. Rectangles that were defined first are preferred
        during interaction. If you press the middle mouse button on a rectangle it is put to the
        back of the internal list, i.e. all other are 'in front of it'.

        \section OP Options
        Options can be set like
        \code
        DefineRectanglesMouseHandler mouse;
        /// set visualized rectangles edge color to red
        mouse.getOptions().edgeColor = Color4D(255,0,0,255);

        /// set visualization to no fill (rectangles boundaries only)
        mouse.getOptions().fillColor[3] = 0;

        /// change the size of the handles (number of pixels, you can be off
        /// an edge but still grab it)
        mouse.getOptions().handleWidth = 7;
        \endcode

        Please read the default options from the default arguments of the
        DefineRectanglesMouseHandler::Options::Options constructor.

        \section MRC Maximum Rectangle Count
        Due to performance issues, one should not use this interactor to manipulate more than some
        hundred rectangles. To avoid an overflow, the maxinum rectangle count is used.

        \section MRS Minimum Rectangle Size
        The minimal size for rectagles is used to avoid very small rectangles that can only
        hardly be manipulated (if the rectangle is only 1x1-pixels, it becomes hard to grab it or
        one of it's edge handles. If the minimum rectangle count is set afterwards, extra
        rectagles are dropped automatically.

        \section TT Thread Safety
        All methods of this class are implemented in a thread-saft manner. Therefore
        no additional locking is neccessary (and Lockable is inherited protectedly)
    */
    class ICLQt_API DefineRectanglesMouseHandler : public MouseHandler, protected utils::Lockable{
      public:
      /// Cummulative Options structure
        class ICLQt_API Options{
        friend class DefineRectanglesMouseHandler;
        /// Constructor showing default options
        Options(const core::Color4D &edgeColor=core::Color4D(0,255,0,255),
                const core::Color4D &fillColor=core::Color4D(0,255,0,50),
                const core::Color4D &centerColor=core::Color4D(0,255,0,255),
                const core::Color4D &metaColor=core::Color4D(0,255,0,255),
                int handleWidth=3, bool visualizeCenter=false,
                bool visualizeHovering=true,
                bool showOffsetText=false,
                bool showSizeText=false,
                bool showCenterText=false,
                bool showMetaData=false,
                int lineWidth=1,
                float textSize=9);
        public:
        core::Color4D edgeColor;     //!< edge color for rect visualization
        core::Color4D fillColor;     //!< fill color for rect visualization (set alpha to 0 to have no fill)
        core::Color4D centerColor;   //!< edge color used for the center visualization
        core::Color4D metaColor;     //!< text color for visualization of meta-data
        int handleWidth;       //!< handle width (amount of pixels, you can be of an edge and still drag it)
        bool visualizeCenter;  //!< if true, the center of each rectangle is visualized
        bool visualizeHovering;//!< if true, the rects boundary are drawn thicker if they are hovered
        bool showOffsetText;   //!< if true, the rects upper left pixel's coordinates are shown (as text)
        bool showSizeText;     //!< if true, the rects size is shown (as text)
        bool showCenterText;   //!< if true, the rects center is shown (as text)
        bool showMetaData;     //!< if true, the meta data is shown as (as text)
        int lineWidth;         //!< linewidth for visualization
        float textSize;        //!< text size used for all texts (note: if this is negative, it's defined in image pixels)
        int xStepping;         //!< can be set to force a given stepping for defined rectangles (x-direction)
        int yStepping;         //!< can be set to force a given stepping for defined rectangles (x-direction)
        bool canDeleteRects;   //!< defines whether rectangles can be deleted using right click
      };

      protected:

      /// Internally used utils::Rect structure
      struct ICLQt_API DefinedRect : public utils::Rect{
        /// grant private member access to DefineRectanglesMouseHandler
        friend class DefineRectanglesMouseHandler;

        /// Edges
        enum Edge{
          T, //!< Top edge
          R, //!< Right edge
          B, //!< Bottom edge
          L  //!< Left edge
        };
        /// Edge states
        enum State{
          nothing, //!< nothing is currently done with this edge
          hovered, //!< this edge is currently hovered (mouse over/not dragged)
          dragged  //!< this edge is currently dragged
        };

        /// State of all 4 edges
        State states[4];

        /// Internal help variable for smooth shifting of the whole rectangle
        utils::Point allDragOffs;

        /// a pointer to the parent DefineRectanglesMouseHandler's Option structure
        DefineRectanglesMouseHandler::Options *options;

        /// internal helper method
        utils::Rect edge(Edge e) const;
        /// internal helper method
        utils::Rect edgei(int i) const;
        /// internal helper method
        utils::Rect inner() const;
        /// internal helper method
        utils::Rect outer() const;
        /// internal helper method
        bool allHovered() const;
        /// internal helper method
        bool allDragged() const;
        /// internal helper method
        bool anyDragged() const;

        /// Constructor
        DefinedRect(const utils::Rect &r=utils::Rect::null, DefineRectanglesMouseHandler::Options *options=0);

        /// event processing if the rectangle was hit
        State event(const MouseEvent &e);

        /// visualization
        void visualize(ICLDrawWidget &w);

        /// this can be used to attach meta data to rectangles
        utils::Any meta;
      };


      int maxRects; //!< maximum count of rectangles
      int minDim;   //!< minimum dimension of rectangles
      std::vector<DefinedRect> rects; //!< list of defined rectanges
      utils::Point currBegin; //!< used for the currently defined rectangle
      utils::Point currCurr;  //!< used for the currently defined rectangle
      DefinedRect *draggedRect; //!< use if any rectangle is currently moved or manipulated
      Options options;  //!< options structure

      private:
      typedef utils::Function<void,const std::vector<utils::Rect> &>  Callback;
      std::map<std::string,Callback> callbacks;
      void callCallbacks();

      public:


      void registerCallback(const std::string &id, Callback cb);
      void unregisterCallback(const std::string &id);

      /// Default constructor with optionally given maximum rectangle count an minimum rectangle dimension
      /** @see \ref MRC
          @see \ref MRS
      */
      DefineRectanglesMouseHandler(int maxRects=10, int minDim=4);

      /// overwrittern MouseHandler method
      void process(const MouseEvent &e);

      /// automatic visualiziation
      /** The given ICLDrawWidget must be locked and reset before */
      void visualize(ICLDrawWidget &w);

      /// grants read/write access to the internal Options structure
      Options &getOptions();

      /// grants read-only access to the internal Options structure (const)
      const Options &getOptions() const;

      /// remove all current rectangles
      void clearAllRects();

      /// remove the first/all rectangles, that contain the given x/y coordinates
      /** Since rectangles might overlap more that one rectangle might contain
          the given coordinates. Set the parameter all to true if you want to delete all
          rectangles that contain the given x/y point. */
      void clearRectAt(int x, int y, bool all=false);

      /// Adds a new rectangels to the internal list
      /** The new rectangle becomes manipulatable automatically and immediately */
      void addRect(const utils::Rect &rect);

      /// sets the maximum number of possible rects (more rectangles cannot be added or defined)
      /** @see \ref MRC */
      void setMaxRects(int maxRects);

      /// sets the minimal dimension for defined rectangles
      void setMinDim(int minDim);

      /// returns the number of rectangles that are currently defined
      int getNumRects() const;

      /// returns the rectangle at given index
      /** If index is < 0 or >= getNumRects(), utils::Rect::null is returned */
      utils::Rect getRectAtIndex(int index) const;

      /// returns all current rects
      std::vector<utils::Rect> getRects() const;

      /// returns the rectangle at given x/y location
      /** Here only the top-most rectangle is returned.
          If you want to obtain all rectangles, that contain the given
          position, you have to use the getAllRects-method */
      utils::Rect getRectAt(int x, int y) const;

      /// returns all rectangles that contain the given location
      std::vector<utils::Rect> getAllRectsAt(int x, int y) const;

      /// returns the current minimun rectangle dimension
      int getMinDim() const;

      /// returns the curren maximum rectangle count
      int getMaxRects() const;

      /// gets the meta data associated with rect at given index
      const utils::Any &getMetaData(int index) const;

      /// gets the meta data associated with rect at given index
      const utils::Any &getMetaDataAt(int x, int y) const;

      /// associates some meta data with rect at given index
      void setMetaData(int index, const utils::Any &data);

      /// sets the meta data at the rect at given x,y-position
      void setMetaDataAt(int x, int y, const utils::Any &data);

      /// brings one rectangle to the front internally
      void bringToFront(int idx);

      /// brings one rectangle to the back internally
      void bringToBack(int idx);
    };
  } // namespace qt
}

