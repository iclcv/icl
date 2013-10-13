/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/SSETypes.h                       **
** Module : ICLUtils                                               **
** Authors: Sergius Gaulik                                         **
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

#include <ICLUtils/BasicTypes.h>

namespace icl{
  namespace utils{

    #ifdef HAVE_SSE2

      // ++ basic SSE types ++ //

      struct Icl128 {
        __m128 v0;
      };

      struct Icl128i {
        __m128i v0;
      };

      struct Icl128d {
        __m128d v0;
      };

      struct Icl256 {
        __m128 v0; __m128 v1;
      };

      struct Icl256i {
        __m128i v0; __m128i v1;
      };

      struct Icl256d {
        __m128d v0; __m128d v1;
      };

      struct Icl512 {
        __m128 v0; __m128 v1; __m128 v2; __m128 v3;
      };

      struct Icl512i {
        __m128i v0; __m128i v1; __m128i v2; __m128i v3;
      };

      struct Icl512d {
        __m128d v0; __m128d v1; __m128d v2; __m128d v3;
      };

      struct Icl1024d {
        __m128d v0; __m128d v1; __m128d v2; __m128d v3;
        __m128d v4; __m128d v5; __m128d v6; __m128d v7;
      };

      // -- basic SSE types -- //


      // ++ advanced SSE types ++ //

      // 4 icl32f values
      struct icl128 : Icl128 {
        inline icl128() {
        }

        inline icl128(const __m128 &v) {
          v0 = v;
        }

        inline icl128(const icl32f *v) {
          v0 = _mm_loadu_ps(v);
        }

        inline icl128(const icl32f v) {
          v0 = _mm_set1_ps(v);
        }

        inline icl128(const Icl128 &v) {
          v0 = v.v0;
        }

        inline icl128(const Icl128i &v) {
          v0 = _mm_cvtepi32_ps(v.v0);
        }

        inline icl128& operator=(const __m128 &v) {
          v0 = v;
          return *this;
        }

        inline icl128& operator=(const icl32f *v) {
          v0 = _mm_loadu_ps(v);
          return *this;
        }

        inline icl128& operator=(const icl32f v) {
          v0 = _mm_set1_ps(v);
          return *this;
        }

        inline icl128& operator=(const Icl128 &v) {
          v0 = v.v0;
          return *this;
        }

        inline icl128& operator=(const Icl128i &v) {
          v0 = _mm_cvtepi32_ps(v.v0);
          return *this;
        }

        inline operator __m128 () const {
          return v0;
        }

        inline icl128& operator+=(const Icl128 &v) {
          v0 = _mm_add_ps(v0, v.v0);
          return *this;
        }

        inline icl128& operator-=(const Icl128 &v) {
          v0 = _mm_sub_ps(v0, v.v0);
          return *this;
        }

        inline icl128& operator*=(const Icl128 &v) {
          v0 = _mm_mul_ps(v0, v.v0);
          return *this;
        }

        inline icl128& operator/=(const Icl128 &v) {
          v0 = _mm_div_ps(v0, v.v0);
          return *this;
        }

        inline icl128& operator&=(const Icl128 &v) {
          v0 = _mm_and_ps(v0, v.v0);
          return *this;
        }

        inline icl128& operator|=(const Icl128 &v) {
          v0 = _mm_or_ps(v0, v.v0);
          return *this;
        }

        inline icl128& operator^=(const Icl128 &v) {
          v0 = _mm_xor_ps(v0, v.v0);
          return *this;
        }

        inline icl128& andnot(const Icl128 &v) {
          v0 = _mm_andnot_ps(v.v0, v0);
          return *this;
        }

        inline icl128& rcp() {
		      v0 = _mm_rcp_ps(v0);
          return *this;
        }

        inline void store(icl32f *v) const {
		      _mm_store_ps(v, v0);
        }

        inline void storeu(icl32f *v) const {
		      _mm_storeu_ps(v, v0);
        }
      };

      // 16 icl8u values or 8 icl16s values or 4 icl32s values
      struct icl128i : Icl128i {
        inline icl128i() {
        }

        inline icl128i(const __m128i &v) {
          v0 = v;
        }

        inline icl128i(const __m128i *v) {
          v0 = _mm_loadu_si128(v);
        }

        inline icl128i(const icl8s *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
        }

        inline icl128i(const icl8u *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
        }

        inline icl128i(const icl16s *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
        }

        inline icl128i(const icl16u *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
        }

        inline icl128i(const icl32s *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
        }

        inline icl128i(const icl32s i0, const icl32s i1, const icl32s i2, const icl32s i3) {
          v0 = _mm_set_epi32(i3, i2, i1, i0);
        }

        inline icl128i(const icl32u *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
        }

        inline icl128i(const icl8s v) {
          v0 = _mm_set1_epi32(v);
        }

        inline icl128i(const icl16s v) {
          v0 = _mm_set1_epi16(v);
        }

        inline icl128i(const icl32s v) {
          v0 = _mm_set1_epi32(v);
        }

        inline icl128i(const Icl128i &v) {
          v0 = v.v0;
        }

        inline icl128i(const Icl128 &v) {
		      //__m128 vMin = _mm_set1_ps(-2147483520.f);
	      	//__m128 vMax = _mm_set1_ps(2147483520.f);
          //v0 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v0, vMin), vMax));
          v0 = _mm_cvtps_epi32(v.v0);
        }

        inline icl128i& operator=(const __m128i &v) {
          v0 = v;
          return *this;
        }

        inline icl128i& operator=(const __m128i *v) {
          v0 = _mm_loadu_si128(v);
          return *this;
        }

        inline icl128i& operator=(const icl8s *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
          return *this;
        }

        inline icl128i& operator=(const icl8u *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
          return *this;
        }

        inline icl128i& operator=(const icl16s *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
          return *this;
        }

        inline icl128i& operator=(const icl16u *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
          return *this;
        }

        inline icl128i& operator=(const icl32s *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
          return *this;
        }

        inline icl128i& operator=(const icl32u *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
          return *this;
        }

        inline icl128i& operator=(const icl8s v) {
          v0 = _mm_set1_epi8(v);
          return *this;
        }

        inline icl128i& operator=(const icl16s v) {
          v0 = _mm_set1_epi16(v);
          return *this;
        }

        inline icl128i& operator=(const icl32s v) {
          v0 = _mm_set1_epi32(v);
          return *this;
        }

        inline icl128i& operator=(const Icl128i &v) {
		      v0 = v.v0;
          return *this;
        }

        inline icl128i& operator=(const Icl128 &v) {
		      //__m128 vMin = _mm_set1_ps(-2147483520.f);
	      	//__m128 vMax = _mm_set1_ps(2147483520.f);
          //v0 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v0, vMin), vMax));
          v0 = _mm_cvtps_epi32(v.v0);
          return *this;
        }

        inline operator __m128i () const {
          return v0;
        }

        inline icl128i& operator&=(const Icl128i &v) {
          v0 = _mm_and_si128(v0, v.v0);
          return *this;
        }

        inline icl128i& operator|=(const Icl128i &v) {
          v0 = _mm_or_si128(v0, v.v0);
          return *this;
        }

        inline icl128i& operator^=(const Icl128i &v) {
          v0 = _mm_xor_si128(v0, v.v0);
          return *this;
        }

        inline icl128i& andnot(const Icl128i &v) {
          v0 = _mm_andnot_si128(v.v0, v0);
          return *this;
        }

        inline void add8(const Icl128i &v) {
          v0 = _mm_add_epi8(v0, v.v0);
        }

        inline void add16(const Icl128i &v) {
          v0 = _mm_add_epi16(v0, v.v0);
        }

        inline void add32(const Icl128i &v) {
          v0 = _mm_add_epi32(v0, v.v0);
        }

        inline void sub8(const Icl128i &v) {
          v0 = _mm_sub_epi8(v0, v.v0);
        }

        inline void sub16(const Icl128i &v) {
          v0 = _mm_sub_epi16(v0, v.v0);
        }

        inline void sub32(const Icl128i &v) {
          v0 = _mm_sub_epi32(v0, v.v0);
        }

        inline void store(__m128i *v) const {
		      _mm_store_si128(v, v0);
        }

        inline void storeu(__m128i *v) const {
		      _mm_storeu_si128(v, v0);
        }

        inline void store(icl8s *v) const {
		      _mm_store_si128((__m128i*)v, v0);
        }

        inline void storeu(icl8s *v) const {
		      _mm_storeu_si128((__m128i*)v, v0);
        }

        inline void store(icl8u *v) const {
		      _mm_store_si128((__m128i*)v, v0);
        }

        inline void storeu(icl8u *v) const {
		      _mm_storeu_si128((__m128i*)v, v0);
        }

        inline void store(icl16s *v) const {
		      _mm_store_si128((__m128i*)v, v0);
        }

        inline void storeu(icl16s *v) const {
		      _mm_storeu_si128((__m128i*)v, v0);
        }

        inline void store(icl16u *v) const {
		      _mm_store_si128((__m128i*)v, v0);
        }

        inline void storeu(icl16u *v) const {
		      _mm_storeu_si128((__m128i*)v, v0);
        }

        inline void store(icl32s *v) const {
		      _mm_store_si128((__m128i*)v, v0);
        }

        inline void storeu(icl32s *v) const {
		      _mm_storeu_si128((__m128i*)v, v0);
        }

        inline void store(icl32u *v) const {
		      _mm_store_si128((__m128i*)v, v0);
        }

        inline void storeu(icl32u *v) const {
		      _mm_storeu_si128((__m128i*)v, v0);
        }
      };

      // 16 icl8u values
      struct icl128i8u : icl128i {
        inline icl128i8u() {
        }

        inline icl128i8u(const __m128i &v) {
          v0 = v;
        }

        inline icl128i8u(const __m128i *v) {
          v0 = _mm_loadu_si128(v);
        }

        inline icl128i8u(const icl8u *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
        }

        inline icl128i8u(const Icl128i &v) {
          v0 = v.v0;
        }

        inline icl128i8u(const Icl128 &v) {
		      //__m128 vMin = _mm_set1_ps(-2147483520.f);
	      	//__m128 vMax = _mm_set1_ps(2147483520.f);
          //v0 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v0, vMin), vMax));
          v0 = _mm_cvtps_epi32(v.v0);
        }

        inline icl128i8u& operator+=(const icl128i8u &v) {
          v0 = _mm_add_epi8(v0, v.v0);
          return *this;
        }

        inline icl128i8u& operator-=(const icl128i8u &v) {
          v0 = _mm_sub_epi8(v0, v.v0);
          return *this;
        }
      };

      // 8 icl16s values
      struct icl128i16s : icl128i {
        inline icl128i16s() {
        }

        inline icl128i16s(const __m128i &v) {
          v0 = v;
        }

        inline icl128i16s(const __m128i *v) {
          v0 = _mm_loadu_si128(v);
        }

        inline icl128i16s(const icl16s *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
        }

        inline icl128i16s(const icl16s v) {
          v0 = _mm_set1_epi16(v);
        }

        inline icl128i16s(const Icl128i &v) {
          v0 = v.v0;
        }

        inline icl128i16s(const Icl128 &v) {
		      //__m128 vMin = _mm_set1_ps(-2147483520.f);
	      	//__m128 vMax = _mm_set1_ps(2147483520.f);
          //v0 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v0, vMin), vMax));
          v0 = _mm_cvtps_epi32(v.v0);
        }

        inline icl128i16s& operator+=(const icl128i16s &v) {
          v0 = _mm_add_epi16(v0, v.v0);
          return *this;
        }

        inline icl128i16s& operator-=(const icl128i16s &v) {
          v0 = _mm_sub_epi16(v0, v.v0);
          return *this;
        }
      };

      // 4 icl32s values
      struct icl128i32s : icl128i {
        inline icl128i32s() {
        }

        inline icl128i32s(const __m128i &v) {
          v0 = v;
        }

        inline icl128i32s(const __m128i *v) {
          v0 = _mm_loadu_si128(v);
        }

        inline icl128i32s(const icl32s *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
        }

        inline icl128i32s(const icl32s i0, const icl32s i1, const icl32s i2, const icl32s i3) {
          v0 = _mm_set_epi32(i3, i2, i1, i0);
        }

        inline icl128i32s(const icl32s v) {
          v0 = _mm_set1_epi32(v);
        }

        inline icl128i32s(const Icl128i &v) {
          v0 = v.v0;
        }

        inline icl128i32s(const Icl128 &v) {
		      //__m128 vMin = _mm_set1_ps(-2147483520.f);
	      	//__m128 vMax = _mm_set1_ps(2147483520.f);
          //v0 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v0, vMin), vMax));
          v0 = _mm_cvtps_epi32(v.v0);
        }

        inline icl128i32s& operator+=(const icl128i32s &v) {
          v0 = _mm_add_epi32(v0, v.v0);
          return *this;
        }

        inline icl128i32s& operator-=(const icl128i32s &v) {
          v0 = _mm_sub_epi32(v0, v.v0);
          return *this;
        }
      };

      // 2 icl64f values
      struct icl128d : Icl128d {
        inline icl128d() {
        }

        inline icl128d(const __m128d &v) {
          v0 = v;
        }

        inline icl128d(const icl64f *v) {
          v0 = _mm_loadu_pd(v);
        }

        inline icl128d(const icl64f v) {
          v0 = _mm_set1_pd(v);
        }

        inline icl128d(const icl128d &v) {
          v0 = v.v0;
        }

        inline icl128d& operator=(const __m128d &v) {
          v0 = v;
          return *this;
        }

        inline icl128d& operator=(const icl64f *v) {
          v0 = _mm_loadu_pd(v);
          return *this;
        }

        inline icl128d& operator=(const icl128d &v) {
          v0 = v.v0;
          return *this;
        }

        inline operator __m128d () const {
          return v0;
        }

        inline icl128d& operator+=(const Icl128d &v) {
          v0 = _mm_add_pd(v0, v.v0);
          return *this;
        }

        inline icl128d& operator-=(const Icl128d &v) {
          v0 = _mm_sub_pd(v0, v.v0);
          return *this;
        }

        inline icl128d& operator*=(const Icl128d &v) {
          v0 = _mm_mul_pd(v0, v.v0);
          return *this;
        }

        inline icl128d& operator/=(const Icl128d &v) {
          v0 = _mm_div_pd(v0, v.v0);
          return *this;
        }

        inline icl128d& operator&=(const Icl128d &v) {
          v0 = _mm_and_pd(v0, v.v0);
          return *this;
        }

        inline icl128d& operator|=(const Icl128d &v) {
          v0 = _mm_or_pd(v0, v.v0);
          return *this;
        }

        inline icl128d& operator^=(const Icl128d &v) {
          v0 = _mm_xor_pd(v0, v.v0);
          return *this;
        }

        inline icl128d& andnot(const Icl128d &v) {
          v0 = _mm_andnot_pd(v.v0, v0);
          return *this;
        }

        inline void store(icl64f *v) const {
		      _mm_store_pd(v, v0);
        }

        inline void storeu(icl64f *v) const {
		      _mm_storeu_pd(v, v0);
        }
      };

      // 8 icl32f values
      struct icl256 : Icl256 {
        inline icl256() {
        }

        inline icl256(const __m128 &vl, const __m128 &vh) {
          v0 = vl;
          v1 = vh;
        }

        inline icl256(const __m128 *v) {
          v0 = *v;
          v1 = *(v+1);
        }

        inline icl256(const icl32f v) {
          v0 = _mm_set1_ps(v);
          v1 = _mm_set1_ps(v);
        }

        inline icl256(const Icl256 &v) {
          v0 = v.v0;
          v1 = v.v1;
        }

        inline icl256(const Icl256i &v) {
          v0 = _mm_cvtepi32_ps(v.v0);
          v1 = _mm_cvtepi32_ps(v.v1);
        }

        inline icl256& operator=(const __m128 *v) {
          v0 = *v;
          v1 = *(v+1);
          return *this;
        }

        inline icl256& operator=(const icl32f v) {
          v0 = _mm_set1_ps(v);
          v1 = _mm_set1_ps(v);
          return *this;
        }

        inline icl256& operator=(const Icl256 &v) {
          v0 = v.v0;
          v1 = v.v1;
          return *this;
        }

        inline icl256& operator=(const Icl256i &v) {
          v0 = _mm_cvtepi32_ps(v.v0);
          v1 = _mm_cvtepi32_ps(v.v1);
          return *this;
        }

        inline icl256& operator+=(const Icl256 &v) {
          v0 = _mm_add_ps(v0, v.v0);
          v1 = _mm_add_ps(v1, v.v1);
          return *this;
        }

        inline icl256& operator-=(const Icl256 &v) {
          v0 = _mm_sub_ps(v0, v.v0);
          v1 = _mm_sub_ps(v1, v.v1);
          return *this;
        }

        inline icl256& operator*=(const Icl256 &v) {
          v0 = _mm_mul_ps(v0, v.v0);
          v1 = _mm_mul_ps(v1, v.v1);
          return *this;
        }

        inline icl256& operator/=(const Icl256 &v) {
          v0 = _mm_div_ps(v0, v.v0);
          v1 = _mm_div_ps(v1, v.v1);
          return *this;
        }

        inline icl256& operator&=(const Icl256 &v) {
          v0 = _mm_and_ps(v0, v.v0);
          v1 = _mm_and_ps(v1, v.v1);
          return *this;
        }

        inline icl256& operator|=(const Icl256 &v) {
          v0 = _mm_or_ps(v0, v.v0);
          v1 = _mm_or_ps(v1, v.v1);
          return *this;
        }

        inline icl256& operator^=(const Icl256 &v) {
          v0 = _mm_xor_ps(v0, v.v0);
          v1 = _mm_xor_ps(v1, v.v1);
          return *this;
        }

        inline icl256& andnot(const Icl256 &v) {
          v0 = _mm_andnot_ps(v.v0, v0);
          v1 = _mm_andnot_ps(v.v1, v1);
          return *this;
        }

        inline icl256& rcp() {
		      v0 = _mm_rcp_ps(v0);
		      v1 = _mm_rcp_ps(v1);
          return *this;
        }

        inline void store(icl32f *v) const {
		      _mm_store_ps(v,   v0);
		      _mm_store_ps(v+4, v1);
        }

        inline void storeu(icl32f *v) const {
		      _mm_storeu_ps(v,   v0);
		      _mm_storeu_ps(v+4, v1);
        }
      };

      // 8 icl32s values or 16 icl16s values
      struct icl256i : Icl256i {
        inline icl256i() {
        }

        inline icl256i(const __m128i &vl, const __m128i &vh) {
          v0 = vl;
          v1 = vh;
        }

        inline icl256i(const __m128i *v) {
          v0 = *v;
          v1 = *(v+1);
        }

        inline icl256i(const icl16s *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
          v1 = _mm_loadu_si128((__m128i*)(v+8));
        }

        inline icl256i(const icl32s *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
          v1 = _mm_loadu_si128((__m128i*)(v+4));
        }

        inline icl256i(const Icl128i &v) {
          const __m128i vk0 = _mm_setzero_si128();
          v0 = _mm_unpacklo_epi8(v.v0, vk0);
          v1 = _mm_unpackhi_epi8(v.v0, vk0);
        }

        inline icl256i(const icl16s v) {
          v0 = _mm_set1_epi16(v);
          v1 = _mm_set1_epi16(v);
        }

        inline icl256i(const icl32s v) {
          v0 = _mm_set1_epi32(v);
          v1 = _mm_set1_epi32(v);
        }

        inline icl256i(const Icl256i &v) {
          v0 = v.v0;
          v1 = v.v1;
        }

        inline icl256i(const Icl256 &v) {
		      //__m128 vMin = _mm_set1_ps(-2147483520.f);
	      	//__m128 vMax = _mm_set1_ps(2147483520.f);
          //v0 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v0, vMin), vMax));
          //v1 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v1, vMin), vMax));
          v0 = _mm_cvtps_epi32(v.v0);
          v1 = _mm_cvtps_epi32(v.v1);
        }

        inline icl256i& operator=(const __m128i *v) {
          v0 = *v;
          v1 = *(v+1);
          return *this;
        }

        inline icl256i& operator=(const icl16s *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
          v1 = _mm_loadu_si128((__m128i*)(v+8));
          return *this;
        }

        inline icl256i& operator=(const icl32s *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
          v1 = _mm_loadu_si128((__m128i*)(v+4));
          return *this;
        }

        inline icl256i& operator=(const Icl128i &v) {
          const __m128i vk0 = _mm_setzero_si128();
          v0 = _mm_unpacklo_epi8(v.v0, vk0);
          v1 = _mm_unpackhi_epi8(v.v0, vk0);
          return *this;
        }

        inline icl256i& operator=(const icl16s v) {
          v0 = _mm_set1_epi16(v);
          v1 = _mm_set1_epi16(v);
          return *this;
        }

        inline icl256i& operator=(const icl32s v) {
          v0 = _mm_set1_epi32(v);
          v1 = _mm_set1_epi32(v);
          return *this;
        }

        inline icl256i& operator=(const Icl256i &v) {
          v0 = v.v0;
          v1 = v.v1;
          return *this;
        }

        inline icl256i& operator=(const Icl256 &v) {
		      //__m128 vMin = _mm_set1_ps(-2147483520.f);
	      	//__m128 vMax = _mm_set1_ps(2147483520.f);
          //v0 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v0, vMin), vMax));
          //v1 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v1, vMin), vMax));
          v0 = _mm_cvtps_epi32(v.v0);
          v1 = _mm_cvtps_epi32(v.v1);
          return *this;
        }

        inline icl256i& operator&=(const Icl256i &v) {
          v0 = _mm_and_si128(v0, v.v0);
          v1 = _mm_and_si128(v1, v.v1);
          return *this;
        }

        inline icl256i& operator|=(const Icl256i &v) {
          v0 = _mm_or_si128(v0, v.v0);
          v1 = _mm_or_si128(v1, v.v1);
          return *this;
        }

        inline icl256i& operator^=(const Icl256i &v) {
          v0 = _mm_xor_si128(v0, v.v0);
          v1 = _mm_xor_si128(v1, v.v1);
          return *this;
        }

        inline icl256i& andnot(const Icl256i &v) {
          v0 = _mm_andnot_si128(v.v0, v0);
          v1 = _mm_andnot_si128(v.v1, v1);
          return *this;
        }

        inline icl128i pack8s() {
          return icl128i(_mm_packs_epi16(v0, v1));
        }

        inline icl128i pack8u() {
          return icl128i(_mm_packus_epi16(v0, v1));
        }

        inline icl128i pack16s() {
          return icl128i(_mm_packs_epi32(v0, v1));
        }

        inline void add16(const Icl256i &v) {
          v0 = _mm_add_epi16(v0, v.v0);
          v1 = _mm_add_epi16(v1, v.v1);
        }

        inline void add32(const Icl256i &v) {
          v0 = _mm_add_epi32(v0, v.v0);
          v1 = _mm_add_epi32(v1, v.v1);
        }

        inline void store(__m128i *v) const {
		      _mm_store_si128(v,   v0);
		      _mm_store_si128(v+1, v1);
        }

        inline void storeu(__m128i *v) const {
		      _mm_storeu_si128(v,   v0);
		      _mm_storeu_si128(v+1, v1);
        }

        inline void store(icl16s *v) const {
		      _mm_store_si128((__m128i*)v,     v0);
		      _mm_store_si128((__m128i*)(v+8), v1);
        }

        inline void storeu(icl16s *v) const {
		      _mm_storeu_si128((__m128i*)v,     v0);
		      _mm_storeu_si128((__m128i*)(v+8), v1);
        }

        inline void store(icl16u *v) const {
		      _mm_store_si128((__m128i*)v,     v0);
		      _mm_store_si128((__m128i*)(v+8), v1);
        }

        inline void storeu(icl16u *v) const {
		      _mm_storeu_si128((__m128i*)v,     v0);
		      _mm_storeu_si128((__m128i*)(v+8), v1);
        }

        inline void store(icl32s *v) const {
		      _mm_store_si128((__m128i*)v,     v0);
		      _mm_store_si128((__m128i*)(v+4), v1);
        }

        inline void storeu(icl32s *v) const {
		      _mm_storeu_si128((__m128i*)v,     v0);
		      _mm_storeu_si128((__m128i*)(v+4), v1);
        }

        inline void store(icl32u *v) const {
		      _mm_store_si128((__m128i*)v,     v0);
		      _mm_store_si128((__m128i*)(v+4), v1);
        }

        inline void storeu(icl32u *v) const {
		      _mm_storeu_si128((__m128i*)v,     v0);
		      _mm_storeu_si128((__m128i*)(v+4), v1);
        }
      };

      // 16 icl16s
      struct icl256i16s : icl256i {
        inline icl256i16s& operator+=(const icl256i16s &v) {
          v0 = _mm_add_epi16(v0, v.v0);
          v1 = _mm_add_epi16(v1, v.v1);
          return *this;
        }

        inline icl256i16s& operator-=(const icl256i16s &v) {
          v0 = _mm_sub_epi16(v0, v.v0);
          v1 = _mm_sub_epi16(v1, v.v1);
          return *this;
        }
      };

      // 8 icl32
      struct icl256i32s : icl256i {
        inline icl256i32s& operator+=(const icl256i32s &v) {
          v0 = _mm_add_epi16(v0, v.v0);
          v1 = _mm_add_epi16(v1, v.v1);
          return *this;
        }

        inline icl256i32s& operator-=(const icl256i32s &v) {
          v0 = _mm_sub_epi16(v0, v.v0);
          v1 = _mm_sub_epi16(v1, v.v1);
          return *this;
        }
      };

      // 4 icl64f values
      struct icl256d : Icl512d {
        // TODO
      };

      // 16 icl32f values
      struct icl512 : Icl512 {
        inline icl512() {
        }

        inline icl512(const __m128 &vll, const __m128 &vlh,
               const __m128 &vhl, const __m128 &vhh) {
          v0 = vll;
          v1 = vlh;
          v2 = vhl;
          v3 = vhh;
        }

        inline icl512(const __m128 *v) {
          v0 = *v;
          v1 = *(v+1);
          v2 = *(v+2);
          v3 = *(v+3);
        }

        inline icl512(const icl8u *v) {
          const __m128i vk0 = _mm_setzero_si128();
          __m128i vt0, vt1, vt2, vt3;

          vt3 = _mm_loadu_si128((__m128i*)v);

          vt1 = _mm_unpacklo_epi8(vt3, vk0);
          vt3 = _mm_unpackhi_epi8(vt3, vk0);

          vt0 = _mm_unpacklo_epi16(vt1, vk0);
          vt1 = _mm_unpackhi_epi16(vt1, vk0);
          vt2 = _mm_unpacklo_epi16(vt3, vk0);
          vt3 = _mm_unpackhi_epi16(vt3, vk0);

          v0 = _mm_cvtepi32_ps(vt0);
          v1 = _mm_cvtepi32_ps(vt1);
          v2 = _mm_cvtepi32_ps(vt2);
          v3 = _mm_cvtepi32_ps(vt3);
        }

        inline icl512(const icl32f *v) {
          v0 = _mm_loadu_ps(v);
          v1 = _mm_loadu_ps(v+4);
          v2 = _mm_loadu_ps(v+8);
          v3 = _mm_loadu_ps(v+12);
        }

        inline icl512(const Icl512 &v) {
          v0 = v.v0;
          v1 = v.v1;
          v2 = v.v2;
          v3 = v.v3;
        }

        inline icl512(const Icl512i &v) {
          v0 = _mm_cvtepi32_ps(v.v0);
          v1 = _mm_cvtepi32_ps(v.v1);
          v2 = _mm_cvtepi32_ps(v.v2);
          v3 = _mm_cvtepi32_ps(v.v3);
        }

        inline icl512(const icl32f v) {
          v0 = _mm_set1_ps(v);
          v1 = _mm_set1_ps(v);
          v2 = _mm_set1_ps(v);
          v3 = _mm_set1_ps(v);
        }

        inline icl512& operator=(const __m128 *v) {
          v0 = *v;
          v1 = *(v+1);
          v2 = *(v+2);
          v3 = *(v+3);
          return *this;
        }

        inline icl512& operator=(const icl32f *v) {
          v0 = _mm_loadu_ps(v);
          v1 = _mm_loadu_ps(v+4);
          v2 = _mm_loadu_ps(v+8);
          v3 = _mm_loadu_ps(v+12);
          return *this;
        }

        inline icl512& operator=(const Icl512 &v) {
          v0 = v.v0;
          v1 = v.v1;
          v2 = v.v2;
          v3 = v.v3;
          return *this;
        }

        inline icl512& operator=(const Icl512i &v) {
		      v0 = _mm_cvtepi32_ps(v.v0);
		      v1 = _mm_cvtepi32_ps(v.v1);
		      v2 = _mm_cvtepi32_ps(v.v2);
		      v3 = _mm_cvtepi32_ps(v.v3);
          return *this;
        }

        inline icl512& operator=(const icl32f v) {
          v0 = _mm_set1_ps(v);
          v1 = _mm_set1_ps(v);
          v2 = _mm_set1_ps(v);
          v3 = _mm_set1_ps(v);
          return *this;
        }

        inline icl512& operator+=(const Icl512 &v) {
          v0 = _mm_add_ps(v0, v.v0);
          v1 = _mm_add_ps(v1, v.v1);
          v2 = _mm_add_ps(v2, v.v2);
          v3 = _mm_add_ps(v3, v.v3);
          return *this;
        }

        inline icl512& operator-=(const Icl512 &v) {
          v0 = _mm_sub_ps(v0, v.v0);
          v1 = _mm_sub_ps(v1, v.v1);
          v2 = _mm_sub_ps(v2, v.v2);
          v3 = _mm_sub_ps(v3, v.v3);
          return *this;
        }

        inline icl512& operator*=(const Icl512 &v) {
          v0 = _mm_mul_ps(v0, v.v0);
          v1 = _mm_mul_ps(v1, v.v1);
          v2 = _mm_mul_ps(v2, v.v2);
          v3 = _mm_mul_ps(v3, v.v3);
          return *this;
        }

        inline icl512& operator/=(const Icl512 &v) {
          v0 = _mm_div_ps(v0, v.v0);
          v1 = _mm_div_ps(v1, v.v1);
          v2 = _mm_div_ps(v2, v.v2);
          v3 = _mm_div_ps(v3, v.v3);
          return *this;
        }

        inline icl512& operator&=(const Icl512 &v) {
          v0 = _mm_and_ps(v0, v.v0);
          v1 = _mm_and_ps(v1, v.v1);
          v2 = _mm_and_ps(v2, v.v2);
          v3 = _mm_and_ps(v3, v.v3);
          return *this;
        }

        inline icl512& operator|=(const Icl512 &v) {
          v0 = _mm_or_ps(v0, v.v0);
          v1 = _mm_or_ps(v1, v.v1);
          v2 = _mm_or_ps(v2, v.v2);
          v3 = _mm_or_ps(v3, v.v3);
          return *this;
        }

        inline icl512& operator^=(const Icl512 &v) {
          v0 = _mm_xor_ps(v0, v.v0);
          v1 = _mm_xor_ps(v1, v.v1);
          v2 = _mm_xor_ps(v2, v.v2);
          v3 = _mm_xor_ps(v3, v.v3);
          return *this;
        }

        inline icl512& andnot(const Icl512 &v) {
          v0 = _mm_andnot_ps(v.v0, v0);
          v1 = _mm_andnot_ps(v.v1, v1);
          v2 = _mm_andnot_ps(v.v2, v2);
          v3 = _mm_andnot_ps(v.v3, v3);
          return *this;
        }

        inline icl512& rcp() {
		      v0 = _mm_rcp_ps(v0);
		      v1 = _mm_rcp_ps(v1);
		      v2 = _mm_rcp_ps(v2);
		      v3 = _mm_rcp_ps(v3);
          return *this;
        }

        inline void store(icl8u *v) const {
		      //__m128 vMin = _mm_set1_ps(-2147483520.f);
	      	//__m128 vMax = _mm_set1_ps(2147483520.f);
          //v0 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v0, vMin), vMax));
          //v1 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v1, vMin), vMax));
          //v2 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v2, vMin), vMax));
          //v3 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v3, vMin), vMax));
          __m128i vt0 = _mm_cvtps_epi32(v0);
          __m128i vt1 = _mm_cvtps_epi32(v1);
          __m128i vt2 = _mm_cvtps_epi32(v2);
          __m128i vt3 = _mm_cvtps_epi32(v3);

          vt0 = _mm_packus_epi16(_mm_packs_epi32(vt0, vt1), _mm_packs_epi32(vt2, vt3));
          _mm_store_si128((__m128i*)v, vt0);
        }

        inline void storeu(icl8u *v) const {
		      //__m128 vMin = _mm_set1_ps(-2147483520.f);
	      	//__m128 vMax = _mm_set1_ps(2147483520.f);
          //v0 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v0, vMin), vMax));
          //v1 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v1, vMin), vMax));
          //v2 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v2, vMin), vMax));
          //v3 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v3, vMin), vMax));
          __m128i vt0 = _mm_cvtps_epi32(v0);
          __m128i vt1 = _mm_cvtps_epi32(v1);
          __m128i vt2 = _mm_cvtps_epi32(v2);
          __m128i vt3 = _mm_cvtps_epi32(v3);

          vt0 = _mm_packus_epi16(_mm_packs_epi32(vt0, vt1), _mm_packs_epi32(vt2, vt3));
          _mm_storeu_si128((__m128i*)v, vt0);
        }

        inline void store(icl32f *v) const {
		      _mm_store_ps(v,    v0);
		      _mm_store_ps(v+4,  v1);
		      _mm_store_ps(v+8,  v2);
		      _mm_store_ps(v+12, v3);
        }

        inline void storeu(icl32f *v) const {
		      _mm_storeu_ps(v,    v0);
		      _mm_storeu_ps(v+4,  v1);
		      _mm_storeu_ps(v+8,  v2);
		      _mm_storeu_ps(v+12, v3);
        }
      };

      // 16 icl32s values
      struct icl512i : Icl512i {
        inline icl512i() {
        }

        inline icl512i(const __m128i &vll, const __m128i &vlh,
                const __m128i &vhl, const __m128i &vhh) {
          v0 = vll;
          v1 = vlh;
          v2 = vhl;
          v3 = vhh;
        }

        inline icl512i(const __m128i *v) {
          v0 = *v;
          v1 = *(v+1);
          v0 = *(v+2);
          v1 = *(v+3);
        }

        inline icl512i(const icl32s *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
          v1 = _mm_loadu_si128((__m128i*)(v+4));
          v2 = _mm_loadu_si128((__m128i*)(v+8));
          v3 = _mm_loadu_si128((__m128i*)(v+12));
        }

        inline icl512i(const Icl512i &v) {
          v0 = v.v0;
          v1 = v.v1;
          v2 = v.v2;
          v3 = v.v3;
        }

        inline icl512i(const Icl256i &v) {
          const __m128i vk0 = _mm_setzero_si128();
          v0 = _mm_unpacklo_epi16(v.v0, vk0);
          v1 = _mm_unpackhi_epi16(v.v0, vk0);
          v2 = _mm_unpacklo_epi16(v.v1, vk0);
          v3 = _mm_unpackhi_epi16(v.v1, vk0);
        }

        inline icl512i(const icl32s v) {
          v0 = _mm_set1_epi32(v);
          v1 = _mm_set1_epi32(v);
          v2 = _mm_set1_epi32(v);
          v3 = _mm_set1_epi32(v);
        }

        inline icl512i(const Icl512 &v) {
		      //__m128 vMin = _mm_set1_ps(-2147483520.f);
	      	//__m128 vMax = _mm_set1_ps(2147483520.f);
          //v0 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v0, vMin), vMax));
          //v1 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v1, vMin), vMax));
          //v2 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v2, vMin), vMax));
          //v3 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v3, vMin), vMax));
          v0 = _mm_cvtps_epi32(v.v0);
          v1 = _mm_cvtps_epi32(v.v1);
          v2 = _mm_cvtps_epi32(v.v2);
          v3 = _mm_cvtps_epi32(v.v3);
        }

        inline icl512i& operator=(const __m128i *v) {
          v0 = *v;
          v1 = *(v+1);
          v0 = *(v+2);
          v1 = *(v+3);
          return *this;
        }

        inline icl512i& operator=(const icl32s *v) {
          v0 = _mm_loadu_si128((__m128i*)v);
          v1 = _mm_loadu_si128((__m128i*)(v+4));
          v2 = _mm_loadu_si128((__m128i*)(v+8));
          v3 = _mm_loadu_si128((__m128i*)(v+12));
          return *this;
        }

        inline icl512i& operator=(const Icl512i &v) {
          v0 = v.v0;
          v1 = v.v1;
          v2 = v.v2;
          v3 = v.v3;
          return *this;
        }

        inline icl512i& operator=(const Icl256i &v) {
          const __m128i vk0 = _mm_setzero_si128();
          v0 = _mm_unpacklo_epi16(v.v0, vk0);
          v1 = _mm_unpackhi_epi16(v.v0, vk0);
          v2 = _mm_unpacklo_epi16(v.v1, vk0);
          v3 = _mm_unpackhi_epi16(v.v1, vk0);
          return *this;
        }

        inline icl512i& operator=(const icl32s v) {
          v0 = _mm_set1_epi32(v);
          v1 = _mm_set1_epi32(v);
          v2 = _mm_set1_epi32(v);
          v3 = _mm_set1_epi32(v);
          return *this;
        }

        inline icl512i& operator=(const Icl512 &v) {
		      //__m128 vMin = _mm_set1_ps(-2147483520.f);
	      	//__m128 vMax = _mm_set1_ps(2147483520.f);
          //v0 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v0, vMin), vMax));
          //v1 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v1, vMin), vMax));
          //v2 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v2, vMin), vMax));
          //v3 = _mm_cvtps_epi32(_mm_min_ps(_mm_max_ps(v.v3, vMin), vMax));
          v0 = _mm_cvtps_epi32(v.v0);
          v1 = _mm_cvtps_epi32(v.v1);
          v2 = _mm_cvtps_epi32(v.v2);
          v3 = _mm_cvtps_epi32(v.v3);
          return *this;
        }

        inline icl512i& operator+=(const Icl512i &v) {
          v0 = _mm_add_epi32(v0, v.v0);
          v1 = _mm_add_epi32(v1, v.v1);
          v2 = _mm_add_epi32(v2, v.v2);
          v3 = _mm_add_epi32(v3, v.v3);
          return *this;
        }

        inline icl512i& operator-=(const Icl512i &v) {
          v0 = _mm_sub_epi32(v0, v.v0);
          v1 = _mm_sub_epi32(v1, v.v1);
          v2 = _mm_sub_epi32(v2, v.v2);
          v3 = _mm_sub_epi32(v3, v.v3);
          return *this;
        }

        inline icl512i& operator&=(const Icl512i &v) {
          v0 = _mm_and_si128(v0, v.v0);
          v1 = _mm_and_si128(v1, v.v1);
          v2 = _mm_and_si128(v2, v.v2);
          v3 = _mm_and_si128(v3, v.v3);
          return *this;
        }

        inline icl512i& operator|=(const Icl512i &v) {
          v0 = _mm_or_si128(v0, v.v0);
          v1 = _mm_or_si128(v1, v.v1);
          v2 = _mm_or_si128(v2, v.v2);
          v3 = _mm_or_si128(v3, v.v3);
          return *this;
        }

        inline icl512i& operator^=(const Icl512i &v) {
          v0 = _mm_xor_si128(v0, v.v0);
          v1 = _mm_xor_si128(v1, v.v1);
          v2 = _mm_xor_si128(v2, v.v2);
          v3 = _mm_xor_si128(v3, v.v3);
          return *this;
        }

        inline icl512i& andnot(const Icl512i &v) {
          v0 = _mm_andnot_si128(v.v0, v0);
          v1 = _mm_andnot_si128(v.v1, v1);
          v2 = _mm_andnot_si128(v.v2, v2);
          v3 = _mm_andnot_si128(v.v3, v3);
          return *this;
        }

        inline icl256i pack16s() {
          return icl256i(_mm_packs_epi32(v0, v1), _mm_packs_epi32(v2, v3));
        }

        inline icl128i pack8s() {
          return icl128i(_mm_packs_epi16(_mm_packs_epi32(v0, v1), _mm_packs_epi32(v2, v3)));
        }

        inline icl128i pack8u() {
          return icl128i(_mm_packus_epi16(_mm_packs_epi32(v0, v1), _mm_packs_epi32(v2, v3)));
        }

        // use += instead
        inline void add32(const Icl512i &v) {
          v0 = _mm_add_epi32(v0, v.v0);
          v1 = _mm_add_epi32(v1, v.v1);
          v2 = _mm_add_epi32(v2, v.v2);
          v3 = _mm_add_epi32(v3, v.v3);
        }

        inline void store(__m128i *v) const {
		      _mm_store_si128(v,   v0);
		      _mm_store_si128(v+1, v1);
		      _mm_store_si128(v+2, v2);
		      _mm_store_si128(v+3, v3);
        }

        inline void storeu(__m128i *v) const {
		      _mm_storeu_si128(v,   v0);
		      _mm_storeu_si128(v+1, v1);
		      _mm_storeu_si128(v+2, v2);
		      _mm_storeu_si128(v+3, v3);
        }

        inline void store(icl32s *v) const {
		      _mm_store_si128((__m128i*)v,      v0);
		      _mm_store_si128((__m128i*)(v+4),  v1);
		      _mm_store_si128((__m128i*)(v+8),  v1);
		      _mm_store_si128((__m128i*)(v+12), v1);
        }

        inline void storeu(icl32s *v) const {
		      _mm_storeu_si128((__m128i*)v,      v0);
		      _mm_storeu_si128((__m128i*)(v+4),  v1);
		      _mm_storeu_si128((__m128i*)(v+8),  v1);
		      _mm_storeu_si128((__m128i*)(v+12), v1);
        }

        inline void store(icl32u *v) const {
		      _mm_store_si128((__m128i*)v,      v0);
		      _mm_store_si128((__m128i*)(v+4),  v1);
		      _mm_store_si128((__m128i*)(v+8),  v1);
		      _mm_store_si128((__m128i*)(v+12), v1);
        }

        inline void storeu(icl32u *v) const {
		      _mm_storeu_si128((__m128i*)v,      v0);
		      _mm_storeu_si128((__m128i*)(v+4),  v1);
		      _mm_storeu_si128((__m128i*)(v+8),  v1);
		      _mm_storeu_si128((__m128i*)(v+12), v1);
        }
      };

      // 8 icl32
      struct icl512i32s : icl512i {
        inline icl512i32s& operator+=(const icl512i32s &v) {
          v0 = _mm_add_epi16(v0, v.v0);
          v1 = _mm_add_epi16(v1, v.v1);
          v2 = _mm_add_epi16(v2, v.v2);
          v3 = _mm_add_epi16(v3, v.v3);
          return *this;
        }

        inline icl512i32s& operator-=(const icl512i32s &v) {
          v0 = _mm_sub_epi16(v0, v.v0);
          v1 = _mm_sub_epi16(v1, v.v1);
          v2 = _mm_sub_epi16(v2, v.v2);
          v3 = _mm_sub_epi16(v3, v.v3);
          return *this;
        }
      };

      // 8 icl64f values
      struct icl512d : Icl512d {
        // TODO
      };

      // 16 icl64f values
      struct icl1024d : Icl1024d {
        // TODO
      };

      // -- advanced SSE types -- //


      // ++ operations on SSE types ++ //

      // ++ arithmetic oparations ++ //

      inline icl128 operator+(icl128 lv, const icl128 &rv) {
        return lv += rv;
      }

      inline icl128 operator-(icl128 lv, const icl128 &rv) {
        return lv -= rv;
      }

      inline icl128 operator*(icl128 lv, const icl128 &rv) {
        return lv *= rv;
      }

      inline icl128 operator/(icl128 lv, const icl128 &rv) {
        return lv /= rv;
      }

      inline icl256 operator+(icl256 lv, const icl256 &rv) {
        return lv += rv;
      }

      inline icl256 operator-(icl256 lv, const icl256 &rv) {
        return lv -= rv;
      }

      inline icl256 operator*(icl256 lv, const icl256 &rv) {
        return lv *= rv;
      }

      inline icl256 operator/(icl256 lv, const icl256 &rv) {
        return lv /= rv;
      }

      inline icl512 operator+(icl512 lv, const icl512 &rv) {
        return lv += rv;
      }

      inline icl512 operator-(icl512 lv, const icl512 &rv) {
        return lv -= rv;
      }

      inline icl512 operator*(icl512 lv, const icl512 &rv) {
        return lv *= rv;
      }

      inline icl512 operator/(icl512 lv, const icl512 &rv) {
        return lv /= rv;
      }

      // -- arithmetic oparations -- //

      // ++ comparison oparations ++ //

      inline icl128 operator==(icl128 lv, const icl128 &rv) {
        lv.v0 = _mm_cmpeq_ps(lv.v0, rv.v0);
        return lv;
      }

      inline icl128 operator!=(icl128 lv, const icl128 &rv) {
        lv.v0 = _mm_cmpneq_ps(lv.v0, rv.v0);
        return lv;
      }

      inline icl128 operator<(icl128 lv, const icl128 &rv) {
        lv.v0 = _mm_cmplt_ps(lv.v0, rv.v0);
        return lv;
      }

      inline icl128 operator>(icl128 lv, const icl128 &rv) {
        lv.v0 = _mm_cmpgt_ps(lv.v0, rv.v0);
        return lv;
      }

      inline icl128 operator<=(icl128 lv, const icl128 &rv) {
        lv.v0 = _mm_cmple_ps(lv.v0, rv.v0);
        return lv;
      }

      inline icl128 operator>=(icl128 lv, const icl128 &rv) {
        lv.v0 = _mm_cmpge_ps(lv.v0, rv.v0);
        return lv;
      }

      inline icl256 operator==(icl256 lv, const icl256 &rv) {
        lv.v0 = _mm_cmpeq_ps(lv.v0, rv.v0);
        lv.v1 = _mm_cmpeq_ps(lv.v1, rv.v1);
        return lv;
      }

      inline icl256 operator!=(icl256 lv, const icl256 &rv) {
        lv.v0 = _mm_cmpneq_ps(lv.v0, rv.v0);
        lv.v1 = _mm_cmpneq_ps(lv.v1, rv.v1);
        return lv;
      }

      inline icl256 operator<(icl256 lv, const icl256 &rv) {
        lv.v0 = _mm_cmplt_ps(lv.v0, rv.v0);
        lv.v1 = _mm_cmplt_ps(lv.v1, rv.v1);
        return lv;
      }

      inline icl256 operator>(icl256 lv, const icl256 &rv) {
        lv.v0 = _mm_cmpgt_ps(lv.v0, rv.v0);
        lv.v1 = _mm_cmpgt_ps(lv.v1, rv.v1);
        return lv;
      }

      inline icl256 operator<=(icl256 lv, const icl256 &rv) {
        lv.v0 = _mm_cmple_ps(lv.v0, rv.v0);
        lv.v1 = _mm_cmple_ps(lv.v1, rv.v1);
        return lv;
      }

      inline icl256 operator>=(icl256 lv, const icl256 &rv) {
        lv.v0 = _mm_cmpge_ps(lv.v0, rv.v0);
        lv.v1 = _mm_cmpge_ps(lv.v1, rv.v1);
        return lv;
      }

      inline icl512 operator==(icl512 lv, const icl512 &rv) {
        lv.v0 = _mm_cmpeq_ps(lv.v0, rv.v0);
        lv.v1 = _mm_cmpeq_ps(lv.v1, rv.v1);
        lv.v2 = _mm_cmpeq_ps(lv.v2, rv.v2);
        lv.v3 = _mm_cmpeq_ps(lv.v3, rv.v3);
        return lv;
      }

      inline icl512 operator!=(icl512 lv, const icl512 &rv) {
        lv.v0 = _mm_cmpneq_ps(lv.v0, rv.v0);
        lv.v1 = _mm_cmpneq_ps(lv.v1, rv.v1);
        lv.v2 = _mm_cmpneq_ps(lv.v2, rv.v2);
        lv.v3 = _mm_cmpneq_ps(lv.v3, rv.v3);
        return lv;
      }

      inline icl512 operator<(icl512 lv, const icl512 &rv) {
        lv.v0 = _mm_cmplt_ps(lv.v0, rv.v0);
        lv.v1 = _mm_cmplt_ps(lv.v1, rv.v1);
        lv.v2 = _mm_cmplt_ps(lv.v2, rv.v2);
        lv.v3 = _mm_cmplt_ps(lv.v3, rv.v3);
        return lv;
      }

      inline icl512 operator>(icl512 lv, const icl512 &rv) {
        lv.v0 = _mm_cmpgt_ps(lv.v0, rv.v0);
        lv.v1 = _mm_cmpgt_ps(lv.v1, rv.v1);
        lv.v2 = _mm_cmpgt_ps(lv.v2, rv.v2);
        lv.v3 = _mm_cmpgt_ps(lv.v3, rv.v3);
        return lv;
      }

      inline icl512 operator<=(icl512 lv, const icl512 &rv) {
        lv.v0 = _mm_cmple_ps(lv.v0, rv.v0);
        lv.v1 = _mm_cmple_ps(lv.v1, rv.v1);
        lv.v2 = _mm_cmple_ps(lv.v2, rv.v2);
        lv.v3 = _mm_cmple_ps(lv.v3, rv.v3);
        return lv;
      }

      inline icl512 operator>=(icl512 lv, const icl512 &rv) {
        lv.v0 = _mm_cmpge_ps(lv.v0, rv.v0);
        lv.v1 = _mm_cmpge_ps(lv.v1, rv.v1);
        lv.v2 = _mm_cmpge_ps(lv.v2, rv.v2);
        lv.v3 = _mm_cmpge_ps(lv.v3, rv.v3);
        return lv;
      }

      // -- comparison oparations -- //

      // ++ logical oparations ++ //

      inline icl128 operator&(icl128 lv, const icl128 &rv) {
        lv.v0 = _mm_and_ps(lv.v0, rv.v0);
        return lv;
      }

      inline icl128 operator|(icl128 lv, const icl128 &rv) {
        lv.v0 = _mm_or_ps(lv.v0, rv.v0);
        return lv;
      }

      inline icl128 operator^(icl128 lv, const icl128 &rv) {
        lv.v0 = _mm_xor_ps(lv.v0, rv.v0);
        return lv;
      }

      inline icl128 andnot(icl128 lv, const icl128 &rv) {
        lv.v0 = _mm_andnot_ps(rv.v0, lv.v0);
        return lv;
      }

      inline icl128i operator&(icl128i lv, const icl128i &rv) {
        lv.v0 = _mm_and_si128(lv.v0, rv.v0);
        return lv;
      }

      inline icl128i operator|(icl128i lv, const icl128i &rv) {
        lv.v0 = _mm_or_si128(lv.v0, rv.v0);
        return lv;
      }

      inline icl128i operator^(icl128i lv, const icl128i &rv) {
        lv.v0 = _mm_xor_si128(lv.v0, rv.v0);
        return lv;
      }

      inline icl128i andnot(icl128i lv, const icl128i &rv) {
        lv.v0 = _mm_andnot_si128(rv.v0, lv.v0);
        return lv;
      }

      inline icl256 operator&(icl256 lv, const icl256 &rv) {
        lv.v0 = _mm_and_ps(lv.v0, rv.v0);
        lv.v1 = _mm_and_ps(lv.v1, rv.v1);
        return lv;
      }

      inline icl256 operator|(icl256 lv, const icl256 &rv) {
        lv.v0 = _mm_or_ps(lv.v0, rv.v0);
        lv.v1 = _mm_or_ps(lv.v1, rv.v1);
        return lv;
      }

      inline icl256 operator^(icl256 lv, const icl256 &rv) {
        lv.v0 = _mm_xor_ps(lv.v0, rv.v0);
        lv.v1 = _mm_xor_ps(lv.v1, rv.v1);
        return lv;
      }

      inline icl256 andnot(icl256 lv, const icl256 &rv) {
        lv.v0 = _mm_andnot_ps(rv.v0, lv.v0);
        lv.v1 = _mm_andnot_ps(rv.v1, lv.v1);
        return lv;
      }

      inline icl256i operator&(icl256i lv, const icl256i &rv) {
        lv.v0 = _mm_and_si128(lv.v0, rv.v0);
        lv.v1 = _mm_and_si128(lv.v1, rv.v1);
        return lv;
      }

      inline icl256i operator|(icl256i lv, const icl256i &rv) {
        lv.v0 = _mm_or_si128(lv.v0, rv.v0);
        lv.v1 = _mm_or_si128(lv.v1, rv.v1);
        return lv;
      }

      inline icl256i operator^(icl256i lv, const icl256i &rv) {
        lv.v0 = _mm_xor_si128(lv.v0, rv.v0);
        lv.v1 = _mm_xor_si128(lv.v1, rv.v1);
        return lv;
      }

      inline icl256i andnot(icl256i lv, const icl256i &rv) {
        lv.v0 = _mm_andnot_si128(rv.v0, lv.v0);
        lv.v1 = _mm_andnot_si128(rv.v1, lv.v1);
        return lv;
      }

      inline icl512 operator&(icl512 lv, const icl512 &rv) {
        lv.v0 = _mm_and_ps(lv.v0, rv.v0);
        lv.v1 = _mm_and_ps(lv.v1, rv.v1);
        lv.v2 = _mm_and_ps(lv.v2, rv.v2);
        lv.v3 = _mm_and_ps(lv.v3, rv.v3);
        return lv;
      }

      inline icl512 operator|(icl512 lv, const icl512 &rv) {
        lv.v0 = _mm_or_ps(lv.v0, rv.v0);
        lv.v1 = _mm_or_ps(lv.v1, rv.v1);
        lv.v2 = _mm_or_ps(lv.v2, rv.v2);
        lv.v3 = _mm_or_ps(lv.v3, rv.v3);
        return lv;
      }

      inline icl512 operator^(icl512 lv, const icl512 &rv) {
        lv.v0 = _mm_xor_ps(lv.v0, rv.v0);
        lv.v1 = _mm_xor_ps(lv.v1, rv.v1);
        lv.v2 = _mm_xor_ps(lv.v2, rv.v2);
        lv.v3 = _mm_xor_ps(lv.v3, rv.v3);
        return lv;
      }

      inline icl512 andnot(icl512 lv, const icl512 &rv) {
        lv.v0 = _mm_andnot_ps(rv.v0, lv.v0);
        lv.v1 = _mm_andnot_ps(rv.v1, lv.v1);
        lv.v2 = _mm_andnot_ps(rv.v2, lv.v2);
        lv.v3 = _mm_andnot_ps(rv.v3, lv.v3);
        return lv;
      }

      inline icl512i operator&(icl512i lv, const icl512i &rv) {
        lv.v0 = _mm_and_si128(lv.v0, rv.v0);
        lv.v1 = _mm_and_si128(lv.v1, rv.v1);
        lv.v2 = _mm_and_si128(lv.v2, rv.v2);
        lv.v3 = _mm_and_si128(lv.v3, rv.v3);
        return lv;
      }

      inline icl512i operator|(icl512i lv, const icl512i &rv) {
        lv.v0 = _mm_or_si128(lv.v0, rv.v0);
        lv.v1 = _mm_or_si128(lv.v1, rv.v1);
        lv.v2 = _mm_or_si128(lv.v2, rv.v2);
        lv.v3 = _mm_or_si128(lv.v3, rv.v3);
        return lv;
      }

      inline icl512i operator^(icl512i lv, const icl512i &rv) {
        lv.v0 = _mm_xor_si128(lv.v0, rv.v0);
        lv.v1 = _mm_xor_si128(lv.v1, rv.v1);
        lv.v2 = _mm_xor_si128(lv.v2, rv.v2);
        lv.v3 = _mm_xor_si128(lv.v3, rv.v3);
        return lv;
      }

      inline icl512i andnot(icl512i lv, const icl512i &rv) {
        lv.v0 = _mm_andnot_si128(rv.v0, lv.v0);
        lv.v1 = _mm_andnot_si128(rv.v1, lv.v1);
        lv.v2 = _mm_andnot_si128(rv.v2, lv.v2);
        lv.v3 = _mm_andnot_si128(rv.v3, lv.v3);
        return lv;
      }

      // -- logical oparations -- //

      // ++ shift operations ++ //

      inline icl128i& operator<<(icl128i &v, const int i) {
        v.v0 = _mm_slli_epi32(v.v0, i);
        return v;
      }

      inline icl128i& operator>>(icl128i &v, const int i) {
        v.v0 = _mm_srai_epi32(v.v0, i);
        return v;
      }

      inline icl256i& operator<<(icl256i &v, const int i) {
        v.v0 = _mm_slli_epi32(v.v0, i);
        v.v1 = _mm_slli_epi32(v.v1, i);
        return v;
      }

      inline icl256i& operator>>(icl256i &v, const int i) {
        v.v0 = _mm_srai_epi32(v.v0, i);
        v.v1 = _mm_srai_epi32(v.v1, i);
        return v;
      }

      inline icl512i& operator<<(icl512i &v, const int i) {
        v.v0 = _mm_slli_epi32(v.v0, i);
        v.v1 = _mm_slli_epi32(v.v1, i);
        v.v2 = _mm_slli_epi32(v.v2, i);
        v.v3 = _mm_slli_epi32(v.v3, i);
        return v;
      }

      inline icl512i& operator>>(icl512i &v, const int i) {
        v.v0 = _mm_srai_epi32(v.v0, i);
        v.v1 = _mm_srai_epi32(v.v1, i);
        v.v2 = _mm_srai_epi32(v.v2, i);
        v.v3 = _mm_srai_epi32(v.v3, i);
        return v;
      }

      // -- shift operations -- //

      // ++ min-max operations ++ //

      inline icl128i8u min(icl128i8u lv, const icl128i8u &rv) {
        lv.v0 = _mm_min_epu8(lv.v0, rv.v0);
        return lv;
      }

      inline icl128i8u max(icl128i8u lv, const icl128i8u &rv) {
        lv.v0 = _mm_max_epu8(lv.v0, rv.v0);
        return lv;
      }

      inline icl128i16s min(icl128i16s lv, const icl128i16s &rv) {
        lv.v0 = _mm_min_epi16(lv.v0, rv.v0);
        return lv;
      }

      inline icl128i16s max(icl128i16s lv, const icl128i16s &rv) {
        lv.v0 = _mm_max_epi16(lv.v0, rv.v0);
        return lv;
      }

      inline icl256i16s min(icl256i16s lv, const icl256i16s &rv) {
        lv.v0 = _mm_min_epi16(lv.v0, rv.v0);
        lv.v1 = _mm_min_epi16(lv.v1, rv.v1);
        return lv;
      }

      inline icl256i16s max(icl256i16s lv, const icl256i16s &rv) {
        lv.v0 = _mm_max_epi16(lv.v0, rv.v0);
        lv.v1 = _mm_max_epi16(lv.v1, rv.v1);
        return lv;
      }

      inline icl128 min(icl128 lv, const icl128 &rv) {
        lv.v0 = _mm_min_ps(lv.v0, rv.v0);
        return lv;
      }

      inline icl128 max(icl128 lv, const icl128 &rv) {
        lv.v0 = _mm_max_ps(lv.v0, rv.v0);
        return lv;
      }

      inline icl256 min(icl256 lv, const icl256 &rv) {
        lv.v0 = _mm_min_ps(lv.v0, rv.v0);
        lv.v1 = _mm_min_ps(lv.v1, rv.v1);
        return lv;
      }

      inline icl256 max(icl256 lv, const icl256 &rv) {
        lv.v0 = _mm_max_ps(lv.v0, rv.v0);
        lv.v1 = _mm_max_ps(lv.v1, rv.v1);
        return lv;
      }

      inline icl512 min(icl512 lv, const icl512 &rv) {
        lv.v0 = _mm_min_ps(lv.v0, rv.v0);
        lv.v1 = _mm_min_ps(lv.v1, rv.v1);
        lv.v2 = _mm_min_ps(lv.v2, rv.v2);
        lv.v3 = _mm_min_ps(lv.v3, rv.v3);
        return lv;
      }

      inline icl512 max(icl512 lv, const icl512 &rv) {
        lv.v0 = _mm_max_ps(lv.v0, rv.v0);
        lv.v1 = _mm_max_ps(lv.v1, rv.v1);
        lv.v2 = _mm_max_ps(lv.v2, rv.v2);
        lv.v3 = _mm_max_ps(lv.v3, rv.v3);
        return lv;
      }

      // -- min-max operations -- //


      // ++ absosulte values ++ //

    #ifdef HAVE_SSE3

      inline icl128i abs8(icl128i v) {
        v.v0 = _mm_abs_epi8(v.v0);
        return v;
      }

      inline icl128i abs16(icl128i v) {
        v.v0 = _mm_abs_epi16(v.v0);
        return v;
      }

      inline icl128i abs32(icl128i v) {
        v.v0 = _mm_abs_epi32(v.v0);
        return v;
      }

      inline icl256i abs16(icl256i v) {
        v.v0 = _mm_abs_epi16(v.v0);
        v.v1 = _mm_abs_epi16(v.v1);
        return v;
      }

      inline icl256i abs32(icl256i v) {
        v.v0 = _mm_abs_epi32(v.v0);
        v.v1 = _mm_abs_epi32(v.v1);
        return v;
      }

      inline icl512i abs32(icl512i v) {
        v.v0 = _mm_abs_epi32(v.v0);
        v.v1 = _mm_abs_epi32(v.v1);
        v.v2 = _mm_abs_epi32(v.v2);
        v.v3 = _mm_abs_epi32(v.v3);
        return v;
      }

    #else

      // TODO: without SSE3

    #endif

      inline icl128 abs(icl128 v) {
        return v.andnot(icl128(-0.0f));
      }

      inline icl256 abs(icl256 v) {
        return v.andnot(icl256(-0.0f));
      }

      inline icl512 abs(icl512 v) {
        return v.andnot(icl512(-0.0f));
      }

      // -- absosulte values -- //


      // ++ squared root ++ //

      inline icl128 sqrt(const icl128 &v) {
        icl128 r;
        r.v0 = _mm_sqrt_ps(v.v0);
        return r;
      }

      inline icl256 sqrt(const icl256 &v) {
        icl256 r;
        r.v0 = _mm_sqrt_ps(v.v0);
        r.v1 = _mm_sqrt_ps(v.v1);
        return r;
      }

      inline icl512 sqrt(const icl512 &v) {
        icl512 r;
        r.v0 = _mm_sqrt_ps(v.v0);
        r.v1 = _mm_sqrt_ps(v.v1);
        r.v2 = _mm_sqrt_ps(v.v2);
        r.v3 = _mm_sqrt_ps(v.v3);
        return r;
      }

      inline icl128d sqrt(const icl128d &v) {
        icl128d r;
        r.v0 = _mm_sqrt_pd(v.v0);
        return r;
      }

      inline icl256d sqrt(const icl256d &v) {
        icl256d r;
        r.v0 = _mm_sqrt_pd(v.v0);
        r.v1 = _mm_sqrt_pd(v.v1);
        return r;
      }

      inline icl512d sqrt(const icl512d &v) {
        icl512d r;
        r.v0 = _mm_sqrt_pd(v.v0);
        r.v1 = _mm_sqrt_pd(v.v1);
        r.v2 = _mm_sqrt_pd(v.v2);
        r.v3 = _mm_sqrt_pd(v.v3);
        return r;
      }

      inline icl1024d sqrt(const icl1024d &v) {
        icl1024d r;
        r.v0 = _mm_sqrt_pd(v.v0);
        r.v1 = _mm_sqrt_pd(v.v1);
        r.v2 = _mm_sqrt_pd(v.v2);
        r.v3 = _mm_sqrt_pd(v.v3);
        r.v4 = _mm_sqrt_pd(v.v4);
        r.v5 = _mm_sqrt_pd(v.v5);
        r.v6 = _mm_sqrt_pd(v.v6);
        r.v7 = _mm_sqrt_pd(v.v7);
        return r;
      }

      // -- squared root -- //


      // ++ cube root ++ //

      inline icl128 cbrt(const icl128 &v) {
        icl128i tmp = icl128i(_mm_castps_si128(v));
        tmp = tmp / icl128i(3) + icl128i(709921077);
        icl128 a = icl128(_mm_castsi128_ps(tmp));
        icl128 a3 = a * a * a;
        return a * (a3 + v + v) * (a3 + a3 + v).rcp();
      }

      inline icl256 cbrt(const icl256 &v) {
        __m128i t0 = _mm_castps_si128(v.v0);
        __m128i t1 = _mm_castps_si128(v.v1);
        icl256i tmp = icl256i(t0, t1);
        tmp = tmp / icl256i(3) + icl256i(709921077);
        icl256 a = icl256(_mm_castsi128_ps(tmp.v0),
                          _mm_castsi128_ps(tmp.v1));
        icl256 a3 = a * a * a;
        return a * (a3 + v + v) * (a3 + a3 + v).rcp();
      }

      inline icl512 cbrt(const icl512 &v) {
        __m128i t0 = _mm_castps_si128(v.v0);
        __m128i t1 = _mm_castps_si128(v.v1);
        __m128i t2 = _mm_castps_si128(v.v2);
        __m128i t3 = _mm_castps_si128(v.v3);
        icl512i tmp = icl512i(t0, t1, t2, t3);
        tmp = tmp / icl512i(3) + icl512i(709921077);
        icl512 a = icl512(_mm_castsi128_ps(tmp.v0),
                          _mm_castsi128_ps(tmp.v1),
                          _mm_castsi128_ps(tmp.v2),
                          _mm_castsi128_ps(tmp.v3));
        icl512 a3 = a * a * a;
        return a * (a3 + v + v) * (a3 + a3 + v).rcp();
      }

      // -- cube root -- //

      // -- operations on SSE types -- //

    #endif

  } // namespace utils
}
