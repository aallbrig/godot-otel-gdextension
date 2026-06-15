#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class OtelTracer;

/// Singleton accessor for the OTel TracerProvider.
/// The underlying SDK provider is configured via OtelInit.
class OtelTracerProvider : public Object {
    GDCLASS(OtelTracerProvider, Object)

    static OtelTracerProvider* _singleton;

protected:
    static void _bind_methods();

public:
    static OtelTracerProvider* get_singleton();

    OtelTracerProvider();
    ~OtelTracerProvider();

    /// Returns a Tracer for the named instrumentation scope.
    Ref<OtelTracer> get_tracer(const String& name, const String& version);
};

}  // namespace godot
