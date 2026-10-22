#!/usr/bin/env bash
# Script manual para rodar antes de commitar (não é hook do git).
# Compila com CMake, roda o ctest e passa o valgrind nos testes e exemplos.
#
# Uso: scripts/pre-commit-check.sh [preset]    (padrão: debug; outro: modules)
#
# Para no primeiro erro e mostra cada comando antes de executá-lo.
set -ex

PRESET=${1:-debug}
cd "$(dirname "$0")/.."

cmake --preset "$PRESET"
cmake --build --preset "$PRESET"
ctest --preset "$PRESET"

for exe in tests/my_graph_test tests/my_list_test tests/tnode_test \
           examples/example1 examples/example2 examples/example3; do
  valgrind --leak-check=full --error-exitcode=1 "build/$PRESET/$exe"
done

set +x
echo "Tudo ok: pode commitar."
