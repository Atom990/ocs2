/******************************************************************************
Copyright (c) 2017, Farbod Farshidian. All rights reserved.

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

#ifndef MRT_ROS_DUMMY_DOUBLE_INTEGRATOR_OCS2_H_
#define MRT_ROS_DUMMY_DOUBLE_INTEGRATOR_OCS2_H_

#include <ros/ros.h>
#include <sensor_msgs/JointState.h>

#include <ocs2_comm_interfaces/test/MRT_ROS_Dummy_Loop.h>

#include "ocs2_double_integrator_example/definitions.h"

namespace ocs2 {
namespace double_integrator {

class MRT_ROS_Dummy_Linear_System : public MRT_ROS_Dummy_Loop<double_integrator::STATE_DIM_, double_integrator::INPUT_DIM_> {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	typedef MRT_ROS_Dummy_Loop<double_integrator::STATE_DIM_, double_integrator::INPUT_DIM_> BASE;

	/**
	 * Constructor.
	 *
	 * @param [in] mrtPtr
	 * @param [in] mrtDesiredFrequency: MRT loop frequency in Hz. This should always set to a positive number.
	 * @param [in] mpcDesiredFrequency: MPC loop frequency in Hz. If set to a positive number, MPC loop
	 * will be simulated to run by this frequency. Note that this might not be the MPC's realtime frequency.
	 */
	MRT_ROS_Dummy_Linear_System(
			const mrt_ptr_t& mrtPtr,
			const scalar_t& mrtDesiredFrequency,
			const scalar_t& mpcDesiredFrequency,
			controlled_system_base_t* system = nullptr,
			Rollout_Settings rolloutSettings = Rollout_Settings())
	: BASE(mrtPtr, mrtDesiredFrequency, mpcDesiredFrequency, system, rolloutSettings)
	{}

	/**
	 * Destructor.
	 */
	virtual ~MRT_ROS_Dummy_Linear_System() = default;

	/**
	 * The initialization of the observation
	 *
	 * @param [in] initObservation: The initial observation.
	 */
	virtual void init(const system_observation_t& initObservation) override {

		BASE::init(initObservation);
	}

protected:
	/**
	 * Launches the visualization node
	 *
	 * @param [in] argc: command line number of inputs.
	 * @param [in] argv: command line inputs' value.
	 */
	virtual void launchVisualizerNode(
			int argc, char* argv[]) override {

		ros::init(argc, argv, "double_integrator_visualization_node");
		ros::NodeHandle n;
		jointPublisher_ = n.advertise<sensor_msgs::JointState>("joint_states", 1);
		ROS_INFO_STREAM("Waiting for visualization subscriber ...");
		while(ros::ok() && jointPublisher_.getNumSubscribers() == 0)
			ros::Rate(100).sleep();
		ROS_INFO_STREAM("Visualization subscriber is connected.");
	}

	/**
	 * Visualizes the current observation.
	 *
	 * @param [in] observation: The current observation.
	 */
	virtual void publishVisualizer(
			const system_observation_t& observation) override {

		sensor_msgs::JointState joint_state;
		joint_state.header.stamp = ros::Time::now();
		joint_state.name.resize(1);
		joint_state.position.resize(1);
		joint_state.name[0] ="slider_to_cart";
		joint_state.position[0] = observation.state()(0);

		jointPublisher_.publish(joint_state);
		std::cout << "Obs " << observation.state()[0] << " , " << observation.state()[1] << std::endl;
		std::cout << "Input " << observation.input()[0] << std::endl;
	}

private:
	ros::Publisher jointPublisher_;
};

} // namespace double_integrator
} // namespace ocs2

#endif /* MRT_ROS_DUMMY_DOUBLE_INTEGRATOR_OCS2_H_ */
