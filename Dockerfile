ARG TENZIR_VERSION=main
FROM ghcr.io/tenzir/tenzir-dev:${TENZIR_VERSION} AS builder

COPY . /tmp/example/

RUN cmake -S /tmp/example -B /tmp/example/build -G Ninja -D CMAKE_INSTALL_PREFIX:STRING="$PREFIX"
RUN cmake --build /tmp/example/build --parallel
RUN cmake --install /tmp/example/build --strip --component Runtime --prefix /tmp/example/install

FROM builder AS test

ENTRYPOINT cmake --build /tmp/example/build --target update-bats

FROM ghcr.io/tenzir/tenzir:${TENZIR_VERSION}

COPY --from=builder --chown=tenzir:tenzir /tmp/example/install /opt/tenzir
