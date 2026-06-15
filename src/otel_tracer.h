#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string.hpp>
#include <memory>

namespace opentelemetry {
namespace v1 {
namespace trace {
class Tracer;
}
}  // namespace v1
}  // namespace opentelemetry

namespace godot {

class OtelSpan;

/// Wraps opentelemetry::trace::Tracer.
class OtelTracer : public RefCounted {
    GDCLASS(OtelTracer, RefCounted)

    std::shared_ptr<opentelemetry::v1::trace::Tracer> _tracer;

protected:
    static void _bind_methods();

public:
    void set_internal_tracer(std::shared_ptr<opentelemetry::v1::trace::Tracer> tracer);

    Ref<OtelSpan> start_span(const String& name);
    Ref<OtelSpan> start_span_with_parent(const String& name, Ref<OtelSpan> parent);
};

}  // namespace godot
