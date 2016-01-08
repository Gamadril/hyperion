# Define the main-project name
project(hyperion)

# define the minimum cmake version (as required by cmake)
cmake_minimum_required(VERSION 2.8)

find_package(Git)

# set the build options
option(ENABLE_LED_LPD6803 "Enable the lpd6803 LED device" OFF)
message(STATUS "ENABLE_LED_LPD6803 = " ${ENABLE_LED_LPD6803})

option(ENABLE_LED_LPD8806 "Enable the lpd8806 LED device" OFF)
message(STATUS "ENABLE_LED_LPD8806 = " ${ENABLE_LED_LPD8806})

option(ENABLE_LED_P9813 "Enable the p9813 LED device" OFF)
message(STATUS "ENABLE_LED_P9813 = " ${ENABLE_LED_P9813})

option(ENABLE_LED_WS2801 "Enable the ws2801 LED device" OFF)
message(STATUS "ENABLE_LED_WS2801 = " ${ENABLE_LED_WS2801})

option(ENABLE_LED_APA102 "Enable the apa102 LED device" OFF)
message(STATUS "ENABLE_LED_APA102 = " ${ENABLE_LED_APA102})

option(ENABLE_LED_TINKERFORGE "Enable the tinkerforge LED device" OFF)
message(STATUS "ENABLE_LED_TINKERFORGE = " ${ENABLE_LED_TINKERFORGE})

option(ENABLE_LED_ADALIGHT "Enable the adalight LED device" OFF)
message(STATUS "ENABLE_LED_ADALIGHT = " ${ENABLE_LED_ADALIGHT})

option(ENABLE_LED_AMBILED "Enable the ambiled LED device" OFF)
message(STATUS "ENABLE_LED_AMBILED = " ${ENABLE_LED_AMBILED})

option(ENABLE_LED_LIGHTPACK "Enable the lightpack LED device" OFF)
message(STATUS "ENABLE_LED_LIGHTPACK = " ${ENABLE_LED_LIGHTPACK})

option(ENABLE_LED_MULTILIGHTPACK "Enable the multilightpack LED device" OFF)
message(STATUS "ENABLE_LED_MULTILIGHTPACK = " ${ENABLE_LED_MULTILIGHTPACK})

option(ENABLE_LED_PAINTPACK "Enable the paintpack LED device" OFF)
message(STATUS "ENABLE_LED_PAINTPACK = " ${ENABLE_LED_PAINTPACK})

option(ENABLE_LED_PIBLASTER "Enable the piblaster LED device" OFF)
message(STATUS "ENABLE_LED_PIBLASTER = " ${ENABLE_LED_PIBLASTER})

option(ENABLE_LED_SEDU "Enable the sedu LED device" OFF)
message(STATUS "ENABLE_LED_SEDU = " ${ENABLE_LED_SEDU})

option(ENABLE_LED_USBASP "Enable the usbasp LED device" OFF)
message(STATUS "ENABLE_LED_USBASP = " ${ENABLE_LED_USBASP})

option(ENABLE_LED_PHILIPSHUE "Enable the philips hue LED device" OFF)
message(STATUS "ENABLE_LED_PHILIPSHUE = " ${ENABLE_LED_PHILIPSHUE})

option(ENABLE_LED_SERIAL_TPM2 "Enable the serial tpm2 LED device" OFF)
message(STATUS "ENABLE_LED_SERIAL_TPM2 = " ${ENABLE_LED_SERIAL_TPM2})

option(ENABLE_LED_ATMOLIGHT "Enable the atmolight LED device" OFF)
message(STATUS "ENABLE_LED_ATMOLIGHT = " ${ENABLE_LED_ATMOLIGHT})

option(ENABLE_LED_WS2812B_PWM "Enable the ws2812 (pwm) LED device" OFF)
message(STATUS "ENABLE_LED_WS2812B_PWM = " ${ENABLE_LED_WS2812B_PWM})

option(ENABLE_LED_TINKERFORGE "Enable the TINKERFORGE device" OFF)
message(STATUS "ENABLE_LED_TINKERFORGE = " ${ENABLE_LED_TINKERFORGE})

option(ENABLE_LED_TCPSOCKET_TPM2 "Enable the tpm2 LED device over tcp socket" OFF)
message(STATUS "ENABLE_LED_TCPSOCKET_TPM2 = " ${ENABLE_LED_TCPSOCKET_TPM2})

option(ENABLE_LED_UDPSOCKET_TPM2 "Enable the tpm2 LED device over udp socket" OFF)
message(STATUS "ENABLE_LED_UDPSOCKET_TPM2 = " ${ENABLE_LED_UDPSOCKET_TPM2})

if(ENABLE_LED_SERIAL_TPM2 OR ENABLE_LED_ADALIGHT OR ENABLE_LED_AMBILED OR ENABLE_LED_ATMOLIGHT OR ENABLE_LED_SEDU)
	set(ENABLE_LED_SERIAL ON)
	mark_as_advanced(ENABLE_LED_SERIAL)
endif()

if(ENABLE_LED_TCPSOCKET_TPM2)
	set(ENABLE_LED_TCPSOCKET ON)
	mark_as_advanced(ENABLE_LED_TCPSOCKET)
endif()

if(ENABLE_LED_UDPSOCKET_TPM2)
    set(ENABLE_LED_UDPSOCKET ON)
    mark_as_advanced(ENABLE_LED_UDPSOCKET)
endif()

option(ENABLE_GRABBER_DISPMANX "Enable the RPi dispmanx grabber" OFF)
message(STATUS "ENABLE_GRABBER_DISPMANX = " ${ENABLE_GRABBER_DISPMANX})

option(ENABLE_GRABBER_V4L2 "Enable the V4L2 grabber" OFF)
message(STATUS "ENABLE_GRABBER_V4L2 = " ${ENABLE_GRABBER_V4L2})

option(ENABLE_GRABBER_X11 "Enable the X11 grabber" OFF)
message(STATUS "ENABLE_GRABBER_X11 = " ${ENABLE_GRABBER_X11})

option(ENABLE_GRABBER_FB "Enable the framebuffer grabber" OFF)
message(STATUS "ENABLE_GRABBER_FB = " ${ENABLE_GRABBER_FB})

option(ENABLE_GRABBER_OSX "Enable the osx grabber" OFF)
message(STATUS "ENABLE_GRABBER_OSX = " ${ENABLE_GRABBER_OSX})

if(ENABLE_GRABBER_DISPMANX OR ENABLE_GRABBER_V4L2 OR ENABLE_GRABBER_X11 OR ENABLE_GRABBER_FB OR ENABLE_GRABBER_OSX)
	set(ENABLE_GRABBER ON)
	mark_as_advanced(ENABLE_GRABBER)
endif()

option(ENABLE_EFFECT_ENGINE "Enable the effect engine" OFF)
message(STATUS "ENABLE_EFFECT_ENGINE = " ${ENABLE_EFFECT_ENGINE})

option(ENABLE_KODI_CONNECTION "Enable Kodi rpc connection" OFF)
message(STATUS "ENABLE_KODI_CONNECTION = " ${ENABLE_KODI_CONNECTION})


option(ENABLE_SERVER_BOBLIGHT "Enable boblight server" OFF)
message(STATUS "ENABLE_SERVER_BOBLIGHT = " ${ENABLE_SERVER_BOBLIGHT})

option(ENABLE_SERVER_JSON "Enable json server" OFF)
message(STATUS "ENABLE_SERVER_JSON = " ${ENABLE_SERVER_JSON})

set(TARGET_DEVICE "" CACHE STRING "Target if cross compiling")
message(STATUS "TARGET_DEVICE = " ${TARGET_DEVICE})


# Createt the configuration file
# configure a header file to pass some of the CMake settings
# to the source code
configure_file("${PROJECT_SOURCE_DIR}/HyperionConfig.h.in" "${PROJECT_BINARY_DIR}/HyperionConfig.h")
include_directories("${PROJECT_BINARY_DIR}")

# Add project specific cmake modules (find, etc)
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Define the global output path of binaries
SET(LIBRARY_OUTPUT_PATH    ${PROJECT_BINARY_DIR}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

# Add the project include directory as additional include path
include_directories(${CMAKE_SOURCE_DIR}/dependencies/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

#set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_BUILD_TYPE "MinSizeRel")

# enable C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

# check if cross compiling
if(NOT TARGET_DEVICE STREQUAL "")
	get_filename_component(TARGET_DEVICE_FOLDER "${CMAKE_SOURCE_DIR}/platforms/${TARGET_DEVICE}" ABSOLUTE)
	include("${TARGET_DEVICE_FOLDER}/build.cmake")
	set(TOOLCHAIN_ARG "-DCMAKE_TOOLCHAIN_FILE=${TARGET_DEVICE_FOLDER}/toolchain.cmake")
endif()
include(ExternalProject)
ExternalProject_Add(poco-lib
    PREFIX "${PROJECT_BINARY_DIR}/external/poco"
    GIT_REPOSITORY "https://github.com/pocoproject/poco.git"
    GIT_TAG "poco-1.6.1"
	INSTALL_DIR ${CMAKE_BINARY_DIR}/poco-lib
	UPDATE_COMMAND ""
    CMAKE_ARGS -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} ${TOOLCHAIN_ARG} -DENABLE_XML=OFF -DENABLE_MONGODB=OFF -DENABLE_NETSSL=OFF -DENABLE_CRYPTO=OFF -DENABLE_DATA=OFF -DENABLE_DATA_SQLITE=OFF -DENABLE_DATA_MYSQL=OFF -DENABLE_DATA_ODBC=OFF -DENABLE_ZIP=OFF  -DENABLE_PAGECOMPILER=OFF -DENABLE_PAGECOMPILER_FILE2PAGE=OFF -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)
ExternalProject_Get_Property(poco-lib INSTALL_DIR)
include_directories(${INSTALL_DIR}/include)
link_directories(${INSTALL_DIR}/lib)
install(DIRECTORY ${INSTALL_DIR}/lib DESTINATION . PATTERN "cmake" EXCLUDE)

if(ENABLE_LED_SERIAL)
ExternalProject_Add(serial-lib
     PREFIX "${PROJECT_BINARY_DIR}/external/serial"
     GIT_REPOSITORY "https://github.com/wjwwood/serial.git"
     GIT_TAG "1.1.6"
	 INSTALL_DIR ${CMAKE_BINARY_DIR}/serial-lib
	 UPDATE_COMMAND ""
	 PATCH_COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/patches/serial/CMakeLists.txt" <SOURCE_DIR>/CMakeLists.txt
	 CMAKE_ARGS ${TOOLCHAIN_ARG} -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)
ExternalProject_Get_Property(serial-lib INSTALL_DIR)
include_directories(${INSTALL_DIR})
link_directories(${INSTALL_DIR}/lib)
endif()

include (cmake/FindRPi.cmake)
if(RPI_FOUND OR TARGET_DEVICE STREQUAL "rpi")
	ExternalProject_Add(rpi-firmware
		PREFIX "${PROJECT_BINARY_DIR}/external/rpi-firmware"
		DOWNLOAD_NAME "firmware.tar.gz"
		URL "https://github.com/Hexxeh/rpi-firmware/tarball/HEAD"
		UPDATE_COMMAND ""
		PATCH_COMMAND ""
		BUILD_COMMAND ""
		CONFIGURE_COMMAND ""
		INSTALL_COMMAND ""
	)
	ExternalProject_Get_Property(rpi-firmware SOURCE_DIR)
	include_directories("${SOURCE_DIR}/vc/sdk/opt/vc/include")
	#TODO broken header files?
	include_directories("${SOURCE_DIR}/vc/sdk/opt/vc/include/interface/vcos/pthreads")
	include_directories("${SOURCE_DIR}/vc/sdk/opt/vc/include/interface/vmcs_host/linux")
	link_directories("${SOURCE_DIR}/vc/hardfp/opt/vc/lib")
endif()

if(ENABLE_EFFECT_ENGINE)
	ExternalProject_Add(lua-lib
	     PREFIX "${PROJECT_BINARY_DIR}/external/lua"
		 URL "http://www.lua.org/ftp/lua-5.3.1.tar.gz"
		 URL_MD5 "797adacada8d85761c079390ff1d9961"
		 UPDATE_COMMAND ""
		 PATCH_COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/patches/lua/CMakeLists.txt" <SOURCE_DIR>/CMakeLists.txt
		 INSTALL_DIR ${CMAKE_BINARY_DIR}/lua-lib
		 CMAKE_ARGS ${TOOLCHAIN_ARG} -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
	)
	ExternalProject_Get_Property(lua-lib INSTALL_DIR)
	include_directories("${INSTALL_DIR}/include")
	link_directories("${INSTALL_DIR}/lib")
endif()

#add libusb and pthreads
#find_package(libusb-1.0 REQUIRED)
#find_package(Threads REQUIRED)

configure_file(bin/install_hyperion.sh ${LIBRARY_OUTPUT_PATH} @ONLY)
configure_file(config/hyperion.config.json ${LIBRARY_OUTPUT_PATH} @ONLY)

# Add the source/lib directories
add_subdirectory(libsrc)
add_subdirectory(src)
#add_subdirectory(test)

# Add the doxygen generation directory
#add_subdirectory(doc)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/effects DESTINATION .)
install(FILES ${PROJECT_SOURCE_DIR}/config/hyperion.config.json DESTINATION config)

# do packaging
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Hyperion ambilight project")
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}")
SET(CPACK_STRIP_FILES ON)
SET(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_EXECUTABLES "hyperion")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 1)

include(CPack)