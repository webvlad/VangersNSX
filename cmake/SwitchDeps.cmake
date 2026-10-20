# Dependencies for the Nintendo Switch build (devkitPro portlibs).
#
# Configure with:
#   cmake -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake ..
#
# Required devkitPro packages: switch-dev switch-cmake switch-pkg-config
#   switch-sdl2 switch-sdl2_net switch-ffmpeg switch-libvorbis switch-libogg switch-zlib
# clunk is not packaged for the Switch; it is built from third_party/clunk.

if(NOT DEFINED ENV{DEVKITPRO})
	message(FATAL_ERROR "DEVKITPRO is not set. Run this from a devkitPro shell.")
endif()

set(SWITCH_PORTLIBS "$ENV{DEVKITPRO}/portlibs/switch" CACHE PATH "devkitPro Switch portlibs")

# The project builds with strict -std=c++17 (no GNU extensions). newlib then hides
# POSIX/BSD names (strdup, M_PI, ...) unless _GNU_SOURCE is defined.
add_compile_definitions(_GNU_SOURCE)

# The engine is 1990s code that relies on x86/old-gcc behaviour (type punning, signed overflow,
# float->int conversions). Keep the optimiser from exploiting that: the terrain renderer came
# out black on the first hardware run. Compile options come after CMAKE_CXX_FLAGS_RELEASE
# (-O2 -ftree-vectorize -ffast-math from the root CMakeLists), so these win.
add_compile_options(
	-fno-strict-aliasing
	-fwrapv
	-fno-fast-math
	-fno-tree-vectorize
	-fno-delete-null-pointer-checks
	-fno-aggressive-loop-optimizations
	-ffp-contract=off
)

# newlib's <ctype.h> macros (_X, _N, ...) clash with identifiers in the engine; see the header.
# C++ only: C sources (iniparser) do use the ctype macros.
add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:SHELL:-include ${PROJECT_SOURCE_DIR}/switch/switch_compat.h>")

# Headers: portlibs keeps everything in one include tree.
set(SDL2_INCLUDE_DIR "${SWITCH_PORTLIBS}/include/SDL2")
set(SDL2_NET_INCLUDE_DIRS "${SWITCH_PORTLIBS}/include/SDL2")
# The engine includes <avcodec.h> and <avformat.h> without the lib*/ prefix, so
# those directories must be on the include path (same set as cmake/FindFFMPEG.cmake).
# Do NOT add libavutil/: it contains time.h, which would shadow the system header.
set(FFMPEG_INCLUDE_DIRS
	"${SWITCH_PORTLIBS}/include/libavcodec"
	"${SWITCH_PORTLIBS}/include/libavformat"
	"${SWITCH_PORTLIBS}/include/libswscale"
	"${SWITCH_PORTLIBS}/include")
set(OGG_INCLUDE_DIR "${SWITCH_PORTLIBS}/include")
set(VORBIS_INCLUDE_DIR "${SWITCH_PORTLIBS}/include")
set(VORBISFILE_INCLUDE_DIR "${SWITCH_PORTLIBS}/include")
set(ZLIB_INCLUDE_DIR "${SWITCH_PORTLIBS}/include")

# clunk is built in-tree.
set(CLUNK_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/third_party/clunk")
set(CLUNK_LIBRARY clunk)

# The per-library variables used by the desktop build stay empty; every
# external library is linked through SWITCH_LIBS (static link line from
# pkg-config, which already has the right order and the transitive deps).
set(SDL2_LIBRARY "")
set(SDL2_NET_LIBRARY "")
set(OGG_LIBRARY "")
set(VORBIS_LIBRARY "")
set(VORBISFILE_LIBRARY "")
set(FFMPEG_LIBRARIES "")
set(ZLIB_LIBRARIES "")

find_program(SWITCH_PKG_CONFIG
	NAMES aarch64-none-elf-pkg-config
	HINTS "${SWITCH_PORTLIBS}/bin" "$ENV{DEVKITPRO}/portlibs/switch/bin")

set(SWITCH_LIBS "")
if(SWITCH_PKG_CONFIG)
	execute_process(
		COMMAND ${SWITCH_PKG_CONFIG} --static --libs
			sdl2 SDL2_net libswscale libswresample libavformat libavcodec libavutil
			vorbisfile vorbis ogg zlib
		OUTPUT_VARIABLE _switch_libs
		ERROR_VARIABLE _switch_libs_err
		RESULT_VARIABLE _switch_libs_rc
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	if(_switch_libs_rc EQUAL 0)
		separate_arguments(SWITCH_LIBS UNIX_COMMAND "${_switch_libs}")
	else()
		message(WARNING "pkg-config failed: ${_switch_libs_err}")
	endif()
endif()

if(NOT SWITCH_LIBS)
	# Best-effort fallback; ffmpeg's own dependencies (dav1d, libass, ...) may be missing.
	message(WARNING "Using the fallback Switch link line; install switch-pkg-config for the exact one.")
	set(SWITCH_LIBS
		-L${SWITCH_PORTLIBS}/lib
		-lSDL2_net -lSDL2 -lEGL -lGLESv2 -lglapi -ldrm_nouveau -lnx
		-lswscale -lswresample -lavformat -lavcodec -lavutil
		-lvorbisfile -lvorbis -logg -lz)
endif()
# Manual override (debugging / unusual setups): -DSWITCH_LIBS_OVERRIDE="-lfoo;-lbar"
if(SWITCH_LIBS_OVERRIDE)
	set(SWITCH_LIBS ${SWITCH_LIBS_OVERRIDE})
endif()
message(STATUS "Switch link line: ${SWITCH_LIBS}")

add_subdirectory("${PROJECT_SOURCE_DIR}/third_party/clunk" "${CMAKE_BINARY_DIR}/third_party/clunk")
