#include <tenzir/argument_parser.hpp>
#include <tenzir/plugin.hpp>

namespace tenzir::plugins::example {

namespace {

class example_operator final : public crtp_operator<example_operator> {
public:
  auto operator()(table_slice events) const -> table_slice {
    // Does nothing with the input.
    return events;
  }

  auto name() const -> std::string override {
    return "example";
  }

  auto optimize(expression const& filter, event_order order) const
    -> optimize_result override {
    (void)order;
    (void)filter;
    return do_not_optimize(*this);
  }

  friend auto inspect(auto& f, example_operator& x) -> bool {
    return f.object(x).fields();
  }
};

class plugin final : public virtual operator_plugin<example_operator> {
public:
  auto signature() const -> operator_signature override {
    return {
      .source = false,
      .transformation = true,
      .sink = false,
    };
  }

  auto parse_operator(parser_interface& p) const -> operator_ptr override {
    auto parser = argument_parser{"example",
      "https://docs.tenzir.com/operators/example"};
    parser.parse(p);
    return std::make_unique<example_operator>();
  }
};

} // namespace

} // namespace tenzir::plugins::example

TENZIR_REGISTER_PLUGIN(tenzir::plugins::example::plugin)
