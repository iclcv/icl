/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMarkers/BCHCode.h                           **
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

#ifndef BCH_CODE_H
#define BCH_CODE_H

#include <bitset>
#include <ICLUtils/BasicTypes.h>
#include <ICLCore/Img.h>

namespace icl{
  
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
  std::ostream &operator<<(std::ostream &s, const DecodedBCHCode2D::Rotation &r);


  /// encodes a given index in range [0,4095] to a BCHBinary code
  BCHCode encode_bch(int idx);

  /// decodes given binary code
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
  DecodedBCHCode decode_bch(const BCHCode &code);
  
  /// decodes given (correctly oriented) byte image patch
  DecodedBCHCode decode_bch(const icl8u data[36]);

  /// decodes given (correctly oriented) Img8u (optionally using its ROI or not)
  DecodedBCHCode decode_bch(const Img8u &image, bool useROI=true) throw (ICLException);

  /// decodes given Img8u (optionally using its ROI or not)
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
  DecodedBCHCode2D decode_bch_2D(const Img8u &image, int maxID=4095, bool useROI=true) throw (ICLException);

  /// interpretes the given 36Bit as 6x6 image and rotates it by clock wise by 90 degree
  BCHCode rotate_code(const BCHCode &in);
} 
#endif
