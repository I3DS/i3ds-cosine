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


void signal_handler ( int signum )
{
  BOOST_LOG_TRIVIAL ( info ) << "do_deactivate()";
  running = false;
}

int main ( int argc, char **argv )
{
  std::string camera_type;
  unsigned int node_id;
  int trigger_scale;
  i3ds::GigECamera::Parameters param;

  po::options_description desc("Allowed camera control options");

  desc.add_options()
  ("help,h", "Produce this message")
  ("node,n", po::value<unsigned int>(&node_id)->default_value(10), "Node ID of camera")

  ("camera-type,t", po::value<std::string>(&camera_type),       "Camera type {hr, tir, stereo} set by launch script")
  ("camera-name,c", po::value<std::string>(&param.camera_name), "Connect via (UserDefinedName) of Camera")
  ("package-size,p", po::value<int>(&param.packet_size)->default_value(8192), "Transport-layer buffersize (MTU).")
  ("package-delay,d", po::value<int>(&param.packet_delay)->default_value(20), "Inter-package delay parameter of camera.")

  ("trigger", po::value<bool>(&param.external_trigger)->default_value(true), "Enable external trigger.")
  ("trigger-node", po::value<NodeID>(&param.trigger_node)->default_value(20), "Node ID of trigger service.")
  ("trigger-source", po::value<int>(&param.trigger_source)->default_value(1), "Trigger generator for camera.")
  ("trigger-camera-output", po::value<int>(&param.camera_output)->default_value(2), "Trigger output for camera.")
  ("trigger-camera-offset", po::value<int>(&param.camera_offset)->default_value(5000), "Trigger offset for camera (us).")

  ("flash", po::value<bool>(&param.support_flash)->default_value(false), "Support wide-angle flash.")
  ("flash-node", po::value<NodeID>(&param.flash_node)->default_value(21), "Node ID of flash service.")
  ("trigger-flash-output", po::value<int>(&param.flash_output)->default_value(8), "Trigger output for flash.")
  ("trigger-flash-offset", po::value<int>(&param.flash_offset)->default_value(4200), "Trigger offset for flash (us).")

  ("pattern", po::value<bool>(&param.support_pattern)->default_value(false), "Support pattern illumination.")
  ("trigger-pattern-output", po::value<int>(&param.pattern_output)->default_value(6), "Trigger output for pattern.")
  ("trigger-pattern-offset", po::value<int>(&param.pattern_offset)->default_value(0), "Trigger offset for pattern (us).")

  ("verbose,v", "Print verbose output")
  ("quiet,q", "Quiet output")
  ("print,p", "Print the camera configuration")
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


  BOOST_LOG_TRIVIAL ( info ) << "Node ID:     " << node_id;
  BOOST_LOG_TRIVIAL ( info ) << "Camera name: " << param.camera_name;
  BOOST_LOG_TRIVIAL ( info ) << "Camera type: " << camera_type;
  BOOST_LOG_TRIVIAL ( info ) << "trigger: " << ((param.external_trigger)  ? "true" : "false");


  param.frame_mode = mode_mono;
  param.pixel_size = 2;

  if ( camera_type == "hr" )
    {
      param.image_count = 1;
      param.data_depth = 12;
      trigger_scale = 2;
    }
  else if ( camera_type == "tir" )
    {
      param.image_count = 1;
      param.data_depth = 16;
      trigger_scale = 30;
    }
  else if ( camera_type == "stereo" )
    {
      param.image_count = 2;
      param.data_depth = 12;
      trigger_scale = 2;
    }
  else
    {
      BOOST_LOG_TRIVIAL ( error ) << "Unknown camera type: " << camera_type << std::endl;
      return -1;
    }

  i3ds::Context::Ptr context = i3ds::Context::Create();;

  i3ds::Server server ( context );

  i3ds::CosineCamera camera ( context, node_id, param, trigger_scale);

  camera.Attach ( server );

  running = true;
  signal ( SIGINT, signal_handler );

  server.Start();

  while ( running )
    {
      sleep ( 1 );
    }

  server.Stop();

  return 0;
}
