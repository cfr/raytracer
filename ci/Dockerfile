#
# Build:  docker build -t aktis-ci ci/
# Run:    docker run --rm -u "$(id -u):$(id -g)" -v "$PWD":/repo aktis-ci ci/format.sh
#         docker run --rm -u "$(id -u):$(id -g)" -v "$PWD":/repo aktis-ci ci/lint.sh
#
FROM debian:trixie-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates \
        git \
        cmake \
        ninja-build \
        clang-19 \
        clang-format-19 \
        clang-tidy-19 \
        shellcheck \
        valgrind \
    && rm -rf /var/lib/apt/lists/* \
    && update-alternatives --install /usr/bin/clang        clang        /usr/bin/clang-19        100 \
    && update-alternatives --install /usr/bin/clang++      clang++      /usr/bin/clang++-19      100 \
    && update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-19 100 \
    && update-alternatives --install /usr/bin/clang-tidy   clang-tidy   /usr/bin/clang-tidy-19   100 \
    && update-alternatives --install /usr/bin/clang-apply-replacements \
           clang-apply-replacements /usr/bin/clang-apply-replacements-19 100 \
    && update-alternatives --install /usr/bin/run-clang-tidy \
           run-clang-tidy /usr/bin/run-clang-tidy-19 100


RUN git clone --depth 1 --branch 1.0.3 \
        https://github.com/g-truc/glm.git /opt/glm \
    && git clone --depth 1 --branch v0.98.1 \
        https://github.com/imneme/pcg-cpp.git /opt/pcg-cpp \
    && rm -rf /opt/glm/.git /opt/pcg-cpp/.git

ENV CC=clang CXX=clang++

ENV HOME=/tmp

WORKDIR /repo
