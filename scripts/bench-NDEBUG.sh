#!/usr/bin/env bash
# Script manual de benchmark, versão mais rápida: compila com -O3 -DNDEBUG e
# sem as checagens de contrato do GSL (preset 'bench'), para comparar de forma
# justa com o gcpp. Roda os benchmarks rápidos (total abaixo de 1 minuto).
# Atenção: o 'make bench' e as tabelas do README usam asserts ligados.
#
# Uso: scripts/bench-NDEBUG.sh
#
# Para no primeiro erro e mostra cada comando antes de executá-lo.
set -ex

cd "$(dirname "$0")/.."

# gcpp e o GSL antigo que ele exige (submódulo dentro do gcpp)
git submodule update --init --recursive

cmake --preset bench
cmake --build --preset bench

# construção de 10M ponteiros: relation_ptr vs shared_ptr
build/bench/tests/quick_bench_sptr

# destruição de lista (100k) e árvore (2^15): unique/shared/relation_ptr
build/bench/tests/quick_bench_list_tree

# destruição de lista com 10M elementos
build/bench/tests/quick_bench_list_tree_long

# grafo pequeno: shared_ptr vs arena vs relation_ptr
build/bench/tests/quick_bench_graph

# grafo V=500 E=150k: shared_ptr, arena, relation_ptr e gcpp
build/bench/tests/long_bench_graph
