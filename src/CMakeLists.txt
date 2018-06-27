find_package (Boost COMPONENTS program_options log REQUIRED)

set(PUREGEV_ROOT "/opt/pleora/ebus_sdk/Ubuntu-x86_64")

set(PV_LIBRARY_PATH "${PUREGEV_ROOT}/lib")

message(STATUS "PUREGEV_ROOT=${PUREGEV_ROOT}")


set (SRCS
   ebus_wrapper.cpp
   cosine_camera.cpp
   )

set (LIBS
  i3ds
  zmq
  pthread
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
  
add_definitions(-D_UNIX_)

set (PLEORA_INCLUDE_DIRECTORIES 
  ${CMAKE_CURRENT_BINARY_DIR}/src/
  ${PUREGEV_ROOT}/include
  )

set (PLEORA_LINK_DIRECTORIES
  ${PUREGEV_ROOT}/lib/
)

include_directories (${PLEORA_INCLUDE_DIRECTORIES})
include_directories ("../include/")

add_executable (i3ds-cosine-camera i3ds_suite_cameras.cpp ${SRCS})
target_compile_options(i3ds-cosine-camera PRIVATE -Wno-unknown-pragmas)
target_link_libraries (i3ds-cosine-camera -L${PUREGEV_ROOT}/lib/ ${LIBS} ${Boost_LIBRARIES})
install(TARGETS i3ds-cosine-camera DESTINATION bin)

#Pleora camera need some environement variables set before running. 
#Therefore a startup script that sets the environement 

#  i3ds-cosine-tir:     10.0.1.111
#  i3ds-cosine-hr:      10.0.1.116
#  i3ds-cosine-stereo:  10.0.1.117

set (START_FILE_NAME "hr" "stereo"  "tir")

foreach(START_FILE ${START_FILE_NAME})
  configure_file(start_pleora.sh  i3ds_cosine_${START_FILE}.sh)
endforeach(START_FILE)
