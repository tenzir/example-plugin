ARG TENZIR_VERSION=main
FROM ghcr.io/tenzir/tenzir-dev:${TENZIR_VERSION} AS example-builder-untested

COPY example /plugins/example

RUN cmake -S /plugins/example -B build-example -G Ninja -D CMAKE_INSTALL_PREFIX:STRING="$PREFIX"
RUN cmake --build build-example --parallel
RUN cmake --install build-example --strip --component Runtime --prefix /plugin/example

FROM example-builder-untested AS example-test

ENV BATS_LIB_PATH=/tmp/tenzir/tenzir/integration/lib
# TODO: Use the update-integration target instead
ENV UPDATE=1

ENTRYPOINT cmake --build build-example --target integration

FROM example-builder-untested AS example-builder

ENV BATS_LIB_PATH=/tmp/tenzir/tenzir/integration/lib
RUN cmake --build build-example --target integration

FROM ghcr.io/tenzir/tenzir:${TENZIR_VERSION}

COPY --from=example-builder --chown=tenzir:tenzir /plugin/example /opt/tenzir
