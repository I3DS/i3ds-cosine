///////////////////////////////////////////////////////////////////////////\file
///
///   Copyright 2018 SINTEF AS
///
///   This Source Code Form is subject to the terms of the Mozilla
///   Public License, v. 2.0. If a copy of the MPL was not distributed
///   with this file, You can obtain one at https://mozilla.org/MPL/2.0/
///
////////////////////////////////////////////////////////////////////////////////

#include <csignal>
#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>
#include <memory>

#include <boost/program_options.hpp>

#include "i3ds/communication.hpp"
#include "cosine_camera.hpp"

#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>


#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace po = boost::program_options;
namespace logging = boost::log;

volatile bool running;

void signal_handler(int signum)
{
  BOOST_LOG_TRIVIAL(info) << "do_deactivate()";
  running = false;
}

int main(int argc, char** argv)
{
  unsigned int node_id;

  std::string camera_type;
  i3ds::CosineParameters param;
  
  po::options_description desc("Allowed camera control options");

  desc.add_options()
  ("help,h", "Produce this message")
  ("node,n", po::value<unsigned int>(&node_id), "Node ID of camera")
  ("camera-name,c", po::value<std::string>(&param.camera_name), "Connect via (UserDefinedName) of Camera")
  ("camera-type,t", po::value<std::string>(&camera_type),       "Camera type {hr, tir, stereo} set by launch script")
  ("free-running,f", po::bool_switch(&param.free_running)->default_value(false), "Free-running sampling")

  ("verbose,v", "Print verbose output")
  ("quite,q", "Quiet ouput")
  ("print", "Print the camera configuration")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);

  if (vm.count("help"))
    {
      std::cout << desc << std::endl;
      return -1;
    }

  if (vm.count("quite"))
    {
      logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::warning);
    }
  else if (!vm.count("verbose"))
    {
      logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
    }

  po::notify(vm);

  BOOST_LOG_TRIVIAL(info) << "Node ID:     " << node_id;
  BOOST_LOG_TRIVIAL(info) << "Camera name: " << param.camera_name;
  BOOST_LOG_TRIVIAL(info) << "Camera type: " << camera_type;

  if (camera_type == "hr")
    {
      param.is_stereo = false;
      param.trigger_scale = 2;
    }
  else if (camera_type == "tir")
    {
      param.is_stereo = false;
      param.trigger_scale = 30;
    }
  else if (camera_type == "stereo")
    {
      param.is_stereo = true;
      param.trigger_scale = 2;
    }
  else
    {
      std::cout << "Unknown camera type: " << camera_type << std::endl;
      return -1;
    }

  i3ds::Context::Ptr context = i3ds::Context::Create();;

  i3ds::Server server(context);

  i3ds::CosineCamera camera(context, node_id, param);

  camera.Attach(server);

  running = true;
  signal(SIGINT, signal_handler);

  server.Start();

  while(running)
    {
      sleep(1);
    }

  server.Stop();

  return 0;
}
