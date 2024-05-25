# shellcheck disable=SC2016

setup() {
  bats_load_library bats-support
  bats_load_library bats-assert
  bats_load_library bats-tenzir

  export_default_node_config
  export TENZIR_PLUGINS="trim"
  setup_node
}

teardown() {
  teardown_node
}

@test "trim strings" {
  check tenzir 'version | put foo=" foo   " | trim foo'
  check tenzir 'version | put foo=" foo   ", bar="   bar" | trim foo'
  check tenzir 'version | put foo=" foo   ", bar="   bar" | trim :string'
}

@test "trimming non-string fields fails" {
  check ! tenzir 'version | trim :uint64'
}
