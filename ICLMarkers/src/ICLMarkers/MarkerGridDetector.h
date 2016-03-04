/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/MarkerGridDetector.h         **
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

#include <ICLUtils/Configurable.h>
#include <ICLUtils/Array2D.h>
#include <ICLUtils/ParamList.h>
#include <ICLMarkers/FiducialDetector.h>

namespace icl{
  namespace markers{

    /// Utility class that allows a regular grid of fiducial markers to be tracked
    /** The MarkerGridDetector is initialized with a definition of a regular marker grid.
        
        */
    class MarkerGridDetector : public utils::Configurable{
      struct Data;   //!< internal data type
      Data *m_data;  //!< hidden data pointer

      /// utility method
      int getPos(int id);

      public:
      /// Marker Grid geometry definition type
      /** The grid is defined by a number of grid cells WxH in horizontal and vertical 
          direction resulting in a total of N=W*H markers. The marker type can chosen freely
          and also the marker IDs that are to be used. By default, the trivial set of 
          marker IDs {0, ..,  N-1} is used. The class returns a 2D array of Fiducials. 
          Markers that are not found result in a null Fiducial in that cell. **/
      class GridDefinition{
        std::map<int,utils::Point> posLUT; //!< internally used lookup for marker IDs/grid-positions
        utils::Size numCells;              //!< grid dimension 
        std::vector<int> ids;              //!< used marker IDs
        std::string markerType;            //!< marker type
        
        public:
        // empty constructor
        GridDefinition();

        /// constructor with given parameters
        GridDefinition(const utils::Size &numCells,
                       const std::vector<int> &markerIDs=std::vector<int>(),
                       const std::string &markerType="bch") throw (utils::ICLException);

        /// returns the grid x/y index for a given marker ID
        /** If the id is not contained in the grid, (-1,-1) is returned */
        utils::Point getPos(int id) const;

        /// returns linear grid position (row-major order!)
        int getIndex(int id) const;

        /// returns the number of grid x-cells
        int getWidth() const;

        /// returns the number of grid y-cells
        int getHeight() const;

        /// returns the number of grid cells (WxH)
        int getDim() const;

        /// returns the grid dimension in cells
        const utils::Size &getSize() const;

        /// returns the marker type (bch is most common!)
        const std::string &getMarkerType() const;

        /// returns the internal list of marker IDs
        const std::vector<int> &getMarkerIDs() const;                
      };
      
      
      /// Empty constructor (results in a null instance that must be initialized using init(..)"
      MarkerGridDetector();

      /// Well defined constructor
      /** Equivalent to calling the empty constructor followed by calling init(...) */
      MarkerGridDetector(const GridDefinition &gridDef,
                         const utils::ParamList &extraParams=utils::ParamList("size","1x1"));

      /// Destructor
      ~MarkerGridDetector();

      /// Initialization function (equivalent to the well defined constructor)
      /** Initializes the underlying marker grid with a numCells marker cells. 
          If the marker ID vector is empty, the trival id set {0, ..., n-1} is used.
          If a non-zero number m of IDs is given, m must be eqaul to numCells.getDim().
          The extraParams list is directly passed to the internal FiducialDetector instance
          */
      void init(const GridDefinition &gridDef,
                const utils::ParamList &extraParams=utils::ParamList("size","1x1"));

      /// returns the internal grid definition
      const GridDefinition &getGridDefinition() const;
      
      /// result data type
      typedef utils::Array2D<Fiducial> Result;


      /// actual detection function
      const Result &detect(const core::ImgBase *image) throw (utils::ICLException);
      
      /// returns whether the instance has already been already initialized
      bool isNull() const;

      /// returns internal fiducial detector instance
      FiducialDetector *getFiducialDetector();
    };
  }
}
