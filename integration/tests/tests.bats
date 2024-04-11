# shellcheck disable=SC2016

setup() {
  bats_load_library bats-support
  bats_load_library bats-assert
  bats_load_library bats-tenzir

  export_default_node_config
  export TENZIR_PLUGINS="example"
  setup_node
}

teardown() {
  teardown_node
}

@test "Check plugin availability" {
  check tenzir 'show plugins | where name == "example" | drop version, kind'
}
