setup_suite() {
  bats_require_minimum_version 1.8.0
  bats_load_library bats-tenzir

  export_default_paths
}

export BATS_LIB_PATH=${BATS_LIB_PATH:+${BATS_LIB_PATH}:}/tmp/tenzir/tenzir/integration/lib
