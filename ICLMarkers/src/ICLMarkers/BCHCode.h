/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/BCHCode.h                    **
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

#include <bitset>
#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLCore/Img.h>

namespace icl{
  namespace markers{
    
    /// used 36Bit BCH Code -> 12Bit data max-Error: 4bit
    typedef std::bitset<36> BCHCode;
  
    /// used to determine wich marker IDs are allowed
    typedef std::bitset<4096> BCHCodeSubSet;
    
    /// BCH decoder result
    struct DecodedBCHCode{
      BCHCode origCode;      //!< given input code
      BCHCode correctedCode; //!< corrected code
      int id;                //!< corresponding id/index (<0 mean error)
      int errors;            //!< number of errors that occured
      
      /// implicit bool cast that asks for successfull decoding
      operator bool() const { return id >= 0; }
  
      /// compares two instance by their error-count
      bool operator<(const DecodedBCHCode &c){
        return errors < c.errors;
      }
    };
  
    /// slightly more comples decoding result that does also contain an rotation value
    struct DecodedBCHCode2D : public DecodedBCHCode{
      DecodedBCHCode2D(){}
      DecodedBCHCode2D(const DecodedBCHCode &code):DecodedBCHCode(code),rot(Rot0){}
      enum Rotation{ Rot0, Rot90, Rot180, Rot270} rot;
    };
  
    /// ostream-operator for DecodedBCHCode2D::Rotation
    ICLMarkers_API std::ostream &operator<<(std::ostream &s, const DecodedBCHCode2D::Rotation &r);
  
    /// Main class for BCH encoding/decoding
    /** Due to some internal buffers, this must be implemented as a class */
    class ICLMarkers_API BCHCoder : public utils::Uncopyable{
      class Impl; //!< internal implementation structure
      Impl *impl; //!< implementation pointer
      
      public:
      /// Default constructor
      BCHCoder();
  
      /// Destructor
      ~BCHCoder();
                  
      /// encodes a given index in range [0,4095] to a BCHBinary code
      static BCHCode encode(int idx);
    
      /// creates an image that show a given bch marker
      /** @param idx which marker
          @param border amount of border pixels (here, we use the unit of marker pixels,
          the maker code is always 6x6 marker pixels)
          @param resultSize first a (2*border+6)x(2*border+6) image of the marker is created.
          Then it is upscaled to the given utils::Size using nearest neighbour interpolation.
          If resultSize is null, the (2*border+6)x(2*border+6) image is returned
          directly without upscaling */
      static core::Img8u createMarkerImage(int idx, int border=2, const utils::Size &resultSize=utils::Size::null);
  
      /// interpretes the given 36Bit as 6x6 image and rotates it by clock wise by 90 degree
      static BCHCode rotateCode(const BCHCode &in);
  
      
      /** We use an error correcting 36Bit BCH code that carries
          12 bit of information (i.e. each possible code is associated
          with a unique index (0 <= index <= 4095). 
          The code provides automatic error correction for inputs that
          have not more than 4 errors. Its minimal hammin distance is 8.
          The higher the code indices, the lower their hamming distance:
          - min hamming distance > 8 : all
          - min hamming distance > 9 : IDs 0-63
          - min hamming distance > 10 : IDs 0-15
          - min hamming distance > 13 : IDs 0-3
          - min hamming distance > 16 : IDs 0-1 
          
          \section BENCH benchmark
          Decoding works quite fast, however the decoder works slightly slower
          if errors have to be corrected. We benchmarked the BCHDecoder on an
          Intel(R) Xeon(R) CPU E5530 running at 2.40GHz, on 32Bit Ubuntu Linux.
          * Decoding without errors (2ns)
          * Decoding with 1,2 and 3 errors (3ns)
          * Decoding with 4 errors (4ns)
          
          \section IMPL Encoder/Decoder Implementation
          We used the code implemented by Robert Morelos-Zaragoza. You can
          find his copyright note in the corresponding .cpp file
          */
      DecodedBCHCode decode(const BCHCode &code);
      
      /// decodes given (correctly oriented) byte image patch
      DecodedBCHCode decode(const icl8u data[36]);
      
      /// decodes given (correctly oriented) core::Img8u (optionally using its ROI or not)
      DecodedBCHCode decode(const core::Img8u &image, bool useROI=true) throw (utils::ICLException);
      
      /// decodes given core::Img8u (optionally using its ROI or not)
      /** Internally, this methods checks for all 4 possible image rotations and returns
          the most plausible result.
          
          \section ROT Rotated Hamming Distances
          If we allow rotations, the expected inter-marker hamming distances gets smaller.
          - min hamming distance > 1:  all
          - min hamming distance > 2:  3080
          - min hamming distance > 4:  2492
          - min hamming distance > 5:  482
          - min hamming distance > 8:  18
          - min hamming distance > 10: 10
          - min hamming distance > 11: 4
          
          <b>please note</b> that it's best to use the first N IDs if you want to use N markers
          */
      DecodedBCHCode2D decode2D(const core::Img8u &image, int maxID=4095, bool useROI=true) throw (utils::ICLException);
      
    };
  } // namespace markers
} 
