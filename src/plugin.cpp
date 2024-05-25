#include <tenzir/argument_parser.hpp>
#include <tenzir/arrow_table_slice.hpp>
#include <tenzir/collect.hpp>
#include <tenzir/plugin.hpp>

#include <arrow/api.h>
#include <arrow/compute/api.h>

#include <unordered_set>

namespace tenzir::plugins::trim {

namespace {

class trim_operator final : public crtp_operator<trim_operator> {
public:
  trim_operator() = default;

  explicit trim_operator(located<std::string> field)
    : field_{std::move(field)} {
  }

  // This function contains the logic of the trim operator when
  // instantiated. It maps from a generator :53
  // of table slices to a generator of
  // table slices. A table slice is simply a batch of events.
  auto operator()(generator<table_slice> input,
                  operator_control_plane& ctrl) const
      -> generator<table_slice> {
    // All code up to the first yield is run synchronously in an operator and
    // is considered the start up phase. This operator doesn't do anything
    // special in this case, so we can signal a successful startup immediately.
    co_yield {};
    // The main loop of the operator exits once the previous operator has
    // finished. Utilize this for control flow. For example, we keep a list
    // of schemas outside of the loop that we already warned for.
    auto warned_for_schemas = std::unordered_set<std::string>{};
    for (auto events : input) {
      // There's one important contract that an operator must always adhere to:
      // if an input batch is empty, the operator must yield. In all other
      // situations, the operator may continue without yielding.
      if (events.rows() == 0) {
        co_yield {};
        continue;
      }
      // We can now start processing the events in the batch. First, we resolve
      // the field for the batch's schema to a set of indices pointing to the
      // field(s) within the schema. This transparently supports resolving
      // concepts and field extractors.
      const auto indices = collect(events.schema().resolve(field_.inner));
      // If the field didn't resolve, we can't do anything with the batch. We
      // warn the user about it and return the input unchanged. We warn only
      // once per schema, and dwe only warn when the specified field was not a
      // type extractor.
      if (indices.empty()) {
        const auto [_, inserted] = warned_for_schemas.insert(
          std::string{events.schema().name()});
        if (inserted and not field_.inner.starts_with(':')) {
          diagnostic::warning("field did not resolve for schema `{}`",
              events.schema())
            .primary(field_.source)
            .emit(ctrl.diagnostics());
        }
        co_yield std::move(events);
        continue;
      }
      // Now we can transform the events in the batch. We're utilizing Apache
      // Arrow's compute function 'utf8_trim' for this, confuring it to trim
      // whitespace.
      const auto trim = [&](struct record_type::field field,
                            std::shared_ptr<arrow::Array> array)
          -> indexed_transformation::result_type {
        static const auto options = arrow::compute::TrimOptions{" \t\n\v\f\r"};
        auto trimmed_array = arrow::compute::CallFunction(
            "utf8_trim", {array}, &options);
        if (not trimmed_array.ok()) {
          diagnostic::error("{}", trimmed_array.status().ToString())
            .primary(field_.source)
            .throw_();
        }
        return {
          {field, trimmed_array.MoveValueUnsafe().make_array()},
        };
      };
      // Lastly, we apply the transformation to all indices and then return the
      // transformed batch.
      auto transformations = std::vector<indexed_transformation>{};
      for (auto index : indices) {
        transformations.push_back({index, trim});
      }
      co_yield transform_columns(std::move(events), std::move(transformations));
    }
  }

  // Return the user-facing name of the operator. Must be a valid identifier.
  auto name() const -> std::string override {
    return "trim";
  }

  // Specify how optimizations affect the operator.
  auto optimize(const expression& filter, event_order order) const
    -> optimize_result override {
    (void)order;
    (void)filter;
    return do_not_optimize(*this);
  }

  // List all fields so that the operator can successfully be transmitted
  // between nodes.
  friend auto inspect(auto& f, trim_operator& x) -> bool {
    return f.object(x).fields(f.field("field", x.field_));
  }

private:
  located<std::string> field_ = {};
};

class plugin final : public virtual operator_plugin<trim_operator> {
public:
  // Provide the signature of the operator for `show operators`.
  auto signature() const -> operator_signature override {
    return {
      .source = false,
      .transformation = true,
      .sink = false,
    };
  }

  // Parse the operator from the parser interface.
  auto parse_operator(parser_interface& p) const -> operator_ptr override {
    auto parser = argument_parser{
      "trim",
      "https://github.com/tenzir/example-plugin",
    };
    auto field = located<std::string>{};
    parser.add(field, "<field>");
    parser.parse(p);
    return std::make_unique<trim_operator>(field);
  }
};

} // namespace

} // namespace tenzir::plugins::trim

TENZIR_REGISTER_PLUGIN(tenzir::plugins::trim::plugin)
