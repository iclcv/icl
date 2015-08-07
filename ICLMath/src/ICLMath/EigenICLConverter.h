
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
			for(uint i = 0; i < COLS; ++i)
				for(uint k = 0; k < ROWS; ++k)
					result(i,k) = E(k,i);
			return result;
		}

		template<typename T, unsigned int COLS, unsigned int ROWS>
		inline Eigen::Matrix<T,ROWS,COLS> iclToEigen(icl::math::FixedMatrix<T,COLS,ROWS> const &ICL) {
			Eigen::Matrix<T,ROWS,COLS> result;
			for(uint i = 0; i < COLS; ++i)
				for(uint k = 0; k < ROWS; ++k)
					result(k,i) = ICL(i,k);
			return result;
		}

		//------------------------------------------------------------------------------------------

		template<typename T>
		icl::math::DynMatrix<T> eigenToICLDyn(Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> const &E) {
			icl::math::DynMatrix<T> result(E.cols(),E.rows());
			for(uint i = 0; i < E.cols(); ++i)
				for(uint k = 0; k < E.rows(); ++k)
					result(i,k) = E(k,i);
			return result;
		}

		template<typename T>
		Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> iclToEigenDyn(icl::math::DynMatrix<T> const &E) {
			Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> result(E.rows(),E.cols());
			for(uint i = 0; i < E.cols(); ++i)
				for(uint k = 0; k < E.rows(); ++k)
					result(k,i) = E(i,k);
			return result;
		}

		//------------------------------------------------------------------------------------------

		#define ICL_INSTANCIATE_ICL_TO_EIGEN(Type,Size) \
		template Eigen::Matrix<Type,Size,Size> iclToEigen<Type,Size,Size>(icl::math::FixedMatrix<Type,Size,Size> const &);\
		template icl::math::FixedMatrix<Type,Size,Size> eigenToICL<Type,Size,Size>(Eigen::Matrix<Type,Size,Size> const &);\
		template Eigen::Matrix<Type,Size,1> iclToEigen<Type,1,Size>(icl::math::FixedMatrix<Type,1,Size> const &);\
		template Eigen::Matrix<Type,1,Size> iclToEigen<Type,Size,1>(icl::math::FixedMatrix<Type,Size,1> const &);\
		template icl::math::FixedMatrix<Type,1,Size> eigenToICL<Type,1,Size>(Eigen::Matrix<Type,Size,1> const &);\
		template icl::math::FixedMatrix<Type,Size,1> eigenToICL<Type,Size,1>(Eigen::Matrix<Type,1,Size> const &);

		#define ICL_INSTANCIATE_ICL_TO_EIGEN_DYN(Type) \
		template icl::math::DynMatrix<Type> eigenToICLDyn<Type>(Eigen::Matrix<Type,Eigen::Dynamic,Eigen::Dynamic> const &E); \
		template Eigen::Matrix<Type,Eigen::Dynamic,Eigen::Dynamic> iclToEigenDyn<Type>(icl::math::DynMatrix<Type> const &E);

		#define ICL_INSTANCIATE_ICL_TO_EIGEN_ALL_SIZES(Type) \
		ICL_INSTANCIATE_ICL_TO_EIGEN(Type,2)\
		ICL_INSTANCIATE_ICL_TO_EIGEN(Type,3)\
		ICL_INSTANCIATE_ICL_TO_EIGEN(Type,4)

		ICL_INSTANCIATE_ICL_TO_EIGEN_ALL_SIZES(int)
		ICL_INSTANCIATE_ICL_TO_EIGEN_ALL_SIZES(float)
		ICL_INSTANCIATE_ICL_TO_EIGEN_ALL_SIZES(double)

		ICL_INSTANCIATE_ICL_TO_EIGEN_DYN(int)
		ICL_INSTANCIATE_ICL_TO_EIGEN_DYN(float)
		ICL_INSTANCIATE_ICL_TO_EIGEN_DYN(double)

		#undef ICL_INSTANCIATE_ICL_TO_EIGEN_ALL_SIZES
		#undef ICL_INSTANCIATE_ICL_TO_EIGEN_DYN
		#undef ICL_INSTANCIATE_ICL_TO_EIGEN

	} // namespace math
} // namespace icl

#endif
