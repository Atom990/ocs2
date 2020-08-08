
#include <ocs2_core/integration/OdeFunc.h>
#include <ocs2_core/loopshaping/dynamics/LoopshapingFilterDynamics.h>

namespace ocs2 {

void LoopshapingFilterDynamics::integrate(scalar_t dt, const vector_t& input) {
  // Set up ODE with ZOH input
  OdeFunc ode_fun(std::bind(&LoopshapingFilterDynamics::computeFlowMap, this, std::placeholders::_1, std::placeholders::_2, input));

  vector_array_t stateTrajectory;
  Observer observer(&stateTrajectory);
  integrator_->integrateAdaptive(ode_fun, observer, filter_state_, 0.0, dt, dt);

  filter_state_ = stateTrajectory.back();
}

vector_t LoopshapingFilterDynamics::computeFlowMap(scalar_t time, const vector_t& filter_state, const vector_t& input) const {
  const auto& filter = loopshapingDefinition_->getInputFilter();
  vector_t filterStateDerivative(filter_state.rows());
  switch (loopshapingDefinition_->getType()) {
    case LoopshapingType::outputpattern:
      filterStateDerivative.noalias() = filter.getA() * filter_state;
      filterStateDerivative.noalias() += filter.getB() * input;
      return filterStateDerivative;
    case LoopshapingType::inputpattern: /* fall through */
    case LoopshapingType::eliminatepattern:
      filterStateDerivative.noalias() = filter.getA() * filter_state;
      filterStateDerivative.noalias() += filter.getB() * input.tail(filter.getNumInputs());
      return filterStateDerivative;
    default:
      throw std::runtime_error("[LoopshapingFilterDynamics::computeFlowMap] invalid loopshaping type");
  }
}

}  // namespace ocs2
