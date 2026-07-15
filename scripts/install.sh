#!/usr/bin/env bash
set -euo pipefail

project_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
prefix=${PREFIX:-"$HOME/.local"}
build_dir=${BUILD_DIR:-"$project_dir/build"}

cmake -S "$project_dir" -B "$build_dir" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$prefix" \
  -DBUILD_TESTING=ON
cmake --build "$build_dir" --parallel
ctest --test-dir "$build_dir" --output-on-failure
cmake --install "$build_dir"

systemctl --user daemon-reload
systemctl --user enable --now kakaotalk-wayland-sni.service
printf 'Installed and started kakaotalk-wayland-sni.service\n'
