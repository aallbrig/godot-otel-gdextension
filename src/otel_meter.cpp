#include "otel_meter.h"

#include <godot_cpp/core/class_db.hpp>

#include <opentelemetry/metrics/meter.h>

#include "otel_counter.h"
#include "otel_histogram.h"

namespace godot {

void OtelMeter::_bind_methods() {
    ClassDB::bind_method(D_METHOD("create_int_counter", "name", "description", "unit"),
                         &OtelMeter::create_int_counter);
    ClassDB::bind_method(D_METHOD("create_double_histogram", "name", "description", "unit"),
                         &OtelMeter::create_double_histogram);
}

void OtelMeter::set_internal_meter(std::shared_ptr<opentelemetry::v1::metrics::Meter> meter) {
    _meter = std::move(meter);
}

Ref<OtelCounter> OtelMeter::create_int_counter(const String& name, const String& description,
                                               const String& unit) {
    Ref<OtelCounter> wrapper;
    wrapper.instantiate();
    if (!_meter) return wrapper;

    auto counter = _meter->CreateUInt64Counter(
        name.utf8().get_data(), description.utf8().get_data(), unit.utf8().get_data());
    wrapper->set_internal_counter(std::move(counter));
    return wrapper;
}

Ref<OtelHistogram> OtelMeter::create_double_histogram(const String& name, const String& description,
                                                      const String& unit) {
    Ref<OtelHistogram> wrapper;
    wrapper.instantiate();
    if (!_meter) return wrapper;

    auto histogram = _meter->CreateDoubleHistogram(
        name.utf8().get_data(), description.utf8().get_data(), unit.utf8().get_data());
    wrapper->set_internal_histogram(std::move(histogram));
    return wrapper;
}

}  // namespace godot
