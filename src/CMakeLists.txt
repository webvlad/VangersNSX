include_directories(
	"${PROJECT_SOURCE_DIR}/lib/xtool"
	"${PROJECT_SOURCE_DIR}/lib/xgraph"
	"${PROJECT_SOURCE_DIR}/lib/xsound"
)

SET(vangers_SRCS
	3d/3d_math.cpp
	3d/3dgraph.cpp
	3d/3dobject.cpp
	3d/dynamics.cpp
	3d/optimize.cpp
	actint/aci_scr.cpp
	actint/actint.cpp
	actint/actintml.cpp
	actint/ascr_fnc.cpp
	actint/ascript.cpp
	actint/chtree.cpp
	actint/sscript.cpp
	backg.cpp
	dast/poly3d.cpp
	iscreen/controls.cpp
	iscreen/hfont.cpp
	iscreen/i_chat.cpp
	iscreen/iextern.cpp
	iscreen/ikeys.cpp
	iscreen/imap_fnc.cpp
	iscreen/iml.cpp
	iscreen/iscr_fnc.cpp
	iscreen/iscreen.cpp
	iscreen/iscript.cpp
	iscreen/ivmap.cpp
	iscreen/script.cpp
	iscreen/smooth.cpp
	network.cpp
	palette.cpp
	particle/bmp2scr.cpp
	particle/deform.cpp
	particle/df.cpp
	particle/light.cpp
	particle/particle.cpp
	particle/partmap.cpp
	particle/pmaptype.cpp
	rle.cpp
	road.cpp
	sound/music.cpp
	sound/sound.cpp
	#sqint/sqibody.cpp #FOR surmap
	#sqint/sqicore.cpp #FOR surmap
	terra/compress.cpp
	terra/huff1.cpp
	terra/land.cpp
	terra/perpslop.cpp
	terra/siderend.cpp #ASM
	terra/slopskip.cpp
	terra/splay.cpp
	terra/vmap.cpp #ASM
	units/effect.cpp
	units/hobj.cpp
	units/items.cpp
	units/mechos.cpp
	units/moveland.cpp
	units/sensor.cpp
	units/track.cpp
	uvs/diagen.cpp
	uvs/screen.cpp
	uvs/univang.cpp
	xjoystick.cpp
	xpad_cursor.cpp
	xsocket.cpp
	zmod_client.cpp
	${WINDOWS_RES}
        actint/layout.h)

if (ANDROID)
	ADD_LIBRARY(vangers STATIC ${vangers_SRCS}
		$<TARGET_OBJECTS:xtool>
		$<TARGET_OBJECTS:xgraph>
		$<TARGET_OBJECTS:xsound>
	)
else()
	ADD_EXECUTABLE(vangers ${vangers_SRCS})

	IF(WIN32)
		SET(WIN_LIB
			wsock32
			#-mwindows
			#-Wl,-subsystem,windows
		)
	ENDIF(WIN32)

	TARGET_LINK_LIBRARIES(vangers
		${SDL2_LIBRARY}
		${OGG_LIBRARY}
		${VORBIS_LIBRARY}
		${VORBISFILE_LIBRARY}
		${CLUNK_LIBRARY}
		${SDL2_NET_LIBRARY}
		${FFMPEG_LIBRARIES}
		${ZLIB_LIBRARIES}
		${OPENGL_LIBRARIES}
		${WIN_LIB}
		${STEAM_LIBS}
		xtool
		xgraph
		xsound
	)

	IF(NINTENDO_SWITCH)
		# Static libraries reference each other in both directions: rescan them.
		# (Plain linker flags: CMake's LINK_GROUP feature is not defined for this toolchain.)
		TARGET_LINK_LIBRARIES(vangers
			-Wl,--start-group xtool xgraph xsound clunk -Wl,--end-group
			${SWITCH_LIBS}
		)
		nx_generate_nacp(
			OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/vangers.nacp
			NAME "Vangers"
			AUTHOR "K-D Lab / KranX"
			VERSION "2.0-switch"
		)
		nx_create_nro(vangers NACP ${CMAKE_CURRENT_BINARY_DIR}/vangers.nacp)
	ENDIF()
endif(ANDROID)