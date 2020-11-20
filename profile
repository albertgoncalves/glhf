#!/usr/bin/env bash

set -euo pipefail

sudo sh -c "echo 1 > /proc/sys/kernel/perf_event_paranoid"
sudo sh -c "echo 0 > /proc/sys/kernel/kptr_restrict"
perf record \
    --call-graph fp \
    "$WD/bin/main" "$WD/src/vert.glsl" "$WD/src/frag.glsl"
perf report
valgrind --tool=cachegrind \
    --branch-sim=yes \
    "$WD/bin/main" "$WD/src/vert.glsl" "$WD/src/frag.glsl"
rm cachegrind.out*
rm perf.data*
rm vgcore* || true
