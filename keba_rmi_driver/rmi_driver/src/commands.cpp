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
 *  Created on: Aug 3, 2017
 *      Author: Doug Smith
 */

#include "rmi_driver/commands.h"
#include <ros/ros.h>
#include <robot_movement_interface/Command.h>
#include <vector>
#include <boost/algorithm/string.hpp>

namespace rmi_driver
{

/**
 * Convert a float into a string with no trailing zeroes
 * @todo move it to a util file
 *
 * @param fval float to convert
 * @param precision max number of decimals
 * @return string representation of fval
 */
std::string floatToStringNoTrailing(float fval, int precision)
{
  std::ostringstream oss;
  oss << std::setprecision(precision) << std::fixed;
  oss << fval;
  auto str = oss.str();
  boost::trim_right_if(str, boost::is_any_of("0"));
  boost::trim_right_if(str, boost::is_any_of("."));

  return str;
}

std::string Command::paramsToString(const std::vector<float>& floatVec, int precision)
{
  if (floatVec.empty())
    return "";

  std::ostringstream oss;

  std::for_each(floatVec.begin(), floatVec.end() - 1, [&](const float & fval)
  {
    auto str_val = floatToStringNoTrailing(fval, precision);
    oss << str_val << " ";

  });

  oss << floatToStringNoTrailing(floatVec.back(), precision);

  auto out_str = oss.str();

  return out_str;
}

CommandHandler::CommandHandler(const robot_movement_interface::Command& sample_command, CommandHandlerFunc f) :
    sample_command_(sample_command), process_func_(std::move(f))
{
}

//Returns True if the sample length is > 0 and it doesn't match msg.
//If sample is "" it will return false as this param isn't used in the match.
bool usedAndNotEqual(const std::string &sample, const std::string &msg)
{
  return sample.length() > 0 && sample.compare(msg) != 0;
}

bool usedAndNotEqual(const std::vector<float> &sample, const std::vector<float> &msg)
{
  return sample.size() > 0 && sample.size() != msg.size();
}

const robot_movement_interface::Command& CommandHandler::getSampleCommand() const
{
  return sample_command_;
}

bool CommandHandler::operator ==(const robot_movement_interface::Command &cmd_msg)
{
  //Check strings for usage and equality
  //Check vectors for usage and length
  if (usedAndNotEqual(sample_command_.command_type, cmd_msg.command_type))
    return false;

  if (usedAndNotEqual(sample_command_.pose_reference, cmd_msg.pose_reference))
    return false;

  if (usedAndNotEqual(sample_command_.pose_type, cmd_msg.pose_type))
    return false;

  if (usedAndNotEqual(sample_command_.pose, cmd_msg.pose))
    return false;

  if (usedAndNotEqual(sample_command_.velocity_type, cmd_msg.velocity_type))
    return false;

  return true; //If it got this far it's a match
}

std::ostream& CommandHandler::dump(std::ostream& o) const
{

  o << "CommandHandler " << getName() << " criteria: " << std::endl;

  if (sample_command_.command_type.length() > 0)
    o << "command_type:" << sample_command_.command_type << std::endl;

  if (sample_command_.pose_reference.length() > 0)
    o << "pose_reference:" << sample_command_.pose_reference << std::endl;
  if (sample_command_.pose_type.length() > 0)
    o << "pose_type:" << sample_command_.pose_type << std::endl;

  if (sample_command_.velocity_type.length() > 0)
    o << "velocity_type:" << sample_command_.velocity_type << std::endl;

  if (sample_command_.velocity.size() > 0)
    o << "velocity (size):" << sample_command_.velocity.size() << std::endl;

  if (sample_command_.pose.size() > 0)
    o << "pose (size):" << sample_command_.pose.size() << std::endl;

  return o;

}

//Eclipse has a fit every time I try to call resize(int) or construct the vector with a size,
//even though it compiles fine, so I have no way to guarantee that the vector isn't empty.
//So, I need to check at() and return a string, not a reference to one.
std::string Command::getCommand() const
{
  try
  {
    return full_command_.at(0).first;
  }
  catch(const std::out_of_range& oor)
  {
    return "";
  }
}

Command::CommandType Command::getType() const
{
  return type_;
}

void Command::setType(CommandType type)
{
  type_ = type;
}

const CommandHandler* CommandRegister::findHandler(const robot_movement_interface::Command &msg_cmd)
{

  auto foundItem = std::find_if(this->handlers().begin(), this->handlers().end(),
                                [&](const std::unique_ptr<CommandHandler> &p)
                                {
                                  return *p.get() == msg_cmd;
                                });

  if (foundItem != std::end(this->handlers()))
  {
    return foundItem->get();
  }
  else
  {
    return nullptr;
  }
}

} //namespace rmi_driver

