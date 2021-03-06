find_package (Boost COMPONENTS program_options log REQUIRED)

set(PUREGEV_ROOT "/opt/pleora/ebus_sdk/Ubuntu-x86_64")
if (NOT(EBUS_INCLUDE_DIR))
  set(EBUS_INCLUDE_DIR "${PUREGEV_ROOT}/include")
endif (NOT(EBUS_INCLUDE_DIR))

set(PV_LIBRARY_PATH "${PUREGEV_ROOT}/lib")

message(STATUS "PUREGEV_ROOT=${PUREGEV_ROOT}")
message(STATUS "EBUS_INCLUDE=${EBUS_INCLUDE_DIR}")


set (SRCS
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
  )


if (BUILD_EMBEDDED OR BUILD_PETALINUX)
  find_file(pylon_INCLUDE_DIR include/ebus)
  find_file(pylon_LIB_DIR lib/ebus)

  include_directories (SYSTEM  "${pylon_INCLUDE_DIR}")
  set (PLEORA_LINK_DIRECTORY -L"${pylon_LIB_DIR}" -L"${pylon_LIB_DIR}/genicam/bin/Linux64_ARM/")
  set (LIBS
    ${LIBS}
    EbTransportLayerLib
    EbUtilsLib
    PtUtilsLib
    GCBase_gcc492_v3_0
    GenApi_gcc492_v3_0
    PtConvertersLib
    MathParser_gcc492_v3_0
    XmlParser_gcc492_v3_0
    Log_gcc492_v3_0
    NodeMapData_gcc492_v3_0
    )
  include_directories (${PLEORA_INCLUDE_DIRECTORIES})
else (BUILD_EMBEDDED OR BUILD_PETALINUX)
  include_directories (${EBUS_INCLUDE_DIR})
  set (PLEORA_LINK_DIRECTORY
    -L"${PUREGEV_ROOT}/lib/"
    )
endif (BUILD_EMBEDDED OR BUILD_PETALINUX)

include_directories ("../include/" )

add_executable (i3ds_cosine_camera i3ds_cosine_camera.cpp ${SRCS})
target_compile_options(i3ds_cosine_camera PRIVATE -Wno-unknown-pragmas)
target_link_libraries (i3ds_cosine_camera ${PLEORA_LINK_DIRECTORY} ${LIBS} ${Boost_LIBRARIES})
install(TARGETS i3ds_cosine_camera DESTINATION bin)

set (CAMERA_TYPES "hr" "stereo"  "tir")

foreach(CAMERA_TYPE ${CAMERA_TYPES})
  configure_file(start_cosine.sh i3ds_cosine_${CAMERA_TYPE})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/i3ds_cosine_${CAMERA_TYPE} DESTINATION bin)
endforeach(CAMERA_TYPE)
