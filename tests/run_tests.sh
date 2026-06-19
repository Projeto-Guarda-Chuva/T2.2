#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT/tests/build"

mkdir -p "$BUILD_DIR"

echo "[1/2] Running Python unit tests"
python3 -m unittest discover -s "$ROOT/tests/python" -v

echo "[2/2] Building and running C unit tests"
gcc \
  -std=c11 \
  -Wall \
  -Wextra \
  -I"$ROOT/tests/c/stubs" \
  -I"$ROOT/ESP-32-P4/Tela/main/header" \
  "$ROOT/tests/c/test_protocol.c" \
  "$ROOT/tests/c/stubs/fake_cjson.c" \
  "$ROOT/ESP-32-P4/Tela/main/source/protocol.c" \
  -o "$BUILD_DIR/test_protocol"

"$BUILD_DIR/test_protocol"

gcc \
  -std=c11 \
  -Wall \
  -Wextra \
  -I"$ROOT/tests/c/stubs" \
  -I"$ROOT/tests/c/stubs/freertos" \
  -I"$ROOT/ods-communication/embedded/components/http_comm/include" \
  -I"$ROOT/ods-communication/embedded/devices/processadora/main" \
  "$ROOT/tests/c/stubs/test_support.c" \
  "$ROOT/tests/c/stubs/fake_esp_runtime.c" \
  "$ROOT/tests/c/stubs/fake_http_comm_registration.c" \
  "$ROOT/tests/c/stubs/fake_freertos.c" \
  "$ROOT/tests/c/stubs/fake_cjson.c" \
  "$ROOT/tests/c/test_processadora_modules.c" \
  "$ROOT/ods-communication/embedded/devices/processadora/main/monitor.c" \
  "$ROOT/ods-communication/embedded/devices/processadora/main/print.c" \
  -o "$BUILD_DIR/test_processadora_modules"

"$BUILD_DIR/test_processadora_modules"

gcc \
  -std=c11 \
  -Wall \
  -Wextra \
  -I"$ROOT/tests/c/stubs" \
  -I"$ROOT/tests/c/stubs/freertos" \
  -I"$ROOT/ods-communication/embedded/components/http_comm/include" \
  "$ROOT/tests/c/stubs/test_support.c" \
  "$ROOT/tests/c/stubs/fake_http_runtime.c" \
  "$ROOT/tests/c/stubs/fake_freertos.c" \
  "$ROOT/tests/c/test_http_comm.c" \
  "$ROOT/ods-communication/embedded/components/http_comm/http_comm.c" \
  -o "$BUILD_DIR/test_http_comm"

"$BUILD_DIR/test_http_comm"
