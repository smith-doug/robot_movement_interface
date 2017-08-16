/*
 * Copyright (c) 2017, Doug Smith
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *  Created on: Aug 4, 2017
 *      Author: Doug Smith
 */

#include "keba_rmi_plugin/commands_keba.h"
#include <string>
#include <vector>

namespace keba_rmi_plugin
{

/**
 * Creates a dynamic parameter string from a float vector
 * @param velocity vector of a full Keba dynamic
 * @return string containing "dyn : velAxis accAxis decAxis jerkAxis vel acc dec jerk velOri accOri decOri jerkOri"
 */

/**
 * Checks for velocity type DYN and adds the param entry if found
 * @param cmd_msg [in] the whole message
 * @param telnet_cmd [in,out] the telnet command to add the dyn entry to
 * @return true if a keba DYN entry was found
 */
bool processKebaDyn(const robot_movement_interface::Command& cmd_msg, Command& telnet_cmd)
{
  if (cmd_msg.velocity_type.compare("DYN") == 0)
  {
    telnet_cmd.addParam("dyn", Command::paramsToString(cmd_msg.velocity));
    return true;
  }
  else
    return false;
}

KebaCommandRegister::KebaCommandRegister() :
    commands_registered_(0)
{
  registerCommands();
}

void KebaCommandRegister::initialize()
{
  num_main_joints_ = 6;
  num_aux_joints_ = 1;

  registerCommands();
}

void KebaCommandRegister::initialize(const std::vector<std::string> &joints)
{
  initialize();
}

void KebaCommandRegister::registerCommands()
{
  if (commands_registered_)
    return;

  command_handlers_.emplace_back(new KebaCommandPtpJoints());
  command_handlers_.emplace_back(new KebaCommandLinQuat());
  command_handlers_.emplace_back(new KebaCommandLinEuler());
  command_handlers_.emplace_back(new KebaCommandAbort());

  //Sample command for lambda usage
  robot_movement_interface::Command cmd;
  cmd.command_type = "TEST";

  CommandHandler chtest(cmd, [](const robot_movement_interface::Command& cmd_msg)
  {
    return Command(Command::CommandType::Cmd, cmd_msg.command_type, cmd_msg.pose_type);
  });

  command_handlers_.emplace_back(new CommandHandler(std::move(chtest)));

  commands_registered_ = true;

}

KebaCommandPtpJoints::KebaCommandPtpJoints()
{
  robot_movement_interface::Command cmd;
  cmd.command_type = "PTP";
  cmd.pose_type = "JOINTS";
  cmd.pose =
  { 0,1,2,3,4,5,6};
//  cmd.velocity_type = "%";
//  cmd.velocity =
//  { 0,1,2,3,4,5,6};

  sample_command_ = cmd;
}

bool KebaCommandPtpJoints::processMsg(const robot_movement_interface::Command& cmd_msg, Command& telnet_cmd) const
{
  std::string command_str = "joint move";
  std::string command_params = "";

  Command cmd(Command::Command::CommandType::Cmd);
  cmd.setCommand("joint move", Command::paramsToString(cmd_msg.pose));

  bool had_a_keba_dyn = processKebaDyn(cmd_msg, cmd);

  if (!had_a_keba_dyn)
  {
    if (cmd_msg.velocity_type.compare("ROS") == 0)
    {
      cmd.addParam("velros", Command::paramsToString(cmd_msg.velocity));

    }
    if (cmd_msg.acceleration_type.compare("ROS") == 0)
    {
      cmd.addParam("accros", Command::paramsToString(cmd_msg.acceleration));
    }
  }

  telnet_cmd = cmd;
  return true;
}

KebaCommandLinQuat::KebaCommandLinQuat()
{
  robot_movement_interface::Command cmd;
  cmd = robot_movement_interface::Command();
  cmd.command_type = "LIN";
  cmd.pose_type = "QUATERNION";
  cmd.pose =
  { 0,1,2,3,4,5,6};
  //cmd.velocity =
  //{ 0};

  sample_command_ = cmd;
}

bool KebaCommandLinQuat::processMsg(const robot_movement_interface::Command& cmd_msg, Command& telnet_cmd) const
{
  Command cmd(Command::CommandType::Cmd);

  auto pose_temp = cmd_msg.pose;

  pose_temp[0] *= 1000.0;
  pose_temp[1] *= 1000.0;
  pose_temp[2] *= 1000.0;

  cmd.setCommand("linq move", Command::paramsToString(pose_temp));

  if (cmd_msg.velocity_type.compare("DYN") == 0)
  {
    cmd.addParam("dyn", Command::paramsToString(cmd_msg.velocity));
  }

  telnet_cmd = cmd;
  return true;

}

KebaCommandLinEuler::KebaCommandLinEuler()
{
  robot_movement_interface::Command cmd;
  cmd = robot_movement_interface::Command();
  cmd.command_type = "LIN";
  cmd.pose_type = "EULER_INTRINSIC_ZYX";
  cmd.pose =
  { 0,1,2,3,4,5};

  sample_command_ = cmd;
}

bool KebaCommandLinEuler::processMsg(const robot_movement_interface::Command& cmd_msg, Command& telnet_cmd) const
{

  Command cmd(Command::CommandType::Cmd);

  auto pose_temp = cmd_msg.pose;

  pose_temp[0] *= 1000.0;
  pose_temp[1] *= 1000.0;
  pose_temp[2] *= 1000.0;

  cmd.setCommand("lin move", Command::paramsToString(pose_temp));

  processKebaDyn(cmd_msg, cmd);

  telnet_cmd = cmd;
  return true;
}

} //namespace keba_rmi_plugin

keba_rmi_plugin::KebaCommandAbort::KebaCommandAbort()
{
  robot_movement_interface::Command cmd;
  cmd.command_type = "ABORT";
  sample_command_ = cmd;
}

bool keba_rmi_plugin::KebaCommandAbort::processMsg(const robot_movement_interface::Command& cmd_msg,
                                                   Command& telnet_cmd) const
{
  Command cmd(Command::CommandType::Get);
  cmd.setCommand("abort", "");
  telnet_cmd = cmd;

  return true;
}
