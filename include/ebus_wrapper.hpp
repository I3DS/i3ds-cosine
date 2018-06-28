///////////////////////////////////////////////////////////////////////////\file
///
///   Copyright 2018 SINTEF AS
///
///   This Source Code Form is subject to the terms of the Mozilla
///   Public License, v. 2.0. If a copy of the MPL was not distributed
///   with this file, You can obtain one at https://mozilla.org/MPL/2.0/
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __I3DS_EBUS_WRAPPER_HPP
#define __I3DS_EBUS_WRAPPER_HPP

#include <thread>

#include <PvDevice.h>
#include <PvPipeline.h>
#include <PvBuffer.h>
#include <PvStream.h>
#include <PvStreamGEV.h>
#include <PvDeviceInfoGEV.h>

// Does sampling operation, returns true if more samples are requested.
typedef std::function<bool(unsigned char *image, unsigned int width, unsigned int height)> Operation;

class EbusWrapper: protected PvDeviceEventSink
{
public:

  EbusWrapper(std::string camera_name,
              Operation operation);

  bool Connect();
  void Disconnect();

  void collectParameters();

  int64_t getParameter(PvString whichParameter);
  int64_t getMaxParameter(PvString whichParameter);
  int64_t getMinParameter(PvString whichParameter);

  bool setIntParameter(PvString whichParameter, int64_t value);

  bool getBooleanParameter(PvString whichParameter);
  void setBooleanParameter(PvString whichParameter, bool status);

  std::string getEnum(PvString whichParameter);
  void setEnum(PvString whichParameter, PvString value, bool dontCheckParameter = false);

  bool checkIfEnumOptionIsOK(PvString whichParameter, PvString value);

  void setTriggerInterval();
  bool checkTriggerInterval(int64_t);

  void Start(bool free_running, int64_t interval, int timeout_ms);
  void Stop();

protected:

  // Inherited from PvDeviceEventSink.
  void OnLinkDisconnected(PvDevice* aDevice);

private:

  bool mConnectionLost;

  PvString mConnectionID;
  PvDevice* mDevice;
  PvGenParameterArray *lParameters;

  static int const trigger_interval_factor;

  //--streaming part
  bool OpenStream();
  void CloseStream();

  bool StartAcquisition();
  bool StopAcquisition();

  void SamplingLoop();

  void DisconnectDevice();
  void TearDown(bool aStopAcquisition);

  PvStream* mStream;
  PvPipeline* mPipeline;

  bool stopSamplingLoop;
  std::thread threadSamplingLoop;

  PvString fetched_ipaddress;
  Operation operation_;

  int timeout_;
};

#endif
