# shellcheck disable=SC2016

setup() {
  bats_load_library bats-support
  bats_load_library bats-assert
  bats_load_library bats-tenzir

  export_default_node_config
  export TENZIR_PLUGINS="example"
  export TENZIR_TQL2=true
  setup_node
}

teardown() {
  teardown_node
}

@test "parse example logs" {
  cat "${BATS_TENZIR_INPUTSDIR}/sample.log" |
    check tenzir 'read_custom_log'
}

@test "parse example logs with argument" {
  cat "${BATS_TENZIR_INPUTSDIR}/sample.log" |
    check tenzir 'read_custom_log time_offset=1h'
}
