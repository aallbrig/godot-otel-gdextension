#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

/// Singleton that initialises and shuts down the OTel SDK.
/// Call configure_stdout() once at startup; call shutdown() on exit.
class OtelInit : public Object {
    GDCLASS(OtelInit, Object)

    static OtelInit* _singleton;

protected:
    static void _bind_methods();

public:
    static OtelInit* get_singleton();

    OtelInit();
    ~OtelInit();

    /// Configure all three providers (traces, metrics, logs) to emit to stdout.
    void configure_stdout();

    /// Configure OTLP HTTP exporters pointed at the given collector URL.
    void configure_otlp_http(const String& endpoint);

    /// Flush pending data and shut down all providers.
    void shutdown();

    bool is_configured() const;

private:
    bool _configured = false;
};

}  // namespace godot
