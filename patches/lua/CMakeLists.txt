cmake_minimum_required(VERSION 2.8)

project(lua)

set(lua_HEADERS
	src/lua.h
	src/lualib.h
	src/lauxlib.h
	src/luaconf.h
)
	
set(lua_SOURCES
    src/lapi.c
	src/lauxlib.c
	src/lbaselib.c
	src/lbitlib.c
	src/lcode.c
	src/lcorolib.c
	src/lctype.c
	src/ldblib.c
	src/ldebug.c
	src/ldo.c
	src/ldump.c
	src/lfunc.c
	src/lgc.c
	src/linit.c
	src/liolib.c
	src/llex.c
	src/lmathlib.c
	src/lmem.c
	src/loadlib.c
	src/lobject.c
	src/lopcodes.c
	src/loslib.c
	src/lparser.c
	src/lstate.c
	src/lstring.c
	src/lstrlib.c
	src/ltable.c
	src/ltablib.c
	src/ltm.c
	src/lua.c
	src/lundump.c
	src/lutf8lib.c
	src/lvm.c
	src/lzio.c
)

add_library(${PROJECT_NAME} STATIC ${lua_SOURCES})

install(TARGETS ${PROJECT_NAME}
	DESTINATION lib
)

install(FILES ${lua_HEADERS}
  DESTINATION include
)
