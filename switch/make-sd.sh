#!/bin/bash
# Assemble an SD card folder: switch/make-sd.sh /path/to/your/Vangers/data [output_dir]
# The game data must come from your own legal copy (e.g. GOG). It is NOT part of this repository.
set -e
DATA="${1:?usage: make-sd.sh /path/to/Vangers/data [output_dir]}"
OUT="${2:-sd}/switch/vangers"
HERE="$(cd "$(dirname "$0")/.." && pwd)"
NRO="$(find "$HERE/build-switch" -name vangers.nro | head -1)"
[ -n "$NRO" ] || { echo "vangers.nro not found, run switch/build.sh first"; exit 1; }
mkdir -p "$OUT/data"
cp "$NRO" "$OUT/vangers.nro"
rsync -a --exclude savegame --exclude 'logfile.txt' --exclude 'options.dat' --exclude 'controls.dat' \
	--exclude 'tmp_f1.txt' --exclude '*.log' --exclude '*.dxvk-cache' "$DATA"/ "$OUT/data/"
echo "Copy '$(dirname "$(dirname "$OUT")")/switch' to the root of your SD card."
