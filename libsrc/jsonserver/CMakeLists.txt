set(JsonServer_SOURCES
	JsonServer.cpp
	JsonClientConnection.cpp
)

add_library(jsonserver
		${JsonServer_SOURCES}
)

target_link_libraries(jsonserver
		hyperion
		hyperion-utils
		PocoFoundation
		PocoJSON
		PocoNet
)
