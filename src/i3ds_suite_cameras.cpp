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
#include "gige_camera_interface.hpp"

#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>


#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

#include "cpp11_make_unique_template.hpp"




namespace po = boost::program_options;
namespace logging = boost::log;


#ifdef TOF_CAMERA
  std::unique_ptr <i3ds::GigeCameraInterface<i3ds::ToFCamera::Measurement500KTopic>> camera;
#else
 std::unique_ptr <i3ds::GigeCameraInterface<i3ds::Camera::FrameTopic>> camera;
#endif



volatile bool running;
unsigned int node_id;
std::string ip_address;
std::string camera_name;
bool camera_freerunning = false;

void signal_handler(int signum)
{
  BOOST_LOG_TRIVIAL(info) << "do_deactivate()";
  running = false;
}



int main(int argc, char** argv)
{
	po::options_description desc("Allowed camera control options");
	  desc.add_options()
	    ("help,h", "Produce this message")
	    ("node,n", po::value<unsigned int>(&node_id)->default_value(DEFAULT_NODE_ID), "Node ID of camera")
	    ("ip-address,i", po::value<std::string>(&ip_address)->default_value(DEFAULT_IP_ADDRESS), "Use IP Address of camera to connect")
	    ("camera-name,c", po::value<std::string>(&camera_name)->default_value(DEFAULT_CAMERA_NAME), "Connect via (UserDefinedName) of Camera")
	    ("free-running,f", po::bool_switch(&camera_freerunning), "Free-running sampling. Default external triggered")

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
/*
	  if (vm.count("camera-name"))
	    {
	      std::string s;
	      s = vm["camera-name"].as< std::vector<std::string>>().front();
	      std::cout << "Using device: "<< s << std::endl;
	    }
*/
	  if (vm.count("quite"))
	    {
	      logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::warning);
	    }
	  else if (!vm.count("verbose"))
	    {
	      logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
	    }

	  po::notify(vm);

/*	  i3ds::Context::Ptr context(i3ds::Context::Create());

	  BOOST_LOG_TRIVIAL(info) << "Connecting to camera with node ID: " << node_id;
	  i3ds::CameraClient camera(context,   frame_.image.nCount = resx_ * resy_ * 2;
	   node_id);
	  BOOST_LOG_TRIVIAL(trace) << "---> [OK]";
*/

	/*
	  // Print config, this is the final command.
	  if (vm.count("print"))
	    {
	      print_camera_settings(&camera);
	    }
	*/



	  BOOST_LOG_TRIVIAL(info) << "test1a";


  i3ds::Context::Ptr context = i3ds::Context::Create();;

  i3ds::Server server(context);


  BOOST_LOG_TRIVIAL(info) << "Using Nodeid: " << node_id;
  BOOST_LOG_TRIVIAL(info) << "Using IP ADDRESS: " << ip_address;
  BOOST_LOG_TRIVIAL(info) << "User defined camera name " << camera_name;



#ifdef TOF_CAMERA
  camera = std::make_unique <i3ds::GigeCameraInterface<i3ds::ToFCamera::Measurement500KTopic>>(context, node_id, 800, 600, ip_address, camera_name, camera_freerunning);
#else
  camera = std::make_unique <i3ds::GigeCameraInterface<i3ds::Camera::FrameTopic>>(context, node_id, 800, 600, ip_address, camera_name, camera_freerunning);
#endif


  BOOST_LOG_TRIVIAL(info) << "AttachServer";
  camera->Attach(server);
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
