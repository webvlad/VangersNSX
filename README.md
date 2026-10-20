# Vangers for Nintendo Switch #

![Vangers](http://cdn.akamai.steamstatic.com/steam/apps/264080/header.jpg?t=1447359431)

[![Join the chat at https://t.me/vangers](https://patrolavia.github.io/telegram-badge/chat.svg)](https://t.me/vangers)

Unofficial Nintendo Switch homebrew port of [KranX/Vangers](https://github.com/KranX/Vangers), the SDL2 version of
*Vangers: One For The Road* by K-D Lab — a video game that combines elements of the racing and role-playing genres.
This fork is based on upstream commit `9a0aaec` (SDL2) and lives in the `switch` branch. It is not affiliated with
K-D Lab or Nintendo, and it contains **no game data**.

All source code is published under the GPLv3 license. `third_party/clunk` is a vendored copy of
[clunk](https://github.com/stalkerg/clunk) under LGPL 2.1.

The resources necessary for the game (maps, sounds, textures, etc.) must come from your own copy of the game
purchased here:

http://store.steampowered.com/app/264080

http://www.gog.com/game/vangers

The port was developed and tested with the GOG version. Do not distribute game data or builds that include it.

## Status ##

Working on hardware (Atmosphère custom firmware): launch from hbmenu, menus, races (D-pad and analog stick), pause
and exit to menu, save/load with the system keyboard, controls remapping, sound, touchscreen (handheld mode),
sleep/resume.

Known issues and limitations:

* Some slowdowns; performance has not been optimized yet.
* Multiplayer is not supported on Switch.
* No 60 FPS option in the menu; change-target and weapon slots 1-4 have no default bindings.
* The 2D interface layer is drawn opaque over the 3D view on this platform. The port works around it by treating
  that colour as transparent inside the game view (see `src/road.cpp` and `lib/xgraph`); the root cause is unknown.

## Required libraries ##

Install through devkitPro (`dkp-pacman` / `pacman`):

* `switch-dev switch-cmake switch-pkg-config`
* `switch-sdl2` (SDL2), `switch-sdl2_net`
* `switch-libvorbis switch-libogg`
* `switch-ffmpeg` (6.0 or newer)
* `switch-zlib`
* clunk — already included in `third_party/clunk`

## Build ##

In a devkitPro shell (MSYS2 on Windows):

```sh
switch/build.sh                              # build-switch/src/vangers.nro; add "clean" for a full rebuild
switch/make-sd.sh /path/to/Vangers/data      # assembles ./sd/switch/vangers/{vangers.nro,data/}
```

Copy the resulting `switch` folder to the root of the SD card. More details are in
[`switch/README.md`](switch/README.md). For other platforms see the upstream
[wiki pages](https://github.com/KranX/Vangers/wiki).

## Running ##

Start the game from hbmenu using **title takeover** (hold **R** while launching a game/title). Applet mode (Album)
has too little memory and may reboot the console. Logs are written to `sd:/switch/vangers/stdout.txt` and
`stderr.txt`; please attach them to bug reports. Saves are stored under `sd:/switch/vangers/data/`.

## Gamepad and touch input ##

Default bindings:

* **Left stick** drives (analog, throttle follows the tilt); **D-pad** also steers/accelerates.
* **A** (right button) turbo, **B** (bottom) fire all weapons, **X** handbrake, **Y** open/pick up,
  **L/R** turn over left/right, **ZL** jump (hold to charge, release to jump), **stick clicks** toggle devices.
* **+** inventory, **−** Escape / pause menu.
* **Right stick** moves the UI cursor; **A** or **ZR** clicks. In races ZR always clicks.
* Save names and chat use the system software keyboard.
* The touchscreen works as a mouse in handheld mode.

Bindings can be changed on the in-game controls screen (buttons are captured from the gamepad; − cancels).
They are stored in `controls.dat` and `options.dat`; delete these files to restore defaults.

## Development notes ##

Working notes of the port (what was fixed and why, in Russian) are in [`switch/PORT_NOTES.md`](switch/PORT_NOTES.md). The upstream multiplayer plans
(`multiplayer-*.md`) and server sources are kept unchanged for reference and are not used by the Switch build.
Compiler flags in `cmake/SwitchDeps.cmake` disable some optimizations that the engine's code does not tolerate.

Upstream project: https://github.com/KranX/Vangers (issues about the original game and other platforms belong there).
