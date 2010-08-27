/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLBlob/SimpleBlobSearcher.h                   **
** Module : ICLBlob                                                **
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

#ifndef ICL_SIMPLE_BLOB_SEARCHER_H
#define ICL_SIMPLE_BLOB_SEARCHER_H

#include <ICLBlob/ImageRegion.h>
#include <ICLCC/Color.h>
#include <ICLUtils/Range.h>
#include <ICLUtils/Uncopyable.h>
namespace icl{

  /// Utility class for region-based colored blob detection
  /** The SimpleBlobSearcher is a re-implementation of the 
      RegionBasedBlobSearcher class. It provides less features, but
      it's usability was increased significantly.
  
      \section ALGO The Detection Algorithm
   
      \subsection PRE Prerequisites
      Given: A set of reference color tuples \f[\{R_i | i \in \{1,N\} \}\f]
      with \f[R_i=(\mbox{RC}_i,\mbox{T}_i,\mbox{S}_i,\mbox{RD}_i, \mbox{BUF}_i)\f]
      where
      - <b>RC</b> is the actual reference color for blobs that a detected
      - <b>T</b> is the Euclidean color distance tolerance for that reference color
      - <b>S</b> contains min- and maximum pixel count of detected blobs of that color
      - <b>RD</b> is an internal RegionDetector instance, dedicated for that color
      - <b>BUF</b> is an internal image Image for that color

      And a given image \f$I\f$
      
      \subsection ACALGO Actual Algorithm (Step by Step)

      - clear(OUTPUT_DATA)
      - SIZE <- size(I)
      - set_size(\f$\mbox{BUF}_i\f$,SIZE)
      - for all pixels (X,Y) in I
        - \f$ \mbox{BUF}_i \leftarrow 255 * ( |I(x,y)-\mbox{RC}_i| < T_i )\f$
      - for \f$i = i .. N\f$
        - \f$\mbox{Regions}_i \leftarrow \mbox{RD}_i.detect(\mbox{BUF}_i)\f$
        - for all \f$r_j \in \mbox{Regions}_i\f$
          - OUTPUT_DATA.add(SimpleBlobSearcher::Blob(\f$r_j\f$, \f$RC_i\f$, i)
  */
  class SimpleBlobSearcher : public Uncopyable{
    public:
    
    /// Internal blob-result type
    struct Blob{
      Blob():region(0),refColorIndex(-1){}
      Blob(const ImageRegion *region, const Color &refColor, int refColorIndex);
      
      /// corresponding regions (provides access to region features)
      const ImageRegion *region;
      
      /// corresponding reference color
      Color refColor;
      
      /// corresponding ref color index
      int refColorIndex;
    };
    
    /// Default constructor
    SimpleBlobSearcher();

    /// Destructor
    ~SimpleBlobSearcher();
    
    /// Adds a new reference color, with threshold and size range
    void add(const Color &color, 
             float thresh, 
             const Range32s &sizeRange);

    /// Updates data for given index
    void adapt(int index, const Color &color, 
               float thresh, const Range32s &sizeRange);

    /// removes reference color at given index
    void remove(int index);
    void clear();
    
    /// Actual detection function (no ROI support yet!)
    /** detects blobs in given image*/

    const std::vector<Blob> &detect(const Img8u &image);
    
  private:
    /// Internal data representation (hidden)
    class Data;
    
    /// internal data pointer
    Data *m_data;
  };
  
  
}

#endif
