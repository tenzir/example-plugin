//    _   _____   __________
//   | | / / _ | / __/_  __/     Visibility
//   | |/ / __ |_\ \  / /          Across
//   |___/_/ |_/___/ /_/       Space and Time
//
// SPDX-FileCopyrightText: (c) 2025 The Tenzir Contributors
// SPDX-License-Identifier: BSD-3-Clause

// This operators read the "custom log" format, which is a simple key-value that
// doesn't actually exist, but that we'll use for demo purposes. Our made up log
// looks as follows:
//
// [TIMESTAMP] [LOG_LEVEL] [USER_ID] [ACTION_TYPE] - MESSAGE
//
// Some example log lines:
//
//   [2025-01-07T17:00:00] [INFO] [user123] [CREATE_POST] - User created a new
//   blog post titled "Understanding AI".
//
//   [2025-01-07T17:05:00] [INFO] [user123] [EDIT_POST] - User edited the blog
//   post "Understanding AI".
//
//   [2025-01-07T17:10:00] [INFO] [user456] [COMMENT] - User commented on
//   "Understanding AI": "Great insights!".
//
//   [2025-01-07T17:15:00] [ERROR] [user123] [DELETE_POST] - User attempted to
//   delete a post that does not exist.
//
//   [2025-01-07T17:20:00] [INFO] [user789] [LIKE] - User liked the blog post
//   "Understanding AI".
//
// Throughout the file, we'll explain things piece-by-piece. Let's start with
// some includes.

#include <tenzir/concept/parseable/tenzir/time.hpp>
#include <tenzir/detail/string.hpp>
#include <tenzir/series_builder.hpp>
#include <tenzir/to_lines.hpp>
#include <tenzir/tql2/plugin.hpp>

// Next, jump to the bottom to read the description of the
// `read_custom_log_plugin` class.

namespace tenzir::plugins::example {
namespace {

// The operator instance for the `read_custom_log` operator. This class
// implements the `crtp_operator`, which in turn implements most of the
// `operator_base` interface for us. We only need to implement `operator()`,
// mapping a generator of elements to another generator of elements.
//
// Note that many other functions are available to be overridden, such as
// `location` for forcing an operator to run inside a node, or `detached` to
// mark an operator as requiring its own thread. Check the base class for more
// information.
class read_custom_log_operator final
    : public crtp_operator<read_custom_log_operator> {
public:
  // This provides a constructor for the operator. Note that operator instances
  // _must_ be default-constructible without any arguments so that they can be
  // transferred between processes.
  read_custom_log_operator() = default;

  // Since our operator also accepts a parameter, we provide a second
  // constructor accepting that parameter. This is what we will use in our code.
  read_custom_log_operator(tenzir::duration time_offset)
    : time_offset_{time_offset} {
  }

  // The name of the operator. Must be unique.
  auto name() const -> std::string override { return "read_custom_log"; }

  // Specifies how the operator handles optimization. Check the
  // `operator_base::optimize` documentation for more information. For this
  // particular operator, we don't do any optimization and choose to act as an
  // optimization barrier in the pipeline.
  auto optimize(const expression& filter,
                event_order order) const -> optimize_result override {
    TENZIR_UNUSED(filter, order);
    return do_not_optimize(*this);
  }

  // Handles serialization and deserialization of the operator instance. This
  // function _must_ capture all member variables of the instance.
  friend auto inspect(auto& f, read_custom_log_operator& x) -> bool {
    return f.object(x).fields(
      f.field("time_offset", x.time_offset_)
      /// If there were further members, they _must_ be added here.
    );
  }

  // This is the main run-loop of the operator instance. It must have one of the
  // following signatures:
  //
  //   (generator<T> input, operator_control_plane &ctrl) -> generator<U>
  //   (operator_control_plane &ctrl) -> generator<U>
  //
  // `T` may be either `table_slice` or `chunk_ptr`, and `U` may be either
  // `table_slice`, `chunk_ptr`, or `std::monostate`.
  //
  // A table slice is series of events. A chunk is a series of bytes. The
  // absence of the input denotes that an operator is a source, and returning a
  // generator of monostate denotes that an operator is a sink.
  //
  // The operator control plane is an escape hatch that allows operators to
  // interact with whatever resides outside of the passed in or generated data.
  //
  // In this case, we're reading a custom line-based log format, so we'll be
  // taking in bytes and returning events.
  //
  // Note that this function is marked `const`, which means that it is not
  // allowed to modify any members of the operator instance. Store mutable state
  // in the function instead.
  auto
  operator()(generator<chunk_ptr> input,
             operator_control_plane& ctrl) const -> generator<table_slice> {
    // Since we have a line-based format, we'll adapt our generator of chunks
    // into a generator of views onto lines using the `to_lines` function from
    // libtenzir, and then for readability will continue with the `read_lines`
    // function below.
    return read_lines(to_lines(std::move(input)), ctrl);
  }

private:
  // This function accepts lines, which are much easier to work with for our
  // format. We dispatch to this from `operator()`.
  auto
  read_lines(generator<std::optional<std::string_view>> input,
             operator_control_plane& ctrl) const -> generator<table_slice> {
    // We set up a builder to create events in, and give it a fixed schema. For
    // more advanced use cases, consider using the `multi_series_builder`
    // instead. As a reminder, this is what our log format looks like:
    //   [TIMESTAMP] [LOG_LEVEL] [USER_ID] [ACTION_TYPE] - MESSAGE
    auto builder = series_builder{type{
        "custom_log",
        record_type{
            {"timestamp", time_type{}},
            {"log_level", string_type{}},
            {"user_id", string_type{}},
            {"action_type", string_type{}},
            {"message", string_type{}},
        },
    }};
    // We want to buffer events for no more than 250ms before returning them.
    // For this, we need to store when we last returned events.
    // Without this logic, the operator would not yield any results until all
    // input has been processed.
    auto last_yield = std::chrono::steady_clock::now();
    // This is the main input loop, which will run until the input stream ends.
    for (auto &&line : input) {
      // Whenever we read a new line, we first check if we've passed our
      // timeout.
      if (last_yield + std::chrono::milliseconds{250} <
          std::chrono::steady_clock::now()) {
        // If we have, we yield the events we've built so far.
        for (auto events : builder.finish_as_table_slice()) {
          co_yield std::move(events);
        }
        // And reset the timer.
        last_yield = std::chrono::steady_clock::now();
      }
      if (!line) {
        // If we did not get a line, we yield control back to the scheduler.
        co_yield {};
        continue;
      }
      // Let's skip empty lines.
      if (line->empty()) {
        continue;
      }
      // Now, we parse each line one-by-one. We pass in the diagnostics handler
      // so that we can tell the user about parse failures.
      parse_line(*line, builder, ctrl.diagnostics());
    }
    // At the end of the input, we flush the builder to get the final events.
    for (auto events : builder.finish_as_table_slice()) {
      co_yield std::move(events);
    }
  }

  auto parse_line(std::string_view line, series_builder& builder,
                  diagnostic_handler& dh) const -> void {
    // Here, we can now finally parse the line piece by piece. Tenzir also ships
    // with a parser combinator library, which can be used to parse more complex
    // formats. But for this one, we'll just go through the line iteratively.
    // We'll start by splitting the line into its components. Here's the format
    // again:
    //
    //   [TIMESTAMP] [LOG_LEVEL] [USER_ID] [ACTION_TYPE] - MESSAGE
    //
    // We'll split by the four first spaces initially:
    auto parts = detail::split(line, " ", 4);
    // If we don't have enough parts, we'll emit a diagnostic and skip this
    // line.
    if (parts.size() != 5) {
      diagnostic::warning("unexpected log format: expected at least 4 spaces")
          .note("got `{}`", line)
          .emit(dh);
      return;
    }
    // Now, let's check if the first four sections are wrapped in square
    // brackets, and if they are remove them.
    for (auto i = 0; i < 4; ++i) {
      if (parts[i].front() != '[' || parts[i].back() != ']') {
        diagnostic::warning("unexpected log format: expected square brackets")
            .note("got `{}`", parts[i])
            .emit(dh);
        return;
      }
      parts[i].remove_prefix(1);
      parts[i].remove_suffix(1);
    }
    // For the last section, we'll check whether we begin with a dash and a
    // space and will then just leave the rest as-is.
    if (not parts[4].starts_with("- ")) {
      diagnostic::warning("unexpected log format: expected a dash and a space")
          .note("got `{}`", parts[4])
          .emit(dh);
      return;
    }
    parts[4].remove_prefix(2);
    // For the first section, we need to additionally parse the timestamp. We'll
    // use Tenzir's built-in timestamp parser for this.
    auto timestamp = time{};
    if (not parsers::time(parts[0], timestamp)) {
      diagnostic::warning("unexpected log format: expected a timestamp")
          .note("got `{}`", parts[0])
          .emit(dh);
      return;
    }
    // Apply our time offset.
    timestamp += time_offset_;
    // Now, we can finally build the event.
    auto event = builder.record();
    event.field("timestamp", timestamp);
    event.field("log_level", parts[1]);
    event.field("user_id", parts[2]);
    event.field("action_type", parts[3]);
    event.field("message", parts[4]);
    // Nested fields could be added by calling
    // `event.field("my_field").record()` and operating on that.
    //
    // We write a simple log message for debugging purposes.
    TENZIR_TRACE("parsed line {}", line);
  }

  // Our data member for the time offset.
  tenzir::duration time_offset_{};
};

// The `read_custom_log_plugin` class is the plugin that registers the operator
// for us. It's a subclass of `operator_plugin2`, which is a plugin that defines
// an operator for TQL2. The plugin is responsible for creating instances of the
// operator from invocations.
//
// Note that a plugin can inherit from any number of plugin types, but the name
// of a plugin must be unique, or Tenzir will fail to start. For this particular
// plugin, the `name()` function is overriden automatically by the
// `operator_plugin2` plugin, which infers the name from its template parameter.
// For most other plugins, you'll need to override the `name()` function
// manually.
class read_custom_log_plugin final
    : public virtual operator_plugin2<read_custom_log_operator> {
public:
  auto make(invocation inv, session ctx) const
      -> failure_or<operator_ptr> override {
    using namespace std::chrono_literals;
    // We create an `argument_parser2` to parse arguments.
    auto parser = argument_parser2::operator_(name());
    // Our operator will accept an optional duration to offset the timestamp in
    // the log. The default for this offset will be zero.
    // Using `named_optional`, we add our argument. You can also use the
    // `positional` and/or `named`. See `argument_parser2.hpp`. If you dont need
    // any arguments, you can skip this part.
    auto time_offset = tenzir::duration{0s};
    parser.named_optional("time_offset", time_offset);
    // We let the argument_parser parse our arguments. The TRY macro ensures
    // that a parsing failure will stop the setup.
    TRY(parser.parse(inv, ctx));
    // Create the operator instance, passing in any arguments that the operator
    // requires.
    return std::make_unique<read_custom_log_operator>(time_offset);
  }
};

} // namespace
} // namespace tenzir::plugins::example

// Lastly, register our plugin.
TENZIR_REGISTER_PLUGIN(tenzir::plugins::example::read_custom_log_plugin)

// Now, jump back up to read the operator instance's description.
