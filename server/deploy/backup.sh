#!/usr/bin/env bash
# Daily copy of Rating.lst (installed to /opt/vangers-server/backup.sh, run by cron).
set -euo pipefail
[ -f /etc/vangers-server.env ] && . /etc/vangers-server.env
SRC=/var/lib/vangers-server/Rating.lst
DST=/var/backups/vangers-server
KEEP="${BACKUP_KEEP:-14}"
[ -f "$SRC" ] || exit 0
mkdir -p "$DST"
cp -p "$SRC" "$DST/Rating-$(date +%Y%m%d-%H%M).lst"
ls -1t "$DST"/Rating-*.lst | tail -n +"$((KEEP + 1))" | xargs -r rm -f --
