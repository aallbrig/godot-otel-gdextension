#include "otel_tracer_provider.h"

#include "otel_tracer.h"

#include <godot_cpp/core/class_db.hpp>

#include <opentelemetry/trace/provider.h>

namespace godot {

OtelTracerProvider* OtelTracerProvider::_singleton = nullptr;

OtelTracerProvider::OtelTracerProvider() {
    _singleton = this;
}

OtelTracerProvider::~OtelTracerProvider() {
    _singleton = nullptr;
}

OtelTracerProvider* OtelTracerProvider::get_singleton() {
    return _singleton;
}

void OtelTracerProvider::_bind_methods() {
    ClassDB::bind_static_method(
        "OtelTracerProvider",
        D_METHOD("get_instance"),
        &OtelTracerProvider::get_singleton);
    ClassDB::bind_method(
        D_METHOD("get_tracer", "name", "version"), &OtelTracerProvider::get_tracer);
}

Ref<OtelTracer> OtelTracerProvider::get_tracer(
    const String& name, const String& version) {
    Ref<OtelTracer> tracer_wrapper;
    tracer_wrapper.instantiate();

    auto provider = opentelemetry::trace::Provider::GetTracerProvider();
    auto tracer = provider->GetTracer(
        name.utf8().get_data(), version.utf8().get_data());
    tracer_wrapper->set_internal_tracer(std::move(tracer));
    return tracer_wrapper;
}

}  // namespace godot
