#SET(CMAKE_SKIP_BUILD_RPATH TRUE)
#SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
if(APPLE)
    SET(CMAKE_INSTALL_RPATH "@executable_path/lib")
else()
    SET(CMAKE_INSTALL_RPATH "\$ORIGIN/lib")
endif()
#SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

add_executable(hyperiond
		hyperiond.cpp)

target_link_libraries(hyperiond
		PocoUtil
		PocoFoundation
		PocoJSON
		hyperion
)

if (ENABLE_SERVER_JSON)
	target_link_libraries(hyperiond jsonserver)
endif ()

if (ENABLE_KODI_CONNECTION)
	target_link_libraries(hyperiond kodiconnector)
endif ()

if (ENABLE_EFFECT_ENGINE)
	target_link_libraries(hyperiond effectengine)
endif ()

if (ENABLE_SERVER_BOBLIGHT)
	target_link_libraries(hyperiond boblightserver)
endif ()

if (ENABLE_GRABBER)
	target_link_libraries(hyperiond framegrabber)
endif ()

target_link_libraries(hyperiond ${CROSS_LIBS})

install(TARGETS hyperiond RUNTIME DESTINATION .)
