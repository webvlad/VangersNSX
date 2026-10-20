// Force-included into every C++ translation unit of the Nintendo Switch build.
//
// newlib's <ctype.h> defines single-letter macros (_U _L _N _S _P _C _X _B) as
// character-class flags. The engine uses some of those names as ordinary
// identifiers (for example `int _X = X << PARTICLE_SHIFT;`), which then fails to
// parse. Include <ctype.h> first so its include guard stays set, then drop the
// macros. In C++ the isalpha()-style macros are not defined by newlib, so
// nothing else depends on them. Not applied to C sources.
#ifndef VANGERS_SWITCH_COMPAT_H
#define VANGERS_SWITCH_COMPAT_H

#include <ctype.h>

// libstdc++ (bits/ctype_base.h) itself needs these macros while its headers are
// parsed, so pull in everything that reaches it BEFORE the macros are removed.
// Include guards keep these headers from being processed a second time.
#include <ios>
#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>
#include <string>

#ifdef _U
#	undef _U
#endif
#ifdef _L
#	undef _L
#endif
#ifdef _N
#	undef _N
#endif
#ifdef _S
#	undef _S
#endif
#ifdef _P
#	undef _P
#endif
#ifdef _C
#	undef _C
#endif
#ifdef _X
#	undef _X
#endif
#ifdef _B
#	undef _B
#endif

#endif
