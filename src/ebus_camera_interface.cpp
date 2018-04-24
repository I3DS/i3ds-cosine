///////////////////////////////////////////////////////////////////////////\file
///
///   Copyright 2018 SINTEF AS
///
///   This Source Code Form is subject to the terms of the Mozilla
///   Public License, v. 2.0. If a copy of the MPL was not distributed
///   with this file, You can obtain one at https://mozilla.org/MPL/2.0/
///
////////////////////////////////////////////////////////////////////////////////





#include "../include/ebus_camera_interface.hpp"


#include <boost/program_options.hpp>

#include "i3ds/core/communication.hpp"
#include "i3ds/emulators/emulated_camera.hpp"

#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>





namespace po = boost::program_options;
namespace logging = boost::log;




EbusCameraInterface::EbusCameraInterface() {
  BOOST_LOG_TRIVIAL(info) << "EbusCameraInterface constructor";
}



