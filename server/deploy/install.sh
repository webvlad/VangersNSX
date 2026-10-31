#!/usr/bin/env bash
# Build vangers_server from THIS checkout and (re)install it as a systemd service.
# Idempotent: the same script is used for the first install and for updates.
# Usage (Ubuntu/Debian VPS):  sudo ./server/deploy/install.sh
set -euo pipefail

if [ "$(id -u)" -ne 0 ]; then echo "Run as root: sudo $0" >&2; exit 1; fi

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
BUILD="$REPO/build-server"
PREFIX=/opt/vangers-server
DATA=/var/lib/vangers-server
ENVFILE=/etc/vangers-server.env

echo "==> Build dependencies"
export DEBIAN_FRONTEND=noninteractive
apt-get update -qq
apt-get install -y -qq --no-install-recommends \
	build-essential cmake git pkg-config zlib1g-dev libsdl2-dev libsdl2-net-dev

echo "==> Build (Release)"
cmake -S "$HERE" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD" -j"$(nproc)" --target vangers_server

echo "==> Install files"
id vangers >/dev/null 2>&1 || useradd --system --home "$DATA" --shell /usr/sbin/nologin vangers
install -d -o root -g root "$PREFIX/bin"
install -d -o vangers -g vangers -m 750 "$DATA" /var/backups/vangers-server
install -m 755 "$BUILD/vangers_server" "$PREFIX/bin/vangers_server.new"
mv -f "$PREFIX/bin/vangers_server.new" "$PREFIX/bin/vangers_server"
install -m 755 "$HERE/backup.sh" "$PREFIX/backup.sh"
if [ ! -f "$ENVFILE" ]; then install -m 644 "$HERE/.env.example" "$ENVFILE"; echo "Created $ENVFILE"; fi
install -m 644 "$HERE/vangers-server.service" /etc/systemd/system/vangers-server.service
install -m 644 "$HERE/logrotate.conf" /etc/logrotate.d/vangers-server
echo '17 4 * * * root /opt/vangers-server/backup.sh' > /etc/cron.d/vangers-server-backup
chmod 644 /etc/cron.d/vangers-server-backup

echo "==> (Re)start service"
systemctl daemon-reload
systemctl enable vangers-server >/dev/null
systemctl restart vangers-server
sleep 2
systemctl --no-pager --lines=8 status vangers-server || true
echo
echo "Done. Port check:  ss -ltnp | grep \"\$(. $ENVFILE; echo \$VANGERS_PORT)\""
echo "Don't forget to open the TCP port in the firewall (see server/deploy/README.md)."
