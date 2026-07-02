// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <array>
#include <cstdint>
#include <utility>

#include <icl/utils/Exception.h>
#include <icl/markers/BCHCode.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::markers {

  /* Binary BCH marker codes. The workhorse is the 6x6 BCH(36,12,t=4) code used by
     ICL's BCH fiducials (see BCHCoder below); SquareBCHCode reuses the same
     machinery for smaller n x n grids.

     The error-correcting core is the classic Berlekamp/Chien BCH decoder of
     Robert Morelos-Zaragoza (http://www.eccpage.com/bch3.c, free for non-
     commercial use), the same lineage ARToolKitPlus uses. Historically the 6x6
     codewords were shipped as a precomputed string table plus a "magic" XOR
     constant in the decoder; both were provably redundant and have been replaced
     by the real systematic encoder in BCHEngine (WHITENING documents what the
     constant actually was). Observable behaviour is pinned by
     tests/test-markers-bch-code.cpp. */

  namespace {

    // Compile-time maxima (largest supported grid is 6x6 over GF(2^6)); the engine
    // is parameterized at runtime but its scratch buffers are sized to these.
    constexpr int MAX_M       = 6;
    constexpr int MAX_FIELD_N = (1 << MAX_M) - 1;   // 63
    constexpr int MAX_LENGTH  = 36;                 // 6*6
    constexpr int MAX_T       = 6;

    /// Reverse the low 36 bits of \a v — the fixed permutation that maps the
    /// legacy 6x6 code's bit order to the printed raster order. Used ONLY by the
    /// 6x6 BCHCoder for ARToolKitPlus bit-compatibility; new codes don't need it.
    inline uint64_t reverse36(uint64_t v) {
      uint64_t r = 0;
      for (int i = 0; i < 36; ++i)
        if (v & (1ull << i)) r |= 1ull << (35 - i);
      return r;
    }

    /// low-m coefficients (x^0..x^(m-1)) of a primitive polynomial for GF(2^m)
    int primitiveTaps(int m) {
      switch (m) {
        case 2: return 0b11;      // x^2 + x + 1
        case 3: return 0b011;     // x^3 + x + 1
        case 4: return 0b0011;    // x^4 + x + 1
        case 5: return 0b00101;   // x^5 + x^2 + 1
        case 6: return 0b000011;  // x^6 + x + 1
        default: throw ICLException("BCHEngine: unsupported field size");
      }
    }

    /// GF(2^m) tables + BCH generator polynomial + systematic encode and
    /// error-correcting decode, parameterized by codeword length and correctable
    /// error count. Immutable after construction; decode() keeps its scratch state
    /// local and is therefore reentrant.
    class BCHEngine {
      int m_m = 0, m_fieldN = 0, m_length = 0, m_t = 0, m_k = 0;
      std::array<int, MAX_FIELD_N + 1> alphaTo{};  ///< exp table: alphaTo[i] = alpha^i
      std::array<int, MAX_FIELD_N + 1> indexOf{};  ///< log table: indexOf[alpha^i] = i
      std::array<int, MAX_LENGTH + 1>  gen{};      ///< generator polynomial (binary coeffs)

    public:
      /// \a length-bit BCH codeword correcting up to \a t errors. Throws if length
      /// exceeds MAX_LENGTH or the code ends up with no information bits.
      BCHEngine(int length, int t) : m_length(length), m_t(t) {
        if (length < 1 || length > MAX_LENGTH || t < 1 || t > MAX_T)
          throw ICLException("BCHEngine: length/t out of supported range");
        m_m = 1;
        while ((1 << m_m) - 1 < length) ++m_m;      // smallest field that fits
        m_fieldN = (1 << m_m) - 1;
        buildField();
        buildGenerator();                            // sets m_k
        if (m_k < 1)
          throw ICLException("BCHEngine: t too large for this length (no data bits)");
      }

      int length() const { return m_length; }
      int k()      const { return m_k; }
      int t()      const { return m_t; }

      /// systematic encoder: k-bit \a id -> length-bit codeword (parity in bits
      /// [0,parity), id in bits [parity,length); LFSR division by gen).
      uint64_t encode(int id) const {
        const int parity = m_length - m_k;
        std::array<int, MAX_LENGTH> bb{};            // parity shift register (binary)
        for (int i = m_k - 1; i >= 0; --i) {
          const int feedback = ((id >> i) & 1) ^ bb[parity - 1];
          if (feedback) {
            for (int j = parity - 1; j > 0; --j) bb[j] = gen[j] ? (bb[j - 1] ^ 1) : bb[j - 1];
            bb[0] = gen[0] ? 1 : 0;
          } else {
            for (int j = parity - 1; j > 0; --j) bb[j] = bb[j - 1];
            bb[0] = 0;
          }
        }
        uint64_t cw = 0;
        for (int i = 0; i < parity; ++i) if (bb[i])         cw |= 1ull << i;
        for (int i = 0; i < m_k;   ++i) if ((id >> i) & 1)  cw |= 1ull << (parity + i);
        return cw;
      }

      /// error-correcting decoder: syndromes -> Berlekamp locator -> Chien search.
      /// returns {correctedCodeword, numErrors}; numErrors > t means uncorrectable.
      std::pair<uint64_t, int> decode(uint64_t code) const {
        const int n = m_fieldN, t2 = 2 * m_t;

        // (1) syndromes S_1..S_2t, in index/log form (-1 == the 0 element)
        std::array<int, 2 * MAX_T + 2> s{};
        int synError = 0;
        for (int i = 1; i <= t2; ++i) {
          int syn = 0;
          for (int j = 0; j < m_length; ++j)
            if (code & (1ull << j)) syn ^= alphaTo[(i * j) % n];
          if (syn) synError = 1;
          s[i] = indexOf[syn];
        }
        if (!synError) return {code, 0};

        // (2) Berlekamp's iteration -> error-locator polynomial elp
        std::array<std::array<int, 2 * MAX_T + 2>, MAX_LENGTH + 1> elp{};
        std::array<int, 2 * MAX_T + 2> d{}, l{}, ulu{};
        d[0] = 0; d[1] = s[1];
        elp[0][0] = 0; elp[0][1] = 1;
        for (int i = 1; i < t2; ++i) { elp[i][0] = -1; elp[i][1] = 0; }
        l[0] = 0; l[1] = 0; ulu[0] = -1; ulu[1] = 0;
        int u = 0;
        do {
          ++u;
          if (d[u] == -1) {
            l[u + 1] = l[u];
            for (int i = 0; i <= l[u]; ++i) {
              elp[i][u + 1] = elp[i][u];
              elp[i][u]     = indexOf[elp[i][u]];
            }
          } else {
            int q = u - 1;
            while (d[q] == -1 && q > 0) --q;
            if (q > 0) {
              int j = q;
              do { --j; if (d[j] != -1 && ulu[q] < ulu[j]) q = j; } while (j > 0);
            }
            l[u + 1] = (l[u] > l[q] + u - q) ? l[u] : l[q] + u - q;
            for (int i = 0; i < t2; ++i) elp[i][u + 1] = 0;
            for (int i = 0; i <= l[q]; ++i)
              if (elp[i][q] != -1)
                elp[i + u - q][u + 1] = alphaTo[(d[u] + n - d[q] + elp[i][q]) % n];
            for (int i = 0; i <= l[u]; ++i) {
              elp[i][u + 1] ^= elp[i][u];
              elp[i][u]      = indexOf[elp[i][u]];
            }
          }
          ulu[u + 1] = u - l[u + 1];

          if (u < t2) {
            d[u + 1] = (s[u + 1] != -1) ? alphaTo[s[u + 1]] : 0;
            for (int i = 1; i <= l[u + 1]; ++i)
              if (s[u + 1 - i] != -1 && elp[i][u + 1] != 0)
                d[u + 1] ^= alphaTo[(s[u + 1 - i] + indexOf[elp[i][u + 1]]) % n];
            d[u + 1] = indexOf[d[u + 1]];
          }
        } while (u < t2 && l[u + 1] <= m_t);

        ++u;
        const int deg = l[u];
        if (deg > m_t) return {code, deg};

        for (int i = 0; i <= deg; ++i) elp[i][u] = indexOf[elp[i][u]];

        // (3) Chien search: roots of elp are the error locations
        std::array<int, 2 * MAX_T + 2>   reg{};
        std::array<int, MAX_FIELD_N + 1> loc{};
        for (int i = 1; i <= deg; ++i) reg[i] = elp[i][u];
        int count = 0;
        for (int i = 1; i <= n; ++i) {
          int q = 1;
          for (int j = 1; j <= deg; ++j)
            if (reg[j] != -1) { reg[j] = (reg[j] + j) % n; q ^= alphaTo[reg[j]]; }
          if (!q) loc[count++] = n - i;
        }
        if (count != deg) return {code, deg};

        bool tooMany = false;
        for (int i = 0; i < deg; ++i) {
          const int li = loc[i];
          if (li < m_length) code ^= (1ull << li);
          else tooMany = true;
        }
        return {code, tooMany ? (m_t + 1) : deg};
      }

    private:
      /// Build GF(2^m) exp/log tables from a primitive polynomial.
      void buildField() {
        const int m = m_m, n = m_fieldN, taps = primitiveTaps(m);
        int mask = 1;
        alphaTo[m] = 0;
        for (int i = 0; i < m; ++i) {
          alphaTo[i] = mask;
          indexOf[mask] = i;
          if ((taps >> i) & 1) alphaTo[m] ^= mask;
          mask <<= 1;
        }
        indexOf[alphaTo[m]] = m;
        mask >>= 1;
        for (int i = m + 1; i < n; ++i) {
          alphaTo[i] = (alphaTo[i - 1] >= mask)
                     ? alphaTo[m] ^ ((alphaTo[i - 1] ^ mask) << 1)
                     : (alphaTo[i - 1] << 1);
          indexOf[alphaTo[i]] = i;
        }
        indexOf[0] = -1;
      }

      /// Build the generator polynomial = product of the minimal polynomials of
      /// alpha^1 .. alpha^(2t). Sets m_k = length - deg(generator).
      void buildGenerator() {
        constexpr int NC = 1024;
        std::array<std::array<int, 21>, NC> cycle{};
        std::array<int, NC> csize{}, minp{}, zeros{};
        const int n = m_fieldN;

        // cyclotomic cosets modulo n
        cycle[0][0] = 0; csize[0] = 1;
        cycle[1][0] = 1; csize[1] = 1;
        int jj = 1, ll = 0;
        do {
          int ii = 0;
          do {
            ++ii;
            cycle[jj][ii] = (cycle[jj][ii - 1] * 2) % n;
            ++csize[jj];
          } while ((cycle[jj][ii] * 2) % n != cycle[jj][0]);

          int test;
          ll = 0;
          do {
            ++ll; test = 0;
            for (int i = 1; i <= jj && !test; ++i)
              for (int k = 0; k < csize[i] && !test; ++k)
                if (ll == cycle[i][k]) test = 1;
          } while (test && ll < n - 1);
          if (!test) { ++jj; cycle[jj][0] = ll; csize[jj] = 1; }
        } while (ll < n - 1);
        const int nocycles = jj;

        const int dmin = 2 * m_t + 1;
        int kaux = 0, rdncy = 0;
        for (int ii = 1; ii <= nocycles; ++ii) {
          minp[kaux] = 0; int test = 0;
          for (int j = 0; j < csize[ii] && !test; ++j)
            for (int root = 1; root < dmin && !test; ++root)
              if (root == cycle[ii][j]) { test = 1; minp[kaux] = ii; }
          if (minp[kaux]) { rdncy += csize[minp[kaux]]; ++kaux; }
        }
        const int noterms = kaux;
        kaux = 1;
        for (int ii = 0; ii < noterms; ++ii)
          for (int j = 0; j < csize[minp[ii]]; ++j) zeros[kaux++] = cycle[minp[ii]][j];

        gen[0] = alphaTo[zeros[1]]; gen[1] = 1;
        for (int ii = 2; ii <= rdncy; ++ii) {
          gen[ii] = 1;
          for (int j = ii - 1; j > 0; --j)
            gen[j] = gen[j] ? (gen[j - 1] ^ alphaTo[(indexOf[gen[j]] + zeros[ii]) % n])
                            : gen[j - 1];
          gen[0] = alphaTo[(indexOf[gen[0]] + zeros[ii]) % n];
        }
        m_k = m_length - rdncy;
      }
    };

    /* Whitening mask for the legacy 6x6 code: XORed into every codeword before it
       becomes the printed pattern (and XORed back out on decode). NOT part of the
       BCH math — XOR-by-a-constant preserves every pairwise Hamming distance, so
       error-correction is unaffected. It only avoids a degenerate marker: without
       it id 0 (the all-zero codeword) would render as a solid black square. This
       exact value is the ARToolKitPlus convention (reverse36(WHITENING)==encode(0)),
       keeping ICL's 6x6 markers bit-compatible with it. */
    constexpr uint64_t WHITENING = 0x8f80b8750ull;

    /// the shared, immutable 6x6 BCH(36,12,t=4) engine (thread-safe local static)
    const BCHEngine &engine6x6() {
      static const BCHEngine e(36, 4);
      return e;
    }

    // 6x6 marker specifics
    constexpr int SIX_PARITY = 24;           // 36 - 12
    constexpr int SIX_MAX_ID = (1 << 12) - 1;

    /// codeword bit-image of a BCHCode (marker interior only, no border)
    BCHCode codeFromImage(const Img8u &image, bool useROI) {
      BCHCode code(0);
      ICLASSERT_THROW(image.getChannels(),
                      ICLException("BCHCoder: input image has no channels"));
      if (useROI) {
        ICLASSERT_THROW(image.getROISize().getDim() == 36,
                        ICLException("BCHCoder: image ROI dim must be 36"));
        Img8u::const_roi_iterator it = image.beginROI(0);
        for (int i = 0; i < 36; ++i, ++it) code[i] = *it;
      } else {
        ICLASSERT_THROW(image.getSize().getDim() == 36,
                        ICLException("BCHCoder: image dim must be 36"));
        Img8u::const_iterator it = image.begin(0);
        for (int i = 0; i < 36; ++i, ++it) code[i] = *it;
      }
      return code;
    }

  } // anonymous namespace

  // ======================= 6x6 BCHCoder (public façade) =======================

  // holds no state of its own — everything lives in engine6x6()
  struct BCHCoder::Impl {};

  BCHCoder::BCHCoder() : impl(new Impl) {}
  BCHCoder::~BCHCoder() { delete impl; }

  BCHCode BCHCoder::encode(int idx) {
    if (idx < 0 || idx > SIX_MAX_ID)
      throw ICLException("invalid bch code ID (allowed: 0 <= index <= 4095)");
    const uint64_t stored = reverse36(engine6x6().encode(idx) ^ WHITENING);
    BCHCode c;
    for (int i = 0; i < 36; ++i) c[i] = (stored >> i) & 1;
    return c;
  }

  DecodedBCHCode BCHCoder::decode(const BCHCode &code) {
    const uint64_t cw = reverse36(code.to_ullong()) ^ WHITENING;
    const auto [corrected, nerr] = engine6x6().decode(cw);

    DecodedBCHCode ret;
    ret.origCode = code;
    if (nerr > 4) {
      ret.errors = 36;
      ret.id     = -1;
    } else {
      ret.errors        = nerr;
      ret.id            = (int)((corrected >> SIX_PARITY) & SIX_MAX_ID);
      ret.correctedCode = nerr ? encode(ret.id) : code;
    }
    return ret;
  }

  DecodedBCHCode BCHCoder::decode(const icl8u data[36]) {
    BCHCode code(0);
    for (int i = 0; i < 36; ++i) code[i] = data[i];
    return decode(code);
  }

  DecodedBCHCode BCHCoder::decode(const Img8u &image, bool useROI) {
    return decode(codeFromImage(image, useROI));
  }

  BCHCode BCHCoder::rotateCode(const BCHCode &in) {
    BCHCode out(0);
    for (int y = 0; y < 6; ++y)
      for (int x = 0; x < 6; ++x)
        if (in[x + 6 * y]) out.set(5 + 6 * x - y);
    return out;
  }

  DecodedBCHCode2D BCHCoder::decode2D(const Img8u &image, int maxID, bool useROI) {
    BCHCode last = codeFromImage(image, useROI);
    DecodedBCHCode2D best = decode(last);
    if (best.id > maxID) { best.errors = 36; best.id = -1; }
    best.rot = DecodedBCHCode2D::Rot0;
    if (!best.errors) return best;

    for (int i = 1; i < 4; ++i) {
      last = rotateCode(last);
      DecodedBCHCode2D curr = decode(last);
      if (curr.id > maxID) continue;
      curr.rot = static_cast<DecodedBCHCode2D::Rotation>(i);
      if (!curr.errors) return curr;
      if (curr < best) best = curr;
    }
    return best;
  }

  std::ostream &operator<<(std::ostream &s, const DecodedBCHCode2D::Rotation &r) {
    return s << (r == DecodedBCHCode2D::Rot0   ? "0 Degree" :
                 r == DecodedBCHCode2D::Rot90  ? "90 Degree" :
                 r == DecodedBCHCode2D::Rot180 ? "180 Degree" :
                 r == DecodedBCHCode2D::Rot270 ? "270 Degree" : "??? Degree");
  }

  Img8u BCHCoder::createMarkerImage(int idx, int border, const Size &resultSize) {
    if (border < 0) throw ICLException("create_bch_marker_image: border must be >= 0");
    const BCHCode c = encode(idx);
    Img8u im(Size(6 + 2 * border, 6 + 2 * border), 1);
    Channel8u ch = im[0];
    im.fill(0);
    for (int y = 0; y < 6; ++y)
      for (int x = 0; x < 6; ++x)
        ch(border + x, border + y) = 255 * (c[x + 6 * y]);
    if (resultSize != Size::null) im.scale(resultSize, interpolateNN);
    return im;
  }

  // ===================== SquareBCHCode (parameterized n x n) ===================

  struct SquareBCHCode::Data {
    int       n;
    BCHEngine engine;
    int       parity;
    uint64_t  idMask;
    uint64_t  whitening;   // checkerboard: id 0 -> busy pattern, not all-black

    Data(int gridSize, int t)
      : n(gridSize), engine(gridSize * gridSize, t) {
      parity = engine.length() - engine.k();
      idMask = (engine.k() >= 64) ? ~0ull : ((1ull << engine.k()) - 1);
      whitening = 0;
      for (int i = 0; i < n * n; ++i) {
        const int x = i % n, y = i / n;
        if ((x + y) & 1) whitening |= 1ull << i;
      }
    }
  };

  SquareBCHCode::SquareBCHCode(int gridSize, int correctable)
    : m_data(new Data(gridSize, correctable)) {}

  SquareBCHCode::~SquareBCHCode() { delete m_data; }

  int SquareBCHCode::gridSize()    const { return m_data->n; }
  int SquareBCHCode::numBits()     const { return m_data->n * m_data->n; }
  int SquareBCHCode::correctable() const { return m_data->engine.t(); }
  int SquareBCHCode::numIds()      const { return 1 << m_data->engine.k(); }
  int SquareBCHCode::minDistance() const { return 2 * m_data->engine.t() + 1; }

  uint64_t SquareBCHCode::encode(int id) const {
    if (id < 0 || id >= numIds())
      throw ICLException("SquareBCHCode::encode: id out of range");
    return m_data->engine.encode(id) ^ m_data->whitening;
  }

  uint64_t SquareBCHCode::rotate90(uint64_t bits) const {
    const int n = m_data->n;
    uint64_t out = 0;
    for (int y = 0; y < n; ++y)
      for (int x = 0; x < n; ++x)
        if (bits & (1ull << (x + n * y))) out |= 1ull << ((n - 1 - y) + n * x);
    return out;
  }

  SquareBCHCode::Decoded SquareBCHCode::decode(uint64_t bits) const {
    const auto [corrected, nerr] = m_data->engine.decode(bits ^ m_data->whitening);
    Decoded d;
    if (nerr > m_data->engine.t()) return d;   // id stays -1
    d.id     = (int)((corrected >> m_data->parity) & m_data->idMask);
    d.errors = nerr;
    return d;
  }

  SquareBCHCode::Decoded SquareBCHCode::decode2D(uint64_t bits) const {
    Decoded best;
    uint64_t cur = bits;
    for (int r = 0; r < 4; ++r) {
      Decoded d = decode(cur);
      if (d) {
        d.rotation = r;
        if (d.errors == 0) return d;                 // exact match wins immediately
        if (!best || d.errors < best.errors) best = d;
      }
      cur = rotate90(cur);
    }
    return best;
  }

  Img8u SquareBCHCode::markerImage(int id, int border, const Size &size) const {
    if (border < 0) throw ICLException("SquareBCHCode::markerImage: border must be >= 0");
    const int n = m_data->n;
    const uint64_t bits = encode(id);
    Img8u im(Size(n + 2 * border, n + 2 * border), 1);
    Channel8u ch = im[0];
    im.fill(0);
    for (int y = 0; y < n; ++y)
      for (int x = 0; x < n; ++x)
        ch(border + x, border + y) = 255 * ((bits >> (x + n * y)) & 1);
    if (size != Size::null) im.scale(size, interpolateNN);
    return im;
  }

} // namespace icl::markers
