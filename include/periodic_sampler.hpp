///////////////////////////////////////////////////////////////////////////\file
///
///   Copyright 2018 SINTEF AS
///
///   This Source Code Form is subject to the terms of the Mozilla
///   Public License, v. 2.0. If a copy of the MPL was not distributed
///   with this file, You can obtain one at https://mozilla.org/MPL/2.0/
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __I3DS_PERIODIC_SAMPLER_HPP
#define __I3DS_PERIODIC_SAMPLER_HPP

#include <thread>
#include <functional>
#include <chrono>

#include "i3ds/sensor.hpp"

namespace i3ds
{

////////////////////////////////////////////////////////////////////////////////
/// Worker class for periodic sampling.
////////////////////////////////////////////////////////////////////////////////

class Sampler
{
public:

  // Does sampling operation, returns true if more samples are requested.
  typedef std::function<bool(unsigned char *image, unsigned long timestamp_us)> Operation;

  Sampler(Operation operation);
  virtual ~Sampler();

  // Starts sampler in a separate thread with periodic rate.
  void Start(SampleRate rate);

  // Stop the sampler if running.
  void Stop();

private:

  typedef std::chrono::high_resolution_clock clock;

  // Run the receiver until stop is called.
  void Run();

  // Sample operation.
  Operation operation_;

  // Rate of periodic sampling.
  clock::duration rate_;

  // Running flag.
  volatile bool running_;

  // Worker thread.
  std::thread worker_;
};

}

#endif
