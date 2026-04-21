// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke

#include <icl/utils/Size.h>
#include <icl/utils/StringUtils.h>
#include <cmath>
#include <istream>
#include <map>
#include <ostream>

namespace icl::utils {

  namespace {
    /// Named-resolution lookup (int only).  Returns {-1,-1} on miss,
    /// mirroring the historic size_from_string behavior.
    const Size &named_size_or_neg1(const std::string &name) {
      static const Size neg1{ -1, -1 };
      static const std::map<std::string, const Size*, std::less<>> m = [] {
        std::map<std::string, const Size*, std::less<>> out;
  #define A(X) out[#X] = &Size::X
        A(null);A(QQVGA);A(CGA);A(QVGA);
        A(HVGA);A(EGA);A(VGA);A(WVGA);A(SVGA);
        A(QHD);A(DVGA);A(XGA);A(XGAP);A(DSVGA);
        A(HD720);A(WXGA);A(WXGAP);A(SXVGA);A(SXGA);
        A(WSXGA);A(SXGAP);A(UXGA);A(HD1080);
        A(WUXGA);A(UD);A(CIF);A(SIF);A(SQCIF);
        A(QCIF);A(PAL);A(NTSC);
  #undef A
        return out;
      }();
      if (auto it = m.find(name); it != m.end()) return *it->second;
      return neg1;
    }
  }

  template<typename T>
  std::ostream &operator<<(std::ostream &os, const SizeT<T> &s) {
    return os << s.width << 'x' << s.height;
  }

  template<typename T>
  std::istream &operator>>(std::istream &is, SizeT<T> &s) {
    char c;
    is >> c;
    is.unget();
    if (c >= '0' && c <= '9') {
      return is >> s.width >> c >> s.height;
    }
    if constexpr (std::is_same_v<T, int>) {
      std::string str;
      is >> str;
      s = named_size_or_neg1(str);
    } else {
      // Named resolutions only meaningful for int; for other T we
      // silently skip the token and leave `s` at its default value.
      std::string str;
      is >> str;
    }
    return is;
  }

  // `explicit SizeT(const std::string&)` — uses operator>> via `parse<>`.
  template<typename T>
  SizeT<T>::SizeT(const std::string &name) {
    *this = parse<SizeT<T>>(name);
  }

  // ---------- explicit instantiations ----------

  template class SizeT<int>;
  template class SizeT<float>;

  template std::ostream &operator<<(std::ostream &, const SizeT<int> &);
  template std::ostream &operator<<(std::ostream &, const SizeT<float> &);
  template std::istream &operator>>(std::istream &, SizeT<int> &);
  template std::istream &operator>>(std::istream &, SizeT<float> &);

  } // namespace icl::utils
