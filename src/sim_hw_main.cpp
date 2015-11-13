/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2015, University of Colorado, Boulder
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Univ of CO, Boulder nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

/* Author: Dave Coleman
   Desc:   Example ros_control main() entry point for controlling robots in ROS
*/

#include <ros_control_boilerplate/generic_hw_control_loop.h>
#include <ros_control_boilerplate/sim_hw_interface.h>
#include <urdf/model.h>

// Get the URDF XML from the parameter server
void loadURDF(ros::NodeHandle& nh, std::string param_name, urdf::Model* &urdf_model)
{
  std::string urdf_string;
  std::string robot_description = "/robot_description";
  urdf_model = new urdf::Model();

  // search and wait for robot_description on param server
  while (urdf_string.empty())
  {
    std::string search_param_name;
    if (nh.searchParam(param_name, search_param_name))
    {
      ROS_INFO_ONCE_NAMED("sim_hw_main", "sim_hw_main node is waiting for model"
                                             " URDF in parameter [%s] on the ROS param server.",
                          search_param_name.c_str());

      nh.getParam(search_param_name, urdf_string);
    }
    else
    {
      ROS_INFO_ONCE_NAMED("sim_hw_main", "sim_hw_main node is waiting for model"
                                             " URDF in parameter [%s] on the ROS param server.",
                          robot_description.c_str());

      nh.getParam(param_name, urdf_string);
    }

    usleep(100000);
  }

  if (!urdf_model->initString(urdf_string))
    ROS_ERROR_STREAM_NAMED("sim_hw_main", "Unable to load URDF model");
  else
    ROS_DEBUG_STREAM_NAMED("sim_hw_main", "Received URDF from param server");
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "sim_hw_interface");
  ros::NodeHandle nh;

  // NOTE: We run the ROS loop in a separate thread as external calls such
  // as service callbacks to load controllers can block the (main) control loop
  ros::AsyncSpinner spinner(3);
  spinner.start();

  urdf::Model* urdf_model;
  loadURDF(nh, "/robot_description", urdf_model);

  // Create the hardware interface specific to your robot
  boost::shared_ptr<ros_control_boilerplate::SimHWInterface> sim_hw_interface;
  sim_hw_interface.reset(new ros_control_boilerplate::SimHWInterface(nh, urdf_model));

  // Start the control loop
  ros_control_boilerplate::GenericHWControlLoop control_loop(nh, sim_hw_interface);

  // Wait until shutdown signal recieved
  ros::waitForShutdown();

  return 0;
}