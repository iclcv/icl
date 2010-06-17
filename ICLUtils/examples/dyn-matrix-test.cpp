/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/examples/dyn-matrix-test.cpp                  **
** Module : ICLUtils                                               **
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

#include <ICLUtils/DynMatrix.h>
#include <ICLUtils/FixedMatrixUtils.h>
#include <ICLUtils/ProgArg.h>
#include <ICLUtils/StackTimer.h>

using namespace icl;

typedef DynMatrix<double> DMat;

double A[] = {
  0.066284,   0.899089,   0.284524,   0.470013,
  0.697046,   0.878033,   0.994972,   0.788343,
  0.397366,   0.369032,   0.452722,   0.556812,
  0.631776,   0.197289,   0.778549,   0.184409,
  0.621997,   0.480217,   0.970152,   0.629711,
  0.620559,   0.638306,   0.843637,   0.904307
};

double iA[]={
  -0.772066,   1.620275,   3.684992,   3.125834,  -5.720848,   0.066077,
  1.270598,   0.859848,   0.262561,   0.941522,  -1.518347,  -0.706346,
  0.269739,  -0.987983,  -3.214629,  -1.297812,   4.887908,  -0.438582,
  -0.687937,  -0.629473,   0.713768,  -1.783785,   0.663873,   1.474105,
}; 

double B[] = {
  0.685154,   0.560154,   0.240915,
  0.470926,   0.459299,   0.257321,
  0.667701,   0.466422,   0.165223,
  0.907473,   0.692640,   0.581566,
  0.594134,   0.589466,   0.785002,
  0.924171,   0.059626,   0.204498,
  0.370823,   0.013687,   0.462970,
  0.449660,   0.637078,   0.629939,
  0.618160,   0.795543,   0.819165,
  0.536261,   0.547491,   0.209449,
  0.724577,   0.155603,   0.368376,
  0.776059,   0.566786,   0.076817
};

double iB[]={ /// factor 1/10
  1.0532676,  -0.0035893,   1.7266161,   0.9945211,  -1.3260908,   6.2181657,   1.5204014,  -2.3275156,  -2.5564312,   0.0670401,   3.6080731,   2.1631083,
  2.8085560,   2.3913092,   2.0074585,   0.7370890,  -0.6304459,  -6.6475317,  -5.9894383,   2.3632692,   2.3179574,   3.8170656,  -5.1461053,   3.6925370,
  -2.9449854,  -1.2820811,  -3.2410802,   0.3907744,   5.1106675,   0.0077297,   5.7235073,   2.8168913,   3.8441930,  -2.8990165,   2.1827609,  -5.6759944
};


double C[] = {
  0.68515,   0.56015,   0.24091,
  0.47093,   0.45930,   0.25732,
  0.00000,   0.00000,   0.00000,
  0.90747,   0.69264,   0.58157,
  0.00000,   0.00000,   0.00000,
  0.92417,   0.05963,   0.20450,
  0.37082,   0.01369,   0.46297,
  0.44966,   0.63708,   0.62994,
  0.61816,   0.79554,   0.81916,
  0.53626,   0.54749,   0.20945,
  0.72458,   0.15560,   0.36838,
  0.77606,   0.56679,   0.07682
};

double iC[] = {
  0.13831,   0.01092,   0.00000,   0.09617,   0.00000,   0.64841,   0.09790,  -0.27589,  -0.31145,   0.03499,   0.35255,   0.28303,
  0.31805,   0.25691,   0.00000,   0.08636,   0.00000,  -0.63715,  -0.63764,   0.21455,   0.20350,   0.41409,  -0.51388,   0.43300,
  -0.35977,  -0.13596,   0.00000,   0.09263,   0.00000,  -0.05869,   0.74375,   0.44427,   0.59444,  -0.34444,   0.26351,  -0.73260
};

double D[] = {
  1,0,0,
  0,2,0,
  0,0,4,
  0,0,0
};

double iD[] = {
  1, 0,  0,   0,
  0, 0.5,0,   0,
  0, 0,  0.25,0
};



double E[] = {
  1,2,
  3,4
};

double E2[] = {
  1,2,
  3,4,
  5,6,
};

double iE[] = {
  -2.00000,   1.00000,
  1.50000,  -0.50000,
};


double F[] = {
  0.3718780,   0.8501537,   0.6145466,   0.2568249,   0.2702512,   0.4481879,   0.0148846,   0.0758894,
  0.7560972,   0.5467102,   0.2074168,   0.9165789,   0.2919305,   0.9037888,   0.0039063,   0.0736349,
  0.4343029,   0.0546702,   0.8458765,   0.5625745,   0.6531613,   0.4769098,   0.0273712,   0.2325393,
  0.1286207,   0.8200671,   0.2313650,   0.2679207,   0.9864421,   0.4391918,   0.0555951,   0.4859667
};

double iF[] = {
  0.0876816,   0.3633569,   0.0674843,  -0.2980389,
  0.9493045,  -0.0318096,  -0.7571829,   0.2514656,
  0.8788338,  -0.6578269,   0.8211368,  -0.5339851,
  -0.4839075,   0.5719757,   0.1603419,  -0.0658788,
  -0.6360976,  -0.1847849,   0.3648721,   0.7592247,
  -0.1226549,   0.4590841,  -0.0761348,   0.0042718,
  -0.0240278,  -0.0196894,   0.0136087,   0.0459825,
  -0.3719909,  -0.0822011,   0.1071590,   0.4525974
};

const int J_DIM = 4;
const double J_DET =  0.053777;
double J[] = {
  0.827244,   0.089357,   0.962606,   0.917073,
  0.105635,   0.613869,   0.046746,   0.095781,
  0.976612,   0.248304,   0.907173,   0.794252,
  0.449633,   0.124991,   0.920790,   0.531229
};
double iJ[] = {
  -2.670293,  -0.966794,   3.811385,  -0.914378,
  -0.244665,   1.648349,  -0.092019,   0.262751,
  -1.762969,  -0.411050,   0.328774,   2.626011,
  5.373498,   1.142943,  -3.774184,  -1.957183,
};

const int K_DIM = 6;
const double K_DET = -0.0087545;
double K[] = {
  0.637750,0.527936,0.019758,0.744938,0.349333,0.638757,
  0.583847,0.727741,0.149663,0.116770,0.056763,0.163995,
  0.428722,0.592850,0.900388,0.119288,0.397983,0.497385,
  0.279848,0.689939,0.845674,0.439895,0.631890,0.872326,
  0.912083,0.857675,0.118353,0.610002,0.166882,0.219193,
  0.704139,0.737904,0.349323,0.934496,0.218207,0.451971
};

double iK[] = {
  2.12666,-0.69989,2.89490,-2.69725,0.14227,-0.80055,
  -2.00766,1.81287,-2.65979,2.39622,0.61764,0.18227,
  -0.55777,-0.34078,1.65817,-0.91536,-0.87850,1.27989,
  -0.91326,-1.28262,-0.89634,0.53093,0.62645,1.41395,
  -1.56067,-5.29049,-1.59218,3.46220,7.64912,-4.51440,
  3.03742,3.60016,1.17280,-1.77183,-5.53921,1.42897
};

void rnd3(double &d){
  d = double(int(d*10000))/10000.0;
}
double rnd3_2(double d){
  return double(int(d*10000))/10000.0;
}


DynMatrix<double> rnd3(DynMatrix<double> M){
  for(unsigned int x=0;x<M.cols();++x){
    for(unsigned int y=0;y<M.rows();++y){
      rnd3(M(x,y));
    }
  }
  return M;
}

template<unsigned int W, unsigned int H>
FixedMatrix<double,W,H> rnd3(FixedMatrix<double,W,H> M){
  for(unsigned int x=0;x<M.cols();++x){
    for(unsigned int y=0;y<M.rows();++y){
      rnd3(M(x,y));
    }
  }
  return M;
}

typedef DynMatrix<double> DMat;

#define NUM 1000*10


void dyn_benchmark(const DMat &M){
  BENCHMARK_THIS_FUNCTION;
  for(int i=0;i<NUM;++i){
    M.pinv();
  }
}

template<unsigned int W, unsigned int H>
void fixed_benchmark(const FixedMatrix<double,W,H> &M){
  BENCHMARK_THIS_FUNCTION;
  for(int i=0;i<NUM;++i){
    pinv(M);
  }
}


int main(int n, char **ppc){
  painit(n,ppc,"-no-pinv -no-bench -no-inv");
  for(int i=0;i<36;++i) iB[i]/=10.0;

  std::cout << "All matrices must be 0!" << std::endl;
  
  
  if(!pa("-no-inv")){
#define IT(A)                                                           \
    DynMatrix<double> square##A(A##_DIM,A##_DIM,A);                     \
    std::cout << "det-difference: " << rnd3_2(square##A.det() - A##_DET) << std::endl; \
    DynMatrix<double> square##A##_Inv(A##_DIM,A##_DIM,i##A);            \
    SHOW(rnd3(square##A.inv()-square##A##_Inv));
    
    IT(J);
    IT(K);
  }

#define MM(A,W,H)                                           \
  DynMatrix<double> dyn##A(W,H,A);                          \
  DynMatrix<double> idyn##A(H,W,i##A);                      \
  DynMatrix<double> my_idyn##A = dyn##A.pinv();             \
  SHOW(rnd3(idyn##A-my_idyn##A));                           \
  FixedMatrix<double,W,H> fix##A(A);                        \
  FixedMatrix<double,H,W> ifix##A(i##A);                    \
  FixedMatrix<double,H,W> my_ifix##A = pinv(fix##A);        \
  SHOW(rnd3(ifix##A-my_ifix##A));

  if(!pa("-no-pinv")){
    MM(A,4,6);
    MM(B,3,12);
    MM(C,3,12);
    MM(D,3,4);
    MM(E,2,2);
    MM(F,8,4);
  }
  
  if(!pa("-no-bench")){
    std::cout << "starting benchmark (1000*10 operations on 4x6 source matrix)" << std::endl;
    DynMatrix<double> dyn(4,6,A);
    FixedMatrix<double,4,6> fix(A);
    
    dyn_benchmark(dyn);
    fixed_benchmark(fix);
  }

}
