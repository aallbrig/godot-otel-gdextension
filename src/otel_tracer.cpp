#include "otel_tracer.h"

#include "otel_span.h"

#include <godot_cpp/core/class_db.hpp>

#include <opentelemetry/trace/span_startoptions.h>
#include <opentelemetry/trace/tracer.h>

namespace godot {

void OtelTracer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("start_span", "name"), &OtelTracer::start_span);
    ClassDB::bind_method(
        D_METHOD("start_span_with_parent", "name", "parent"),
        &OtelTracer::start_span_with_parent);
}

void OtelTracer::set_internal_tracer(
    std::shared_ptr<opentelemetry::v1::trace::Tracer> tracer) {
    _tracer = std::move(tracer);
}

Ref<OtelSpan> OtelTracer::start_span(const String& name) {
    Ref<OtelSpan> span_wrapper;
    span_wrapper.instantiate();
    if (!_tracer) return span_wrapper;

    auto span = _tracer->StartSpan(name.utf8().get_data());
    span_wrapper->set_internal_span(std::move(span));
    return span_wrapper;
}

Ref<OtelSpan> OtelTracer::start_span_with_parent(
    const String& name, Ref<OtelSpan> parent) {
    Ref<OtelSpan> span_wrapper;
    span_wrapper.instantiate();
    if (!_tracer) return span_wrapper;

    opentelemetry::trace::StartSpanOptions opts;
    if (parent.is_valid() && parent->get_internal_span()) {
        opts.parent = parent->get_internal_span()->GetContext();
    }

    auto span = _tracer->StartSpan(name.utf8().get_data(), opts);
    span_wrapper->set_internal_span(std::move(span));
    return span_wrapper;
}

}  // namespace godot
