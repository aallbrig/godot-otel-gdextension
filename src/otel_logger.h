#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

#include <memory>

namespace opentelemetry {
namespace v1 {
namespace logs {
class Logger;
}
}  // namespace v1
}  // namespace opentelemetry

namespace godot {

/// Wraps opentelemetry::logs::Logger.
class OtelLogger : public RefCounted {
    GDCLASS(OtelLogger, RefCounted)

    std::shared_ptr<opentelemetry::v1::logs::Logger> _logger;

protected:
    static void _bind_methods();

public:
    enum Severity {
        SEV_TRACE = 1,
        SEV_DEBUG = 5,
        SEV_INFO = 9,
        SEV_WARN = 13,
        SEV_ERROR = 17,
        SEV_FATAL = 21,
    };

    void set_internal_logger(std::shared_ptr<opentelemetry::v1::logs::Logger> logger);

    void emit(int severity, const String& body, const Dictionary& attributes);
    void trace(const String& body, const Dictionary& attributes);
    void debug(const String& body, const Dictionary& attributes);
    void info(const String& body, const Dictionary& attributes);
    void warn(const String& body, const Dictionary& attributes);
    void error(const String& body, const Dictionary& attributes);
    void fatal(const String& body, const Dictionary& attributes);
};

}  // namespace godot

VARIANT_ENUM_CAST(godot::OtelLogger::Severity)
