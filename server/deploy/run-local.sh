#!/usr/bin/env bash
# Local check: build vangers_server and run it in the foreground (Ctrl+C = clean exit, Rating.lst saved).
# Works in: Linux / WSL (Ubuntu), or the MSYS2 "MINGW64" shell on Windows.
#   Linux/WSL:  sudo apt install build-essential cmake pkg-config zlib1g-dev libsdl2-dev libsdl2-net-dev
#   MSYS2:      pacman -S --needed mingw-w64-x86_64-{gcc,cmake,ninja,SDL2,SDL2_net,zlib}
# Usage: ./server/deploy/run-local.sh [port]
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
PORT="${1:-2197}"
BUILD="$REPO/build-server-local"
RUN="$REPO/server-local-run"      # Rating.lst and the log end up here

GEN=()
command -v ninja >/dev/null 2>&1 && GEN=(-G Ninja)
cmake -S "$HERE" -B "$BUILD" "${GEN[@]}" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD" -j --target vangers_server

mkdir -p "$RUN"
BIN="$BUILD/vangers_server"; [ -x "$BIN" ] || BIN="$BUILD/vangers_server.exe"
cd "$RUN"
echo "Starting on TCP $PORT (Ctrl+C to stop)"
exec "$BIN" "/p$PORT" /s /b-
