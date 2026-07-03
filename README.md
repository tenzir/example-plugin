# Tenzir Example Plugin

> [!IMPORTANT]
> This repository is sunsetted and no longer represents the recommended path for
> extending Tenzir. We keep it available as historical reference for the former
> node plugin model.
>
> Stay tuned: Tenzir is changing to become usable as a library. That direction
> will provide a newer and more direct integration path for embedding and
> extending Tenzir in your own systems.

This repository shows the former node plugin model by implementing a simple
operator `read_custom_log` that parses a [custom line-based log
format](bats/data/inputs/sample.log).

The operator's C++ implementation can be found in
[`builtins/operators/read_custom_log.cpp`](builtins/operators/read_custom_log.cpp)
and is extensively commented.

## Build and run

These instructions are preserved for historical reference. Run `docker compose
up --build` to build and start a Tenzir node with the example plugin.

Use `docker compose run --build tenzir '<pipeline>'` to interact with the node
on the command-line, or set the following environment variables to connect your
node to [app.tenzir.com][app]:

```bash
export TENZIR_TOKEN='<token>'

# This one is only necessary if you're using a self-hosted version of the
# Tenzir Platform.
export TENZIR_PLATFORM_CONTROL_ENDPOINT='<url>'
```

## Run tests

This repository defines additional tests using
[BATS](https://bats-core.readthedocs.io/en/stable/writing-tests.html). Use
`docker compose run --build tests` to execute your tests and update the
reference files automatically.

## Further Resources

The current Tenzir source tree still contains [plugins][plugins-source] and
[builtins][builtins-source] that may be useful as implementation references.
They do not change this repository's status as a sunsetted example.

If you have any questions, feel free to reach out in the [#developers channel
on Discord][discord].

## Contribute your plugin

We no longer recommend starting new integrations from this plugin template. For
new work, wait for the upcoming library-based integration path or discuss your
use case with us first.

[tenzir]: https://github.com/tenzir/tenzir
[app]: https://app.tenzir.com
[plugins-source]: https://github.com/tenzir/tenzir/tree/main/plugins
[builtins-source]: https://github.com/tenzir/tenzir/tree/main/libtenzir/builtins
[discord]: https://docs.tenzir.com/discord
