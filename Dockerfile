ARG TENZIR_VERSION=main
FROM ghcr.io/tenzir/tenzir-dev:${TENZIR_VERSION} AS builder

COPY . /tmp/trim/

RUN cmake -S /tmp/trim -B /tmp/trim/build -G Ninja -D CMAKE_INSTALL_PREFIX:STRING="$PREFIX"
RUN cmake --build /tmp/trim/build --parallel
RUN cmake --install /tmp/trim/build --strip --component Runtime --prefix /tmp/trim/install

FROM builder AS test

ENV BATS_LIB_PATH=/tmp/tenzir/tenzir/integration/lib

ENTRYPOINT cmake --build /tmp/trim/build --target update-integration

FROM ghcr.io/tenzir/tenzir:${TENZIR_VERSION}

COPY --from=builder --chown=tenzir:tenzir /tmp/trim/install /opt/tenzir
