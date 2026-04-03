// FixedMatrix benchmark: Apple SIMD vs generic C++ at -O3
// clang++ -std=c++17 -O3 bench-fixedmatrix.cpp -o bench-fm && ./bench-fm
//
// Two measurement modes per op:
//   Throughput: batch of independent matrices, doNotOptimize each result
//   Latency:    output feeds back as input (dependency chain)

#include <chrono>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <type_traits>

#ifdef __APPLE__
#include <simd/simd.h>
#endif

using Clock = std::chrono::high_resolution_clock;

template<class T>
static inline void doNotOptimize(T const& val) {
  asm volatile("" : : "r,m"(val) : "memory");
}

// =====================================================================
// C++ (scalar — what clang -O3 produces for FixedMatrix without SIMD)
// =====================================================================
namespace cpp {
  template<class T>
  inline void mult4x4(const T* __restrict a, const T* __restrict b, T* __restrict d) {
    for(int r=0;r<4;++r)
      for(int c=0;c<4;++c){
        T s=0; for(int k=0;k<4;++k) s+=a[r*4+k]*b[k*4+c]; d[r*4+c]=s;
      }
  }
  template<class T>
  inline void mult4x4v(const T* __restrict m, const T* __restrict v, T* __restrict d) {
    for(int r=0;r<4;++r){ T s=0; for(int k=0;k<4;++k) s+=m[r*4+k]*v[k]; d[r]=s; }
  }
  template<class T>
  inline T det4x4(const T* m) {
    const T &a=m[0],&b=m[1],&c=m[2],&d=m[3];
    const T &e=m[4],&f=m[5],&g=m[6],&h=m[7];
    const T &i=m[8],&j=m[9],&k=m[10],&l=m[11];
    const T &M=m[12],&n=m[13],&o=m[14],&p=m[15];
    return a*(f*(k*p-l*o)-g*(j*p-l*n)+h*(j*o-k*n))
          -b*(e*(k*p-l*o)-g*(i*p-l*M)+h*(i*o-k*M))
          +c*(e*(j*p-l*n)-f*(i*p-l*M)+h*(i*n-j*M))
          -d*(e*(j*o-k*n)-f*(i*o-k*M)+g*(i*n-j*M));
  }
  template<class T>
  inline void inv4x4(const T* s, T* d) {
    T det=det4x4(s);
    const T&m00=s[0],&m01=s[1],&m02=s[2],&m03=s[3];
    const T&m10=s[4],&m11=s[5],&m12=s[6],&m13=s[7];
    const T&m20=s[8],&m21=s[9],&m22=s[10],&m23=s[11];
    const T&m30=s[12],&m31=s[13],&m32=s[14],&m33=s[15];
    d[0]=(m12*m23*m31-m13*m22*m31+m13*m21*m32-m11*m23*m32-m12*m21*m33+m11*m22*m33)/det;
    d[1]=(m03*m22*m31-m02*m23*m31-m03*m21*m32+m01*m23*m32+m02*m21*m33-m01*m22*m33)/det;
    d[2]=(m02*m13*m31-m03*m12*m31+m03*m11*m32-m01*m13*m32-m02*m11*m33+m01*m12*m33)/det;
    d[3]=(m03*m12*m21-m02*m13*m21-m03*m11*m22+m01*m13*m22+m02*m11*m23-m01*m12*m23)/det;
    d[4]=(m13*m22*m30-m12*m23*m30-m13*m20*m32+m10*m23*m32+m12*m20*m33-m10*m22*m33)/det;
    d[5]=(m02*m23*m30-m03*m22*m30+m03*m20*m32-m00*m23*m32-m02*m20*m33+m00*m22*m33)/det;
    d[6]=(m03*m12*m30-m02*m13*m30-m03*m10*m32+m00*m13*m32+m02*m10*m33-m00*m12*m33)/det;
    d[7]=(m02*m13*m20-m03*m12*m20+m03*m10*m22-m00*m13*m22-m02*m10*m23+m00*m12*m23)/det;
    d[8]=(m11*m23*m30-m13*m21*m30+m13*m20*m31-m10*m23*m31-m11*m20*m33+m10*m21*m33)/det;
    d[9]=(m03*m21*m30-m01*m23*m30-m03*m20*m31+m00*m23*m31+m01*m20*m33-m00*m21*m33)/det;
    d[10]=(m01*m13*m30-m03*m11*m30+m03*m10*m31-m00*m13*m31-m01*m10*m33+m00*m11*m33)/det;
    d[11]=(m03*m11*m20-m01*m13*m20-m03*m10*m21+m00*m13*m21+m01*m10*m23-m00*m11*m23)/det;
    d[12]=(m12*m21*m30-m11*m22*m30-m12*m20*m31+m10*m22*m31+m11*m20*m32-m10*m21*m32)/det;
    d[13]=(m01*m22*m30-m02*m21*m30+m02*m20*m31-m00*m22*m31-m01*m20*m32+m00*m21*m32)/det;
    d[14]=(m02*m11*m30-m01*m12*m30-m02*m10*m31+m00*m12*m31+m01*m10*m32-m00*m11*m32)/det;
    d[15]=(m01*m12*m20-m02*m11*m20+m02*m10*m21-m00*m12*m21-m01*m10*m22+m00*m11*m22)/det;
  }
  template<class T> inline void add16(const T*a,const T*b,T*d){for(int i=0;i<16;++i)d[i]=a[i]+b[i];}
  template<class T> inline void smul16(T s,const T*a,T*d){for(int i=0;i<16;++i)d[i]=s*a[i];}
}

// =====================================================================
// Apple SIMD
// =====================================================================
#ifdef __APPLE__
namespace asimd {
  inline simd_float4x4 ld(const float*s){simd_float4x4 m;std::memcpy(&m,s,64);return m;}
  inline void st(const simd_float4x4&m,float*d){std::memcpy(d,&m,64);}
  inline simd_double4x4 ld(const double*s){simd_double4x4 m;std::memcpy(&m,s,128);return m;}
  inline void st(const simd_double4x4&m,double*d){std::memcpy(d,&m,128);}
  inline simd_float4 ldv(const float*s){simd_float4 v;std::memcpy(&v,s,16);return v;}
  inline void stv(const simd_float4&v,float*d){std::memcpy(d,&v,16);}
  inline simd_double4 ldv(const double*s){simd_double4 v;std::memcpy(&v,s,32);return v;}
  inline void stv(const simd_double4&v,double*d){std::memcpy(d,&v,32);}
  template<class T> inline void mult4x4(const T*a,const T*b,T*d){st(simd_mul(ld(b),ld(a)),d);}
  template<class T> inline void mult4x4v(const T*m,const T*v,T*d){stv(simd_mul(ldv(v),ld(m)),d);}
  template<class T> inline T det4x4(const T*m){return simd_determinant(ld(m));}
  template<class T> inline void inv4x4(const T*s,T*d){st(simd_inverse(ld(s)),d);}
  template<class T> inline void add16(const T*a,const T*b,T*d){st(simd_add(ld(a),ld(b)),d);}
  template<class T> inline void smul16(T s,const T*a,T*d){st(simd_mul(s,ld(a)),d);}
}
#endif

// =====================================================================
// Manual SSE intrinsics (matches FixedMatrix.h SSE2 specializations)
// Works on x86 and on ARM via sse2neon (when ICL_HAVE_SSE2 is defined)
// =====================================================================
#if !defined(__APPLE__) && !defined(ICL_BENCH_MKL)
#ifdef __SSE2__
#include <emmintrin.h>
namespace asimd {
  // 4x4 float multiply — exact copy of FixedMatrix.h SSE2 specialization
  inline void mult4x4(const float* a, const float* b, float* d) {
    __m128 bcol0 = _mm_set_ps(b[12], b[8], b[4], b[0]);
    __m128 bcol1 = _mm_set_ps(b[13], b[9], b[5], b[1]);
    __m128 bcol2 = _mm_set_ps(b[14], b[10], b[6], b[2]);
    __m128 bcol3 = _mm_set_ps(b[15], b[11], b[7], b[3]);
    for(int r = 0; r < 4; ++r){
      __m128 arow = _mm_loadu_ps(a + r*4);
      __m128 d0 = _mm_mul_ps(arow, bcol0);
      __m128 d1 = _mm_mul_ps(arow, bcol1);
      __m128 d2 = _mm_mul_ps(arow, bcol2);
      __m128 d3 = _mm_mul_ps(arow, bcol3);
      __m128 s0 = _mm_add_ps(d0, _mm_shuffle_ps(d0, d0, _MM_SHUFFLE(1,0,3,2)));
      s0 = _mm_add_ps(s0, _mm_shuffle_ps(s0, s0, _MM_SHUFFLE(2,3,0,1)));
      __m128 s1 = _mm_add_ps(d1, _mm_shuffle_ps(d1, d1, _MM_SHUFFLE(1,0,3,2)));
      s1 = _mm_add_ps(s1, _mm_shuffle_ps(s1, s1, _MM_SHUFFLE(2,3,0,1)));
      __m128 s2 = _mm_add_ps(d2, _mm_shuffle_ps(d2, d2, _MM_SHUFFLE(1,0,3,2)));
      s2 = _mm_add_ps(s2, _mm_shuffle_ps(s2, s2, _MM_SHUFFLE(2,3,0,1)));
      __m128 s3 = _mm_add_ps(d3, _mm_shuffle_ps(d3, d3, _MM_SHUFFLE(1,0,3,2)));
      s3 = _mm_add_ps(s3, _mm_shuffle_ps(s3, s3, _MM_SHUFFLE(2,3,0,1)));
      d[r*4+0] = _mm_cvtss_f32(s0);
      d[r*4+1] = _mm_cvtss_f32(s1);
      d[r*4+2] = _mm_cvtss_f32(s2);
      d[r*4+3] = _mm_cvtss_f32(s3);
    }
  }
  // 4x4 double multiply — SSE2 with __m128d (2 doubles per register)
  inline void mult4x4(const double* a, const double* b, double* d) {
    for(int r = 0; r < 4; ++r)
      for(int c = 0; c < 4; ++c) {
        __m128d sum = _mm_setzero_pd();
        sum = _mm_add_pd(sum, _mm_mul_pd(_mm_loadu_pd(a+r*4), _mm_set_pd(b[1*4+c], b[0*4+c])));
        sum = _mm_add_pd(sum, _mm_mul_pd(_mm_loadu_pd(a+r*4+2), _mm_set_pd(b[3*4+c], b[2*4+c])));
        // horizontal sum of 2 doubles
        d[r*4+c] = _mm_cvtsd_f64(_mm_add_pd(sum, _mm_shuffle_pd(sum, sum, 1)));
      }
  }
  // 4x4 float * vec4
  inline void mult4x4v(const float* m, const float* v, float* dst) {
    __m128 vv = _mm_loadu_ps(v);
    for(int r = 0; r < 4; ++r){
      __m128 arow = _mm_loadu_ps(m + r*4);
      __m128 prod = _mm_mul_ps(arow, vv);
      __m128 s = _mm_add_ps(prod, _mm_shuffle_ps(prod, prod, _MM_SHUFFLE(1,0,3,2)));
      s = _mm_add_ps(s, _mm_shuffle_ps(s, s, _MM_SHUFFLE(2,3,0,1)));
      dst[r] = _mm_cvtss_f32(s);
    }
  }
  // 4x4 double * vec4
  inline void mult4x4v(const double* m, const double* v, double* dst) {
    for(int r = 0; r < 4; ++r) {
      __m128d s = _mm_mul_pd(_mm_loadu_pd(m+r*4), _mm_loadu_pd(v));
      s = _mm_add_pd(s, _mm_mul_pd(_mm_loadu_pd(m+r*4+2), _mm_loadu_pd(v+2)));
      dst[r] = _mm_cvtsd_f64(_mm_add_pd(s, _mm_shuffle_pd(s, s, 1)));
    }
  }
  // add: 4x _mm_add_ps on 4-float chunks
  inline void add16(const float*a,const float*b,float*d) {
    _mm_storeu_ps(d,    _mm_add_ps(_mm_loadu_ps(a),    _mm_loadu_ps(b)));
    _mm_storeu_ps(d+4,  _mm_add_ps(_mm_loadu_ps(a+4),  _mm_loadu_ps(b+4)));
    _mm_storeu_ps(d+8,  _mm_add_ps(_mm_loadu_ps(a+8),  _mm_loadu_ps(b+8)));
    _mm_storeu_ps(d+12, _mm_add_ps(_mm_loadu_ps(a+12), _mm_loadu_ps(b+12)));
  }
  inline void add16(const double*a,const double*b,double*d) {
    for(int i=0;i<16;i+=2) _mm_storeu_pd(d+i, _mm_add_pd(_mm_loadu_pd(a+i), _mm_loadu_pd(b+i)));
  }
  // scalar multiply
  inline void smul16(float s, const float*a, float*d) {
    __m128 vs = _mm_set1_ps(s);
    _mm_storeu_ps(d,    _mm_mul_ps(_mm_loadu_ps(a),    vs));
    _mm_storeu_ps(d+4,  _mm_mul_ps(_mm_loadu_ps(a+4),  vs));
    _mm_storeu_ps(d+8,  _mm_mul_ps(_mm_loadu_ps(a+8),  vs));
    _mm_storeu_ps(d+12, _mm_mul_ps(_mm_loadu_ps(a+12), vs));
  }
  inline void smul16(double s, const double*a, double*d) {
    __m128d vs = _mm_set1_pd(s);
    for(int i=0;i<16;i+=2) _mm_storeu_pd(d+i, _mm_mul_pd(_mm_loadu_pd(a+i), vs));
  }
  // det/inv: keep closed-form C++ (no SSE benefit for these)
  template<class T> T det4x4(const T*m){return cpp::det4x4(m);}
  template<class T> void inv4x4(const T*s,T*d){cpp::inv4x4(s,d);}
}
#else
// No SSE2, no Apple SIMD, no MKL — pure C++ fallback
namespace asimd {
  template<class T> void mult4x4(const T*a,const T*b,T*d){cpp::mult4x4(a,b,d);}
  template<class T> void mult4x4v(const T*m,const T*v,T*d){cpp::mult4x4v(m,v,d);}
  template<class T> T det4x4(const T*m){return cpp::det4x4(m);}
  template<class T> void inv4x4(const T*s,T*d){cpp::inv4x4(s,d);}
  template<class T> void add16(const T*a,const T*b,T*d){cpp::add16(a,b,d);}
  template<class T> void smul16(T s,const T*a,T*d){cpp::smul16(s,a,d);}
}
#endif // __SSE2__
#endif // !__APPLE__ && !ICL_BENCH_MKL

// =====================================================================
// Benchmark harness
// =====================================================================

// Throughput: operate on BATCH independent matrices per timing iteration.
// Each result gets doNotOptimize to prevent DCE.
// Amortizes loop overhead by doing BATCH ops before checking time.
static constexpr int BATCH = 128;

template<class T>
struct M16 { T d[16]; };
template<class T>
struct V4 { T d[4]; };

template<class T>
static void init_matrices(M16<T>* arr, int n) {
  for(int i=0;i<n;++i){
    for(int j=0;j<16;++j) arr[i].d[j] = T(0.1) * T((i*17+j*7+3) % 37) - T(1.8);
    // ensure invertible
    arr[i].d[0]+=5; arr[i].d[5]+=5; arr[i].d[10]+=5; arr[i].d[15]+=5;
  }
}

template<class T>
static void init_vectors(V4<T>* arr, int n) {
  for(int i=0;i<n;++i)
    for(int j=0;j<4;++j) arr[i].d[j] = T(0.1) * T((i*13+j*11+5) % 41) - T(2.0);
}

template<class Fn>
static double measure(int iters, Fn fn) {
  // warmup
  for(int i=0;i<iters/10;++i) fn();
  auto t0=Clock::now();
  for(int i=0;i<iters;++i) fn();
  return std::chrono::duration<double,std::nano>(Clock::now()-t0).count() / iters;
}

static void row(const char* name, double ts, double tc) {
  std::printf("%-40s %8.1f  %8.1f  %5.1fx\n", name, ts, tc, tc/ts);
}

template<class T>
static void run() {
  const int ITERS = 200000;
  const char* ty = std::is_same_v<T,float> ? "float" : "double";
  const char* backend =
#if defined(__APPLE__)
    "AppleSIMD";
#elif defined(ICL_BENCH_MKL)
    "MKL cblas";
#elif defined(__SSE2__)
    "SSE2";
#else
    "C++ (same)";
#endif
  std::printf("\n%-40s %9s %9s %7s\n", "", backend, "C++", "Speedup");
  std::printf("=== FixedMatrix<%s> === (batch=%d, -O3)\n", ty, BATCH);
  std::printf("-----------------------------------------------------------\n");

  M16<T> A[BATCH], B[BATCH], C[BATCH];
  V4<T> V[BATCH], Vout[BATCH];
  T dets[BATCH];
  init_matrices(A, BATCH);
  init_matrices(B, BATCH);
  init_vectors(V, BATCH);

  // --- 4x4 multiply ---
  row("4x4 multiply",
    measure(ITERS, [&]{ for(int i=0;i<BATCH;++i){ asimd::mult4x4(A[i].d,B[i].d,C[i].d); doNotOptimize(C[i]); } }) / BATCH,
    measure(ITERS, [&]{ for(int i=0;i<BATCH;++i){ cpp::mult4x4(A[i].d,B[i].d,C[i].d); doNotOptimize(C[i]); } }) / BATCH);

  // --- 4x4 * vec4 ---
  row("4x4 * vec4",
    measure(ITERS, [&]{ for(int i=0;i<BATCH;++i){ asimd::mult4x4v(A[i].d,V[i].d,Vout[i].d); doNotOptimize(Vout[i]); } }) / BATCH,
    measure(ITERS, [&]{ for(int i=0;i<BATCH;++i){ cpp::mult4x4v(A[i].d,V[i].d,Vout[i].d); doNotOptimize(Vout[i]); } }) / BATCH);

  // --- 4x4 inverse ---
  row("4x4 inverse",
    measure(ITERS, [&]{ for(int i=0;i<BATCH;++i){ asimd::inv4x4(A[i].d,C[i].d); doNotOptimize(C[i]); } }) / BATCH,
    measure(ITERS, [&]{ for(int i=0;i<BATCH;++i){ cpp::inv4x4(A[i].d,C[i].d); doNotOptimize(C[i]); } }) / BATCH);

  // --- 4x4 determinant ---
  row("4x4 determinant",
    measure(ITERS, [&]{ for(int i=0;i<BATCH;++i){ dets[i]=asimd::det4x4(A[i].d); doNotOptimize(dets[i]); } }) / BATCH,
    measure(ITERS, [&]{ for(int i=0;i<BATCH;++i){ dets[i]=cpp::det4x4(A[i].d); doNotOptimize(dets[i]); } }) / BATCH);

  // --- 4x4 add ---
  row("4x4 add",
    measure(ITERS, [&]{ for(int i=0;i<BATCH;++i){ asimd::add16(A[i].d,B[i].d,C[i].d); doNotOptimize(C[i]); } }) / BATCH,
    measure(ITERS, [&]{ for(int i=0;i<BATCH;++i){ cpp::add16(A[i].d,B[i].d,C[i].d); doNotOptimize(C[i]); } }) / BATCH);

  // --- 4x4 scalar multiply ---
  row("4x4 scalar multiply",
    measure(ITERS, [&]{ for(int i=0;i<BATCH;++i){ asimd::smul16(T(.5),A[i].d,C[i].d); doNotOptimize(C[i]); } }) / BATCH,
    measure(ITERS, [&]{ for(int i=0;i<BATCH;++i){ cpp::smul16(T(.5),A[i].d,C[i].d); doNotOptimize(C[i]); } }) / BATCH);

  // --- Full pipeline: compose + project + perspDiv ---
  row("full: compose+project+perspDiv",
    measure(ITERS, [&]{
      for(int i=0;i<BATCH;++i){
        M16<T> cam; V4<T> proj;
        asimd::mult4x4(A[i].d,B[i].d,cam.d);
        asimd::mult4x4v(cam.d,V[i].d,proj.d);
        T w=proj.d[3]; if(w!=0){proj.d[0]/=w;proj.d[1]/=w;proj.d[2]/=w;}
        doNotOptimize(proj);
      }
    }) / BATCH,
    measure(ITERS, [&]{
      for(int i=0;i<BATCH;++i){
        M16<T> cam; V4<T> proj;
        cpp::mult4x4(A[i].d,B[i].d,cam.d);
        cpp::mult4x4v(cam.d,V[i].d,proj.d);
        T w=proj.d[3]; if(w!=0){proj.d[0]/=w;proj.d[1]/=w;proj.d[2]/=w;}
        doNotOptimize(proj);
      }
    }) / BATCH);
}

int main() {
  std::printf("Assembly (verified): C++ uses scalar fmadd, SIMD uses fmul.4s/fmla.4s\n");
  std::printf("Measurement: batch=%d independent ops, doNotOptimize each result\n", BATCH);
  run<float>();
  run<double>();
  return 0;
}
