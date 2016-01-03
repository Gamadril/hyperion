cmake_minimum_required(VERSION 2.8)

project(hyperion-remote)

set(hyperion-remote_HEADERS
	CustomParameter.h
	JsonConnection.h
	ColorTransformValues.h
	ColorValue.h
)

set(hyperion-remote_SOURCES
	hyperion-remote.cpp
	JsonConnection.cpp
)

add_executable(hyperion-remote
	${hyperion-remote_SOURCES}
)

target_link_libraries(hyperion-remote
	PocoFoundation
	PocoJSON
	PocoNet
)

