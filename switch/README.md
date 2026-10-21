# Vangers for Nintendo Switch (homebrew, unofficial)

Unofficial port of [KranX/Vangers](https://github.com/KranX/Vangers) (SDL2 version of *Vangers: One For The Road*
by K-D Lab) to Nintendo Switch homebrew (Atmosphère custom firmware + hbmenu, libnx / switch-sdl2).
Not affiliated with K-D Lab or Nintendo. **This repository contains no game data** — you need your own legal copy
(for example from GOG). Code is GPLv3 like upstream; `third_party/clunk` is LGPL 2.1.

Русский: неофициальный порт Vangers на Switch (кастомная прошивка). Данных игры в репозитории нет — нужна своя
копия (GOG). Сборка и установка описаны ниже.

## Status
Works on hardware: launch, menus, races (D-pad and analog stick), pause/exit, save/load (system keyboard), controls
remapping, sound, touchscreen (handheld), sleep/resume. Known issues: some slowdowns, no 60 FPS option,
video/FFmpeg and memory limits not fully checked, online play not supported.

## Requirements
devkitPro with: `switch-dev switch-cmake switch-pkg-config switch-sdl2 switch-sdl2_net switch-ffmpeg
switch-libvorbis switch-libogg switch-zlib`.

## Build
    switch/build.sh            # produces build-switch/src/vangers.nro  (add "clean" to rebuild from scratch)
    switch/make-sd.sh /path/to/Vangers/data    # assembles ./sd/switch/vangers/{vangers.nro,data/}

Copy the resulting `switch` folder to the SD card root.

## Run
Launch from hbmenu using **title takeover** (hold **R** while starting a game/title). Applet mode (Album) has too
little memory and may reboot the console. Logs go to `sd:/switch/vangers/stdout.txt` and `stderr.txt`.

## Controls (defaults)
- Right stick: pointer. A (right button) or ZR: click. In races A is the "turbo" key, ZR always clicks.
- Minus: Escape / pause. Plus: inventory.
- Driving: left stick (analog, tilt = throttle) or D-pad; A turbo; B fire; X handbrake; Y open; L/R flip;
  ZL jump (hold to charge, release to jump); stick clicks toggle devices.
- Text input (save name, chat): system software keyboard.
- Touchscreen works as a mouse in handheld mode.
Controls can be remapped in the in-game settings.

## Notes
The 2D interface layer arrives filled with an opaque colour over the 3D view on this platform; the port treats that
colour as transparent inside the game view (see `src/road.cpp`, `lib/xgraph`). The root cause is unknown.
Compiler flags in `cmake/SwitchDeps.cmake` disable some optimizations the engine's code does not tolerate.

## Language
The game language is chosen by a command-line flag (`-russian`) in the original; on the Switch create an empty file
`sd:/switch/vangers/russian` to start in Russian (delete it to go back to English).

## 60 FPS (experimental)
Create an empty file `sd:/switch/vangers/fps60` to run the game at 60 FPS (physics and rendering; the world logic still
ticks at 20 Hz). Delete the file to return to 20 FPS. Heavy scenes may drop below 60.

## Debug log
By default `stdout.txt` only gets start-up information and errors. Create an empty file `sd:/switch/vangers/debug`
to also write per-5-second FPS statistics (`PERF`) and a per-stage breakdown of every frame longer than 120 ms (`SLOW frame`).
Please attach such a log when reporting performance problems.
