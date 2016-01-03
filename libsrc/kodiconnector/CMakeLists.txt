add_library(kodiconnector
	KodiConnector.cpp
)

target_link_libraries(kodiconnector
	hyperion
	PocoNet
)
