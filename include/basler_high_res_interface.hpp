
///////////////////////////////////////////////////////////////////////////\file
///
///   Copyright 2018 SINTEF AS
///
///   This Source Code Form is subject to the terms of the Mozilla
///   Public License, v. 2.0. If a copy of the MPL was not distributed
///   with this file, You can obtain one at https://mozilla.org/MPL/2.0/
///
////////////////////////////////////////////////////////////////////////////////


#ifndef __I3DS_BASLER_HIGH_RES_INTERFACE_HPP
#define __I3DS_BASLER_HIGH_RES_INTERFACE_HPP



#include "i3ds/camera_sensor.hpp"
#include "i3ds/periodic.hpp"

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>

#include <pylon/gige/BaslerGigEInstantCamera.h>

#include <thread>



// Namespace for using pylon objects.
//using namespace Pylon;


// Does sampling operation, returns true if more samples are requested.
typedef std::function<bool(unsigned char *image, unsigned long timestamp_us)> Operation;


class BaslerHighResInterface
{
public:

  BaslerHighResInterface(std::string const & connectionString,  std::string const  &camera_name, bool free_running, Operation operation);
  void initialiseCamera();
  void stopSampling();
  void startSamplingLoop();
  void startSampling();
  void openForParameterManipulation();
  void closeForParameterManipulation();
  void do_activate();
  void do_start();
  void do_stop();
  void do_deactivate();

  void connect();


  void setGain (int64_t value);
  int64_t getGain ();

  int64_t getShutterTime();
  void setShutterTime(int64_t value);

  void setRegionEnabled(bool regionEnabled);
  bool getRegionEnabled();


  void setTriggerInterval(int64_t);
  bool checkTriggerInterval(int64_t);


  int64_t getMaxShutterTime();
  void setMaxShutterTime(int64_t);


  void setRegion(PlanarRegion region);
  PlanarRegion getRegion();


  bool getAutoExposureEnabled ();
  void setAutoExposureEnabled (bool enabled);

private:

  std::unique_ptr <Pylon::CBaslerGigEInstantCamera> camera;

  // Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
  // is initialized during the lifetime of this object.
  Pylon::PylonAutoInitTerm autoInitTerm;

  std::thread threadSamplingLoop;

  //const char *cameraName;
  //std::unique_ptr<std::string> cameraName_;
 std::string cameraName_;


  // Sample operation.

   std::unique_ptr<char> ConnectionID;
   bool free_running_;
   Operation operation_;

   float sample_rate_in_Hz_;

   typedef std::chrono::high_resolution_clock clock;
};

#endif
