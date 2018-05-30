
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



#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>

#include <pylon/gige/BaslerGigEInstantCamera.h>

// Use sstream to create image names including integer
#include <sstream>
#include <thread>

#include <unistd.h>


#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#undef BOOST_LOG_TRIVIAL
#define BOOST_LOG_TRIVIAL(info) cout

// Namespace for using pylon objects.
using namespace Pylon;

// Namespace for using GenApi objects
using namespace GenApi;

// Namespace for using opencv objects.
using namespace cv;

// Namespace for using cout.
using namespace std;

class BaslerHighResInterface
{
public:

  BaslerHighResInterface();
  void initialiseCamera();
  void stopSampling();
  void startSamplingLoop();
  void startSampling();
  void openForParameterManipulation();
  void closeForParameterManipulation();

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

#if 0
  void setRegion(PlanarRegion region);
  PlanarRegion getRegion();
#endif

  bool getAutoExposureEnabled ();
  void setAutoExposureEnabled (bool enabled);

private:

  CInstantCamera *camera;

  // Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
  // is initialized during the lifetime of this object.
  Pylon::PylonAutoInitTerm autoInitTerm;

  std::thread threadSamplingLoop;

  CBaslerGigEInstantCamera *cameraParameters;


};

#endif
