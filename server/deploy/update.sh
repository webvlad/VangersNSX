#!/usr/bin/env bash
# Pull the latest commit of the current branch and reinstall.  Usage: sudo ./server/deploy/update.sh
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
OWNER="$(stat -c %U "$REPO")"
# git runs as the owner of the checkout (root would hit "dubious ownership" and write root-owned files)
sudo -u "$OWNER" git -C "$REPO" pull --ff-only
exec "$HERE/install.sh"
