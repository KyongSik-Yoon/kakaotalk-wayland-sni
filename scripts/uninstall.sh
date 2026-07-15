#!/usr/bin/env bash
set -euo pipefail

prefix=${PREFIX:-"$HOME/.local"}

systemctl --user disable --now kakaotalk-wayland-sni.service 2>/dev/null || true
rm -f -- "$prefix/bin/kakaotalk-wayland-sni"
rm -f -- "$prefix/share/systemd/user/kakaotalk-wayland-sni.service"
systemctl --user daemon-reload
printf 'Removed kakaotalk-wayland-sni. Configuration was kept.\n'
