
# Define the current source locations
set(CURRENT_HEADER_DIR ${CMAKE_SOURCE_DIR}/include/boblightserver)
set(CURRENT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/libsrc/boblightserver)

# Group the headers that go through the MOC compiler
set(		${CURRENT_HEADER_DIR}/BoblightServer.h
		${CURRENT_SOURCE_DIR}/BoblightClientConnection.h
)

set(BoblightServer_HEADERS
)

set(BoblightServer_SOURCES
		${CURRENT_SOURCE_DIR}/BoblightServer.cpp
		${CURRENT_SOURCE_DIR}/BoblightClientConnection.cpp
)

add_library(boblightserver
		${BoblightServer_HEADERS}
		${BoblightServer_SOURCES}
)

target_link_libraries(boblightserver
		hyperion
        hyperion-utils
)
