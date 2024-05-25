# Tenzir Example Plugin

This is an example plugin for Tenzir, adding a `trim` operaotr that removes
whitespace from string fields.

## Build and run

Simply run `docker compose up` to build and start a Tenzir node with your
additional plugin.

Use `docker compose run tenzir '<pipeline>'` to interact with the node on the
command-line, or set the following environment variables connect your node to
app.tenzir.com:

```
export TENZIR_PLUGINS__PLATFORM__API_KEY='<api-key>'
export TENZIR_PLUGINS__PLATFORM__TENANT_ID='<tenant-id>'
```

## Learn how to write a plugin

We think that learning how to build a plugin is best done by example. Tenzir
ships with a variety of [plugins][plugins-source] and
[builtins][builtins-source] to get inspired by and to learn from.

If you have any questions, feel free to reach out in the [#developers channel
on Discord][discord].

## Run tests

Every plugin defines additional tests using
[BATS](https://bats-core.readthedocs.io/en/stable/writing-tests.html). Use
`docker compose run --build tests` to execute your tests and update the
reference files automatically.

## Contribute your plugin

If you want to upstream your plugin so that it is bundled with every Tenzir
installation, open a PR that adds it to the [`plugins/` directory in the
`tenzir/tenzir` repository][plugins-source]. If your  plugin has no
dependencies, consider contributing it as a builtin instead. Builtins are
located in the [`libtenzir/builtins/` directory in the `tenzir/tenzir`
repositorsbuiltins-source].

[plugins-source]: https://github.com/tenzir/tenzir/tree/main/plugins
[builtins-source]: https://github.com/tenzir/tenzir/tree/main/libtenzir/builtins
[discord]: https://docs.tenzir.com/discord
