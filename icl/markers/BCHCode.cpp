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

  /* Binary BCH(36,12,t=4) marker code — the 6x6 bit pattern used by ICL's BCH
     fiducials. It carries a 12-bit id (0..4095), corrects up to 4 bit errors, and
     has minimum Hamming distance 9 (larger for the lowest ids: 10 for ids 0..63,
     11 for ids 0..15 — so a marker set should always use the lowest ids).

     The error-correcting core is the classic Berlekamp/Chien BCH decoder of
     Robert Morelos-Zaragoza (http://www.eccpage.com/bch3.c, free for non-
     commercial use), the same lineage ARToolKitPlus uses. Historically the 4096
     codewords were shipped as a precomputed string table plus a "magic" XOR
     constant in the decoder; both were provably redundant and have been replaced
     by the real systematic encoder here (WHITENING documents what the constant
     actually was). The observable behaviour is pinned by tests/test-markers-bch-
     code.cpp. */

  namespace {

    // --- code parameters (6x6 BCH marker) ---
    constexpr int M       = 6;              // Galois field GF(2^M)
    constexpr int FIELD_N = (1 << M) - 1;   // 63  (# nonzero field elements)
    constexpr int LENGTH  = 36;             // codeword length (6*6 bits)
    constexpr int K       = 12;             // information bits  -> 4096 ids
    constexpr int T       = 4;              // correctable bit errors (d = 2T+1 = 9)
    constexpr int PARITY  = LENGTH - K;     // 24 parity bits
    constexpr int MAX_ID  = (1 << K) - 1;   // 4095

    /* Whitening mask: XORed into every codeword before it becomes the printed 6x6
       pattern (and XORed straight back out at the start of decoding). It is NOT
       part of the BCH math — XOR-by-a-constant preserves every pairwise Hamming
       distance, so error-correction is completely unaffected by it. Its only job
       is to avoid degenerate markers: without it id 0 (the all-zero codeword)
       would render as a solid black square — no interior structure, impossible to
       detect or to orient. This exact value is the ARToolKitPlus BCH convention
       (same Morelos-Zaragoza lineage), which keeps ICL markers bit-compatible
       with it; by construction reverse36(WHITENING) == encode(0). */
    constexpr uint64_t WHITENING = 0x8f80b8750ull;

    /// Reverse the low LENGTH(36) bits of \a v. A fixed permutation mapping the
    /// code's bit order to the printed 6x6 raster order (bit index = col + 6*row);
    /// independent of the whitening above.
    inline uint64_t reverse36(uint64_t v) {
      uint64_t r = 0;
      for (int i = 0; i < LENGTH; ++i)
        if (v & (1ull << i)) r |= 1ull << (LENGTH - 1 - i);
      return r;
    }

    /// GF(2^6) tables + the BCH generator polynomial + systematic encode and
    /// error-correcting decode. Immutable after construction (see engine()), so a
    /// single shared instance serves all callers; decode() keeps its scratch
    /// state local and is therefore reentrant.
    class BCHEngine {
      std::array<int, FIELD_N + 1> alphaTo{};  ///< exp table: alphaTo[i] = alpha^i
      std::array<int, FIELD_N + 1> indexOf{};  ///< log table: indexOf[alpha^i] = i
      std::array<int, PARITY + 1>  gen{};      ///< generator polynomial (binary coeffs)

    public:
      BCHEngine() { buildField(); buildGenerator(); }

      /// Systematic encoder: 12-bit \a id -> 36-bit codeword. Parity occupies bits
      /// [0,PARITY), the id occupies bits [PARITY,LENGTH). (LFSR division by gen.)
      uint64_t encode(int id) const {
        std::array<int, PARITY> bb{};                 // parity shift register (binary)
        for (int i = K - 1; i >= 0; --i) {
          const int feedback = ((id >> i) & 1) ^ bb[PARITY - 1];
          if (feedback) {
            for (int j = PARITY - 1; j > 0; --j) bb[j] = gen[j] ? (bb[j - 1] ^ 1) : bb[j - 1];
            bb[0] = gen[0] ? 1 : 0;                    // generator's constant term is 1
          } else {
            for (int j = PARITY - 1; j > 0; --j) bb[j] = bb[j - 1];
            bb[0] = 0;
          }
        }
        uint64_t cw = 0;
        for (int i = 0; i < PARITY; ++i) if (bb[i])         cw |= 1ull << i;
        for (int i = 0; i < K;      ++i) if ((id >> i) & 1) cw |= 1ull << (PARITY + i);
        return cw;
      }

      /// Error-correcting decoder. Returns {correctedCodeword, numErrors};
      /// numErrors > T means the word was uncorrectable. Three phases: compute the
      /// 2T syndromes, find the error-locator polynomial via Berlekamp's iteration,
      /// then locate the flipped bits via a Chien search.
      std::pair<uint64_t, int> decode(uint64_t code) const {
        const int n = FIELD_N, t2 = 2 * T;

        // (1) syndromes S_1..S_2T, stored in index/log form (-1 == the 0 element)
        std::array<int, 2 * T + 2> s{};
        int synError = 0;
        for (int i = 1; i <= t2; ++i) {
          int syn = 0;
          for (int j = 0; j < LENGTH; ++j)
            if (code & (1ull << j)) syn ^= alphaTo[(i * j) % n];
          if (syn) synError = 1;
          s[i] = indexOf[syn];
        }
        if (!synError) return {code, 0};               // clean word, nothing to do

        // (2) Berlekamp's iterative algorithm -> error-locator polynomial elp.
        // elp[coeff][step]; d = discrepancy, l = elp degree, ulu = step - degree.
        std::array<std::array<int, 2 * T + 2>, LENGTH + 1> elp{};
        std::array<int, 2 * T + 2> d{}, l{}, ulu{};
        d[0] = 0; d[1] = s[1];
        elp[0][0] = 0; elp[0][1] = 1;
        for (int i = 1; i < t2; ++i) { elp[i][0] = -1; elp[i][1] = 0; }
        l[0] = 0; l[1] = 0; ulu[0] = -1; ulu[1] = 0;
        int u = 0;
        do {
          ++u;
          if (d[u] == -1) {                            // zero discrepancy: copy down
            l[u + 1] = l[u];
            for (int i = 0; i <= l[u]; ++i) {
              elp[i][u + 1] = elp[i][u];
              elp[i][u]     = indexOf[elp[i][u]];
            }
          } else {
            // find the earlier step q with nonzero discrepancy and largest ulu
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

          if (u < t2) {                                // form the next discrepancy
            d[u + 1] = (s[u + 1] != -1) ? alphaTo[s[u + 1]] : 0;
            for (int i = 1; i <= l[u + 1]; ++i)
              if (s[u + 1 - i] != -1 && elp[i][u + 1] != 0)
                d[u + 1] ^= alphaTo[(s[u + 1 - i] + indexOf[elp[i][u + 1]]) % n];
            d[u + 1] = indexOf[d[u + 1]];
          }
        } while (u < t2 && l[u + 1] <= T);

        ++u;
        const int deg = l[u];                          // number/degree of errors
        if (deg > T) return {code, deg};               // more than T errors: give up

        for (int i = 0; i <= deg; ++i) elp[i][u] = indexOf[elp[i][u]];

        // (3) Chien search: the roots of elp point at the flipped bit positions
        std::array<int, 2 * T + 2>   reg{};
        std::array<int, FIELD_N + 1> loc{};
        for (int i = 1; i <= deg; ++i) reg[i] = elp[i][u];
        int count = 0;
        for (int i = 1; i <= n; ++i) {
          int q = 1;
          for (int j = 1; j <= deg; ++j)
            if (reg[j] != -1) { reg[j] = (reg[j] + j) % n; q ^= alphaTo[reg[j]]; }
          if (!q) loc[count++] = n - i;                // a root -> error location n-i
        }
        if (count != deg) return {code, deg};          // #roots != degree: unsolvable

        bool tooMany = false;
        for (int i = 0; i < deg; ++i) {
          const int li = loc[i];
          if (li < LENGTH) code ^= (1ull << li);       // flip the erroneous bit back
          else tooMany = true;                         // root outside the shortened code
        }
        return {code, tooMany ? (T + 1) : deg};
      }

    private:
      /// Build GF(2^6) exp/log tables from the primitive polynomial x^6 + x + 1.
      void buildField() {
        int mask = 1;
        alphaTo[M] = 0;
        for (int i = 0; i < M; ++i) {
          alphaTo[i] = mask;
          indexOf[mask] = i;
          if (i == 0 || i == 1) alphaTo[M] ^= mask;    // primitive-poly taps p0,p1
          mask <<= 1;
        }
        indexOf[alphaTo[M]] = M;
        mask >>= 1;
        for (int i = M + 1; i < FIELD_N; ++i) {
          alphaTo[i] = (alphaTo[i - 1] >= mask)
                     ? alphaTo[M] ^ ((alphaTo[i - 1] ^ mask) << 1)
                     : (alphaTo[i - 1] << 1);
          indexOf[alphaTo[i]] = i;
        }
        indexOf[0] = -1;
      }

      /// Build the generator polynomial = product of the minimal polynomials of
      /// alpha^1 .. alpha^(2T) (its degree is exactly PARITY = 24).
      void buildGenerator() {
        constexpr int NC = 1024;
        std::array<std::array<int, 21>, NC> cycle{};
        std::array<int, NC> csize{}, minp{}, zeros{};
        const int n = FIELD_N;

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

        // the cosets that contain a root in {1 .. 2T} form the generator's factors
        const int dmin = 2 * T + 1;
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

        // generator polynomial coefficients (become binary once complete)
        gen[0] = alphaTo[zeros[1]]; gen[1] = 1;
        for (int ii = 2; ii <= rdncy; ++ii) {
          gen[ii] = 1;
          for (int j = ii - 1; j > 0; --j)
            gen[j] = gen[j] ? (gen[j - 1] ^ alphaTo[(indexOf[gen[j]] + zeros[ii]) % n])
                            : gen[j - 1];
          gen[0] = alphaTo[(indexOf[gen[0]] + zeros[ii]) % n];
        }
      }
    };

    /// The single shared, immutable engine (thread-safe C++11 local static init).
    const BCHEngine &engine() {
      static const BCHEngine e;
      return e;
    }

    /// codeword bit-image of a BCHCode (marker interior only, no border)
    BCHCode codeFromImage(const Img8u &image, bool useROI) {
      BCHCode code(0);
      ICLASSERT_THROW(image.getChannels(),
                      ICLException("BCHCoder: input image has no channels"));
      if (useROI) {
        ICLASSERT_THROW(image.getROISize().getDim() == LENGTH,
                        ICLException("BCHCoder: image ROI dim must be 36"));
        Img8u::const_roi_iterator it = image.beginROI(0);
        for (int i = 0; i < LENGTH; ++i, ++it) code[i] = *it;
      } else {
        ICLASSERT_THROW(image.getSize().getDim() == LENGTH,
                        ICLException("BCHCoder: image dim must be 36"));
        Img8u::const_iterator it = image.begin(0);
        for (int i = 0; i < LENGTH; ++i, ++it) code[i] = *it;
      }
      return code;
    }

  } // anonymous namespace

  // The public BCHCoder holds no state of its own — everything lives in engine().
  struct BCHCoder::Impl {};

  BCHCoder::BCHCoder() : impl(new Impl) {}
  BCHCoder::~BCHCoder() { delete impl; }

  BCHCode BCHCoder::encode(int idx) {
    if (idx < 0 || idx > MAX_ID)
      throw ICLException("invalid bch code ID (allowed: 0 <= index <= 4095)");
    const uint64_t stored = reverse36(engine().encode(idx) ^ WHITENING);
    BCHCode c;
    for (int i = 0; i < LENGTH; ++i) c[i] = (stored >> i) & 1;
    return c;
  }

  DecodedBCHCode BCHCoder::decode(const BCHCode &code) {
    const uint64_t cw = reverse36(code.to_ullong()) ^ WHITENING;
    const auto [corrected, nerr] = engine().decode(cw);

    DecodedBCHCode ret;
    ret.origCode = code;
    if (nerr > T) {
      ret.errors = LENGTH;
      ret.id     = -1;
    } else {
      ret.errors        = nerr;
      ret.id            = (int)((corrected >> PARITY) & MAX_ID);
      ret.correctedCode = nerr ? encode(ret.id) : code;
    }
    return ret;
  }

  DecodedBCHCode BCHCoder::decode(const icl8u data[36]) {
    BCHCode code(0);
    for (int i = 0; i < LENGTH; ++i) code[i] = data[i];
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
    if (best.id > maxID) { best.errors = LENGTH; best.id = -1; }
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

} // namespace icl::markers
