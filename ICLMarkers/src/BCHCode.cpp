/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/.cpp                                    **
** Module : ICLMarkers                                             **
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

#include <vector>
#include <algorithm>

#include <ICLUtils/SmartPtr.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/StackTimer.h>
#include <ICLUtils/FixedVector.h>

#include <ICLMarkers/BCHCode.h>

namespace icl {
  
  /** source copyright: (source code found at http://www.eccpage.com/bch3.c)
      
      COPYRIGHT NOTICE: This computer program is free for non-commercial purposes.
      You may implement this program for any non-commercial application. You may 
      also implement this program for commercial purposes, provided that you
      obtain my written permission. Any modification of this program is covered
      by this copyright.
      == Copyright (c) 1994-7,  Robert Morelos-Zaragoza. All rights reserved.  ==
      
      
      Our code design was also inspired by the ARToolkitPlus implementation
      that also uses Robert Morelos-Zaragoza's BCH-En/Decoder.
      
      ARToolkitPlusSRC/src/extra/BCH_original.txt for details/orig. copyright notice
      Copyright of the derived and new portions of this work
      (C) 2006 Graz University of Technology
      author: Thomas Pintaric
      ARToolkitPlus can also be used under the terms of the GNU public license
  */

  /// this class implements a (36-bit binary BCH en/decoder)
  /** the code can carry 12 bits of information and it can be
      extracted event if 4 errors occur */
  namespace {
    struct BCHCoder{
      enum Constants{
        BCH_DEFAULT_M      = 6,
        BCH_DEFAULT_LENGTH = 36,
        BCH_DEFAULT_T      = 4,
        BCH_DEFAULT_K      = 12,
        BCH_MAX_M          = 6,
        BCH_MAX_P          = 7,   // MAX_M+1
        BCH_MAX_LUT        = 64,  // 2^MAX_M
        BCH_MAX_SQ         = 8    // SQRT(MAX_LUT) -- (?)
      };
      int t, m, n, length, k, d;
      FixedMatrix<int,BCH_MAX_LUT,BCH_MAX_LUT> m_elp; 
      FixedColVector<int,BCH_MAX_P> p;
      FixedColVector<int,BCH_MAX_LUT> alpha_to,index_of,g,m_d,m_l,m_ulu,m_s,m_root,m_loc,m_reg;
      
      typedef int64_t _64bits;

      private:
      BCHCoder(){
        // {{{ open
        m = BCH_DEFAULT_M;
        length = BCH_DEFAULT_LENGTH;
        t = BCH_DEFAULT_T;
        std::fill(p.begin(),p.begin()+m+1,0);
        p[0] = p[1] = 1;
        n = (1 << m) - 1;

        // generate galois field GF(2**m)
        int mask = 1;
        alpha_to[m] = 0;
        for (int i = 0; i < m; i++) {
          alpha_to[i] = mask;
          index_of[alpha_to[i]] = i;
          if (p[i] != 0){
            alpha_to[m] ^= mask;
          }
          mask <<= 1;
        }
        index_of[alpha_to[m]] = m;
        mask >>= 1;
        for (int i = m + 1; i < n; i++) {
          if (alpha_to[i - 1] >= mask){
            alpha_to[i] = alpha_to[m] ^ ((alpha_to[i - 1] ^ mask) << 1);
          }else{
            alpha_to[i] = alpha_to[i - 1] << 1;
          }
          index_of[alpha_to[i]] = i;
        }
        index_of[0] = -1;
          
        gen_poly();      
      }
      // }}}

      bool gen_poly(){
        // {{{ open
        int ii, jj, ll, kaux;
        int test, aux, nocycles, root, noterms, rdncy;
        int cycle[1024][21], size[1024], min[1024], zeros[1024];
        
        /* Generate cycle sets modulo n, n = 2**m - 1 */
        cycle[0][0] = 0;
        size[0] = 1;
        cycle[1][0] = 1;
        size[1] = 1;
        jj = 1;			/* cycle set index */
        
        
        do {
          /* Generate the jj-th cycle set */
          ii = 0;
          do {
            ii++;
            cycle[jj][ii] = (cycle[jj][ii - 1] * 2) % n;
            size[jj]++;
            aux = (cycle[jj][ii] * 2) % n;
          } while (aux != cycle[jj][0]);
          /* Next cycle set representative */
          ll = 0;
          do {
            ll++;
            test = 0;
            for (ii = 1; ((ii <= jj) && (!test)); ii++)	
              /* Examine previous cycle sets */
              for (kaux = 0; ((kaux < size[ii]) && (!test)); kaux++)
                if (ll == cycle[ii][kaux])
                  test = 1;
          } while ((test) && (ll < (n - 1)));
          if (!(test)) {
            jj++;	/* next cycle set index */
            cycle[jj][0] = ll;
            size[jj] = 1;
          }
        } while (ll < (n - 1));
        nocycles = jj;		/* number of cycle sets modulo n */
        d = 2 * t + 1;
        
        /* Search for roots 1, 2, ..., d-1 in cycle sets */
        kaux = 0;
        rdncy = 0;
        for (ii = 1; ii <= nocycles; ii++) {
          min[kaux] = 0;
          test = 0;
          for (jj = 0; ((jj < size[ii]) && (!test)); jj++)
            for (root = 1; ((root < d) && (!test)); root++)
              if (root == cycle[ii][jj])  {
                test = 1;
                min[kaux] = ii;
              }
          if (min[kaux]) {
            rdncy += size[min[kaux]];
            kaux++;
          }
        }
        noterms = kaux;
        kaux = 1;
        for (ii = 0; ii < noterms; ii++)
          for (jj = 0; jj < size[min[ii]]; jj++) {
            zeros[kaux] = cycle[min[ii]][jj];
            kaux++;
          }
        
        k = length - rdncy;
        
        if (k<0){
          return false;
        }
        
        /* Compute the generator polynomial */
        g[0] = alpha_to[zeros[1]];
        g[1] = 1;		/* g(x) = (X + zeros[1]) initially */
        for (ii = 2; ii <= rdncy; ii++) {
          g[ii] = 1;
          for (jj = ii - 1; jj > 0; jj--)
            if (g[jj] != 0)
              g[jj] = g[jj - 1] ^ alpha_to[(index_of[g[jj]] + zeros[ii]) % n];
            else
              g[jj] = g[jj - 1];
          g[0] = alpha_to[(index_of[g[0]] + zeros[ii]) % n];
        }
        return true;
      }
      // }}}

      public:
      
      static BCHCoder *getInstance(){
        // {{{ open
        static SmartPtr<BCHCoder> instance(new BCHCoder);
        return instance.get();
      }
      // }}}

      /// not used because, we pre-encoded the whole data-set
      void encode_bch(int *bb, const int *data){
        // {{{ open
        int    i, j;
        int    feedback;
        
        for (i = 0; i < length - k; i++)
          bb[i] = 0;
        for (i = k - 1; i >= 0; i--) {
          feedback = data[i] ^ bb[length - k - 1];
          if (feedback != 0) {
            for (j = length - k - 1; j > 0; j--)
              if (g[j] != 0)
                bb[j] = bb[j - 1] ^ feedback;
              else
                bb[j] = bb[j - 1];
            bb[0] = g[0] && feedback;
          } else {
            for (j = length - k - 1; j > 0; j--)
              bb[j] = bb[j - 1];
            bb[0] = 0;
          }
        }
      }
      // }}}

      // adapted version that works on an encoded int64_t,
      // the original version got an int[]
      int decode_bch_2(int64_t &code){
        // {{{ open
        int i, j, u, q, t2, count = 0, syn_error = 0;
        bool too_many_errors = false;
        int64_t one = 1;
        t2 = 2 * t;
        
        /* first form the syndromes */
        for (i = 1; i <= t2; i++) {
          m_s[i] = 0;
          for (j = 0; j < length; j++)
            if ((code&(one<<j)))  //TODO check
              m_s[i] ^= alpha_to[(i * j) % n];
          if (m_s[i] != 0)
            syn_error = 1; /* set error flag if non-zero syndrome */
          /*
              * Note:    If the code is used only for ERROR DETECTION, then
              *          exit program here indicating the presence of errors.
              */
          /* convert syndrome from polynomial form to index form  */
          m_s[i] = index_of[m_s[i]];
        }
        
        
        if (syn_error) {	/* if there are errors, try to correct them */
          /*
              * Compute the error location polynomial via the Berlekamp
              * iterative algorithm. Following the terminology of Lin and
              * Costello's book :   d[u] is the 'mu'th discrepancy, where
              * u='mu'+1 and 'mu' (the Greek letter!) is the step number
              * ranging from -1 to 2*t (see L&C),  l[u] is the degree of
              * the elp at that step, and u_l[u] is the difference between
              * the step number and the degree of the elp. 
              */
          /* initialise table entries */
          m_d[0] = 0;			/* index form */
          m_d[1] = m_s[1];		/* index form */
          m_elp(0,0) = 0;//[0][0] = 0;		/* index form */
          m_elp(1,0) = 1;		/* polynomial form */
          for (i = 1; i < t2; i++) {
            m_elp(0,i) = -1;	/* index form */
            m_elp(1,i) = 0;	/* polynomial form */
          }
          m_l[0] = 0;
          m_l[1] = 0;
          m_ulu[0] = -1;
          m_ulu[1] = 0;
          u = 0;
          
          do {
            u++;
            if (m_d[u] == -1) {
              m_l[u + 1] = m_l[u];
              for (i = 0; i <= m_l[u]; i++) {
                m_elp(u + 1,i) = m_elp(u,i);
                m_elp(u,i) = index_of[m_elp(u,i)];
              }
            } else
              /*
                  * search for words with greatest m_ulu[q] for
                  * which m_d[q]!=0 
                  */
              {
                q = u - 1;
                while ((m_d[q] == -1) && (q > 0))
                  q--;
                /* have found first non-zero m_d[q]  */
                if (q > 0) {
                  j = q;
                  do {
                    j--;
                    if ((m_d[j] != -1) && (m_ulu[q] < m_ulu[j]))
                      q = j;
                  } while (j > 0);
                }
            
                /*
                    * have now found q such that m_d[u]!=0 and
                    * m_ulu[q] is maximum 
                    */
                /* store degree of new elp polynomial */
                if (m_l[u] > m_l[q] + u - q)
                  m_l[u + 1] = m_l[u];
                else
                  m_l[u + 1] = m_l[q] + u - q;
            
                /* form new elp(x) */
                for (i = 0; i < t2; i++)
                  m_elp(u + 1,i) = 0;
                for (i = 0; i <= m_l[q]; i++)
                  if (m_elp(q,i) != -1)
                    m_elp(u + 1,i + u - q) = 
                    alpha_to[(m_d[u] + n - m_d[q] + m_elp(q,i)) % n];
                for (i = 0; i <= m_l[u]; i++) {
                  m_elp(u + 1,i) ^= m_elp(u,i);
                  m_elp(u,i) = index_of[m_elp(u,i)];
                }
              }
            m_ulu[u + 1] = u - m_l[u + 1];
        
            /* form (u+1)th discrepancy */
            if (u < t2) {	
              /* no discrepancy computed on last iteration */
              if (m_s[u + 1] != -1)
                m_d[u + 1] = alpha_to[m_s[u + 1]];
              else
                m_d[u + 1] = 0;
              for (i = 1; i <= m_l[u + 1]; i++)
                if ((m_s[u + 1 - i] != -1) && (m_elp(u + 1,i) != 0))
                  m_d[u + 1] ^= alpha_to[(m_s[u + 1 - i] 
                                          + index_of[m_elp(u + 1,i)]) % n];
              /* put m_d[u+1] into index form */
              m_d[u + 1] = index_of[m_d[u + 1]];	
            }
          } while ((u < t2) && (m_l[u + 1] <= t));
      
          u++;
          if (m_l[u] <= t) {/* Can correct errors */
            /* put elp into index form */
            for (i = 0; i <= m_l[u]; i++)
              m_elp(u,i) = index_of[m_elp(u,i)];
        
            /* Chien search: find roots of the error location polynomial */
            for (i = 1; i <= m_l[u]; i++)
              m_reg[i] = m_elp(u,i);
            count = 0;
            for (i = 1; i <= n; i++) {
              q = 1;
              for (j = 1; j <= m_l[u]; j++)
                if (m_reg[j] != -1) {
                  m_reg[j] = (m_reg[j] + j) % n;
                  q ^= alpha_to[m_reg[j]];
                }
              if (!q) {	/* store root and error
                            * location number indices */
                m_root[count] = i;
                m_loc[count] = n - i;
                count++;
              }
            }
            if (count == m_l[u])	
              {
                /* no. roots = degree of elp hence <= t errors */
                for (i = 0; i < m_l[u]; i++)
                  {
                    int li = m_loc[i];
                    if(li<BCH_DEFAULT_LENGTH){
                      //code[m_loc[i]].flip(); //^= 1;
                      code ^= (one << li);
                    }
                    else too_many_errors = true;
                  }
              }
            else	/* elp has degree >t hence cannot solve */
              {
                return(m_l[u]); // number of errors
            
              }
          }
        }
        if(too_many_errors) return(BCH_DEFAULT_T+1);
        else return(syn_error == 0 ? 0 : m_l[u]); // number of errors
      }
      // }}}
    };
  }
  
  BCHCode encode_bch(int idx){
    // {{{ open
    static const char *bch_codes[4096]={
      "XhqhUc","XNlua5","XxMD3K","X39ozF","XplQIr","XVq5mQ","XF9W73","X-MJvm","XdMcMW","XJ9ril","Xtqy-w","XZllrT","Xl9VQJ","XRM8ey","XBl1Zf","X7qMD+","XfRfQB","XLWweG","XvDFZ9","X1gmDg","XnWOMi","XTR7iZ","XDgY-U","X9DHrv","XbDaIP","XHgtms","XrRA7p","XXWjv0","XjgTU6","XPD+ab","XzW33C","X5RKzN","XgwvI1","XMngmo","XwGp7t","X27CvO","Xon4UM","XUwRaD","XE7I3a","X+GXz7","XcGqQh","XI7de8","XswkZH","XYnzDA","Xk79Mu","XQGUiV","XAnN-Y","X6w0rj","XeTxMS","XK2eix","XuBn-k","X0aErX","Xm26Q-","XSTPee","XCaGZz","X8BZDI","XaBsUE","XGabaL","XqTi34","XW2Bzd","Xia-In","XOBSm2","Xy2L7R","X4T2vq",
      "rhtoiz","rNiDMI","rxLur-","r3+h-e","rpiJek","rVtWQX","rF+5DS","r-LQZx","rdLlaR","rJ+yUq","rttrzn","rZic32","rl+Mm4","rRL1Id","rBi8vE","r7tV7L","rfOmma","rLZFI7","rvEwvM","r1ff7D","rnZHat","rTOYUO","rDf7z1","r9EO3o","rbEjeY","rHfAQj","rrOtDu","rXZaZV","rjfKiH","rPE3MA","rzZ+rh","r5OT-8","rgvCeU","rMopQv","rwJgDi","r24vZZ","rooXi9","rUvIMg","rE4RrB","r+J4-G","rcJzmC","rI4kIN","rsvdv6","rYoq7b","rk40ap","rQJNU0","rAoUzP","r6v93s","reUEa3","rK1nUm","ruyezr","r0dx3Q","rm1ZmK","rSUGIF","rCdPvc","r8y675","rayBif","rGdiM+","rqUbrJ","rW1s-y","rid2ew","rOyLQT","ry1SDW","r4U-Zl",
      "HhQqZ0","HNXdDp","HxCkQs","H3hzeP","HpX9-N","HVQUrC","HFhNMb","H-C0i6","HdCv7g","HJhgv9","HtQpIG","HZXCmB","Hlh43v","HRCRzU","HBXIUZ","H7QXai","Hfrs3T","HLkbzw","HvNiUl","H18BaW","Hnk-7+","HTrSvf","HD8LIy","H9N2mJ","HbNx-F","HH8erK","HrrnM5","HXkEic","Hj86Zm","HPNPD3","HzkGQQ","H5rZer","HgSc-d","HM3rr4","HwAyML","H2bliE","Ho3VZq","HUS8DR","HEb1Q2","H+AMen","HcAh3X","HIbuzk","HsSDUx","HY3oaS","HkbQ7I","HQA5vz","HA3WIe","H6SJm-","Hexa7A","HKmtvH","HuHAI8","H06jmh","HmmT3j","HSx+zY","HC63UV","H8HKau","HaHfZO","HG6wDt","HqxFQo","HWmme1","Hi6O-7","HOH7ra","HymYMD","H4xHiM",
      "bhPzvV","bNYk7u","bxFdmj","b3eqIY","bpY0z8","bVPN3h","bFeUaA","b-F9UH","bdFCDD","bJepZM","btPge7","bZYvQa","bleXro","bRFI-1","bBYRiO","b7P4Mt","bfsBr2","bLji-n","bvKbiq","b1-sMR","bnj2DL","bTsLZE","bD-Sed","b9K-Q4","bbKEze","bH-n3-","brseaI","bXjxUz","bj-Zvx","bPKG7S","bzjPmX","b5s6Ik","bgVlzy","bM0y3J","bwzra+","b2ccUf","bo0Mvl","bUV17W","bEc8mT","b+zVIw","bczorQ","bIcD-r","bsVuim","bY0hM3","bkcJD5","bQzWZc","bA05eF","b6VQQK","beujDb","bKpAZ6","buIteN","b05aQC","bmpKrs","bSu3-P","bC5+i0","b8ITMp","baImvZ","bG5F7i","bquwmv","bWpfIU","bi5HzG","bOIY3B","byp7ag","b4uOU9",
      "5hTSGz","5N2-oI","5xB25-","53aLxe","5p2bSk","5VTscX","5FaB1S","5-BiBx","5dBPOR","5Ja6gq","5tTZXn","5Z2GF2","5laeK4","5RBxkd","5B2E9E","57TntL","5fwUKa","5Ln9k7","5vG09M","517NtD","5nndOt","5TwqgO","5D7zX1","59GkFo","5bGRSY","5H74cj","5rwX1u","5XnIBV","5j7gGH","5PGvoA","5znC5h","55wpx8","5gR+SU","5MWTcv","5wDK1i","52g3BZ","5oWtG9","5URaog","5Egj5B","5+DAxG","5cD7KC","5IgOkN","5sRH96","5YWYtb","5kgwOp","5QDfg0","5AWmXP","56RFFs","5eq8O3","5KlVgm","5uMMXr","5091FQ","5mlrKK","5SqckF","5C9l9c","58Myt5","5aM5Gf","5G9Qo+","5qqJ5J","5WlWxy","5i9uSw","5OMhcT","5ylo1W","54qDBl",
      "zhULgc","zN12O5","zxy-FK","z3dSXF","zp1ikr","zVUBKQ","zFdst3","z-yb9m","zdyGoW","zJdZGl","ztU6xw","zZ1P5T","zldncJ","zRyESy","zB1xBf","z7Ue1+","zfvNcB","zLo0SG","zvJ9B9","z14U1g","znokoi","zTvzGZ","zD4qxU","z9Jd5v","zbJIkP","zH4XKs","zrv4tp","zXoR90","zj4pg6","zPJCOb","zzovFC","z5vgXN","zgO3k1","zMZKKo","zwETtt","z2f+9O","zoZAgM","zUOjOD","zEfaFa","z+EtX7","zcEYch","zIfHS8","zsOOBH","zYZ71A","zkfFou","zQEmGV","zAZfxY","z6Ow5j","zet1oS","zKiMGx","zuLVxk","z0+85X","zmiyc-","zStlSe","zC+cBz","z8Lr1I","zaLWgE","zG+JOL","zqtQF4","zWi5Xd","zi+Dkn","zOLoK2","zyihtR","z4tu9q",
      "Phx79V","PNmOtu","PxHHKj","P36YkY","PpmwX8","PVxfFh","PF6mOA","P-HFgH","PdH+1D","PJ6TBM","PtxKS7","PZm3ca","Pl6t5o","PRHax1","PBmjGO","P7xAot","PfS552","PL3Qxn","PvAJGq","P1bWoR","Pn3u1L","PTShBE","PDboSd","P9ADc4","PbA8Xe","PHbVF-","PrSMOI","PX31gz","Pjbr9x","PPActS","Pz3lKX","P5Sykk","PgrPXy","PMk6FJ","PwNZO+","P28Ggf","Poke9l","PUrxtW","PE8EKT","P+Nnkw","PcNS5Q","PI8-xr","Psr2Gm","PYkLo3","Pk8b15","PQNsBc","PAkBSF","P6ricK","PeQR1b","PKX4B6","PuCXSN","P0hIcC","PmXg5s","PSQvxP","PChCG0","P8Cpop","PaCU9Z","PGh9ti","PqQ0Kv","PWXNkU","PihdXG","POCqFB","PyXzOg","P4Qkg9",
      "jhuYB0","jNpH1p","jxIOcs","j357SP","jppFxN","jVum5C","jF5fob","j-IwG6","jdI3tg","jJ5K99","jtuTkG","jZp+KB","jl5AFv","jRIjXU","jBpagZ","j7utOi","jfVWFT","jL0JXw","jvzQgl","j1c5OW","jn0Dt+","jTVo9f","jDchky","j9zuKJ","jbz1xF","jHcM5K","jrVVo5","jX08Gc","jjcyBm","jPzl13","jz0ccQ","j5VrSr","jgsGxd","jMjZ54","jwK6oL","j2-PGE","jojnBq","jUsE1R","jE-xc2","j+KeSn","jcKLFX","jI-2Xk","jss-gx","jYjSOS","jk-itI","jQKB9z","jAjske","j6sbK-","jePItA","jKYX9H","juF4k8","j0eRKh","jmYpFj","jSPCXY","jCevgV","j8FgOu","jaFNBO","jGe01t","jqP9co","jWYUS1","jiekx7","jOFz5a","jyYqoD","j4PdGM",
      "1h1bN0","1NUsjp","1xdB+s","13yiqP","1pUSRN","1V1-fC","1Fy2Yb","1-dLC6","1ddeVg","1Jyxb9","1t1E2G","1ZUnyB","1lyPJv","1Rd6nU","1BUZ6Z","171Gui","1fodJT","1Lvqnw","1v4z6l","11JkuW","1nvUV+","1To9bf","1DJ02y","194NyJ","1b4gRF","1HJvfK","1roCY5","1XvpCc","1jJRNm","1P44j3","1zvX+Q","15oIqr","1gZtRd","1MOaf4","1wfjYL","12EACE","1oO+Nq","1UZTjR","1EEK+2","1+f3qn","1cfwJX","1IEfnk","1sZm6x","1YOFuS","1kE7VI","1QfObz","1AOH2e","16ZYy-","1eirVA","1KtcbH","1u+l28","10Lyyh","1mt8Jj","1SiVnY","1CLM6V","18+1uu","1a+uNO","1GLhjt","1qio+o","1WtDq1","1iL5R7","1O+Qfa","1ytJYD","14iWCM",
      "vh2ibV","vNTBVu","vxasyj","v3Bb2Y","vpTLn8","vV22Jh","vFB-uA","v-aS6H","vdanjD","vJBENM","vt2xq7","vZTe+a","vlBGfo","vRaZR1","vBT6CO","v72PYt","vfnkf2","vLwzRn","vv7qCq","v1GdYR","vnwNjL","vTn0NE","vDG9qd","v97U+4","vb7pne","vHGCJ-","vrnvuI","vXwg6z","vjGIbx","vP7XVS","vzw4yX","v5nR2k","vgWAny","vMRjJJ","vwgau+","v2Dt6f","voR3bl","vUWKVW","vEDTyT","v+g+2w","vcgFfQ","vIDmRr","vsWfCm","vYRwY3","vkDYj5","vQgHNc","vAROqF","v6W7+K","velyjb","vKqlN6","vu9cqN","v0Mr+C","vmq1fs","vSlMRP","vCMVC0","v898Yp","va9DbZ","vGMoVi","vqlhyv","vWqu2U","viMWnG","vO9JJB","vyqQug","v4l569",
      "Lhpw6c","LNufu5","Lx5mJK","L3IFnF","Lpu72r","LVpOyQ","LFIHV3","L-5Ybm","Ld5tYW","LJIaCl","LtpjRw","LZuAfT","LlI++J","LR5Tqy","LBuKNf","L7p3j+","Lf0u+B","LLVhqG","LvcoN9","L1zDjg","LnV5Yi","LT0QCZ","LDzJRU","L9cWfv","Lbcr2P","LHzcys","Lr0lVp","LXVyb0","Ljz866","LPcVub","LzVMJC","L501nN","Lgje21","LMsxyo","Lw-EVt","L2KnbO","LosP6M","LUj6uD","LEKZJa","L+-Gn7","Lc-b+h","LIKsq8","LsjBNH","LYsijA","LkKSYu","LQ--CV","LAs2RY","L6jLfj","LeYgYS","LKPvCx","LueCRk","L0FpfX","LmPR+-","LSY4qe","LCFXNz","L8eIjI","Laed6E","LGFquL","LqYzJ4","LWPknd","LiFU2n","LOe9y2","LyP0VR","L4YNbq",
      "fhmFCz","fNxmYI","fx6ff-","f3HwRe","fpxYqk","fVmH+X","fFHOjS","f-67Nx","fd6AuR","fJHj6q","ftmann","fZxtJ2","flH3y4","fR6K2d","fBxTbE","f7m+VL","ff3Dya","fLSo27","fvbhbM","f1AuVD","fnSWut","fT3J6O","fDAQn1","f9b5Jo","fbbyqY","fHAl+j","fr3cju","fXSrNV","fjA1CH","fPbMYA","fzSVfh","f538R8","fgknqU","fMrE+v","fw8xji","f2NeNZ","forGC9","fUkZYg","fEN6fB","f+8PRG","fc8iyC","fINB2N","fsksb6","fYrbVb","fkNLup","fQ8260","fAr-nP","f6kSJs","feXpu3","fKQC6m","fuhvnr","f0CgJQ","fmQIyK","fSXX2F","fCC4bc","f8hRV5","fahkCf","fGCzY+","fqXqfJ","fWQdRy","fiCNqw","fOh0+T","fyQ9jW","f4XUNl",
      "9hiQPV","9Nt5hu","9x+WWj","93LJEY","9pthL8","9Viulh","9FLD8A","9-+osH","9d+VHD","9JL8pM","9ti147","9ZtMwa","9lLcTo","9R+rd1","9Bty0O","97ilAt","9fZOT2","9LO7dn","9vfY0q","91EHAR","9nOfHL","9TZwpE","9DEF4d","99fmw4","9bfTLe","9HE+l-","9rZ38I","9XOKsz","9jEaPx","9PfthS","9zOAWX","95ZjEk","9go4Ly","9MvRlJ","9w4I8+","92JXsf","9ovvPl","9UoghW","9EJpWT","9+4CEw","9c49TQ","9IJUdr","9soN0m","9Yv0A3","9kJqH5","9Q4dpc","9Avk4F","96ozwK","9e16Hb","9KUPp6","9udG4N","90yZwC","9mUxTs","9S1edP","9Cyn00","98dEAp","9ad-PZ","9GyShi","9q1LWv","9WU2EU","9iysLG","9OdblB","9yUi8g","941Bs9",
      "DhlJp0","DNqWHp","Dx95ws","D3MQ4P","DpqodN","DVlDTC","DFMuAb","D-9h06","Dd9Mhg","DJM1P9","Dtl8EG","DZqVWB","DlMllv","DR9yLU","DBqrsZ","D7lc8i","DfWHlT","DLRYLw","Dvg7sl","D1DO8W","DnRmh+","DTWFPf","DDDwEy","D9gfWJ","DbgKdF","DHD3TK","DrW+A5","DXRT0c","DjDjpm","DPgAH3","DzRtwQ","D5Wa4r","DgnXdd","DMwIT4","Dw7RAL","D2G40E","DowCpq","DUnpHR","DEGgw2","D+7v4n","Dc70lX","DIGNLk","DsnUsx","DYw98S","DkGzhI","DQ7kPz","DAwdEe","D6nqW-","De2ZhA","DKTGPH","DuaPE8","D0B6Wh","DmTElj","DS2nLY","DCBesV","D8ax8u","Daa2pO","DGBLHt","Dq2Swo","DWT-41","DiBBd7","DOaiTa","DyTbAD","D42s0M",
      "ThY90z","TNPUAI","TxeNT-","T3F0de","TpPq4k","TVYdwX","TFFkHS","T-ezpx","Tde48R","TJFRsq","TtYILn","TZPXl2","TlFvW4","TRegEd","TBPpPE","T7YChL","Tfj-Wa","TLsSE7","Tv-LPM","T1K2hD","Tnss8t","TTjbsO","TDKiL1","T9-Blo","Tb-64Y","THKPwj","TrjGHu","TXsZpV","TjKx0H","TP-eAA","TzsnTh","T5jEd8","Tg0V4U","TMV8wv","Twc1Hi","T2zMpZ","ToVc09","TU0rAg","TEzyTB","T+cldG","TccQWC","TIz5EN","Ts0WP6","TYVJhb","Tkzh8p","TQcus0","TAVDLP","T60ols","TepT83","TKu+sm","Tu53Lr","T0IKlQ","TmuaWK","TSptEF","TCIAPc","T85jh5","Ta5O0f","TGI7A+","TqpYTJ","TWuHdy","TiIf4w","TO5wwT","TyuFHW","T4pmpl",
      "nhX0sc","nNQN85","nxhUlK","n3C9LF","npQzEr","nVXkWQ","nFCdh3","n-hqPm","ndhXAW","nJCI0l","ntXRdw","nZQ4TT","nlCCwJ","nRhp4y","nBQgpf","n7XvH+","nfk2wB","nLrL4G","nv8Sp9","n1N-Hg","nnrBAi","nTki0Z","nDNbdU","n98sTv","nb8ZEP","nHNGWs","nrkPhp","nXr6P0","njNEs6","nP8n8b","nzrelC","n5kxLN","ng3ME1","nMS1Wo","nwb8ht","n2AVPO","noSlsM","nU3y8D","nEArla","n+bcL7","ncbJwh","nIAW48","ns35pH","nYSQHA","nkAoAu","nQbD0V","nASudY","n63hTj","nemKAS","nKx30x","nu6+dk","n0HTTX","nmxjw-","nSmA4e","nCHtpz","n86aHI","na6HsE","nGHY8L","nqm7l4","nWxOLd","niHmEn","nO6FW2","nyxwhR","n4mfPq",
      "Zhz3eI","ZNcKQz","ZxVTDe","Z30+Z-","ZpcAiX","ZVzjMk","ZF0arx","Z-Vt-S","ZdVYmq","ZJ0HIR","ZtzOv2","ZZc77n","Zl0Fad","ZRVmU4","ZBcfzL","Z7zw3E","ZfI1a7","ZL5MUa","ZvuVzD","Z1p83M","Zn5ymO","ZTIlIt","ZDpcvo","Z9ur71","ZbuWij","ZHpJMY","ZrIQrV","ZX55-u","ZjpDeA","ZPuoQH","Zz5hD8","Z5IuZh","ZgFLiv","ZMe2MU","ZwP-rZ","Z2YS-i","Zoeieg","ZUFBQ9","ZEYsDG","Z+PbZB","ZcPGaN","ZIYZUC","ZsF6zb","ZYeP36","ZkYnm0","ZQPEIp","ZAexvs","Z6Fe7P","ZeKNmm","ZK-0I3","Zus9vQ","Z0jU7r","Zm-kaF","ZSKzUK","ZCjqz5","Z8sd3c","ZasIe+","ZGjXQf","ZqK4Dy","ZW-RZJ","ZijpiT","ZOsCMw","Zy-vrl","Z4Kg-W",
      "thA+I5","tNbTmc","txSK7F","t333vK","tpbtUQ","tVAaar","tF3j3m","t-SAz3","tdS7Ql","tJ3OeW","ttAHZT","tZbYDw","tl3wMy","tRSfiJ","tBbm-+","t7AFrf","tfH8MG","tL6ViB","tvxM-g","t1m1r9","tn6rQZ","tTHcei","tDmlZv","t9xyDU","tbx5Us","tHmQaP","trHJ30","tX6Wzp","tjmuIb","tPxhm6","tz6o7N","t5HDvC","tgCSUo","tMh-a1","twQ23O","t2XLzt","tohbID","tUCsmM","tEXB77","t+Qiva","tcQPM8","tIX6ih","tsCZ-A","tYhGrH","tkXeQV","tQQxeu","tAhEZj","t6CnDY","teNUQx","tK89eS","tur0ZX","t0kNDk","tm8dMe","tSNqi-","tCkz-I","t8rkrz","tarRIL","tGk4mE","tqNX7d","tW8Iv4","tikgU2","tOrvan","ty8C3q","t4NpzR",
      "JhJGzu","JN4Z3V","Jxv6aY","J3oPUj","Jp4nvh","JVJE78","JFoxmH","J-veIA","JdvLrM","JJo2-D","JtJ-ia","JZ4SM7","JloiD1","JRvBZo","JB4set","J7JbQO","JfyIDn","JLdXZ2","JvU4eR","J11RQq","JndprE","JTyC-L","JD1vi4","J9UgMd","JbUNv-","JH107e","Jry9mz","JXdUII","Jj1kzS","JPUz3x","Jzdqak","J5ydUX","JgLYvJ","JM+H7y","JwtOmf","J2i7I+","Jo+FzW","JULm3l","JEifaw","J+twUT","Jct3Dr","JIiKZQ","JsLTe3","JY++Qm","JkiArc","JQtj-5","JA+aiK","J6LtMF","JeEWr6","JKfJ-b","JuOQiC","J0Z5MN","JmfDDP","JSEoZs","JCZhep","J8OuQ0","JaO1zi","JGZM3Z","JqEVaU","JWf8Uv","JiZyvB","JOOl7G","Jyfcm9","J4ErIg",
      "dhGP-p","dN76r0","dxwZMP","d3nGis","dp7eZC","dVGxDN","dFnEQ6","d-wneb","ddwS39","dJn-zg","dtG2UB","dZ7LaG","dlnb7U","dRwsvv","dB7BIi","d7GimZ","dfBR7w","dLa4vT","dvTXIW","d12Iml","dnag3f","dTBvz+","dD2CUJ","d9Tpay","dbTUZK","dH29DF","drB0Qc","dXaNe5","dj2d-3","dPTqrm","dzazMr","d5BkiQ","dgM7Z4","dM9ODd","dwqHQE","d2lYeL","do9w-R","dUMfrq","dElmMn","d+qFi2","dcq+7k","dIlTvX","dsMKIS","dY93mx","dklt3z","dQqazI","dA9jU-","d6MAae","deD53H","dKgQzA","duRJUh","d0WWa8","dmgu7Y","dSDhvj","dCWoIu","d8RDmV","daR8-t","dGWVrO","dqDMM1","dWg1io","diWrZa","dORcD7","dyglQM","d4DyeD",
      "7hKCk5","7N-pKc","7xsgtF","73jv9K","7p-XgQ","7VKIOr","7FjRFm","7-s4X3","7dszcl","7JjkSW","7tKdBT","7Z-q1w","7lj0oy","7RsNGJ","7B-Ux+","77K95f","7fFEoG","7LenGB","7vPexg","71Yx59","7neZcZ","7TFGSi","7DYPBv","79P61U","7bPBgs","7HYiOP","7rFbF0","7XesXp","7jY2kb","7PPLK6","7zeStN","75F-9C","7gIogo","7M5DO1","7wuuFO","72phXt","7o5JkD","7UIWKM","7Ep5t7","7+uQ9a","7culo8","7IpyGh","7sIrxA","7Y5c5H","7kpMcV","7Qu1Su","7A58Bj","76IV1Y","7ezmcx","7KcFSS","7uVwBX","700f1k","7mcHoe","7SzYG-","7C07xI","78VO5z","7aVjkL","7G0AKE","7qzttd","7Wca94","7i0Kg2","7OV3On","7yc+Fq","74zTXR",
      "BhNvSI","BN8gcz","Bxrp1e","B3kCB-","Bp84GX","BVNRok","BFkI5x","B-rXxS","BdrqKq","BJkdkR","BtNk92","BZ8ztn","Blk9Od","BRrUg4","BB8NXL","B7N0FE","BfCxO7","BLhega","BvQnXD","B1XEFM","Bnh6KO","BTCPkt","BDXG9o","B9QZt1","BbQsGj","BHXboY","BrCi5V","BXhBxu","BjX-SA","BPQScH","BzhL18","B5C2Bh","BgHhGv","BM6uoU","BwxD5Z","B2moxi","Bo6QSg","BUH5c9","BEmW1G","B+xJBB","BcxcON","BImrgC","BsHyXb","BY6lF6","BkmVK0","BQx8kp","BA619s","B6HMtP","BeAfKm","BKbwk3","BuSF9Q","B03mtr","BmbOOF","BSA7gK","BC3YX5","B8SHFc","BaSaS+","BG3tcf","BqAA1y","BWbjBJ","Bi3TGT","BOS+ow","Byb35l","B4AKxW",
      "RhElxp","RNfy50","RxOroP","R3ZcGs","RpfMBC","RVE11N","RFZ8c6","R-OVSb","RdOoF9","RJZDXg","RtEugB","RZfhOG","RlZJtU","RROW9v","RBf5ki","R7EQKZ","RfLjtw","RL+A9T","RvttkW","R1iaKl","Rn+KFf","RTL3X+","RDi+gJ","R9tTOy","RbtmBK","RHiF1F","RrLwcc","RX+fS5","RjiHx3","RPtY5m","Rz+7or","R5LOGQ","RgyzB4","RMdk1d","RwUdcE","R21qSL","Rod0xR","RUyN5q","RE1Uon","R+U9G2","RcUCtk","RI1p9X","RsygkS","RYdvKx","Rk1XFz","RQUIXI","RAdRg-","R6y4Oe","ReJBFH","RK4iXA","Ruvbgh","R0osO8","Rm42tY","RSJL9j","RCoSku","R8v-KV","RavExt","RGon5O","RqJeo1","RW4xGo","RioZBa","ROvG17","Ry4PcM","R4J6SD",
      "lhDcXu","lNgrFV","lxRyOY","l3Wlgj","lpgV9h","lVD8t8","lFW1KH","l-RMkA","ldRh5M","lJWuxD","ltDDGa","lZgoo7","llWQ11","lRR5Bo","lBgWSt","l7DJcO","lfMa1n","lL9tB2","lvqASR","l1ljcq","ln9T5E","lTM+xL","lDl3G4","l9qKod","lbqf9-","lHlwte","lrMFKz","lX9mkI","ljlOXS","lPq7Fx","lz9YOk","l5MHgX","lgBq9J","lMadty","lwTkKf","l22zk+","loa9XW","lUBUFl","lE2NOw","l+T0gT","lcTv1r","lI2gBQ","lsBpS3","lYaCcm","lk245c","lQTRx5","lAaIGK","l6BXoF","leGs56","lK7bxb","luwiGC","l0nBoN","lm7-1P","lSGSBs","lCnLSp","l8w2c0","lawxXi","lGneFZ","lqGnOU","lW7Egv","lin69B","lOwPtG","ly7GK9","l4GZkg",
      "3h8Xnu","3NNIJV","3xkRuY","33r46j","3pNCbh","3V8pV8","3FrgyH","3-kv2A","3dk0fM","3JrNRD","3t8UCa","3ZN9Y7","3lrzj1","3RkkNo","3BNdqt","378q+O","3fhZjn","3LCGN2","3vXPqR","31Q6+q","3nCEfE","3ThnRL","3DQeC4","39XxYd","3bX2b-","3HQLVe","3rhSyz","3XC-2I","3jQBnS","3PXiJx","3zCbuk","35hs6X","3g6JbJ","3MHWVy","3wm5yf","32xQ2+","3oHonW","3U6DJl","3Exuuw","3+mh6T","3cmMjr","3Ix1NQ","3s68q3","3YHV+m","3kxlfc","3QmyR5","3AHrCK","366cYF","3ebHf6","3KAYRb","3u37CC","30SOYN","3mAmjP","3SbFNs","3CSwqp","383f+0","3a3Kni","3GS3JZ","3qb+uU","3WAT6v","3iSjbB","3O3AVG","3yAty9","34ba2g",
      "xh-4Rp","xNKRf0","xxjIYP","x3sXCs","xpKvNC","xV-gjN","xFsp+6","x-jCqb","xdj9J9","xJsUng","xt-N6B","xZK0uG","xlsqVU","xRjdbv","xBKk2i","x7-zyZ","xfe6Vw","xLFPbT","xvYG2W","x1PZyl","xnFxJf","xTeen+","xDPn6J","x9YEuy","xbY-NK","xHPSjF","xreL+c","xXF2q5","xjPsR3","xPYbfm","xzFiYr","x5eBCQ","xg5QN4","xMI5jd","xwpW+E","x2uJqL","xoIhRR","xU5ufq","xEuDYn","x+poC2","xcpVVk","xIu8bX","xs512S","xYIMyx","xkucJz","xQprnI","xAIy6-","x65lue","xecOJH","xKz7nA","xu0Y6h","x0VHu8","xmzfVY","xScwbj","xCVF2u","x80myV","xa0TRt","xGV+fO","xqc3Y1","xWzKCo","xiVaNa","xO0tj7","xyzA+M","x4cjqD",
      "NhgMqI","NND1+z","NxW8je","N3RVN-","NpDlCX","NVgyYk","NFRrfx","N-WcRS","NdWJyq","NJRW2R","Ntg5b2","NZDQVn","NlRoud","NRWD64","NBDunL","N7ghJE","Nf9Ku7","NLM36a","Nvl+nD","N1qTJM","NnMjyO","NT9A2t","NDqtbo","N9laV1","NblHCj","NHqYYY","Nr97fV","NXMORu","NjqmqA","NPlF+H","NzMwj8","N59fNh","Nga0Cv","NMBNYU","Nw2UfZ","N2T9Ri","NoBzqg","NUak+9","NETdjG","N+2qNB","Nc2XuN","NITI6C","NsaRnb","NYB4J6","NkTCy0","NQ2p2p","NABgbs","N6avVP","Ne72ym","NKGL23","NunSbQ","N0w-Vr","NmGBuF","NS7i6K","NCwbn5","N8nsJc","NanZq+","NGwG+f","Nq7Pjy","NWG6NJ","NiwECT","NOnnYw","NyGefl","N47xRW",
      "hhfV25","hNE8yc","hxZ1VF","h3OMbK","hpEc6Q","hVfrur","hFOyJm","h-Zln3","hdZQ+l","hJO5qW","htfWNT","hZEJjw","hlOhYy","hRZuCJ","hBEDR+","h7foff","hf+TYG","hLL+CB","hvi3Rg","h1tKf9","hnLa+Z","hT+tqi","hDtANv","h9ijjU","hbiO6s","hHt7uP","hr+YJ0","hXLHnp","hjtf2b","hPiwy6","hzLFVN","h5+mbC","hgd96o","hMyUu1","hw1NJO","h2U0nt","hoyq2D","hUddyM","hEUkV7","h+1zba","hc14Y8","hIURCh","hsdIRA","hYyXfH","hkUv+V","hQ1gqu","hAypNj","h6dCjY","he4-+x","hKJSqS","huoLNX","h0v2jk","hmJsYe","hS4bC-","hCviRI","h8oBfz","hao62L","hGvPyE","hq4GVd","hWJZb4","hivx62","hOoeun","hyJnJq","h44EnR",
      "-hbAdp","-NAjT0","-x3aAP","-3St0s","-pA3pC","-VbKHN","-FSTw6","--3+4b","-d3Fl9","-JSmLg","-tbfsB","-ZAw8G","-lSYhU","-R3HPv","-BAOEi","-7b7WZ","-f6yhw","-LHlPT","-vmcEW","-1xrWl","-nH1lf","-T6ML+","-DxVsJ","-9m88y","-bmDpK","-HxoHF","-r6hwc","-XHu45","-jxWd3","-PmJTm","-zHQAr","-5650Q","-ghip4","-MCBHd","-wXswE","-2Qb4L","-oCLdR","-Uh2Tq","-EQ-An","-+XS02","-cXnhk","-IQEPX","-shxES","-YCeWx","-kQGlz","-QXZLI","-AC6s-","-6hP8e","-e8klH","-KNzLA","-ukqsh","-0rd88","-mNNhY","-S80Pj","-Cr9Eu","-8kUWV","-akpdt","-GrCTO","-q8vA1","-WNg0o","-irIpa","-OkXH7","-yN4wM","-48R4D",
      "FhctLu","FNzalV","Fx0j8Y","F3VAsj","Fpz+Ph","FVcTh8","FFVKWH","F-03EA","Fd0wTM","FJVfdD","Ftcm0a","FZzFA7","FlV7H1","FR0Opo","FBzH4t","F7cYwO","Ff5rHn","FLIcp2","Fvpl4R","F1uywq","FnI8TE","FT5VdL","FDuM04","F9p1Ad","FbpuP-","FHuhhe","Fr5oWz","FXIDEI","Fju5LS","FPpQlx","FzIJ8k","F55WsX","FgebPJ","FMFshy","FwYBWf","F2PiE+","FoFSLW","FUe-ll","FEP28w","F+YLsT","FcYeHr","FIPxpQ","FseE43","FYFnwm","FkPPTc","FQY6d5","FAFZ0K","F6eGAF","Fe-dT6","FKKqdb","Fujz0C","F0skAN","FmKUHP","FS-9ps","FCs04p","F8jNw0","FajgLi","FGsvlZ","Fq-C8U","FWKpsv","FisRPB","FOj4hG","FyKXW9","F4-IEg",
      "Vh7nE5","VNGEWc","VxnxhF","V3wePK","VpGGsQ","VV7Z8r","VFw6lm","V-nPL3","Vdniwl","VJwB4W","Vt7spT","VZGbHw","VlwLAy","VRn20J","VBG-d+","V77STf","VfapAG","VLBC0B","Vv2vdg","V1TgT9","VnBIwZ","VTaX4i","VDT4pv","V92RHU","Vb2kss","VHTz8P","Vraql0","VXBdLp","VjTNEb","VP20W6","VzB9hN","V5aUPC","Vg9Fso","VMMm81","VwlflO","V2qwLt","VoMYED","VU9HWM","VEqOh7","V+l7Pa","VclAA8","VIqj0h","Vs9adA","VYMtTH","Vkq3wV","VQlK4u","VAMTpj","V69+HY","VegDwx","VKDo4S","VuWhpX","V0RuHk","VmDWAe","VSgJ0-","VCRQdI","V8W5Tz","VaWyEL","VGRlWE","Vqgchd","VWDrP4","ViR1s2","VOWM8n","VyDVlq","V4g8LR",
      "ph4e4I","pNJxwz","pxoEHe","p3vnp-","ppJP0X","pV46Ak","pFvZTx","p-oGdS","pdobWq","pJvsER","pt4BP2","pZJihn","plvS8d","pRo-s4","pBJ2LL","p74LlE","pfdg87","pLyvsa","pv1CLD","p1UplM","pnyRWO","pTd4Et","pDUXPo","p91Ih1","pb1d0j","pHUqAY","prdzTV","pXykdu","pjUU4A","pP19wH","pzy0H8","p5dNph","pg+w0v","pMLfAU","pwimTZ","p2tFdi","poL74g","pU+Ow9","pEtHHG","p+iYpB","pcit8N","pItasC","ps+jLb","pYLAl6","pkt+W0","pQiTEp","pALKPs","p6+3hP","pefuWm","pKEhE3","puZoPQ","p0ODhr","pmE58F","pSfQsK","pCOJL5","p8ZWlc","paZr4+","pGOcwf","pqflHy","pWEypJ","piO80T","pOZVAw","pyEMTl","p4f1dW",
      "WhpmvP","WNuF7s","Wx5wmp","W3IfI0","WpuHz6","WVpY3b","WFI7aC","W-5OUN","Wd5jDB","WJIAZG","Wtpte9","WZuaQg","WlIKri","WR53-Z","WBu+iU","W7pTMv","Wf0orW","WLVD-l","Wvcuiw","W1zhMT","WnVJDJ","WT0WZy","WDz5ef","W9cQQ+","Wbclzc","WHzy35","Wr0raK","WXVcUF","WjzMvr","WPc17Q","WzV8m3","W50VIm","WgjEzE","WMsn3L","Ww-ea4","W2KxUd","WosZvn","WUjG72","WEKPmR","W+-6Iq","Wc-BrS","WIKi-x","Wsjbik","WYssMX","WkK2D-","WQ-LZe","WAsSez","W6j-QI","WeYCDh","WKPpZ8","WuegeH","W0FvQA","WmPXru","WSYI-V","WCFRiY","W8e4Mj","Waezv1","WGFk7o","WqYdmt","WWPqIO","WiF0zM","WOeN3D","WyPUaa","W4Y9U7",
      "qhmfZY","qNxwDj","qx6FQu","q3HmeV","qpxO-H","qVm7rA","qFHYMh","q-6Hi8","qd6a7a","qJHtv7","qtmAIM","qZxjmD","qlHT3t","qR6+zO","qBx3U1","q7mKao","qf3h3R","qLSuzq","qvbDUn","q1Aoa2","qnSQ74","qT35vd","qDAWIE","q9bJmL","qbbc-z","qHArrI","qr3yM-","qXSlie","qjAVZk","qPb8DX","qzS1QS","q53Mex","qgkx-f","qMrer+","qw8nMJ","q2NEiy","qor6Zw","qUkPDT","qENGQW","q+8Zel","qc8s33","qINbzm","qskiUr","qYrBaQ","qkN-7K","qQ8SvF","qArLIc","q6k2m5","qeXv7C","qKQgvN","quhpI6","q0CCmb","qmQ43p","qSXRz0","qCCIUP","q8hXas","qahqZU","qGCdDv","qqXkQi","qWQzeZ","qiC9-9","qOhUrg","qyQNMB","q4X0iG",
      "Gh1BiF","GNUiMK","Gxdbr5","G3ys-c","GpU2em","GV1LQ3","GFySDQ","G-d-Zr","GddEaT","GJynUw","Gt1ezl","GZUx3W","GlyZm+","GRdGIf","GBUPvy","G7167J","Gfozmg","GLvkI9","Gv4dvG","G1Jq7B","Gnv0av","GToNUU","GDJUzZ","G9493i","Gb4Ce0","GHJpQp","GrogDs","GXvvZP","GjJXiN","GP4IMC","GzvRrb","G5o4-6","GgZjeO","GMOAQt","GwftDo","G2EaZ1","GoOKi7","GUZ3Ma","GEE+rD","G+fT-M","GcfmmA","GIEFIH","GsZwv8","GYOf7h","GkEHaj","GQfYUY","GAO7zV","G6ZO3u","GeilaX","GKtyUk","Gu+rzx","G0Lc3S","GmtMmI","GSi1Iz","GCL8ve","G8+V7-","Ga+oid","GGLDM4","GqiurL","GWth-E","GiLJeq","GO+WQR","Gyt5D2","G4iQZn",
      "ah2sUe","aNTba-","axai3I","a3BBzz","apT-Ix","aV2SmS","aFBL7X","a-a2vk","adaxM2","aJBein","at2n-q","aZTErR","alB6QL","aRaPeE","aBTGZd","a72ZD4","afnqQD","aLwdeM","av7kZ7","a1GzDa","anw9Mo","aTnUi1","aDGN-O","a970rt","ab7vIV","aHGgmu","arnp7j","aXwCvY","ajG4U8","aP7Rah","azwI3A","a5nXzH","agWaIZ","aMRtmi","awgA7v","a2DjvU","aoRTUG","aUW+aB","aED33g","a+gKz9","acgfQb","aIDwe6","asWFZN","aYRmDC","akDOMs","aQg7iP","aARY-0","a6WHrp","aelcMQ","aKqrir","au9y-m","a0Mlr3","amqVQ5","aSl8ec","aCM1ZF","a89MDK","aa9hUy","aGMuaJ","aqlD3+","aWqozf","aiMQIl","aO95mW","ayqW7T","a4lJvw",
      "4hYNBY","4NP01j","4xe9cu","43FUSV","4pPkxH","4VYz5A","4FFqoh","4-edG8","4deIta","4JFX97","4tY4kM","4ZPRKD","4lFpFt","4ReCXO","4BPvg1","47YgOo","4fjLFR","4Ls2Xq","4v--gn","41KSO2","4nsit4","4TjB9d","4DKskE","49-bKL","4b-Gxz","4HKZ5I","4rj6o-","4XsPGe","4jKnBk","4P-E1X","4zsxcS","45jeSx","4g01xf","4MVM5+","4wcVoJ","42z8Gy","4oVyBw","4U0l1T","4EzccW","4+crSl","4ccWF3","4IzJXm","4s0Qgr","4YV5OQ","4kzDtK","4Qco9F","4AVhkc","460uK5","4ep3tC","4KuK9N","4u5Tk6","40I+Kb","4muAFp","4SpjX0","4CIagP","485tOs","4a5YBU","4GIH1v","4qpOci","4Wu7SZ","4iIFx9","4O5m5g","4yufoB","44pwGG",
      "yhXU9P","yNQ9ts","yxh0Kp","y3CNk0","ypQdX6","yVXqFb","yFCzOC","y-hkgN","ydhR1B","yJC4BG","ytXXS9","yZQIcg","ylCg5i","yRhvxZ","yBQCGU","y7Xpov","yfkS5W","yLr-xl","yv82Gw","y1NLoT","ynrb1J","yTksBy","yDNBSf","y98ic+","yb8PXc","yHN6F5","yrkZOK","yXrGgF","yjNe9r","yP8xtQ","yzrEK3","y5knkm","yg38XE","yMSVFL","ywbMO4","y2A1gd","yoSr9n","yU3ct2","yEAlKR","y+bykq","ycb55S","yIAQxx","ys3JGk","yYSWoX","ykAu1-","yQbhBe","yASoSz","y63DcI","yem+1h","yKxTB8","yu6KSH","y0H3cA","ymxt5u","ySmaxV","yCHjGY","y86Aoj","ya6791","yGHOto","yqmHKt","yWxYkO","yiHwXM","yO6fFD","yyxmOa","y4mFg7",
      "OhiWge","ONtJO-","Ox+QFI","O3L5Xz","OptDkx","OVioKS","OFLhtX","O-+u9k","Od+1o2","OJLMGn","OtiVxq","OZt85R","OlLycL","OR+lSE","OBtcBd","O7ir14","OfZYcD","OLOHSM","OvfOB7","O1E71a","OnOFoo","OTZmG1","ODEfxO","O9fw5t","Obf3kV","OHEKKu","OrZTtj","OXO+9Y","OjEAg8","OPfjOh","OzOaFA","O5ZtXH","OgoIkZ","OMvXKi","Ow44tv","O2JR9U","OovpgG","OUoCOB","OEJvFg","O+4gX9","Oc4Ncb","OIJ0S6","Oso9BN","OYvU1C","OkJkos","OQ4zGP","OAvqx0","O6od5p","Oe1GoQ","OKUZGr","Oud6xm","O0yP53","OmUnc5","OS1ESc","OCyxBF","O8de1K","OadLgy","OGy2OJ","Oq1-F+","OWUSXf","Oiyikl","OOdBKW","OyUstT","O41b9w",
      "ihl5GF","iNqQoK","ix9J55","i3MWxc","ipquSm","iVlhc3","iFMo1Q","i-9DBr","id98OT","iJMVgw","itlMXl","iZq1FW","ilMrK+","iR9ckf","iBql9y","i7lytJ","ifW7Kg","iLROk9","ivgH9G","i1DYtB","inRwOv","iTWfgU","iDDmXZ","i9gFFi","ibg+S0","iHDTcp","irWK1s","iXR3BP","ijDtGN","iPgaoC","izRj5b","i5WAx6","ignRSO","iMw4ct","iw7X1o","i2GIB1","iowgG7","iUnvoa","iEGC5D","i+7pxM","ic7UKA","iIG9kH","isn098","iYwNth","ikGdOj","iQ7qgY","iAwzXV","i6nkFu","ie2POX","iKT6gk","iuaZXx","i0BGFS","imTeKI","iS2xkz","iCBE9e","i8ant-","iaaSGd","iGB-o4","iq225L","iWTLxE","iiBbSq","iOascR","iyTB12","i42iBn",
      "0hQkCF","0NXzYK","0xCqf5","03hdRc","0pXNqm","0VQ0+3","0Fh9jQ","0-CUNr","0dCpuT","0JhC6w","0tQvnl","0ZXgJW","0lhIy+","0RCX2f","0BX4by","07QRVJ","0friyg","0LkB29","0vNsbG","018bVB","0nkLuv","0Tr26U","0D8-nZ","09NSJi","0bNnq0","0H8E+p","0rrxjs","0XkeNP","0j8GCN","0PNZYC","0zk6fb","05rPR6","0gSyqO","0M3l+t","0wAcjo","02brN1","0o31C7","0USMYa","0EbVfD","0+A8RM","0cADyA","0Ibo2H","0sShb8","0Y3uVh","0kbWuj","0QAJ6Y","0A3QnV","06S5Ju","0exAuX","0Kmj6k","0uHanx","006tJS","0mm3yI","0SxK2z","0C6Tbe","08H+V-","0aHFCd","0G6mY4","0qxffL","0WmwRE","0i6Yqq","0OHH+R","0ymOj2","04x7Nn",
      "uhPd6e","uNYqu-","uxFzJI","u3eknz","upYU2x","uVP9yS","uFe0VX","u-FNbk","udFgY2","uJevCn","utPCRq","uZYpfR","uleR+L","uRF4qE","uBYXNd","u7PIj4","ufsb+D","uLjsqM","uvKBN7","u1-ija","unjSYo","uTs-C1","uD-2RO","u9KLft","ubKe2V","uH-xyu","ursEVj","uXjnbY","uj-P68","uPK6uh","uzjZJA","u5sGnH","ugVr2Z","uM0cyi","uwzlVv","u2cybU","uo086G","uUVVuB","uEcMJg","u+z1n9","uczu+b","uIchq6","usVoNN","uY0DjC","ukc5Ys","uQzQCP","uA0JR0","u6VWfp","ueutYQ","uKpaCr","uuIjRm","u05Af3","ump++5","uSuTqc","uC5KNF","u8I3jK","uaIw6y","uG5fuJ","uqumJ+","uWpFnf","ui572l","uOIOyW","uypHVT","u4uYbw",
      "KhqDbP","KNloVs","KxMhyp","K39u20","KplWn6","KVqJJb","KF9QuC","K-M56N","KdMyjB","KJ9lNG","Ktqcq9","KZlr+g","Kl91fi","KRMMRZ","KBlVCU","K7q8Yv","KfRFfW","KLWmRl","KvDfCw","K1gwYT","KnWYjJ","KTRHNy","KDgOqf","K9D7++","KbDAnc","KHgjJ5","KrRauK","KXWt6F","Kjg3br","KPDKVQ","KzWTy3","K5R+2m","KgwpnE","KMnCJL","KwGvu4","K27g6d","KonIbn","KUwXV2","KE74yR","K+GR2q","KcGkfS","KI7zRx","KswqCk","KYndYX","Kk7Nj-","KQG0Ne","KAn9qz","K6wU+I","KeTnjh","KK2EN8","KuBxqH","K0ae+A","Km2Gfu","KSTZRV","KCa6CY","K8BPYj","KaBib1","KGaBVo","KqTsyt","KW2b2O","KiaLnM","KOB2JD","Ky2-ua","K4TS67",
      "ehtuNY","eNihjj","exLo+u","e3+DqV","epi5RH","eVtQfA","eF+JYh","e-LWC8","edLrVa","eJ+cb7","ettl2M","eZiyyD","el+8Jt","eRLVnO","eBiM61","e7t1uo","efOwJR","eLZfnq","evEm6n","e1fFu2","enZ7V4","eTOObd","eDfH2E","e9EYyL","ebEtRz","eHfafI","erOjY-","eXZACe","ejf+Nk","ePETjX","ezZK+S","e5O3qx","egvgRf","eMovf+","ewJCYJ","e24pCy","eooRNw","eUv4jT","eE4X+W","e+JIql","ecJdJ3","eI4qnm","esvz6r","eYokuQ","ek4UVK","eQJ9bF","eAo02c","e6vNy5","eeUeVC","eK1xbN","euyE26","e0dnyb","em1PJp","eSU6n0","eCdZ6P","e8yGus","eaybNU","eGdsjv","eqUB+i","eW1iqZ","eidSR9","eOy-fg","ey12YB","e4ULCG",
      "8hxHse","8NmY8-","8xH7lI","836OLz","8pmmEx","8VxFWS","8F6whX","8-HfPk","8dHKA2","8J630n","8tx+dq","8ZmTTR","8l6jwL","8RHA4E","8Bmtpd","87xaH4","8fSJwD","8L3W4M","8vA5p7","81bQHa","8n3oAo","8TSD01","8DbudO","89AhTt","8bAMEV","8Hb1Wu","8rS8hj","8X3VPY","8jbls8","8PAy8h","8z3rlA","85ScLH","8grZEZ","8MkGWi","8wNPhv","8286PU","8okEsG","8Urn8B","8E8elg","8+NxL9","8cN2wb","8I8L46","8srSpN","8Yk-HC","8k8BAs","8QNi0P","8Akbd0","86rsTp","8eQXAQ","8KXI0r","8uCRdm","80h4T3","8mXCw5","8SQp4c","8ChgpF","88CvHK","8aC0sy","8GhN8J","8qQUl+","8WX9Lf","8ihzEl","8OCkWW","8yXdhT","84QqPw",
      "ChuO0F","CNp7AK","CxIYT5","C35Hdc","Cppf4m","CVuww3","CF5FHQ","C-Impr","CdIT8T","CJ5+sw","Ctu3Ll","CZpKlW","Cl5aW+","CRItEf","CBpAPy","C7ujhJ","CfVQWg","CL05E9","CvzWPG","C1cJhB","Cn0h8v","CTVusU","CDcDLZ","C9zoli","CbzV40","CHc8wp","CrV1Hs","CX0MpP","Cjcc0N","CPzrAC","Cz0yTb","C5Vld6","Cgs64O","CMjPwt","CwKGHo","C2-Zp1","Cojx07","CUseAa","CE-nTD","C+KEdM","CcK-WA","CI-SEH","CssLP8","CYj2hh","Ck-s8j","CQKbsY","CAjiLV","C6sBlu","CeP48X","CKYRsk","CuFILx","C0eXlS","CmYvWI","CSPgEz","CCepPe","C8FCh-","CaF90d","CGeUA4","CqPNTL","CWY0dE","Cieq4q","COFdwR","CyYkH2","C4Pzpn",
      "ShT2pY","SN2LHj","SxBSwu","S3a-4V","Sp2BdH","SVTiTA","SFabAh","S-Bs08","SdBZha","SJaGP7","StTPEM","SZ26WD","SlaElt","SRBnLO","SB2es1","S7Tx8o","Sfw0lR","SLnNLq","SvGUsn","S17982","Snnzh4","STwkPd","SD7dEE","S9GqWL","SbGXdz","SH7ITI","SrwRA-","SXn40e","Sj7Cpk","SPGpHX","SzngwS","S5wv4x","SgRKdf","SMW3T+","SwD+AJ","S2gT0y","SoWjpw","SURAHT","SEgtwW","S+Da4l","ScDHl3","SIgYLm","SsR7sr","SYWO8Q","SkgmhK","SQDFPF","SAWwEc","S6RfW5","SeqMhC","SKl1PN","SuM8E6","S09VWb","Smlllp","SSqyL0","SC9rsP","S8Mc8s","SaMJpU","SG9WHv","Sqq5wi","SWlQ4Z","Si9od9","SOMDTg","SyluAB","S4qh0G",
      "mhU-PP","mN1Shs","mxyLWp","m3d2E0","mp1sL6","mVUblb","mFdi8C","m-yBsN","mdy6HB","mJdPpG","mtUG49","mZ1Zwg","mldxTi","mRyedZ","mB1n0U","m7UEAv","mfv9TW","mLoUdl","mvJN0w","m140AT","mnoqHJ","mTvdpy","mD4k4f","m9Jzw+","mbJ4Lc","mH4Rl5","mrvI8K","mXoXsF","mj4vPr","mPJghQ","mzopW3","m5vCEm","mgOTLE","mMZ+lL","mwE384","m2fKsd","moZaPn","mUOth2","mEfAWR","m+EjEq","mcEOTS","mIf7dx","msOY0k","mYZHAX","mkffH-","mQEwpe","mAZF4z","m6OmwI","metVHh","mKi8p8","muL14H","m0+MwA","mmicTu","mStrdV","mC+y0Y","m8LlAj","maLQP1","mG+5ho","mqtWWt","mWiJEO","mi+hLM","mOLulD","myiD8a","m4tos7",
      "Yhg8-j","YNDVrY","YxWMMV","Y3R1iu","YpDrZA","YVgcDH","YFRlQ8","Y-Wyeh","YdW537","YJRQza","YtgJUD","YZDWaM","YlRu7O","YRWhvt","YBDoIo","Y7gDm1","Yf9+7q","YLMTvR","YvlKI2","Y1q3mn","YnMt3d","YT9az4","YDqjUL","Y9lAaE","Ybl7ZI","YHqODz","Yr9HQe","YXMYe-","Yjqw-X","YPlfrk","YzMmMx","Y59FiS","YgaUZ+","YMB9Df","Yw20Qy","Y2TNeJ","YoBd-T","YUaqrw","YETzMl","Y+2kiW","Yc2R7m","YIT4v3","YsaXIQ","YYBImr","YkTg3F","YQ2vzK","YABCU5","Y6apac","Ye7S3N","YKG-zC","Yun2Ub","Y0wLa6","YmGb70","YS7svp","YCwBIs","Y8nimP","YanP-v","YGw6rU","Yq7ZMZ","YWGGii","YiweZg","YOnxD9","YyGEQG","Y47neB",
      "shf1zs","sNEM3P","sxZVa0","s3O8Up","spEyvb","sVfl76","sFOcmN","s-ZrIC","sdZWrG","sJOJ-B","stfQig","sZE5M9","slODDZ","sRZoZi","sBEhev","s7fuQU","sf+3Dl","sLLKZW","sviTeT","s1t+Qw","snLAry","sT+j-J","sDtai+","s9itMf","sbiYv5","sHtH7c","sr+OmF","sXL7IK","sjtFzQ","sPim3r","szLfam","s5+wU3","sgdNvL","sMy07E","sw19md","s2UUI4","soykz2","sUdz3n","sEUqaq","s+1dUR","sc1IDx","sIUXZS","ssd4eX","sYyRQk","skUpre","sQ1C--","sAyviI","s6dgMz","se4Lr8","sKJ2-h","suo-iA","s0vSMH","smJiDV","sS4BZu","sCvsej","s8obQY","saoGzo","sGvZ31","sq46aO","sWJPUt","sivnvD","sOoE7M","syJxm7","s44eIa",
      "Ih8RI-","INN4me","IxkX7z","I3rIvI","IpNgUS","IV8vax","IFrC3k","I-kpzX","IdkUQn","IJr9e2","It80ZR","IZNNDq","IlrdME","IRkqiL","IBNz-4","I78krd","IfhPMM","ILC6iD","IvXZ-a","I1QGr7","InCeQ1","IThxeo","IDQEZt","I9XnDO","IbXSUu","IHQ-aV","Irh23Y","IXCLzj","IjQbIh","IPXsm8","IzCB7H","I5hivA","Ig65Ui","IMHQaZ","IwmJ3U","I2xWzv","IoHuIB","IU6hmG","IExo79","I+mDvg","Icm8M6","IIxVib","Is6M-C","IYH1rN","IkxrQP","IQmces","IAHlZp","I66yD0","Ieb7Qr","IKAOeQ","Iu3HZ3","I0SYDm","ImAwMc","ISbfi5","ICSm-K","I83FrF","Ia3+IJ","IGSTmy","IqbK7f","IWA3v+","IiStUW","IO3aal","IyAj3w","I4bAzT",
      "ch-IeK","cNKXQF","cxj4Dc","c3sRZ5","cpKpi3","cV-CMm","cFsvrr","c-jg-Q","cdjNmw","cJs0IT","ct-9vW","cZKU7l","clskaf","cRjzU+","cBKqzJ","c7-d3y","cfeGa9","cLFZUg","cvY6zB","c1PP3G","cnFnmU","cTeEIv","cDPxvi","c9Ye7Z","cbYLip","cHP2M0","cre-rP","cXFS-s","cjPieC","cPYBQN","czFsD6","c5ebZb","cg5Wit","cMIJMO","cwpQr1","c2u5-o","coIDea","cU5oQ7","cEuhDM","c+puZD","ccp1aH","cIuMUA","cs5Vzh","cYI838","ckuymY","cQplIj","cAIcvu","c65r7V","cecYmk","cKzHIX","cu0OvS","c0V77x","cmzFaz","cScmUI","cCVfz-","c80w3e","ca03e4","cGVKQd","cqcTDE","cWz+ZL","ciVAiR","cO0jMq","cyzarn","c4ct-2",
      "6h7xXs","6NGeFP","6xnnO0","63wEgp","6pG69b","6V7Pt6","6FwGKN","6-nZkC","6dns5G","6JwbxB","6t7iGg","6ZGBo9","6lw-1Z","6RnSBi","6BGLSv","6772cU","6fav1l","6LBgBW","6v2pST","61TCcw","6nB45y","6TaRxJ","6DTIG+","692Xof","6b2q95","6HTdtc","6rakKF","6XBzkK","6jT9XQ","6P2UFr","6zBNOm","65a0g3","6g9f9L","6MMwtE","6wlFKd","62qmk4","6oMOX2","6U97Fn","6EqYOq","6+lHgR","6cla1x","6IqtBS","6s9ASX","6YMjck","6kqT5e","6Ql+x-","6AM3GI","669Koz","6egh58","6KDuxh","6uWDGA","60RooH","6mDQ1V","6Sg5Bu","6CRWSj","68WJcY","6aWcXo","6GRrF1","6qgyOO","6WDlgt","6iRV9D","6OW8tM","6yD1K7","64gMka",
      "Ah4Exj","ANJn5Y","AxoeoV","A3vxGu","ApJZBA","AV4G1H","AFvPc8","A-o6Sh","AdoBF7","AJviXa","At4bgD","AZJsOM","Alv2tO","ARoL9t","ABJSko","A74-K1","AfdCtq","ALyp9R","Av1gk2","A1UvKn","AnyXFd","ATdIX4","ADURgL","A914OE","Ab1zBI","AHUk1z","Arddce","AXyqS-","AjU0xX","AP1N5k","AzyUox","A5d9GS","Ag+mB+","AMLF1f","Awiwcy","A2tfSJ","AoLHxT","AU+Y5w","AEt7ol","A+iOGW","Acijtm","AItA93","As+tkQ","AYLaKr","AktKFF","AQi3XK","AAL+g5","A6+TOc","AefoFN","AKEDXC","AuZugb","A0OhO6","AmEJt0","ASfW9p","ACO5ks","A8ZQKP","AaZlxv","AGOy5U","AqfroZ","AWEcGi","AiOMBg","AOZ119","AyE8cG","A4fVSB",
      "QhbaSK","QNAtcF","Qx3A1c","Q3SjB5","QpATG3","QVb+om","QFS35r","Q-3KxQ","Qd3fKw","QJSwkT","QtbF9W","QZAmtl","QlSOOf","QR37g+","QBAYXJ","Q7bHFy","Qf6cO9","QLHrgg","QvmyXB","Q1xlFG","QnHVKU","QT68kv","QDx19i","Q9mMtZ","QbmhGp","QHxuo0","Qr6D5P","QXHoxs","QjxQSC","QPm5cN","QzHW16","Q56JBb","QghsGt","QMCboO","QwXi51","Q2QBxo","QoC-Sa","QUhSc7","QEQL1M","Q+X2BD","QcXxOH","QIQegA","QshnXh","QYCEF8","QkQ6KY","QQXPkj","QACG9u","Q6hZtV","Qe8qKk","QKNdkX","Qukk9S","Q0rztx","QmN9Oz","QS8UgI","QCrNX-","Q8k0Fe","QakvS4","QGrgcd","Qq8p1E","QWNCBL","Qir4GR","QOkRoq","QyNI5n","Q48Xx2",
      "khcjk-","kNzAKe","kx0ttz","k3Va9I","kpzKgS","kVc3Ox","kFV+Fk","k-0TXX","kd0mcn","kJVFS2","ktcwBR","kZzf1q","klVHoE","kR0YGL","kBz7x4","k7cO5d","kf5loM","kLIyGD","kvprxa","k1uc57","knIMc1","kT51So","kDu8Bt","k9pV1O","kbpogu","kHuDOV","kr5uFY","kXIhXj","kjuJkh","kPpWK8","kzI5tH","k55Q9A","kgeBgi","kMFiOZ","kwYbFU","k2PsXv","koF2kB","kUeLKG","kEPSt9","k+Y-9g","kcYEo6","kIPnGb","kseexC","kYFx5N","kkPZcP","kQYGSs","kAFPBp","k6e610","ke-zcr","kKKkSQ","kujdB3","k0sq1m","kmK0oc","kS-NG5","kCsUxK","k8j95F","kajCkJ","kGspKy","kq-gtf","kWKv9+","kisXgW","kOjIOl","kyKRFw","k4-4XT",
      "2hJ62-","2N4Pye","2xvGVz","23oZbI","2p4x6S","2VJeux","2FonJk","2-vEnX","2dv-+n","2JoSq2","2tJLNR","2Z42jq","2losYE","2RvbCL","2B4iR4","27JBfd","2fy4YM","2LdRCD","2vUIRa","211Xf7","2ndv+1","2Tygqo","2D1pNt","29UCjO","2bU96u","2H1UuV","2ryNJY","2Xd0nj","2j1q2h","2PUdy8","2zdkVH","25yzbA","2gLO6i","2M+7uZ","2wtYJU","22iHnv","2o+f2B","2ULwyG","2EiFV9","2+tmbg","2ctTY6","2Ii+Cb","2sL3RC","2Y+KfN","2kia+P","2Qttqs","2A+ANp","26Ljj0","2eEQ+r","2Kf5qQ","2uOWN3","20ZJjm","2mfhYc","2SEuC5","2CZDRK","28OofF","2aOV2J","2GZ8yy","2qE1Vf","2WfMb+","2iZc6W","2OOrul","2yfyJw","24ElnT",
      "whGZqK","wN7G+F","wxwPjc","w3n6N5","wp7EC3","wVGnYm","wFnefr","w-wxRQ","wdw2yw","wJnL2T","wtGSbW","wZ7-Vl","wlnBuf","wRwi6+","wB7bnJ","w7GsJy","wfBXu9","wLaI6g","wvTRnB","w124JG","wnaCyU","wTBp2v","wD2gbi","w9TvVZ","wbT0Cp","wH2NY0","wrBUfP","wXa9Rs","wj2zqC","wPTk+N","wzadj6","w5BqNb","wgMHCt","wM9YYO","wwq7f1","w2lORo","wo9mqa","wUMF+7","wElwjM","w+qfND","wcqKuH","wIl36A","wsM+nh","wY9TJ8","wkljyY","wQqA2j","wA9tbu","w6MaVV","weDJyk","wKgW2X","wuR5bS","w0WQVx","wmgouz","wSDD6I","wCWun-","w8RhJe","waRMq4","wGW1+d","wqD8jE","wWgVNL","wiWlCR","wORyYq","wygrfn","w4DcR2",
      "MhzTRj","MNc+fY","MxV3YV","M30KCu","MpcaNA","MVztjH","MF0A+8","M-Vjqh","MdVOJ7","MJ07na","MtzY6D","MZcHuM","Ml0fVO","MRVwbt","MBcF2o","M7zmy1","MfIVVq","ML58bR","Mvu122","M1pMyn","Mn5cJd","MTIrn4","MDpy6L","M9uluE","MbuQNI","MHp5jz","MrIW+e","MX5Jq-","MjphRX","MPuufk","Mz5DYx","M5IoCS","MgF-N+","MMeSjf","MwPL+y","M2Y2qJ","MoesRT","MUFbfw","MEYiYl","M+PBCW","McP6Vm","MIYPb3","MsFG2Q","MYeZyr","MkYxJF","MQPenK","MAen65","M6FEuc","MeK9JN","MK-UnC","MusN6b","M0j0u6","Mm-qV0","MSKdbp","MCjk2s","M8szyP","Mas4Rv","MGjRfU","MqKIYZ","MW-XCi","MijvNg","MOsgj9","My-p+G","M4KCqB",
      "ghAKns","gNb3JP","gxS+u0","g33T6p","gpbjbb","gVAAV6","gF3tyN","g-Sa2C","gdSHfG","gJ3YRB","gtA7Cg","gZbOY9","gl3mjZ","gRSFNi","gBbwqv","g7Af+U","gfHMjl","gL61NW","gvx8qT","g1mV+w","gn6lfy","gTHyRJ","gDmrC+","g9xcYf","gbxJb5","gHmWVc","grH5yF","gX6Q2K","gjmonQ","gPxDJr","gz6uum","g5Hh63","ggC2bL","gMhLVE","gwQSyd","g2X-24","gohBn2","gUCiJn","gEXbuq","g+Qs6R","gcQZjx","gIXGNS","gsCPqX","gYh6+k","gkXEfe","gQQnR-","gAheCI","g6CxYz","geN0f8","gK8NRh","gurUCA","g0k9YH","gm8zjV","gSNkNu","gCkdqj","g8rq+Y","garXno","gGkIJ1","gqNRuO","gW846t","gikCbD","gOrpVM","gy8gy7","g4Nv2a",
      "+hEr4K","+NfcwF","+xOlHc","+3Zyp5","+pf803","+VEVAm","+FZMTr","+-O1dQ","+dOuWw","+JZhET","+tEoPW","+ZfDhl","+lZ58f","+ROQs+","+BfJLJ","+7EWly","+fLt89","+L+asg","+vtjLB","+1iAlG","+n++WU","+TLTEv","+DiKPi","+9t3hZ","+btw0p","+HifA0","+rLmTP","+X+Fds","+ji74C","+PtOwN","+z+HH6","+5LYpb","+gyd0t","+MdqAO","+wUzT1","+21kdo","+odU4a","+Uy9w7","+E10HM","++UNpD","+cUg8H","+I1vsA","+syCLh","+Ydpl8","+k1RWY","+QU4Ej","+AdXPu","+6yIhV","+eJbWk","+K4sEX","+uvBPS","+0oihx","+m4S8z","+SJ-sI","+Co2L-","+8vLle","+ave44","+Goxwd","+qJEHE","+W4npL","+ioP0R","+Ov6Aq","+y4ZTn","+4JGd2",
      "EhDyE-","ENglWe","ExRchz","E3WrPI","Epg1sS","EVDM8x","EFWVlk","E-R8LX","EdRDwn","EJWo42","EtDhpR","EZguHq","ElWWAE","ERRJ0L","EBgQd4","E7D5Td","EfMAAM","EL9j0D","Evqada","E1ltT7","En93w1","ETMK4o","EDlTpt","E9q+HO","EbqFsu","EHlm8V","ErMflY","EX9wLj","EjlYEh","EPqHW8","Ez9OhH","E5M7PA","EgBksi","EMaz8Z","EwTqlU","E22dLv","EoaNEB","EUB0WG","EE29h9","E+TUPg","EcTpA6","EI2C0b","EsBvdC","EYagTN","Ek2IwP","EQTX4s","EAa4pp","E6BRH0","EeGiwr","EK7B4Q","Euwsp3","E0nbHm","Em7LAc","ESG205","ECn-dK","E8wSTF","EawnEJ","EGnEWy","EqGxhf","EW7eP+","EinGsW","EOwZ8l","Ey76lw","E4GPLT",
      "UhKgLs","UN-vlP","UxsC80","U3jpsp","Up-RPb","UVK4h6","UFjXWN","U-sIEC","UdsdTG","UJjqdB","UtKz0g","UZ-kA9","UljUHZ","URs9pi","UB-04v","U7KNwU","UfFeHl","ULexpW","UvPE4T","U1Ynww","UnePTy","UTF6dJ","UDYZ0+","U9PGAf","UbPbP5","UHYshc","UrFBWF","UXeiEK","UjYSLQ","UPP-lr","Uze28m","U5FLs3","UgIuPL","UM5hhE","UwuoWd","U2pDE4","Uo55L2","UUIQln","UEpJ8q","U+uWsR","UcurHx","UIpcpS","UsIl4X","UY5ywk","Ukp8Te","UQuVd-","UA5M0I","U6I1Az","UezwT8","UKcfdh","UuVm0A","U00FAH","Umc7HV","USzOpu","UC0H4j","U8VYwY","UaVtLo","UG0al1","Uqzj8O","UWcAst","Ui0+PD","UOVThM","UycKW7","U4z3Ea",
      "ohNpdj","oN8CTY","oxrvAV","o3kg0u","op8IpA","oVNXHH","oFk4w8","o-rR4h","odrkl7","oJkzLa","otNqsD","oZ8d8M","olkNhO","oRr0Pt","oB89Eo","o7NUW1","ofCnhq","oLhEPR","ovQxE2","o1XeWn","onhGld","oTCZL4","oDX6sL","o9QP8E","obQipI","oHXBHz","orCswe","oXhb4-","ojXLdX","oPQ2Tk","ozh-Ax","o5CS0S","ogHDp+","oM6oHf","owxhwy","o2mu4J","oo6WdT","oUHJTw","oEmQAl","o+x50W","ocxyhm","oImlP3","osHcEQ","oY6rWr","okm1lF","oQxMLK","oA6Vs5","o6H88c","oeAFlN","oKbmLC","ouSfsb","o03w86","ombYh0","oSAHPp","oC3OEs","o8S7WP","oaSAdv","oG3jTU","oqAaAZ","oWbt0i","oi33pg","oOSKH9","oybTwG","o4A+4B"
    };

    static SmartPtr<icl8u> indices;
    if(!indices){
      const char *lut = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+-\0";
      indices = new icl8u[256];
      for(const char *p=lut;*p;++p){
        indices.get()[(int)*p] = (icl8u)(p-lut);
      }
    }
    
    if(idx < 0 || idx > 4095) throw ICLException("invalid bch code ID (allowed: 0 <= index <= 4095)");
    
    const char *pc = bch_codes[idx];
    
    BCHCode c;
    for(int i=0;i<6;++i){
      //int idx = (int)(std::find(lut,lut+64,pc[i]) - lut);
      int idx = indices.get()[(int)pc[i]];
      for(int j=0;j<6;++j){
        c[i*6+j] = idx & (1<<j);
      }
      // next 6 bits of c are used from
    }
    return c;
  }
  // }}}
  
  DecodedBCHCode decode_bch(const BCHCode &code){
    // {{{ open
    BENCHMARK_THIS_FUNCTION;
    
    int64_t c=0,o=1;
    for(int i=0;i<36;++i){
      if(code[35-i]){
        c |= o << i;
      }
    }
    c ^= 0x8f80b8750ll;
    int err = BCHCoder::getInstance()->decode_bch_2(c);

    DecodedBCHCode ret;
    ret.origCode = code;
    
    if(err > BCHCoder::BCH_DEFAULT_T){
      ret.errors = 36;
      ret.id = -1;
    }else{
      ret.errors = err;
      ret.id = 0;
      for(int i=24,j=0;i<36;++i,++j){
        if(c & (o<<i)){
          ret.id |= (1 << j);
        }
      }
      ret.correctedCode = err ? encode_bch(ret.id) : ret.origCode;
    }
    return ret;
  }
  // }}}



  
  DecodedBCHCode decode_bch(const icl8u data[36]){
    BCHCode code(0);
    for(unsigned int i=0;i<36;++i){
      code[i] = data[i];
    }
    return decode_bch(code);
  }
  
  
  static BCHCode code_from_image(const Img8u &image, bool useROI) throw (ICLException){
    BCHCode code(0);
    ICLASSERT_THROW(image.getChannels(), ICLException("code_from_image(const Img8u&,bool): input image has not channels"));
    if(useROI){
      ICLASSERT_THROW(image.getROISize().getDim() == 36, ICLException("code_from_image(const Img8u&,true): images ROI dim must be 36"));
      Img8u::const_roi_iterator it = image.beginROI(0);
      for(unsigned int i=0;i<36;++i,++it){
        code[i] = *it;
      }
    }else{
      ICLASSERT_THROW(image.getSize().getDim() == 36, ICLException("decode_bch(const Img8u&,false): image dim must be 36"));
      Img8u::const_iterator it = image.begin(0);
      for(unsigned int i=0;i<36;++i,++it){
        code[i] = *it;
      }
    }    
    return code;
  }
  
  DecodedBCHCode decode_bch(const Img8u &image, bool useROI) throw (ICLException){
    return decode_bch(code_from_image(image,useROI));
  }
  
  
  BCHCode rotate_code(const BCHCode &in){
    BCHCode out(0);
    for(int y=0;y<6;++y){
      for(int x=0;x<6;++x){
        if(in[x+6*y]) out.set(5+6*x-y);
      }
    }
    return out;
  }

  DecodedBCHCode2D decode_bch_2D(const Img8u &image, int maxID, bool useROI) throw (ICLException){
    BCHCode last = code_from_image(image,useROI);
    DecodedBCHCode2D best = decode_bch(last);
    if(best.id > maxID){
      best.errors = 36;
      best.id = -1;
    }
    best.rot = DecodedBCHCode2D::Rot0;
    if(!best.errors) return best;

    for(int i=1;i<4;++i){
      last = rotate_code(last);
      DecodedBCHCode2D curr = decode_bch(last);
      if(curr.id > maxID) continue; //curr.errors = 36;
      curr.rot = (DecodedBCHCode2D::Rotation)i;
      if(!curr.errors) return curr;
      if(curr < best){
        best = curr;
      }
    }
    return best;
  }

  std::ostream &operator<<(std::ostream &s, const DecodedBCHCode2D::Rotation &r){
    return s << (r == DecodedBCHCode2D::Rot0 ? "0 Degree" :
                 r == DecodedBCHCode2D::Rot90 ? "90 Degree" :
                 r == DecodedBCHCode2D::Rot180 ? "180 Degree" :
                 r == DecodedBCHCode2D::Rot270 ? "270 Degree" :
                 "??? Degree");
  }

} 
