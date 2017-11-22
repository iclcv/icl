/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/AdvancedMarkerGridDetector.h **
** Module : ICLMarkers                                             **
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

#include <ICLMarkers/MarkerGridDetector.h>
#include <ICLUtils/Array2D.h>
#include <ICLUtils/ParamList.h>
#include <ICLUtils/VisualizationDescription.h>
#include <ICLMarkers/Fiducial.h>

namespace icl{
  namespace markers{

    /// Extension of the MarkerGridDetector class that provides a more advanced output
	  class ICLMarkers_API AdvancedMarkerGridDetector : public MarkerGridDetector{
      public:
      typedef MarkerGridDetector Super;

	  class ICLMarkers_API AdvancedGridDefinition : public Super::GridDefinition{
        utils::Size32f markerBounds; //!< size of a single marker in mm

        ///  size of the whole grid
        /** The size is measured from the left-edge of the left-most marker to the
            right-edge of the right-most marker and vertically analogously  */
        utils::Size32f gridBounds;

        public:
        // empty constructor
        AdvancedGridDefinition(){}

        /// constructor with given parameters
        AdvancedGridDefinition(const utils::Size &numCells,
                               const utils::Size32f &markerBounds,
                               const utils::Size32f &gridBounds,
                               const std::vector<int> &markerIDs=std::vector<int>(),
                               const std::string &markerType="bch") throw (utils::ICLException);

        /// returns internal marker-bounds value
        const utils::Size32f &getMarkerBounds() const {
          return markerBounds;
        }
        /// returns internal grid-bounds value
        const utils::Size32f &getGridBounds() const {
          return gridBounds;
        }

        /// computes the bounds of the given cell
        utils::Rect32f getBounds(int x, int y) const;

      };

      /// internal data-class the represents a more sophisticated
	  class ICLMarkers_API Marker{
        int id;      //!< marker id
        bool found;  //!< was it found
        public:

        /// internal key-point structure
        struct ICLMarkers_API KeyPoints{
          utils::Point32f ur;     //!< upper right point
          utils::Point32f lr;     //!< upper left point
          utils::Point32f ll;     //!< lower left point
          utils::Point32f ul;     //!< upper left point
          utils::Point32f center; //!< center point

          /// helper function
          void setup(const utils::Point32f corners[4], const utils::Point32f &center);

          // internally used function that concatenates all points
          std::vector<utils::Point32f> data() const;

          // internally used function that concatenates all points (without the center point)
          std::vector<utils::Point32f> corners() const;

          /// appends the corner points to the given vector
          void appendCornersTo(std::vector<utils::Point32f> &dst) const;

        };

        private:
        /// internal key-points (in image space, updated at runtime)
        KeyPoints imagePts;
        /// internal key-points (in grid space, initialized at construction time)
        KeyPoints gridPts;

        public:
        /// creates an empty marker
        Marker();

        /// create a marker with given ID and layout (in grid-space)
        Marker(int id, const utils::Point32f gridPoints[4], const utils::Point32f &center);

        /// sets current image points
        void setImagePoints(const utils::Point32f corners[4],
                            const utils::Point32f &center=utils::Point32f::null);

		/// for sorting (is that needed)
		inline bool operator<(const Marker &m) const { return id < m.id; }

        /// sets the 'found' flag
        void setFound(bool found){
          this->found = found;
        }

        bool wasFound() const {
          return found;
        }

        /// returns current image points
        const KeyPoints &getImagePoints() const{
          return imagePts;
        }
        /// returns static grid points
        const KeyPoints &getGridPoints() const {
          return gridPts;
        }

        /// returns current image points (unconst version)
        KeyPoints &getImagePoints() {
          return imagePts;
        }


        /// visualizes the marker boundary and the center
        void visTo(utils::VisualizationDescription &vd) const;

        /// utilty function
        void getImagePointsTo(utils::Point32f *dst) const;

        /// utility method that transforms the marker's image points ...
        template<class Transform>
        inline void transformImagePointsTo(Marker &dst, Transform t) const{
          dst.imagePts.ur = t(imagePts.ur);
          dst.imagePts.lr = t(imagePts.lr);
          dst.imagePts.ll = t(imagePts.ll);
          dst.imagePts.ul = t(imagePts.ul);
          dst.imagePts.center = t(imagePts.center);
        }
      };

      /// Represents whole grid of markers
	  class ICLMarkers_API MarkerGrid : public utils::Array2D<Marker>{
        typedef utils::Array2D<Marker> Super; //!< convenience typedef
        AdvancedGridDefinition gridDef;       //!< internal metrics

        public:
        /// creates an empty 0x0 grid
        MarkerGrid();

        /// creates a grid with given grid-definition (note: 1x1 grids can cause errors)
        MarkerGrid(const AdvancedGridDefinition &def);

        /// deferred initialization of a grid
        void init(const AdvancedGridDefinition &def);

        /// updates the image-points of all contained markers according to the given 2D-array of fiducials
        void update(const MarkerGridDetector::Result &r);

        /// returns the internal grid definitiion
        const AdvancedGridDefinition &getGridDef() const {
          return gridDef;
        }

        /// returns the marker for the given ID
        const Marker &getMarker(int id) const throw (utils::ICLException);

        /// visualizes the whole grid (i.e. each marker)
        utils::VisualizationDescription vis() const;

        /// transforms all markers using a transformation function
        template<class Transform>
        inline void transformImagePointsTo(MarkerGrid &dst, Transform t) const{
          if(getSize() != dst.getSize()){
            dst = MarkerGrid(gridDef);
          }
          for(int i=0;i<getDim();++i){
            operator[](i).transformImagePointsTo(dst[i],t);
          }
        }
      };

      /// creates null-detector instance
      AdvancedMarkerGridDetector();

      /// creates a valid instance
      AdvancedMarkerGridDetector(const AdvancedGridDefinition &def);

      /// (re)-initializes the detector
      void init(const AdvancedGridDefinition &def);

      /// detects the grid in the given image
      const MarkerGrid &detect(const core::ImgBase *image);

      /// returns the internal marker grid
      const MarkerGrid *getMarkerGrid() const { return &grid; }

      private:
      MarkerGrid grid; //!< internal marker grid
    };
  }
}
