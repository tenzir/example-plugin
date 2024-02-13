FROM tenzir/tenzir-dev:main AS example-builder-untested

COPY example /plugins/example

RUN cmake -S /plugins/example -B build-example -G Ninja -D CMAKE_INSTALL_PREFIX:STRING="$PREFIX"
RUN cmake --build build-example --parallel
RUN cmake --install build-example --strip --component Runtime --prefix /plugin/example

FROM example-builder-untested AS example-test

ENV BATS_LIB_PATH=/tmp/tenzir/tenzir/integration/lib
ENV UPDATE=1

ENTRYPOINT cmake --build build-example --target integration

FROM example-builder-untested AS example-builder

ENV BATS_LIB_PATH=/tmp/tenzir/tenzir/integration/lib
RUN cmake --build build-example --target integration

FROM tenzir/tenzir:main

COPY --from=example-builder --chown=tenzir:tenzir /plugin/example /opt/tenzir
