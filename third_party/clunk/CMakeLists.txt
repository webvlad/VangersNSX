# clunk 3D audio library, built as a static library for the Nintendo Switch.
# Sources are in the clunk/ subdirectory so that <clunk/clunk.h> resolves when
# this directory is on the include path.
include(CheckCXXSymbolExists)

set(CLUNK_SOURCES
	clunk/buffer.cpp
	clunk/clunk_ex.cpp
	clunk/context.cpp
	clunk/distance_model.cpp
	clunk/kemar.c
	clunk/logger.cpp
	clunk/object.cpp
	clunk/sample.cpp
	clunk/sdl_ex.cpp
	clunk/source.cpp
	clunk/stream.cpp
	clunk/clunk_c.cpp
)

add_library(clunk STATIC ${CLUNK_SOURCES})
target_include_directories(clunk PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/clunk ${SDL2_INCLUDE_DIR})
# Static library: no dllimport/visibility decoration.
target_compile_definitions(clunk PUBLIC CLUNKAPI=)

check_cxx_symbol_exists(pow10f "math.h" HAVE_POW10F_FUNCTION)
check_cxx_symbol_exists(log2f "math.h" HAVE_LOG2F_FUNCTION)
if(NOT HAVE_POW10F_FUNCTION)
	target_compile_definitions(clunk PRIVATE NEED_POW10F)
endif()
if(NOT HAVE_LOG2F_FUNCTION)
	target_compile_definitions(clunk PRIVATE NEED_LOG2F)
endif()
