/*
 *    This file is part of CasADi.
 *
 *    CasADi -- A symbolic framework for dynamic optimization.
 *    Copyright (C) 2010-2014 Joel Andersson, Joris Gillis, Moritz Diehl,
 *                            K.U. Leuven. All rights reserved.
 *    Copyright (C) 2011-2014 Greg Horn
 *
 *    CasADi is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    CasADi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with CasADi; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include "subassign.hpp"

using namespace std;

namespace casadi {

  SubAssign::SubAssign(const MX& x, const MX& y, const Slice& i, const Slice& j) : i_(i), j_(j) {
    setDependencies(x, y);
    casadi_error("not ready");
  }

  SubAssign* SubAssign::clone() const {
    return new SubAssign(*this);
  }

  void SubAssign::evalD(cp_double* input, p_double* output,
                        int* itmp, double* rtmp) {
    evalGen<double>(input, output, itmp, rtmp);
  }

  void SubAssign::evalSX(cp_SXElement* input, p_SXElement* output,
                         int* itmp, SXElement* rtmp) {
    evalGen<SXElement>(input, output, itmp, rtmp);
  }

  template<typename T>
  void SubAssign::evalGen(const T* const* arg, T* const* res,
                          int* itmp, T* rtmp) {
    casadi_error("not ready");
  }

  void SubAssign::spFwd(cp_bvec_t* arg,
                     p_bvec_t* res, int* itmp, bvec_t* rtmp) {
    casadi_error("not ready");
  }

  void SubAssign::spAdj(p_bvec_t* arg,
                     p_bvec_t* res, int* itmp, bvec_t* rtmp) {
    casadi_error("not ready");
  }

  void SubAssign::printPart(std::ostream &stream, int part) const {
    if (part==0) {
      stream << "(";
    } else if (part==1) {
      stream << "[" << i_ << ", " << j_ << "]=";
    } else {
      stream << ")";
    }
  }

  void SubAssign::evalMX(const std::vector<MX>& arg, std::vector<MX>& res) {
    casadi_error("not ready");
  }

  void SubAssign::evalFwd(const std::vector<std::vector<MX> >& fseed,
                          std::vector<std::vector<MX> >& fsens) {
    casadi_error("not ready");
  }

  void SubAssign::evalAdj(const std::vector<std::vector<MX> >& aseed,
                          std::vector<std::vector<MX> >& asens) {
    casadi_error("not ready");
  }

  void SubAssign::generate(const std::vector<int>& arg, const std::vector<int>& res,
                           CodeGenerator& g) const {
    casadi_error("not ready");
  }

} // namespace casadi
