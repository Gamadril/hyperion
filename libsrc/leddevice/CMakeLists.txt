#add libusb and pthreads (required for the Lighpack usb device)
#find_package(libusb-1.0 REQUIRED)
#find_package(Threads REQUIRED)

include_directories(
#		../../include/hidapi
#		${LIBUSB_1_INCLUDE_DIRS} # for Lightpack device
)

set(Leddevice_SOURCES
		LedDeviceFactory.cpp
		LedDeviceTest.cpp

#		${CURRENT_SOURCE_DIR}/LedDeviceLightpack.cpp
#		${CURRENT_SOURCE_DIR}/LedDeviceMultiLightpack.cpp
#		${CURRENT_SOURCE_DIR}/LedDevicePaintpack.cpp
#		${CURRENT_SOURCE_DIR}/LedDevicePiBlaster.cpp		
#		${CURRENT_SOURCE_DIR}/LedDeviceHyperionUsbasp.cpp
#		${CURRENT_SOURCE_DIR}/LedDevicePhilipsHue.cpp
)

if(ENABLE_LED_SERIAL)
	list(APPEND Leddevice_SOURCES LedRs232Device.cpp)
endif()

if(ENABLE_LED_TCPSOCKET)
	list(APPEND Leddevice_SOURCES TcpSocketDevice.cpp)
endif()

if(ENABLE_LED_UDPSOCKET)
	list(APPEND Leddevice_SOURCES UdpSocketDevice.cpp)
endif()

if(ENABLE_LED_SERIAL_TPM2)
	list(APPEND Leddevice_SOURCES LedDeviceTpm2Serial.cpp)
endif()

if(ENABLE_LED_ADALIGHT)
	list(APPEND Leddevice_SOURCES LedDeviceAdalight.cpp)
endif()

if(ENABLE_LED_AMBILED)
	list(APPEND Leddevice_SOURCES LedDeviceAmbiLed.cpp)
endif()

if(ENABLE_LED_ATMOLIGHT)
	list(APPEND Leddevice_SOURCES LedDeviceAtmo.cpp)
endif()

if(ENABLE_LED_SEDU)
	list(APPEND Leddevice_SOURCES LedDeviceSedu.cpp)
endif()

if(ENABLE_LED_TCPSOCKET_TPM2)
	list(APPEND Leddevice_SOURCES LedDeviceTpm2TcpSocket.cpp)
endif()

if(ENABLE_LED_UDPSOCKET_TPM2)
	list(APPEND Leddevice_SOURCES LedDeviceTpm2UdpSocket.cpp)
endif()

if(ENABLE_LED_APA102 OR ENABLE_LED_LPD6803 OR ENABLE_LED_LPD8806 OR ENABLE_LED_P9813 OR ENABLE_LED_WS2801)
	list(APPEND Leddevice_SOURCES LedSpiDevice.cpp)
endif()

if(ENABLE_LED_APA102)
	list(APPEND Leddevice_SOURCES LedDeviceAPA102.cpp)
endif()

if(ENABLE_LED_LPD6803)
	list(APPEND Leddevice_SOURCES LedDeviceLpd6803.cpp)
endif()

if(ENABLE_LED_LPD8806)
	list(APPEND Leddevice_SOURCES LedDeviceLpd8806.cpp)
endif()

if(ENABLE_LED_P9813)
	list(APPEND Leddevice_SOURCES LedDeviceP9813.cpp)
endif()

if(ENABLE_LED_WS2801)
	list(APPEND Leddevice_SOURCES LedDeviceWs2801.cpp)
endif()

if(ENABLE_LED_WS2812B_PWM)
	list(APPEND Leddevice_SOURCES LedDeviceWS2812bPwm.cpp)
endif()

add_library(leddevice
	${Leddevice_SOURCES}
)

target_link_libraries(leddevice
	hyperion-utils
#	${LIBUSB_1_LIBRARIES} #apt-get install libusb-1.0-0-dev
#	${CMAKE_THREAD_LIBS_INIT}
)

if(ENABLE_LED_SERIAL)
	target_link_libraries(leddevice serial)
	if(UNIX)
		target_link_libraries(leddevice pthread)
	endif()
endif()

if(ENABLE_LED_TCPSOCKET OR ENABLE_LED_UDPSOCKET)
    target_link_libraries(leddevice PocoNet)
endif()

#if(APPLE)
#	target_link_libraries(leddevice hidapi-mac)
#else()
#	target_link_libraries(leddevice hidapi-libusb)
#endif()
