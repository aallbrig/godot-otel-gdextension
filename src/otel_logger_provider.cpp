#include "otel_logger_provider.h"

#include <godot_cpp/core/class_db.hpp>

#include <opentelemetry/logs/provider.h>

#include "otel_logger.h"

namespace godot {

OtelLoggerProvider* OtelLoggerProvider::_singleton = nullptr;

OtelLoggerProvider::OtelLoggerProvider() { _singleton = this; }

OtelLoggerProvider::~OtelLoggerProvider() { _singleton = nullptr; }

OtelLoggerProvider* OtelLoggerProvider::get_singleton() { return _singleton; }

void OtelLoggerProvider::_bind_methods() {
    ClassDB::bind_static_method("OtelLoggerProvider", D_METHOD("get_instance"),
                                &OtelLoggerProvider::get_singleton);
    ClassDB::bind_method(D_METHOD("get_logger", "name"), &OtelLoggerProvider::get_logger);
}

Ref<OtelLogger> OtelLoggerProvider::get_logger(const String& name) {
    Ref<OtelLogger> logger_wrapper;
    logger_wrapper.instantiate();

    auto provider = opentelemetry::logs::Provider::GetLoggerProvider();
    auto logger = provider->GetLogger(name.utf8().get_data());
    logger_wrapper->set_internal_logger(std::move(logger));
    return logger_wrapper;
}

}  // namespace godot
