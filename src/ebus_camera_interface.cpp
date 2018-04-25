///////////////////////////////////////////////////////////////////////////\file
///
///   Copyright 2018 SINTEF AS
///
///   This Source Code Form is subject to the terms of the Mozilla
///   Public License, v. 2.0. If a copy of the MPL was not distributed
///   with this file, You can obtain one at https://mozilla.org/MPL/2.0/
///
////////////////////////////////////////////////////////////////////////////////



#include <iostream>

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



bool EbusCameraInterface::connect() {
  BOOST_LOG_TRIVIAL(info) << "Connecting to camera";
  
  std::cout << "--> ConnectDevice " << mConnectionID.GetAscii() << std::endl;

  // Connect to the selected Device
  PvResult lResult = PvResult::Code::INVALID_PARAMETER;
  mDevice = PvDevice::CreateAndConnect(mConnectionID, &lResult);
  if (!lResult.IsOK()) {
      BOOST_LOG_TRIVIAL(info) << "CreateAndConnect problem";
  	return false;
  }

  // Register this class as an event sink for PvDevice call-backs
  mDevice->RegisterEventSink(this);

  // Clear connection lost flag as we are now connected to the device
  mConnectionLost = false;
  
  return true;
}
  
  
  

void EbusCameraInterface::OnLinkDisconnected(PvDevice *aDevice) {
  BOOST_LOG_TRIVIAL(info) << "=====> PvDeviceEventSink::OnLinkDisconnected callback";

  mConnectionLost = true;

	// IMPORTANT:
	// The PvDevice MUST NOT be explicitly disconnected from this callback.
	// Here we just raise a flag that we lost the device and let the main loop
	// of the application (from the main application thread) perform the
	// disconnect.
	//

}
