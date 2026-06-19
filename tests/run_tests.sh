#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT/tests/build"

mkdir -p "$BUILD_DIR"
cleanup() {
  rm -rf "$BUILD_DIR" "$ROOT/tests/python/__pycache__" "$ROOT/Iluminacao/Tela/Teste/__pycache__"
}

trap cleanup EXIT
cleanup
mkdir -p "$BUILD_DIR"

echo "[1/2] Running Python unit tests"
PYTHONDONTWRITEBYTECODE=1 python3 -B -m unittest discover -s "$ROOT/tests/python" -v

echo "[2/2] Building and running C unit tests"
gcc \
  -std=c11 \
  -Wall \
  -Wextra \
  -I"$ROOT/tests/c/stubs" \
  -I"$ROOT/Iluminacao/Tela/main/header" \
  "$ROOT/tests/c/test_protocol.c" \
  "$ROOT/tests/c/stubs/fake_cjson.c" \
  "$ROOT/Iluminacao/Tela/main/source/protocol.c" \
  -o "$BUILD_DIR/test_protocol"

"$BUILD_DIR/test_protocol"

gcc \
  -std=c11 \
  -Wall \
  -Wextra \
  -I"$ROOT/tests/c/stubs" \
  -I"$ROOT/Iluminacao/Tela/main/header" \
  "$ROOT/tests/c/stubs/test_support.c" \
  "$ROOT/tests/c/stubs/fake_http_runtime.c" \
  "$ROOT/tests/c/stubs/fake_tela_runtime.c" \
  "$ROOT/tests/c/test_http_server.c" \
  "$ROOT/Iluminacao/Tela/main/source/http_server.c" \
  -o "$BUILD_DIR/test_http_server"

"$BUILD_DIR/test_http_server"

gcc \
  -std=c11 \
  -Wall \
  -Wextra \
  -I"$ROOT/tests/c/stubs" \
  -I"$ROOT/Iluminacao/led/main" \
  "$ROOT/tests/c/stubs/test_support.c" \
  "$ROOT/tests/c/stubs/fake_tela_runtime.c" \
  "$ROOT/tests/c/test_lighting_manager.c" \
  "$ROOT/Iluminacao/led/main/lighting_manager.c" \
  -o "$BUILD_DIR/test_lighting_manager"

"$BUILD_DIR/test_lighting_manager"

gcc \
  -std=c11 \
  -Wall \
  -Wextra \
  -I"$ROOT/tests/c/stubs" \
  -I"$ROOT/Iluminacao/Tela/main/header" \
  "$ROOT/tests/c/stubs/test_support.c" \
  "$ROOT/tests/c/stubs/fake_app_runtime.c" \
  "$ROOT/tests/c/test_wifi_manager.c" \
  "$ROOT/Iluminacao/Tela/main/source/wifi_manager.c" \
  -o "$BUILD_DIR/test_wifi_manager"

"$BUILD_DIR/test_wifi_manager"

gcc \
  -std=c11 \
  -Wall \
  -Wextra \
  -I"$ROOT/tests/c/stubs" \
  -I"$ROOT/Iluminacao/Tela/main/header" \
  "$ROOT/tests/c/stubs/test_support.c" \
  "$ROOT/tests/c/stubs/fake_freertos.c" \
  "$ROOT/tests/c/stubs/fake_app_runtime.c" \
  "$ROOT/tests/c/stubs/fake_module_runtime.c" \
  "$ROOT/tests/c/test_tela_main.c" \
  "$ROOT/Iluminacao/Tela/main/source/main.c" \
  -o "$BUILD_DIR/test_tela_main"

"$BUILD_DIR/test_tela_main"

gcc \
  -std=c11 \
  -Wall \
  -Wextra \
  -I"$ROOT/tests/c/stubs" \
  -I"$ROOT/Iluminacao/led/main" \
  "$ROOT/tests/c/stubs/test_support.c" \
  "$ROOT/tests/c/stubs/fake_freertos.c" \
  "$ROOT/tests/c/stubs/fake_app_runtime.c" \
  "$ROOT/tests/c/stubs/fake_module_runtime.c" \
  "$ROOT/tests/c/test_led_main.c" \
  "$ROOT/Iluminacao/led/main/ main.c" \
  -o "$BUILD_DIR/test_led_main"

"$BUILD_DIR/test_led_main"
