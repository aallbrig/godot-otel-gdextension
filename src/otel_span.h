#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <memory>

namespace opentelemetry {
namespace v1 {
namespace trace {
class Span;
}
}  // namespace v1
}  // namespace opentelemetry

namespace godot {

/// Wraps opentelemetry::trace::Span.
/// Ref-counted so GDScript can hold it across frames without leaking.
class OtelSpan : public RefCounted {
    GDCLASS(OtelSpan, RefCounted)

    std::shared_ptr<opentelemetry::v1::trace::Span> _span;
    bool _ended = false;

protected:
    static void _bind_methods();

public:
    enum Status {
        STATUS_UNSET = 0,
        STATUS_OK = 1,
        STATUS_ERROR = 2,
    };

    void set_internal_span(std::shared_ptr<opentelemetry::v1::trace::Span> span);
    std::shared_ptr<opentelemetry::v1::trace::Span> get_internal_span() const;

    void set_attribute(const String& key, const Variant& value);
    void set_status(int status_code, const String& description);
    void add_event(const String& name, const Dictionary& attributes);
    void end();

    String get_trace_id() const;
    String get_span_id() const;
    bool is_recording() const;
    bool is_valid() const;
};

}  // namespace godot

VARIANT_ENUM_CAST(godot::OtelSpan::Status)
