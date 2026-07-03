#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
NEW_TESTS="$ROOT/new_tests"
UNITY="$NEW_TESTS/unity"
SUPPORT="$NEW_TESTS/support"
BUILD_DIR="$NEW_TESTS/build"
RESULTS_DIR="$NEW_TESTS/results"

cleanup() { rm -rf "$BUILD_DIR"; }
trap cleanup EXIT
cleanup
mkdir -p "$BUILD_DIR" "$RESULTS_DIR"

CFLAGS=(-std=c11 -Wall -Wextra -I"$UNITY" -I"$SUPPORT")

overall=0

echo "[audio] Atuador de Audio"
gcc "${CFLAGS[@]}" \
  -I"$NEW_TESTS/audio/mocks" \
  -I"$ROOT/Audio/main" \
  "$UNITY/unity.c" \
  "$SUPPORT/test_results.c" \
  "$NEW_TESTS/audio/mocks/fake_audio_log.c" \
  "$NEW_TESTS/audio/mocks/fake_audio_manager.c" \
  "$NEW_TESTS/audio/test_audio_manager.c" \
  -o "$BUILD_DIR/test_audio_manager"
"$BUILD_DIR/test_audio_manager" "$RESULTS_DIR/audio.json" || overall=$?

echo
echo "[grafico] Atuador Grafico"
gcc "${CFLAGS[@]}" \
  -I"$NEW_TESTS/grafico/mocks" \
  -I"$NEW_TESTS/cjson" \
  -I"$ROOT/Iluminacao/Tela/main/header" \
  "$UNITY/unity.c" \
  "$SUPPORT/test_results.c" \
  "$NEW_TESTS/cjson/cJSON.c" \
  "$NEW_TESTS/grafico/mocks/fake_grafico_log.c" \
  "$NEW_TESTS/grafico/mocks/fake_display.c" \
  "$NEW_TESTS/grafico/test_protocol.c" \
  "$ROOT/Iluminacao/Tela/main/source/protocol.c" \
  -o "$BUILD_DIR/test_protocol"
"$BUILD_DIR/test_protocol" "$RESULTS_DIR/grafico.json" || overall=$?

echo
echo "[grafico-http] Conector HTTP do Atuador Grafico"
gcc "${CFLAGS[@]}" \
  -I"$NEW_TESTS/grafico/mocks" \
  -I"$ROOT/Iluminacao/Tela/main/header" \
  "$UNITY/unity.c" \
  "$SUPPORT/test_results.c" \
  "$NEW_TESTS/grafico/mocks/fake_grafico_log.c" \
  "$NEW_TESTS/grafico/mocks/fake_http_server.c" \
  "$NEW_TESTS/grafico/test_http_server.c" \
  "$ROOT/Iluminacao/Tela/main/source/http_server.c" \
  -o "$BUILD_DIR/test_http_server"
"$BUILD_DIR/test_http_server" "$RESULTS_DIR/grafico_http.json" || overall=$?

echo
echo "[iluminacao] Atuador de Iluminacao"
gcc "${CFLAGS[@]}" \
  -I"$NEW_TESTS/iluminacao/mocks" \
  -I"$ROOT/Iluminacao/led/main" \
  "$UNITY/unity.c" \
  "$SUPPORT/test_results.c" \
  "$NEW_TESTS/iluminacao/mocks/fake_iluminacao_log.c" \
  "$NEW_TESTS/iluminacao/mocks/fake_led_strip.c" \
  "$NEW_TESTS/iluminacao/test_lighting_manager.c" \
  "$ROOT/Iluminacao/led/main/lighting_manager.c" \
  -o "$BUILD_DIR/test_lighting_manager"
"$BUILD_DIR/test_lighting_manager" "$RESULTS_DIR/iluminacao.json" || overall=$?

echo
echo "[motor_movel] Atuador de Motor Movel"
gcc "${CFLAGS[@]}" \
  -I"$NEW_TESTS/motor_movel/mocks" \
  "$UNITY/unity.c" \
  "$SUPPORT/test_results.c" \
  "$NEW_TESTS/motor_movel/mocks/fake_motor_log.c" \
  "$NEW_TESTS/motor_movel/mocks/fake_arduino.c" \
  "$NEW_TESTS/motor_movel/mocks/fake_wifi.c" \
  "$NEW_TESTS/motor_movel/mocks/test_log_capture.c" \
  "$NEW_TESTS/motor_movel/test_motor_movel.c" \
  "$NEW_TESTS/motor_movel/motor_movel.c" \
  -o "$BUILD_DIR/test_motor_movel"
"$BUILD_DIR/test_motor_movel" "$RESULTS_DIR/motor_movel.json" || overall=$?

exit "$overall"
