# Ambiente C++ para o projeto cycles
#   Prioridade: CMake + Ninja (pip) + Clang 23 com libc++ e suporte a módulos C++
#   Mantido:    Bazel via bazelisk (respeita o .bazelversion)
# Espelha o ambiente onde o Claude (claude.ai) testou: Ubuntu 24.04, x86_64.
FROM ubuntu:24.04

ARG LLVM_VERSION=23.1.3
ARG CMAKE_VERSION=4.4.4
ARG NINJA_VERSION=1.13.2
ARG BAZELISK_VERSION=v1.29.0
ENV DEBIAN_FRONTEND=noninteractive

# Base: gcc 13 (para comparação), depuração, GSL (gcpp nos benchmarks), pipx
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential gdb valgrind git curl ca-certificates xz-utils \
      python3 pipx libmsgsl-dev \
    && rm -rf /var/lib/apt/lists/*

# LLVM/Clang oficial do GitHub, extraindo só o necessário (~1 GB):
# compilador, clang-scan-deps (módulos), ferramentas, libc++ e os std.cppm
RUN mkdir -p /opt/llvm \
    && curl -fsSL "https://github.com/llvm/llvm-project/releases/download/llvmorg-${LLVM_VERSION}/LLVM-${LLVM_VERSION}-Linux-X64.tar.xz" \
     | tar -xJ -C /opt/llvm --strip-components=1 --wildcards \
         '*/bin/clang' '*/bin/clang++' '*/bin/clang-[0-9]*' '*/bin/clang-scan-deps' \
         '*/bin/clangd' '*/bin/clang-format' '*/bin/clang-tidy' \
         '*/bin/lld' '*/bin/ld.lld' '*/bin/llvm-symbolizer' '*/bin/llvm-ar' '*/bin/llvm-ranlib' \
         '*/lib/clang/*' '*/include/c++/*' '*/include/x86_64-unknown-linux-gnu/*' \
         '*/lib/x86_64-unknown-linux-gnu/*' '*/share/libc++/*' \
    && echo /opt/llvm/lib/x86_64-unknown-linux-gnu > /etc/ld.so.conf.d/llvm.conf \
    && ldconfig

# Importante: PATH, não symlinks. Com symlinks em /usr/local/bin o
# clang-scan-deps não acha os headers da libc++ e o 'import std' quebra.
ENV PATH=/opt/llvm/bin:$PATH CC=clang CXX=clang++

# CMake, Ninja e cpplint via pip (pipx), versões fixadas
ENV PIPX_HOME=/opt/pipx PIPX_BIN_DIR=/usr/local/bin
RUN pipx install "cmake==${CMAKE_VERSION}" \
    && pipx install "ninja==${NINJA_VERSION}" \
    && pipx install cpplint
ENV CMAKE_GENERATOR=Ninja

# Bazel via bazelisk
RUN curl -fsSL -o /usr/local/bin/bazel \
      "https://github.com/bazelbuild/bazelisk/releases/download/${BAZELISK_VERSION}/bazelisk-linux-amd64" \
    && chmod +x /usr/local/bin/bazel

# Usuário 'ubuntu' (UID 1000, já existe na imagem) e diretórios dos volumes
RUN mkdir -p /home/ubuntu/.claude /home/ubuntu/.cache \
    && chown -R ubuntu:ubuntu /home/ubuntu/.claude /home/ubuntu/.cache
USER ubuntu
RUN clang++ --version && cmake --version && ninja --version