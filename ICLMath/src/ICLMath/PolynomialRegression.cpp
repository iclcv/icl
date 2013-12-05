/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/PolynomialRegression.cpp           **
** Module : ICLMath                                                **
** Authors: Christof Elbrechter                                    **
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


#include <ICLMath/PolynomialRegression.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/ConfigFile.h>

using namespace icl::utils;

namespace icl{
  namespace math{
    
    namespace{
      template<class T>
      struct ConstAttrib : public PolynomialRegressionAttrib<T>{
        T t;
        ConstAttrib(const T &t):t(t){}
        virtual T compute(const T *) const{ return t; }
        virtual std::string toString() const{
          return str(t);
        }
      };
    
      template<class T>
      struct GenPowerAttrib : public PolynomialRegressionAttrib<T>{
        int idx;
        T exponent;
        GenPowerAttrib(int idx, const T &exponent):idx(idx),exponent(exponent){}
        virtual T compute(const T *row) const{ return ::pow(row[idx], exponent); }

        virtual std::string toString() const{
          return "x"+str(idx)+"^"+str(exponent);
        }
      };

      template <class T, int EXPONENT>
      struct PowerAttrib : public PolynomialRegressionAttrib<T>{
        int idx;
        PowerAttrib(int idx):idx(idx){}
        virtual T compute(const T *row) const{ return icl::utils::power<T,EXPONENT>(row[idx]); }
        virtual std::string toString() const{
          return "x"+str(idx)+"^"+str(EXPONENT);
        }

      };

      template<class T>
      struct GenMixedAttrib : public PolynomialRegressionAttrib<T>{
        std::vector<int> idxs;
        GenMixedAttrib(const std::vector<int> &idxs):idxs(idxs){}
        virtual T compute(const T *row) const{
          float product = 1;
          for(size_t i=0;i<idxs.size();++i){
            product *= row[idxs[i]];
          }
          return product;
        }

        virtual std::string toString() const{
          std::ostringstream stream;
          for(size_t i=0;i<idxs.size();++i){
            stream << "x" << idxs[i];
            if(i < idxs.size()-1) stream << "*";
          }
          return stream.str();
        }

      };

      template<class T, int N>
      struct MixedAttrib : public PolynomialRegressionAttrib<T>{
        int idxs[N];
        MixedAttrib(const int idxs[N]){
          std::copy(idxs,idxs+N,this->idxs);
        }
        virtual T compute(const T *row) const{
          if(N == 1) return row[idxs[0]];
          if(N == 2) return row[idxs[0]] * row[idxs[1]];
          if(N == 3) return row[idxs[0]] * row[idxs[1]] * row[idxs[2]];
          if(N == 4) return row[idxs[0]] * row[idxs[1]] * row[idxs[2]] * row[idxs[3]];
          if(N == 5) return row[idxs[0]] * row[idxs[1]] * row[idxs[2]] * row[idxs[3]] * row[idxs[4]];

          float product = 1;
          for(size_t i=0;i<N;++i){
            product *= row[idxs[i]];
          }
          return product;
        }

        virtual std::string toString() const{
          std::ostringstream stream;
          for(size_t i=0;i<N;++i){
            stream << "x" << idxs[i];
            if(i < N-1) stream << "*";
          }
          return stream.str();
        }

      };

      inline std::string remove_spaces(const std::string s){
        std::ostringstream stream;
        for(size_t i=0;i<s.length();++i){
          if(s[i] != ' ') stream << s[i];
        }
        return stream.str();
      }
    
      inline int get_idx(const std::string &s){
        return icl::utils::parse<int>(s.substr(1));
      }

      inline bool is_int(float f){
        return !(f - (int)f);
      }

      template<class T>
      void apply_params(const std::vector<const PolynomialRegressionAttrib<T>*> &ps, const T *xrow, T *xs){
        for(size_t i=0;i<ps.size();++i){
          xs[i] = ps[i]->compute(xrow);
        }
      }

    }

    template<class T>
    PolynomialRegression<T>::PolynomialRegression(const std::string &function){
      m_result.setup(function);
    }
    
  

    template<class T>
    const typename PolynomialRegression<T>::Result &
    PolynomialRegression<T>::apply(const typename PolynomialRegression<T>::Matrix &xs, 
                                   const typename PolynomialRegression<T>::Matrix &ys,bool useSVD){
      ICLASSERT_THROW(xs.rows() == ys.rows(),ICLException("PolynomialRegression::apply: xs.rows() must be equal to ys.rows()"));
      const int &M  = m_result.m_attribMaxIndex;
      ICLASSERT_THROW((int)xs.cols() > M,ICLException("PolynomialRegression::apply: maximum attribute index found is " + str(M) + " but the given data matrix (xs) has only " + str(xs.cols()) + " columns"));
      m_buf.setBounds(m_result.m_attribs.size(), xs.rows());
    
      for(unsigned int i=0;i<xs.rows();++i){
        apply_params(m_result.m_attribs, xs.row_begin(i), m_buf.row_begin(i));
      }
    
      m_buf.pinv(useSVD).mult(ys, m_result.m_params);
      //m_result.m_params.reshape(m_result.m_params.rows(), m_result.m_params.cols());
      
      return m_result;
    }

    
    template<class T>
    const typename PolynomialRegression<T>::Matrix &PolynomialRegression<T>::Result::operator()
      (const typename PolynomialRegression<T>::Matrix &xs) const{
      
      m_xbuf.setBounds(m_attribs.size(), xs.rows());
      for(unsigned i=0;i<xs.rows();++i){
        apply_params(m_attribs, xs.row_begin(i), m_xbuf.row_begin(i));
      }

      m_xbuf.mult(m_params,m_resultBuf);
      
      return m_resultBuf;
    }
  
    template<class T>
    std::string PolynomialRegression<T>::getFunctionString() const{
      std::ostringstream stream;
      for(size_t i=0;i<m_result.m_attribs.size();++i){
        stream << m_result.m_attribs[i]->toString();
        if(i < m_result.m_attribs.size()-1) stream << " + ";
      }
      return stream.str();
    }
  
    template<class T>
    std::string PolynomialRegression<T>::Result::toString(const std::vector<std::string> &rowLabels) const {
      std::ostringstream stream;
      std::cout << "Polyonomial Regression Result:" << std::endl;
      for(unsigned int y=0;y<getParams().cols();++y){
        if(rowLabels.size() > y){
          stream << rowLabels[y] << " = ";
        }else{
          stream << "y[" << y << "] = ";
        }
        std::vector<const PolynomialRegressionAttrib<T>*> atts = getAttribs();
        for(size_t x=0;x<atts.size();++x){
          if(atts[x]->toString() == "1"){
            stream << (x ? fabs(getParams()(y,x)) : getParams()(x,y) )<< " ";
          }else{
            stream << (x ? fabs(getParams()(y,x)) : getParams()(x,y) )<< " * " << atts[x]->toString();
          }
          if(x < atts.size()-1){
            if(getParams()(x,y) < 0){
              stream << " - ";
            }else{
              stream << " - ";
            }
          }
        }
        stream << std::endl;
      }
      stream << std::endl;
      return stream.str();
    }


    template<class T>
    void PolynomialRegression<T>::Result::setup(const std::string &function){
      m_function = function;
      
      std::vector<std::string> ts = icl::utils::tok(remove_spaces(function),"+");
      int maxIdx = -1;
      for(size_t i=0;i<ts.size();++i){
        const std::string &s =  ts[i];
        if(s.find('^',0) != std::string::npos){
          std::vector<std::string> ab = icl::utils::tok(s,"^");
          ICLASSERT_THROW(ab.size() == 2, ICLException("PolynomialRegression: error in token: " + s));
          int idx = get_idx(ab[0]);
          if(idx > maxIdx) maxIdx = idx;
          
          float exponent = parse<float>(ab[1]);
          if(is_int(exponent) && exponent < 6){
            int e = exponent;
            switch(e){
              case 1: m_attribs.push_back(new PowerAttrib<T,1>(idx)); break;
              case 2: m_attribs.push_back(new PowerAttrib<T,2>(idx)); break;
              case 3: m_attribs.push_back(new PowerAttrib<T,3>(idx)); break;
              case 4: m_attribs.push_back(new PowerAttrib<T,4>(idx)); break;
              case 5: m_attribs.push_back(new PowerAttrib<T,5>(idx)); break;
              default: m_attribs.push_back(new GenPowerAttrib<T>(idx,e)); break;
            }
          }else{
            m_attribs.push_back(new GenPowerAttrib<T>(idx,exponent)); break;
          }
        }else if(s.find('*') != std::string::npos){
          std::vector<std::string> vars = tok(s,"*");
          std::vector<int> idxs(vars.size());
          for(size_t j=0;j<idxs.size();++j){
            idxs[j] = get_idx(vars[j]);
            if(idxs[j] > maxIdx) maxIdx = idxs[j];
          }
          switch(idxs.size()){
            case 2: m_attribs.push_back(new MixedAttrib<T,2>(&*idxs.begin())); break;
            case 3: m_attribs.push_back(new MixedAttrib<T,3>(&*idxs.begin())); break;
            case 4: m_attribs.push_back(new MixedAttrib<T,4>(&*idxs.begin())); break;
            case 5: m_attribs.push_back(new MixedAttrib<T,5>(&*idxs.begin())); break;
            default: m_attribs.push_back(new GenMixedAttrib<T>(idxs)); break;
          }
        }else if(s[0] == 'x'){
          int idx = get_idx(s);
          if(idx > maxIdx) maxIdx = idx;
          m_attribs.push_back(new MixedAttrib<T,1>(&idx));
        }else{
          m_attribs.push_back(new ConstAttrib<T>(parse<int>(s)));
        }
      }
      m_attribMaxIndex = maxIdx;      
    }    

    template<class T>
    void PolynomialRegression<T>::Result::save(const std::string &xmlFileName) const {
      DEBUG_LOG(toString());
      ConfigFile cfg;
      cfg.setPrefix("config.polynomial-regression-result.");
      cfg["function"] = m_function;
      cfg["parameters.dim.cols"] = (int)m_params.cols();
      cfg["parameters.dim.rows"] = (int)m_params.rows();
      cfg["parameters.values"] = str(m_params);
      
      cfg.save(xmlFileName);
     }
    
    template<class T>
    PolynomialRegression<T>::Result::Result(const std::string &xmlFileName){
      ConfigFile cfg(xmlFileName);
      cfg.setPrefix("config.polynomial-regression-result.");
      setup(cfg["function"].as<std::string>());
      
      m_params.setBounds(cfg["parameters.dim.cols"].as<int>(),
                         cfg["parameters.dim.rows"].as<int>());


      std::istringstream stream(cfg["parameters.values"].as<std::string>());
      stream >> m_params;
    }

    template class ICL_MATH_API PolynomialRegression<float>;
    template class ICL_MATH_API PolynomialRegression<double>;
  }
}
