// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/BasicTypes.h>
#include <icl/core/Img.h>
#include <bitset>
#include <cstdint>
#include <vector>

namespace icl::markers {
  /// used 36Bit BCH Code -> 12Bit data max-Error: 4bit
  using BCHCode = std::bitset<36>;

  /// used to determine wich marker IDs are allowed
  using BCHCodeSubSet = std::bitset<4096>;

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
  class ICLMarkers_API BCHCoder {
    class Impl; //!< internal implementation structure
    Impl *impl; //!< implementation pointer

    public:
    BCHCoder(const BCHCoder&) = delete;
    BCHCoder& operator=(const BCHCoder&) = delete;

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
    DecodedBCHCode decode(const core::Img8u &image, bool useROI=true);

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
    DecodedBCHCode2D decode2D(const core::Img8u &image, int maxID=4095, bool useROI=true);

  };


  /// Well-known predefined square-BCH marker configurations (grid size + error
  /// correction t). Names read BCH_<n>x<n>_t<t>. See SquareBCHCode::presetTable().
  enum class SquareBCHPreset {
    BCH_3x3_t1,   ///< 3x3, t=1:   32 ids, d=3   (few, rotation-poor)
    BCH_4x4_t1,   ///< 4x4, t=1: 2048 ids, d=3
    BCH_4x4_t2,   ///< 4x4, t=2:   64 ids, d=5   (fully rotation-safe)
    BCH_5x5_t2,   ///< 5x5, t=2: 32768 ids, d=5
    BCH_5x5_t3,   ///< 5x5, t=3: 1024 ids, d=7
    BCH_5x5_t4,   ///< 5x5, t=4:   32 ids, d=9   (fully rotation-safe)
    BCH_6x6_t4,   ///< 6x6, t=4: 4096 ids, d=9   (same id space as legacy BCHCoder)
  };

  /// Static features of a SquareBCHPreset (see SquareBCHCode::presetTable()).
  struct SquareBCHPresetInfo {
    SquareBCHPreset preset;
    const char *name;    ///< e.g. "BCH_4x4_t2"
    int gridSize;        ///< n
    int correctable;     ///< t
    int numBits;         ///< n*n
    int maxIds;          ///< raw code space (1<<k)
    int minDistance;     ///< design minimum distance (2t+1)
  };


  /// Square (n x n) binary BCH marker code — generalizes the 6x6 BCHCoder.
  /** BCHCoder is hard-wired to the 6x6 / GF(2^6) / 12-bit-id marker used by ICL's
      FiducialDetector. SquareBCHCode is the same BCH machinery (systematic
      encoder + Berlekamp/Chien decoder) parameterized over the grid size, so it
      can produce the smaller markers wanted for dense coded targets:

      | grid | bits | field  | t=1 ids | t=2 ids | t=3 ids | t=4 ids |
      |------|------|--------|---------|---------|---------|---------|
      | 3x3  |  9   | GF(2^4)|   32    |    2    |   -     |   -     |
      | 4x4  | 16   | GF(2^5)|  2048   |   64    |    2    |   -     |
      | 5x5  | 25   | GF(2^5)| 1M+     |  ~32k   |  1024   |   32    |
      | 6x6  | 36   | GF(2^6)| 16M+    |   ~16k  |  ~1k    |   4096  |

      (exact counts come from numIds(); the table is indicative). As with the 6x6
      code the lowest ids carry the largest inter-marker distance, so a target
      should always allocate ids from 0 upward.

      Bit layout: bit index = col + n*row. The rendered pattern is the systematic
      codeword XORed with a fixed checkerboard whitening mask (a Hamming isometry,
      so it does not affect error-correction) that keeps id 0 from being a
      degenerate all-black square. There is no ARToolKitPlus bit-reversal here —
      this is a fresh code, not the legacy 6x6 one. */
  class ICLMarkers_API SquareBCHCode {
    struct Data;
    Data *m_data;

  public:
    /// \a gridSize (n, 3..6) square marker correcting up to \a correctable errors.
    /** Throws if the resulting code has no information bits (t too large for n). */
    SquareBCHCode(int gridSize, int correctable);
    /// construct one of the predefined well-known configurations
    explicit SquareBCHCode(SquareBCHPreset preset);
    ~SquareBCHCode();

    /// features of a preset (grid, t, id count, distance)
    static SquareBCHPresetInfo presetInfo(SquareBCHPreset preset);
    /// the full feature table over all presets
    static std::vector<SquareBCHPresetInfo> presetTable();

    SquareBCHCode(const SquareBCHCode &) = delete;
    SquareBCHCode &operator=(const SquareBCHCode &) = delete;

    int gridSize()    const;   ///< n
    int numBits()     const;   ///< n*n codeword length
    int correctable() const;   ///< t (design min distance is 2t+1)
    int numIds()      const;   ///< number of usable ids (1<<k)
    int minDistance() const;   ///< design minimum distance (2t+1)

    /// rendered n*n bit pattern for \a id (bit index = col + n*row); throws if out of range
    uint64_t encode(int id) const;
    /// rotate an n*n bit pattern 90 degrees clockwise
    uint64_t rotate90(uint64_t bits) const;

    /// decode result: id (<0 on failure), corrected error count, and — for
    /// decode2D — the number of clockwise quarter-turns applied to reach the
    /// canonical orientation.
    struct Decoded {
      int id       = -1;
      int errors   = 0;
      int rotation = 0;
      explicit operator bool() const { return id >= 0; }
    };

    /// decode a correctly-oriented n*n pattern
    Decoded decode(uint64_t bits) const;
    /// decode trying all 4 orientations, returning the first exact / best match
    Decoded decode2D(uint64_t bits) const;

    /// Minimum Hamming distance between the marker patterns of any two DISTINCT
    /// ids in \a ids, over ALL relative rotations (each pattern vs the 4
    /// rotations of the other). This is the separation the decoder actually has
    /// when markers may appear at any orientation — robust decoding of e errors
    /// needs it >= 2e+1. The plain (unrotated) code distance is always an upper
    /// bound; rotation only shrinks it. (Self-rotation/orientation ambiguity is
    /// not included.) Returns numBits() for sets of fewer than two ids.
    int rotatedMinDistance(const std::vector<int> &ids) const;

    /// Greedily assemble a rotation-robust marker set: scan ids 0,1,2,... and
    /// keep each whose pattern stays at rotated Hamming distance >=
    /// \a minRotatedDistance from every already-chosen marker (all relative
    /// orientations), up to \a maxCount ids (0 = no cap). Returns the chosen ids
    /// — an automated way to pick a marker set that survives rotation. The
    /// returned set satisfies rotatedMinDistance(set) >= minRotatedDistance.
    std::vector<int> selectRotationRobustIds(int minRotatedDistance,
                                             int maxCount = 0) const;

    /// render \a id as a marker image (n*n cells + \a border cells), optionally
    /// upscaled to \a size with nearest-neighbour interpolation
    core::Img8u markerImage(int id, int border = 1,
                            const utils::Size &size = utils::Size::null) const;
  };

  } // namespace icl::markers