find_package (Boost COMPONENTS program_options log REQUIRED)




set(PUREGEV_ROOT "/opt/pleora/ebus_sdk/Ubuntu-x86_64")

#set(PUREGEV_ROOT    "$ENV{PUREGEV_ROOT}")
set(PV_LIBRARY_PATH "${PUREGEV_ROOT}/lib")
#export GENICAM_ROOT_V3_0=$GENICAM_ROOT
#if $ENV{PUREGEV_ROOT}

message(STATUS "PUREGEV_ROOT=${PUREGEV_ROOT}")


set (SRCS
   # emulated_camera.cpp
  # periodic_sampler.cpp
   ebus_camera_interface.cpp
  )

set (LIBS
  i3ds
  zmq
  pthread
  )

set (PLEORA_LIBS 
  PvBase
  PvDevice
  PvBuffer
  PvGenICam
  PvStream
  PvTransmitter
  PvVirtualDevice
  PvCameraBridge
  PvAppUtils
  PvPersistence
  PvSystem
  PvSerial

  libSimpleImagingLib.so
  )

set (APPS
 # cosine-tir
  i3ds_suite_emulator
  )
  
add_definitions(-D_UNIX_)

set (PLEORA_INCLUDE_DIRECTORIES 
  ${CMAKE_CURRENT_BINARY_DIR}/src/
  ${PUREGEV_ROOT}/include
  )

set (PLEORA_LINK_DIRECTORIES
  ${PUREGEV_ROOT}/lib/
)

include_directories ("../include/")





#BASLER

#BASLER_TOF
set (GENTL_ROOT /opt/BaslerToF)
set (GENAPI_POSTFIX gcc_v3_0_Basler_pylon_v5_0)
set (BASLER_TOF_LIBDIR  ${GENTL_ROOT}/lib64)
set (BASLER_TOF_INCLUDES ${GENTL_ROOT}/include)
set (BASLER_TOF_LIBS 
      GenApi_${GENAPI_POSTFIX} 
      GCBase_${GENAPI_POSTFIX} 
      dl 
      pthread
)


message(STATUS "BASLER_TOF_LIBDIR=${BASLER_TOF_LIBDIR}")

set (BASLER_TOF_LIB_FLAGS "-Wl,-enable-new-dtags -Wl,-rpath ${BASLER_TOF_LIBDIR} -Wl,-rpath ${BASLER_TOF_LIBDIR}/gentlproducer/gtl -Wl,-E"
)

set (BASLER_TOF_CXX_FLAGS 
      -std=c++11 -Wall -Wno-unknown-pragmas
)


#Basler HR (Pylon 5)
#Taken From a Makefile in the Sample directory

# The program to build
#NAME       := basler_high_res_interface

# Installation directories for pylon
set (PYLON_ROOT ?= /opt/pylon5)

# Build tools and flags
#LD         := $(CXX)
#CPPFLAGS   := $(shell $(PYLON_ROOT)/bin/pylon-config --cflags) -std=c++11
#CXXFLAGS   := -g -O0  #e.g., CXXFLAGS=-g -O0 for debugging
#LDFLAGS    := $(shell $(PYLON_ROOT)/bin/pylon-config --libs-rpath)
#LDLIBS     := $(shell $(PYLON_ROOT)/bin/pylon-config --libs) -lopencv_core -lopencv_highgui -lpthread

# Basler HR sample END



# Basler HR (For compiling 0n development computer with cmake
set (PYLON_ROOT /opt/pylon5)

execute_process(COMMAND ${PYLON_ROOT}/bin/pylon-config --cflags OUTPUT_VARIABLE BASLER_HR_CPPFLAGS)
string(STRIP ${BASLER_HR_CPPFLAGS} BASLER_HR_CPPFLAGS)
string(REGEX REPLACE "\n$" "" BASLER_HR_CPPFLAGS "${BASLER_HR_CPPFLAGS}") #Strip newline
message(STATUS "BASLER_HR_CPPFLAGS2=${BASLER_HR_CPPFLAGS}")

set (BASLER_HR_CXXFLAGS -g -O0)  #e.g., CXXFLAGS=-g -O0 for debugging
message(STATUS "BASLER_HR_CXXFLAGS=${BASLER_HR_CXXFLAGS}")

execute_process(COMMAND ${PYLON_ROOT}/bin/pylon-config --libs-rpath OUTPUT_VARIABLE BASLER_HR_LDFLAGS)
string(STRIP ${BASLER_HR_LDFLAGS} ${BASLER_HR_LDFLAGS})
string(REGEX REPLACE "\n$" "" BASLER_HR_LDFLAGS "${BASLER_HR_LDFLAGS}")  #Strip newline
message(STATUS "BASLER_HR_LDFLAGS=${BASLER_HR_LDFLAGS}")

execute_process(COMMAND ${PYLON_ROOT}/bin/pylon-config --libs OUTPUT_VARIABLE BASLER_HR_LDLIBS) 
#-lopencv_core -lopencv_highgui -lpthread
string(STRIP "${BASLER_HR_LDLIBS}" ${BASLER_HR_LDLIBS})
string(REGEX REPLACE "\n$" "" BASLER_HR_LDLIBS "${BASLER_HR_LDLIBS}") #Strip newline
message(STATUS "BASLER_HR_LDLIBS=${BASLER_HR_LDLIBS}")


#Extra opencv used in Basler HR under testing. Can be removed.
set (BASLER_EXTRA_OPEN_CV
  
  opencv_core 
  opencv_highgui 
  ) 



#Pleora camera need some environement variables set before running. 
#Therefore a startup script that sets the environement 

set (START_FILE_NAME "Hr" "Stereo"  "Tir")
foreach(START_FILE ${START_FILE_NAME})
  configure_file(start_pleora.sh  start_pleora_${START_FILE}.sh)
endforeach(START_FILE)




##foreach(a ${APPS})
##  add_executable (${a} ${a}.cpp ${SRCS})
##  target_link_libraries (${a} ${LIBS} ${Boost_LIBRARIES})
#  install(TARGETS ${a} DESTINATION bin)
#endforeach(a)


#  i3ds-thermal:    10.0.1.111
#  i3ds-highres:    10.0.1.116
#  i3ds-stereo:     10.0.1.117
#  i3ds-basler-hr:  10.0.1.115
#  i3ds-basler-tof: 10.0.1.110


  
  
  
#PLEORA hr
add_executable (PleoraHr i3ds_suite_emulator.cpp ${SRCS})
  target_include_directories(PleoraHr PRIVATE ${PLEORA_INCLUDE_DIRECTORIES})
  target_compile_definitions(PleoraHr PRIVATE EBUS_CAMERA=1 HR_CAMERA=1 DEFAULT_NODE_ID=16 
    DEFAULT_IP_ADDRESS="10.0.1.116" DEFAULT_CAMERA_NAME="i3ds-highres")
  target_compile_options(PleoraHr PRIVATE -Wno-unknown-pragmas)
  target_link_libraries (PleoraHr -L${PUREGEV_ROOT}/lib/ ${PLEORA_LIBS} ${LIBS} ${Boost_LIBRARIES})
  install(TARGETS PleoraHr DESTINATION bin)
  
#PLEORA TIR
add_executable (PleoraTir i3ds_suite_emulator.cpp ${SRCS})
  target_include_directories(PleoraTir PRIVATE ${PLEORA_INCLUDE_DIRECTORIES})
  target_compile_definitions(PleoraTir PRIVATE EBUS_CAMERA=1  DEFAULT_NODE_ID=11 
    DEFAULT_IP_ADDRESS="10.0.1.111" DEFAULT_CAMERA_NAME="i3ds-thermal")
  target_compile_options(PleoraTir PRIVATE -Wno-unknown-pragmas)
  target_link_libraries (PleoraTir -L${PUREGEV_ROOT}/lib/ ${PLEORA_LIBS} ${LIBS} ${Boost_LIBRARIES})
  install(TARGETS PleoraTir DESTINATION bin)


#PLEORA Stereo
  add_executable (PleoraStereo i3ds_suite_emulator.cpp ${SRCS})
  target_include_directories(PleoraStereo PRIVATE ${PLEORA_INCLUDE_DIRECTORIES})
  target_compile_definitions(PleoraStereo PRIVATE BASLER_TOF_CXX_FLAG STEREO_CAMERA=1 
    EBUS_CAMERA=1 DEFAULT_NODE_ID=17 DEFAULT_IP_ADDRESS="10.0.1.117" DEFAULT_CAMERA_NAME="i3ds-stereo")
  target_compile_options(PleoraStereo PRIVATE -Wno-unknown-pragmas)
  target_link_libraries (PleoraStereo ${PROTOBUF_LIBRARY} -L${PUREGEV_ROOT}/lib/ ${PLEORA_LIBS} ${LIBS} ${Boost_LIBRARIES})
  install(TARGETS PleoraStereo DESTINATION bin)
  


#Basler HR
 add_executable (BaslerHr i3ds_suite_emulator.cpp basler_high_res_interface.cpp )
 #set_target_properties(BaslerHr INCLUDE_DIRECTORIES ${BASLER_HR_CPPFLAGS})
 #target_include_directories(BaslerHr PRIVATE ${BASLER_HR_CPPFLAGS})
 target_compile_definitions(BaslerHr PRIVATE BASLER_CAMERA=1 HR_CAMERA=1 DEFAULT_NODE_ID=15
    DEFAULT_IP_ADDRESS="10.0.1.115" DEFAULT_CAMERA_NAME="i3ds-basler-hr" )
 message(STATUS "BASLER_HR_CPPFLAGS3=${BASLER_HR_CPPFLAGS}")
 target_compile_options(BaslerHr PRIVATE ${BASLER_HR_CPPFLAGS} -Wno-unknown-pragmas)
 target_link_libraries (BaslerHr ${BASLER_HR_LIB_FLAGS} ${BASLER_HR_LDFLAGS} ${BASLER_HR_LDLIBS} ${LIBS} ${Boost_LIBRARIES} ${BASLER_EXTRA_OPEN_CV})
 #install(TARGETS BaslerHr DESTINATION bin)



#Basler ToF
  add_executable (BaslerToF i3ds_suite_emulator.cpp basler_tof_interface.cpp )
  target_include_directories(BaslerToF PRIVATE ${BASLER_TOF_INCLUDES})
  target_compile_definitions(BaslerToF PRIVATE BASLER_CAMERA=1 TOF_CAMERA=1 DEFAULT_NODE_ID=10 
   DEFAULT_IP_ADDRESS="10.0.1.110" DEFAULT_CAMERA_NAME="i3ds-basler-tof" )
  target_compile_options(BaslerToF PRIVATE -Wno-unknown-pragmas)
  target_link_libraries (BaslerToF   -L${BASLER_TOF_LIBDIR}  ${BASLER_TOF_LIBS} ${BASLER_TOF_LIB_FLAGS} ${LIBS} ${Boost_LIBRARIES})
  install(TARGETS BaslerToF DESTINATION bin)
  



  


# Wide angle flash
#SerialCommunicator

  add_executable (SerialCommunicator SerialCommunicator.cpp)
  target_link_libraries (SerialCommunicator ${LIBS} ${Boost_LIBRARIES})
  install(TARGETS SerialCommunicator DESTINATION bin)

