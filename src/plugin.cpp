#include <tenzir/plugin.hpp>

namespace tenzir::plugins::example {

namespace {

class example_plugin final : public virtual plugin {
public:
  auto name() const -> std::string override { return "example"; }
};

} // namespace

} // namespace tenzir::plugins::example

TENZIR_REGISTER_PLUGIN(tenzir::plugins::example::example_plugin)
