# Tenzir Example Plugin

This is an example plugin for Tenzir. Simply run `docker compose up` to build
and start a Tenzir node with your additional plugin. Use `docker compose run
tenzir '<pipeline>'` to interact with the node, or set the following
environment variables and to connect your node to app.tenzir.com:

```
export TENZIR_PLUGINS__PLATFORM__API_KEY='<api-key>'
export TENZIR_PLUGINS__PLATFORM__TENANT_ID='<tenant-id>'
```

## Write Tests

Every plugin defines additional tests using
[BATS](https://bats-core.readthedocs.io/en/stable/writing-tests.html). Use
`docker compose run tests` to execute your tests and update the reference files
automatically.
