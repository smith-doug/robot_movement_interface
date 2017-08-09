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
 *  Created on: Aug 1, 2017
 *      Author: Doug Smith
 */



#ifndef INCLUDE_DRIVER_H_
#define INCLUDE_DRIVER_H_

#include <ros/ros.h>

#include "connector.h"
#include "commands.h"

#include "commands_keba.h"

#include <sensor_msgs/JointState.h>
#include <robot_movement_interface/CommandList.h>

#include <boost/asio.hpp>
#include <unordered_map>

namespace keba_rmi_driver
{


  class Driver
  {
  public:
    Driver();

    void start();

    void addConnection(std::string host, int port, std::shared_ptr<CommandRegister> commands);

    void publishJointState();

    void subCB_CommandList(const robot_movement_interface::CommandListConstPtr &msg)
    {
      commandListCb(*msg);
    }

    bool commandListCb(const robot_movement_interface::CommandList &msg);



  protected:

    ros::NodeHandle nh;

    std::unordered_map<int32_t, std::shared_ptr<Connector>> conn_map_;

    //Connector connector_;

    int conn_num_ = 0;

    boost::asio::io_service io_service_;

    ros::Subscriber command_list_sub_;


    ros::Publisher joint_state_publisher_;

    std::thread pub_thread_;

    //std::vector<CommandHandler> cmd_handlers_;  //###testing


    std::shared_ptr<CommandRegister> cmd_register_;

  };
}



#endif /* INCLUDE_DRIVER_H_ */