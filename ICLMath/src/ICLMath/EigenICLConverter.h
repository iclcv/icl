
#pragma once

#ifdef ICL_HAVE_EIGEN3

#include <ICLMath/FixedMatrix.h>
#include <ICLMath/FixedVector.h>
#include <ICLMath/DynMatrix.h>
#include <ICLMath/DynVector.h>
#include <Eigen/Core>

namespace icl {
	namespace math {

		//------------------------------------------------------------------------------------------

		template<typename T, int COLS, int ROWS> // need ints here. otherwise template parameter cannot be deduced
		inline icl::math::FixedMatrix<T,COLS,ROWS> eigenToICL(Eigen::Matrix<T,ROWS,COLS> const &E) {
			icl::math::FixedMatrix<T,COLS,ROWS> result;
			for(unsigned int i = 0; i < COLS; ++i)
				for(unsigned int k = 0; k < ROWS; ++k)
					result(i,k) = E(k,i);
			return result;
		}

		template<typename T, unsigned int COLS, unsigned int ROWS>
		inline Eigen::Matrix<T,ROWS,COLS> iclToEigen(icl::math::FixedMatrix<T,COLS,ROWS> const &ICL) {
			Eigen::Matrix<T,ROWS,COLS> result;
			for(unsigned int i = 0; i < COLS; ++i)
				for(unsigned int k = 0; k < ROWS; ++k)
					result(k,i) = ICL(i,k);
			return result;
		}

		//------------------------------------------------------------------------------------------

		template<typename T, unsigned int COLS, unsigned int ROWS>
		inline Eigen::Map<Eigen::Matrix<T,ROWS,COLS,Eigen::RowMajor>,Eigen::Aligned,Eigen::Stride<0,0> >
		iclToEigenRowMajorShallow(icl::math::FixedMatrix<T,COLS,ROWS> &ICL) {
			Eigen::Map<Eigen::Matrix<T,ROWS,COLS,Eigen::RowMajor>,Eigen::Aligned,Eigen::Stride<0,0> > result(ICL.data());
			return result;
		}

		template<typename T, unsigned int COLS, unsigned int ROWS>
		inline Eigen::Map<Eigen::Matrix<T,ROWS,COLS,Eigen::ColMajor>,Eigen::Unaligned,Eigen::Stride<1,COLS> >
		iclToEigenColMajorShallow(icl::math::FixedMatrix<T,COLS,ROWS> &ICL) {
			Eigen::Map<Eigen::Matrix<T,ROWS,COLS,Eigen::ColMajor>,Eigen::Unaligned,Eigen::Stride<1,COLS> > result(ICL.data());
			return result;
		}

		//------------------------------------------------------------------------------------------

		template<typename T>
		inline icl::math::DynMatrix<T> eigenToICLDyn(Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> const &E) {
			icl::math::DynMatrix<T> result(E.cols(),E.rows());
			for(uint i = 0; i < E.cols(); ++i)
				for(uint k = 0; k < E.rows(); ++k)
					result(i,k) = E(k,i);
			return result;
		}

		template<typename T>
		inline Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> iclToEigenDyn(icl::math::DynMatrix<T> const &E) {
			Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> result(E.rows(),E.cols());
			for(uint i = 0; i < E.cols(); ++i)
				for(uint k = 0; k < E.rows(); ++k)
					result(k,i) = E(i,k);
			return result;
		}

		//------------------------------------------------------------------------------------------

		template<typename T>
		inline Eigen::Map<Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic,Eigen::RowMajor>,Eigen::Aligned >
		iclToEigenDynRowMajorShallow(icl::math::DynMatrix<T> &ICL) {
			Eigen::Map<Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic,Eigen::RowMajor>,Eigen::Aligned> result(ICL.data(),ICL.rows(),ICL.cols());
			return result;
		}

		template<typename T>
		inline Eigen::Map<Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic,Eigen::ColMajor>,Eigen::Unaligned,Eigen::Stride<Eigen::Dynamic,Eigen::Dynamic> >
		iclToEigenDynColMajorShallow(icl::math::DynMatrix<T> &ICL) {
			Eigen::Map<Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic,Eigen::ColMajor>,Eigen::Unaligned,Eigen::Stride<Eigen::Dynamic,Eigen::Dynamic> > result(ICL.data(),ICL.rows(),ICL.cols(),Eigen::Stride<Eigen::Dynamic,Eigen::Dynamic>(1,ICL.cols()));
			return result;
		}

		//------------------------------------------------------------------------------------------

		#define ICL_INSTANCIATE_ICL_TO_EIGEN(Type,Size) \
		template Eigen::Matrix<Type,Size,Size> iclToEigen<Type,Size,Size>(icl::math::FixedMatrix<Type,Size,Size> const &);\
		template Eigen::Matrix<Type,Size,1> iclToEigen<Type,1,Size>(icl::math::FixedMatrix<Type,1,Size> const &);\
		template Eigen::Matrix<Type,1,Size> iclToEigen<Type,Size,1>(icl::math::FixedMatrix<Type,Size,1> const &);\
		template icl::math::FixedMatrix<Type,Size,Size> eigenToICL<Type,Size,Size>(Eigen::Matrix<Type,Size,Size> const &);\
		template icl::math::FixedMatrix<Type,1,Size> eigenToICL<Type,1,Size>(Eigen::Matrix<Type,Size,1> const &);\
		template icl::math::FixedMatrix<Type,Size,1> eigenToICL<Type,Size,1>(Eigen::Matrix<Type,1,Size> const &);

		#define ICL_INSTANCIATE_ICL_TO_EIGEN_DYN(Type) \
		template icl::math::DynMatrix<Type> eigenToICLDyn<Type>(Eigen::Matrix<Type,Eigen::Dynamic,Eigen::Dynamic> const &); \
		template Eigen::Matrix<Type,Eigen::Dynamic,Eigen::Dynamic> iclToEigenDyn<Type>(icl::math::DynMatrix<Type> const &);

		//template Eigen::Map<Eigen::Matrix<Type,Eigen::Dynamic,Eigen::Dynamic,Eigen::RowMajor>> iclToEigenDynShallow<Type>(icl::math::DynMatrix<Type> &);

		#define ICL_INSTANCIATE_ICL_TO_EIGEN_SHALLOW(Type,Size,TypeSuffix)																	\
		typedef Eigen::Map<Eigen::Matrix<Type,Size,Size,Eigen::ColMajor>,Eigen::Unaligned,Eigen::Stride<1,Size> >							\
			ICLEigenMapColMajor##Size##TypeSuffix;																							\
		typedef Eigen::Map<Eigen::Matrix<Type,Size,Size,Eigen::RowMajor>,Eigen::Aligned,Eigen::Stride<0,0> >								\
			ICLEigenMapRowMajor##Size##TypeSuffix;																							\
		template ICLEigenMapRowMajor##Size##TypeSuffix iclToEigenRowMajorShallow<Type,Size,Size>(icl::math::FixedMatrix<Type,Size,Size> &);	\
		template ICLEigenMapColMajor##Size##TypeSuffix iclToEigenColMajorShallow<Type,Size,Size>(icl::math::FixedMatrix<Type,Size,Size> &);

		#define ICL_INSTANCIATE_ICL_TO_EIGEN_SHALLOW_DYN(Type,TypeSuffix)																						\
		typedef Eigen::Map<Eigen::Matrix<Type,Eigen::Dynamic,Eigen::Dynamic,Eigen::ColMajor>,Eigen::Unaligned,Eigen::Stride<Eigen::Dynamic,Eigen::Dynamic> >	\
			ICLEigenMapColMajorDyn##TypeSuffix;																											\
		typedef Eigen::Map<Eigen::Matrix<Type,Eigen::Dynamic,Eigen::Dynamic,Eigen::RowMajor>,Eigen::Aligned>													\
			ICLEigenMapRowMajorDyn##TypeSuffix;																											\
		template ICLEigenMapColMajorDyn##TypeSuffix iclToEigenDynColMajorShallow(icl::math::DynMatrix<Type> &ICL);										\
		template ICLEigenMapRowMajorDyn##TypeSuffix iclToEigenDynRowMajorShallow(icl::math::DynMatrix<Type> &ICL);

		#define ICL_INSTANCIATE_ICL_TO_EIGEN_ALL_SIZES(Type,TypeSuffix) \
		ICL_INSTANCIATE_ICL_TO_EIGEN_SHALLOW(Type,2,TypeSuffix)\
		ICL_INSTANCIATE_ICL_TO_EIGEN_SHALLOW(Type,3,TypeSuffix)\
		ICL_INSTANCIATE_ICL_TO_EIGEN_SHALLOW(Type,4,TypeSuffix)\
		ICL_INSTANCIATE_ICL_TO_EIGEN_SHALLOW_DYN(Type,TypeSuffix)\
		ICL_INSTANCIATE_ICL_TO_EIGEN_DYN(Type)\
		ICL_INSTANCIATE_ICL_TO_EIGEN(Type,2)\
		ICL_INSTANCIATE_ICL_TO_EIGEN(Type,3)\
		ICL_INSTANCIATE_ICL_TO_EIGEN(Type,4)

		ICL_INSTANCIATE_ICL_TO_EIGEN_ALL_SIZES(int,i)
		ICL_INSTANCIATE_ICL_TO_EIGEN_ALL_SIZES(float,f)
		ICL_INSTANCIATE_ICL_TO_EIGEN_ALL_SIZES(double,d)

		#undef ICL_INSTANCIATE_ICL_TO_EIGEN_ALL_SIZES
		#undef ICL_INSTANCIATE_ICL_TO_EIGEN_DYN
		#undef ICL_INSTANCIATE_ICL_TO_EIGEN
		#undef ICL_INSTANCIATE_ICL_TO_EIGEN_SHALLOW
		#undef ICL_INSTANCIATE_ICL_TO_EIGEN_SHALLOW_DYN

	} // namespace math
} // namespace icl

#endif // ICL_HAVE_EIGEN3
