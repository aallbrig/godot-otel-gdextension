#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string.hpp>

#include <memory>

namespace opentelemetry {
namespace v1 {
namespace metrics {
class Meter;
}
}  // namespace v1
}  // namespace opentelemetry

namespace godot {

class OtelCounter;
class OtelHistogram;

/// Wraps opentelemetry::metrics::Meter.
class OtelMeter : public RefCounted {
    GDCLASS(OtelMeter, RefCounted)

    std::shared_ptr<opentelemetry::v1::metrics::Meter> _meter;

protected:
    static void _bind_methods();

public:
    void set_internal_meter(std::shared_ptr<opentelemetry::v1::metrics::Meter> meter);

    Ref<OtelCounter> create_int_counter(const String& name, const String& description,
                                        const String& unit);

    Ref<OtelHistogram> create_double_histogram(const String& name, const String& description,
                                               const String& unit);
};

}  // namespace godot
