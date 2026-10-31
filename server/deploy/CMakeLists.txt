# Standalone build of vangers_server only (no Clunk/FFmpeg/Vorbis needed).
#   cmake -S server/deploy -B build-server -DCMAKE_BUILD_TYPE=Release
#   cmake --build build-server -j
# Dependencies: SDL2, SDL2_net, zlib, a C++17 compiler.
cmake_minimum_required(VERSION 3.16)
project(vangers_server_standalone C CXX)

get_filename_component(VANGERS_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(CMAKE_MODULE_PATH "${VANGERS_ROOT}/cmake")

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 REQUIRED)
find_package(SDL2_net REQUIRED)
find_package(ZLIB REQUIRED)

# Same flags as the root CMakeLists.txt (the sources assume signed char).
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
add_definitions(-D_USE_MATH_DEFINES)
add_compile_options(-fsigned-char)

execute_process(COMMAND git rev-parse --abbrev-ref HEAD
	WORKING_DIRECTORY "${VANGERS_ROOT}" OUTPUT_VARIABLE GIT_BRANCH
	OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
execute_process(COMMAND git rev-parse HEAD
	WORKING_DIRECTORY "${VANGERS_ROOT}" OUTPUT_VARIABLE GIT_COMMIT_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

include_directories(SYSTEM ${SDL2_INCLUDE_DIR} ${SDL2_NET_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIR})
include_directories(
	"${VANGERS_ROOT}/lib/xtool"
	"${VANGERS_ROOT}/lib/xgraph"
	"${VANGERS_ROOT}/src"
	"${VANGERS_ROOT}/server"
)

set(XTOOL_DIR "${VANGERS_ROOT}/lib/xtool")
add_library(xtool STATIC
	${XTOOL_DIR}/xtcore.cpp
	${XTOOL_DIR}/xrecorder/xrec.cpp
	${XTOOL_DIR}/port.cpp
	${XTOOL_DIR}/xerrhead.cpp
	${XTOOL_DIR}/xbuffer/xbcnvin.cpp
	${XTOOL_DIR}/xbuffer/xbcnvout.cpp
	${XTOOL_DIR}/xbuffer/xbcore.cpp
	${XTOOL_DIR}/xbuffer/xbsearch.cpp
	${XTOOL_DIR}/xmsgbuffer/xmsgbuf.cpp
	${XTOOL_DIR}/xresource/zip_resource.cpp
	${XTOOL_DIR}/xstream/xscnvin.cpp
	${XTOOL_DIR}/xstream/xscnvout.cpp
	${XTOOL_DIR}/xstream/xscore.cpp
	${XTOOL_DIR}/xstream/xsenlv.cpp
	${XTOOL_DIR}/xstream/xsgetlin.cpp
	${XTOOL_DIR}/xstream/xsinput.cpp
	${XTOOL_DIR}/xstream/xsoutput.cpp
	${XTOOL_DIR}/xstream/xsrdwr.cpp
	${XTOOL_DIR}/xstream/xsserv.cpp
	${XTOOL_DIR}/xutil/xclock.cpp
	${XTOOL_DIR}/iniparser/dictionary.c
	${XTOOL_DIR}/iniparser/iniparser.c
	${XTOOL_DIR}/lang.cpp
)
target_compile_definitions(xtool PUBLIC
	"GIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\""
	"GIT_BRANCH=\"${GIT_BRANCH}\""
	"GITHUB_COMMIT_HASH=\"\""
	"GITHUB_BRANCH=\"\"")
if(WIN32)
	target_link_libraries(xtool dbghelp)
endif()

add_executable(vangers_server
	${VANGERS_ROOT}/server/kdsplus.cpp
	${VANGERS_ROOT}/src/xsocket.cpp
	${VANGERS_ROOT}/lib/xtool/lang.cpp
	${VANGERS_ROOT}/lib/xgraph/xgraph_dummy.cpp
	${VANGERS_ROOT}/server/server.cpp
)
if(WIN32)
	target_link_libraries(vangers_server ${SDL2_LIBRARY} ${SDL2_NET_LIBRARY} ws2_32 xtool ${ZLIB_LIBRARIES})
else()
	find_package(Threads REQUIRED)
	target_link_libraries(vangers_server ${SDL2_LIBRARY} ${SDL2_NET_LIBRARY} xtool ${ZLIB_LIBRARIES} Threads::Threads)
endif()
