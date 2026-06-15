#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include <memory>
#include <string>

namespace opentelemetry {
namespace v1 {
namespace metrics {
template <typename T>
class Counter;
}
}  // namespace v1
}  // namespace opentelemetry

namespace godot {

/// Wraps a uint64_t OTel Counter instrument.
class OtelCounter : public RefCounted {
    GDCLASS(OtelCounter, RefCounted)

    std::shared_ptr<opentelemetry::v1::metrics::Counter<uint64_t>> _counter;

protected:
    static void _bind_methods();

public:
    void set_internal_counter(
        std::shared_ptr<opentelemetry::v1::metrics::Counter<uint64_t>> counter);

    void add(int64_t value, const Dictionary& attributes);
};

}  // namespace godot
