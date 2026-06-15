#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class OtelLogger;

/// Singleton accessor for the OTel LoggerProvider.
class OtelLoggerProvider : public Object {
    GDCLASS(OtelLoggerProvider, Object)

    static OtelLoggerProvider* _singleton;

protected:
    static void _bind_methods();

public:
    static OtelLoggerProvider* get_singleton();

    OtelLoggerProvider();
    ~OtelLoggerProvider();

    Ref<OtelLogger> get_logger(const String& name);
};

}  // namespace godot
