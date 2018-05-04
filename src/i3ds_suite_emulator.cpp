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

#include <boost/program_options.hpp>

#include "i3ds/communication.hpp"
#include "emulated_camera.hpp"

#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>





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
	po::options_description desc("Allowed camera control options");

	  desc.add_options()
	    ("help,h", "Produce this message")
	//    ("node", po::value<unsigned int>(&node_id)->required(), "Node ID of camera")

	    ("device,d",
		po::value <std::vector<std::string>>(),
		"Which camera to connect to.\n"
		"May be [camera name | ipadress | mac adress]\n"
		"Actual name and IP adresses at the moment:\n"
		"\ti3ds-thermal:  10.0.1.111\n"
		"\t[no name set]: 10.0.1.115\n"
		"\ti3ds-highres:  10.0.1.116\n"
		"\ti3ds-stereo:   10.0.1.117\n"
	    )


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

	  if (vm.count("device"))
	    {
	      std::string s;
	      s = vm["device"].as< std::vector<std::string>>().front();
	      std::cout << "Using device: "<< s << std::endl;
	      //return -1;
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

/*	  i3ds::Context::Ptr context(i3ds::Context::Create());

	  BOOST_LOG_TRIVIAL(info) << "Connecting to camera with node ID: " << node_id;
	  i3ds::CameraClient camera(context, node_id);
	  BOOST_LOG_TRIVIAL(trace) << "---> [OK]";
*/

	/*
	  // Print config, this is the final command.
	  if (vm.count("print"))
	    {
	      print_camera_settings(&camera);
	    }
	*/






  i3ds::Context::Ptr context = i3ds::Context::Create();;

  i3ds::Server server(context);

  // \ Todo Use Template?
  i3ds::EmulatedCamera<i3ds::CameraMeasurement4MCodec> camera(context, 10, 800, 600);

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
