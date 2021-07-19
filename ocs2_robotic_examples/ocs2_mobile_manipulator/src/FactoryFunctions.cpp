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

#include <pinocchio/fwd.hpp>  // forward declarations must be included first.

#include "ocs2_mobile_manipulator/FactoryFunctions.h"

#include <ocs2_core/misc/LoadData.h>
#include <ocs2_pinocchio_interface/urdf.h>

namespace ocs2 {
namespace mobile_manipulator {

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
PinocchioInterface createPinocchioInterface(const std::string& robotUrdfPath) {
  // add 6 DoF for the floating base
  // pinocchio::JointModelComposite jointComposite(2);
  // jointComposite.addJoint(pinocchio::JointModelTranslation());
  // jointComposite.addJoint(pinocchio::JointModelSphericalZYX());

  // return getPinocchioInterfaceFromUrdfFile(robotUrdfPath, jointComposite);
  return getPinocchioInterfaceFromUrdfFile(robotUrdfPath);
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
PinocchioInterface createPinocchioInterface(const ::urdf::ModelInterfaceSharedPtr& urdfTree, const std::vector<std::string>& jointNames) {
  using joint_pair_t = std::pair<const std::string, std::shared_ptr<::urdf::Joint>>;

  // remove extraneous joints from urdf
  ::urdf::ModelInterfaceSharedPtr newModel = std::make_shared<::urdf::ModelInterface>(*urdfTree);
  for (joint_pair_t& jointPair : newModel->joints_) {
    if (std::find(jointNames.begin(), jointNames.end(), jointPair.first) == jointNames.end()) {
      jointPair.second->type = urdf::Joint::FIXED;
    }
  }

  // // add 6 DoF for the floating base
  // pinocchio::JointModelComposite jointComposite(2);
  // jointComposite.addJoint(pinocchio::JointModelTranslation());
  // jointComposite.addJoint(pinocchio::JointModelSphericalZYX());

  // return getPinocchioInterfaceFromUrdfModel(newModel, jointComposite);
  return getPinocchioInterfaceFromUrdfModel(newModel);
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
MobileManipulatorModelInfo createMobileManipulatorModelInfo(const PinocchioInterface& interface, const ManipulatorModelType& type) {
  const auto& model = interface.getModel();

  MobileManipulatorModelInfoTpl<scalar_t> info;
  info.manipulatorModelType = type;
  info.generalizedCoordinatesNum = 9;
  info.actuatedDofNum = info.generalizedCoordinatesNum;
  info.stateDim = info.generalizedCoordinatesNum;
  info.inputDim = info.actuatedDofNum;

  return info;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
ManipulatorModelType loadManipulatorType(const std::string& configFilePath, const std::string& fieldName) {
  boost::property_tree::ptree pt;
  boost::property_tree::read_info(configFilePath, pt);
  const size_t type = pt.template get<size_t>(fieldName);
  return static_cast<ManipulatorModelType>(type);
}

}  // namespace mobile_manipulator
}  // namespace ocs2
