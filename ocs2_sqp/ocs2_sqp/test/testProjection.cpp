/******************************************************************************
Copyright (c) 2020, Farbod Farshidian. All rights reserved.

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

#include <gtest/gtest.h>

#include "ocs2_sqp/ConstraintProjection.h"

#include <ocs2_oc/test/testProblemsGeneration.h>

TEST(test_projection, testProjectionQR) {
  const auto constraint = ocs2::getRandomConstraints(30, 20, 10);

  auto result = ocs2::qrConstraintProjection(constraint);
  const auto projection = std::move(result.first);
  const auto pseudoInverse = std::move(result.second);

  // range of Pu is in null-space of D
  ASSERT_TRUE((constraint.dfdu * projection.dfdu).isZero());

  // D * Px cancels the C term
  ASSERT_TRUE((constraint.dfdx + constraint.dfdu * projection.dfdx).isZero());

  // D * Pe cancels the e term
  ASSERT_TRUE((constraint.f + constraint.dfdu * projection.f).isZero());

  ASSERT_EQ(pseudoInverse.rows(), constraint.dfdu.rows());
  ASSERT_EQ(pseudoInverse.cols(), constraint.dfdu.cols());

  ASSERT_TRUE((pseudoInverse * constraint.dfdu.transpose()).isIdentity());
  ASSERT_TRUE((pseudoInverse.transpose() * constraint.dfdx).isApprox(-projection.dfdx));
  ASSERT_TRUE((pseudoInverse.transpose() * constraint.f).isApprox(-projection.f));
}

TEST(test_projection, testProjectionLU) {
  const auto constraint = ocs2::getRandomConstraints(30, 20, 10);

  auto result = ocs2::luConstraintProjection(constraint);
  ASSERT_EQ(result.second.rows(), 0);
  ASSERT_EQ(result.second.cols(), 0);

  const auto projection = std::move(result.first);

  // range of Pu is in null-space of D
  ASSERT_TRUE((constraint.dfdu * projection.dfdu).isZero());

  // D * Px cancels the C term
  ASSERT_TRUE((constraint.dfdx + constraint.dfdu * projection.dfdx).isZero());

  // D * Pe cancels the e term
  ASSERT_TRUE((constraint.f + constraint.dfdu * projection.f).isZero());

  auto resultWithPseudoInverse = ocs2::luConstraintProjection(constraint, true);
  const auto projectionWithPseudoInverse = std::move(resultWithPseudoInverse.first);
  ASSERT_TRUE(projection.f.isApprox(projectionWithPseudoInverse.f));
  ASSERT_TRUE(projection.dfdx.isApprox(projectionWithPseudoInverse.dfdx));
  ASSERT_TRUE(projection.dfdu.isApprox(projectionWithPseudoInverse.dfdu));

  const auto pseudoInverse = std::move(resultWithPseudoInverse.second);
  ASSERT_EQ(pseudoInverse.rows(), constraint.dfdu.rows());
  ASSERT_EQ(pseudoInverse.cols(), constraint.dfdu.cols());

  ASSERT_TRUE((pseudoInverse * constraint.dfdu.transpose()).isIdentity());
  ASSERT_TRUE((pseudoInverse.transpose() * constraint.dfdx).isApprox(-projection.dfdx));
  ASSERT_TRUE((pseudoInverse.transpose() * constraint.f).isApprox(-projection.f));
}

TEST(test_projection, testProjectionMultiplierCoefficients) {
  const size_t stateDim = 30;
  const size_t inputDim = 20;
  const size_t constraintDim = 10;

  const auto cost = ocs2::getRandomCost(stateDim, inputDim);
  const auto dynamics = ocs2::getRandomDynamics(stateDim, inputDim);
  const auto constraint = ocs2::getRandomConstraints(stateDim, inputDim, constraintDim);

  auto result = ocs2::qrConstraintProjection(constraint);
  const auto projection = std::move(result.first);
  const auto pseudoInverse = std::move(result.second);

  const auto projectionMultiplierCoefficients = extractProjectionMultiplierCoefficients(dynamics, cost, projection, pseudoInverse);

  const ocs2::matrix_t dfdx = -pseudoInverse * (cost.dfdux + cost.dfduu * projection.dfdx);
  const ocs2::matrix_t dfdu = -pseudoInverse * (cost.dfduu * projection.dfdu);
  const ocs2::matrix_t dfdcostate = -pseudoInverse * dynamics.dfdu.transpose();
  const ocs2::vector_t f = -pseudoInverse * (cost.dfdu + cost.dfduu * projection.f);

  ASSERT_TRUE(projectionMultiplierCoefficients.dfdx.isApprox(dfdx));
  ASSERT_TRUE(projectionMultiplierCoefficients.dfdu.isApprox(dfdu));
  ASSERT_TRUE(projectionMultiplierCoefficients.dfdcostate.isApprox(dfdcostate));
  ASSERT_TRUE(projectionMultiplierCoefficients.f.isApprox(f));
}
