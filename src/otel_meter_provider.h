#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class OtelMeter;

/// Singleton accessor for the OTel MeterProvider.
class OtelMeterProvider : public Object {
    GDCLASS(OtelMeterProvider, Object)

    static OtelMeterProvider* _singleton;

protected:
    static void _bind_methods();

public:
    static OtelMeterProvider* get_singleton();

    OtelMeterProvider();
    ~OtelMeterProvider();

    Ref<OtelMeter> get_meter(const String& name, const String& version);
};

}  // namespace godot
