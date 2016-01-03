# Check if the platform is a raspberry pi

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	file(STRINGS "/proc/cpuinfo" CPUINFO)
	foreach(line ${CPUINFO})
		if(${line} MATCHES "BCM2708" OR ${line} MATCHES "BCM2835")
			message(STATUS "Raspberry Pi detected")
			set(RPI_FOUND true CACHE BOOL "Raspberry Pi detected")
		endif()
	endforeach()
endif()

mark_as_advanced(RPI_FOUND)