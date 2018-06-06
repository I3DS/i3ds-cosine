#!/bin/bash

# Get the executable's absolute path
DIRNAME=`dirname $0`
DIRNAME=`cd $DIRNAME/..; pwd`
DIRNAME= 
# set env. vars


#/opt/pleora/ebus_sdk/Ubuntu-x86_64
export PUREGEV_ROOT=/opt/pleora/ebus_sdk/Ubuntu-x86_64
#export PUREGEV_ROOT=$DIRNAME
export GENICAM_ROOT=$PUREGEV_ROOT/lib/genicam
export GENICAM_ROOT_V3_0=$GENICAM_ROOT
export GENICAM_LOG_CONFIG=$DIRNAME/lib/genicam/log/config/DefaultLogging.properties
export GENICAM_LOG_CONFIG_V3_0=$GENICAM_LOG_CONFIG
export GENICAM_CACHE_V3_0=$HOME/.config/Pleora/genicam_cache_v3_0
export GENICAM_CACHE=$GENICAM_CACHE_V3_0
mkdir -p $GENICAM_CACHE

# add to the LD_LIBRARIES_PATH
if ! echo ${LD_LIBRARY_PATH} | /bin/grep -q $PUREGEV_ROOT/lib; then
   if [ "$LD_LIBRARY_PATH" = "" ]; then
      LD_LIBRARY_PATH=$PUREGEV_ROOT/lib
   else
      LD_LIBRARY_PATH=$PUREGEV_ROOT/lib:${LD_LIBRARY_PATH}
   fi
fi

if [ `uname -m` == "x86_64" ]; then
  GENICAM_LIB_DIR=bin/Linux64_x64
else
  GENICAM_LIB_DIR=bin/Linux32_i86
fi

if ! echo ${LD_LIBRARY_PATH} | /bin/grep -q $GENICAM_ROOT/$GENICAM_LIB_DIR; then
   LD_LIBRARY_PATH=$GENICAM_ROOT/$GENICAM_LIB_DIR:${LD_LIBRARY_PATH}
fi

export LD_LIBRARY_PATH

./Pleora${START_FILE} $@

