/******************************************************************************
Copyright (c) 2020, Ruben Grandia. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#include <ocs2_core/loopshaping/LoopshapingPreComputation.h>
#include <ocs2_core/loopshaping/constraint/LoopshapingConstraintInputPattern.h>

namespace ocs2 {

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
VectorFunctionLinearApproximation LoopshapingConstraintInputPattern::getLinearApproximation(scalar_t t, const vector_t& x,
                                                                                            const vector_t& u,
                                                                                            const PreComputation& preComp) const {
  const auto& preCompLS = cast<LoopshapingPreComputation>(preComp);
  const auto& preComp_system = preCompLS.getSystemPreComputation();
  const auto& x_system = preCompLS.getSystemState();
  const auto& u_system = preCompLS.getSystemInput();
  const auto& x_filter = preCompLS.getFilterState();
  const auto& u_filter = preCompLS.getFilteredInput();

  const auto g_system = systemConstraint_->getLinearApproximation(t, x_system, u_system, preComp_system);

  VectorFunctionLinearApproximation g;
  g.f = std::move(g_system.f);

  g.dfdx.resize(g.f.rows(), x.rows());
  g.dfdx.leftCols(x_system.rows()) = g_system.dfdx;
  g.dfdx.rightCols(x_filter.rows()).setZero();

  g.dfdu.resize(g.f.rows(), u.rows());
  g.dfdu.leftCols(u_system.rows()) = g_system.dfdu;
  g.dfdu.rightCols(u_filter.rows()).setZero();

  return g;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
VectorFunctionQuadraticApproximation LoopshapingConstraintInputPattern::getQuadraticApproximation(scalar_t t, const vector_t& x,
                                                                                                  const vector_t& u,
                                                                                                  const PreComputation& preComp) const {
  const auto& preCompLS = cast<LoopshapingPreComputation>(preComp);
  const auto& preComp_system = preCompLS.getSystemPreComputation();
  const auto& x_system = preCompLS.getSystemState();
  const auto& u_system = preCompLS.getSystemInput();
  const auto& x_filter = preCompLS.getFilterState();
  const auto& u_filter = preCompLS.getFilteredInput();

  const auto h_system = systemConstraint_->getQuadraticApproximation(t, x_system, u_system, preComp_system);

  VectorFunctionQuadraticApproximation h;
  h.f = std::move(h_system.f);

  h.dfdx.resize(h.f.rows(), x.rows());
  h.dfdx.leftCols(x_system.rows()) = h_system.dfdx;
  h.dfdx.rightCols(x_filter.rows()).setZero();

  h.dfdu.resize(h.f.rows(), u.rows());
  h.dfdu.leftCols(u_system.rows()).noalias() = h_system.dfdu;
  h.dfdu.rightCols(u_filter.rows()).setZero();

  h.dfdxx.resize(h.f.rows());
  h.dfduu.resize(h.f.rows());
  h.dfdux.resize(h.f.rows());
  for (size_t i = 0; i < h.f.rows(); i++) {
    h.dfdxx[i].setZero(x.rows(), x.rows());
    h.dfdxx[i].topLeftCorner(x_system.rows(), x_system.rows()) = h_system.dfdxx[i];

    h.dfduu[i].setZero(u.rows(), u.rows());
    h.dfduu[i].topLeftCorner(u_system.rows(), u_system.rows()) = h_system.dfduu[i];

    h.dfdux[i].setZero(u.rows(), x.rows());
    h.dfdux[i].topLeftCorner(u_system.rows(), x_system.rows()).noalias() = h_system.dfdux[i];
  }

  return h;
}

}  // namespace ocs2
