set(TOOLCHAIN_URL "http://releases.linaro.org/components/toolchain/binaries/latest-5.1/arm-linux-gnueabihf/gcc-linaro-5.1-2015.08-x86_64_arm-linux-gnueabihf.tar.xz")

set(CMAKE_SYSTEM_NAME Linux)

set(TOOLCHAIN_DIR $ENV{TOOLCHAIN_DIR})
message(STATUS "TOOLCHAIN dir: " ${TOOLCHAIN_DIR})

if(NOT EXISTS ${TOOLCHAIN_DIR})
	message(STATUS "Downloading Raspberry Pi toolchain...")
	file(DOWNLOAD ${TOOLCHAIN_URL} ${TOOLCHAIN_DIR}/toolchain.tar.xz STATUS status)
	list(GET status 0 status_code)
	if(NOT status_code EQUAL 0)
	  message(FATAL_ERROR "error: download failed")
	endif()
	message(STATUS "Extracting Raspberry Pi toolchain...")
	execute_process(COMMAND tar --strip-components=1 -xJf ${TOOLCHAIN_DIR}/toolchain.tar.xz WORKING_DIRECTORY ${TOOLCHAIN_DIR})
	execute_process(COMMAND rm ${TOOLCHAIN_DIR}/toolchain.tar.xz)
endif()

SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-g++)
SET(CMAKE_STRIP ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-strip)

SET(CMAKE_FIND_ROOT_PATH  ${TOOLCHAIN_DIR})
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)

