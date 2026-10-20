#!/bin/bash
# Build the Nintendo Switch homebrew (.nro). Run inside a devkitPro shell (MSYS2 on Windows).
# Usage: switch/build.sh [clean]
set -e
cd "$(dirname "$0")/.."
export DEVKITPRO="${DEVKITPRO:-/opt/devkitpro}"
export PATH="$DEVKITPRO/devkitA64/bin:$DEVKITPRO/tools/bin:$PATH"
[ -f "$DEVKITPRO/cmake/Switch.cmake" ] || { echo "devkitPro / switch-cmake not found in $DEVKITPRO"; exit 1; }
[ "$1" = "clean" ] && rm -rf build-switch
mkdir -p build-switch && cd build-switch
cmake -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/Switch.cmake" -DCMAKE_BUILD_TYPE=Release ..
make -j"$(nproc 2>/dev/null || echo 8)"
echo "Result: $(find . -name vangers.nro | head -1)"
