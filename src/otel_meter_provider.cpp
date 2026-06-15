#include "otel_meter_provider.h"

#include <godot_cpp/core/class_db.hpp>

#include <opentelemetry/metrics/provider.h>

#include "otel_meter.h"

namespace godot {

OtelMeterProvider* OtelMeterProvider::_singleton = nullptr;

OtelMeterProvider::OtelMeterProvider() { _singleton = this; }

OtelMeterProvider::~OtelMeterProvider() { _singleton = nullptr; }

OtelMeterProvider* OtelMeterProvider::get_singleton() { return _singleton; }

void OtelMeterProvider::_bind_methods() {
    ClassDB::bind_static_method("OtelMeterProvider", D_METHOD("get_instance"),
                                &OtelMeterProvider::get_singleton);
    ClassDB::bind_method(D_METHOD("get_meter", "name", "version"), &OtelMeterProvider::get_meter);
}

Ref<OtelMeter> OtelMeterProvider::get_meter(const String& name, const String& version) {
    Ref<OtelMeter> meter_wrapper;
    meter_wrapper.instantiate();

    auto provider = opentelemetry::metrics::Provider::GetMeterProvider();
    auto meter = provider->GetMeter(name.utf8().get_data(), version.utf8().get_data());
    meter_wrapper->set_internal_meter(std::move(meter));
    return meter_wrapper;
}

}  // namespace godot
