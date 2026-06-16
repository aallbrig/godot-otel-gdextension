#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include <opentelemetry/metrics/sync_instruments.h>

#include <memory>

namespace godot {

/// Wraps a double OTel Histogram instrument.
class OtelHistogram : public RefCounted {
    GDCLASS(OtelHistogram, RefCounted)

    std::shared_ptr<opentelemetry::v1::metrics::Histogram<double>> _histogram;

protected:
    static void _bind_methods();

public:
    void set_internal_histogram(
        std::shared_ptr<opentelemetry::v1::metrics::Histogram<double>> histogram);

    void record(double value, const Dictionary& attributes);
};

}  // namespace godot
