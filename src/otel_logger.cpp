#include "otel_logger.h"

#include <godot_cpp/core/class_db.hpp>

#include <opentelemetry/logs/log_record.h>
#include <opentelemetry/logs/logger.h>
#include <opentelemetry/logs/severity.h>

namespace godot {

void OtelLogger::_bind_methods() {
    ClassDB::bind_method(
        D_METHOD("emit", "severity", "body", "attributes"), &OtelLogger::emit);
    ClassDB::bind_method(
        D_METHOD("trace", "body", "attributes"), &OtelLogger::trace);
    ClassDB::bind_method(
        D_METHOD("debug", "body", "attributes"), &OtelLogger::debug);
    ClassDB::bind_method(
        D_METHOD("info", "body", "attributes"), &OtelLogger::info);
    ClassDB::bind_method(
        D_METHOD("warn", "body", "attributes"), &OtelLogger::warn);
    ClassDB::bind_method(
        D_METHOD("error", "body", "attributes"), &OtelLogger::error);
    ClassDB::bind_method(
        D_METHOD("fatal", "body", "attributes"), &OtelLogger::fatal);

    BIND_ENUM_CONSTANT(SEV_TRACE);
    BIND_ENUM_CONSTANT(SEV_DEBUG);
    BIND_ENUM_CONSTANT(SEV_INFO);
    BIND_ENUM_CONSTANT(SEV_WARN);
    BIND_ENUM_CONSTANT(SEV_ERROR);
    BIND_ENUM_CONSTANT(SEV_FATAL);
}

void OtelLogger::set_internal_logger(
    std::shared_ptr<opentelemetry::v1::logs::Logger> logger) {
    _logger = std::move(logger);
}

void OtelLogger::emit(int severity, const String& body, const Dictionary& attributes) {
    if (!_logger) return;

    auto record = _logger->CreateLogRecord();
    if (!record) return;

    record->SetSeverity(static_cast<opentelemetry::logs::Severity>(severity));
    record->SetBody(opentelemetry::nostd::string_view(body.utf8().get_data()));
    record->SetTimestamp(opentelemetry::common::SystemTimestamp(
        std::chrono::system_clock::now()));

    Array keys = attributes.keys();
    for (int i = 0; i < keys.size(); ++i) {
        std::string key = String(keys[i]).utf8().get_data();
        Variant val = attributes[keys[i]];
        switch (val.get_type()) {
            case Variant::BOOL:
                record->SetAttribute(key, static_cast<bool>(val));
                break;
            case Variant::INT:
                record->SetAttribute(key, static_cast<int64_t>(val));
                break;
            case Variant::FLOAT:
                record->SetAttribute(key, static_cast<double>(val));
                break;
            default:
                record->SetAttribute(
                    key, opentelemetry::nostd::string_view(
                             String(val).utf8().get_data()));
                break;
        }
    }

    _logger->EmitLogRecord(std::move(record));
}

void OtelLogger::trace(const String& body, const Dictionary& attributes) {
    emit(SEV_TRACE, body, attributes);
}

void OtelLogger::debug(const String& body, const Dictionary& attributes) {
    emit(SEV_DEBUG, body, attributes);
}

void OtelLogger::info(const String& body, const Dictionary& attributes) {
    emit(SEV_INFO, body, attributes);
}

void OtelLogger::warn(const String& body, const Dictionary& attributes) {
    emit(SEV_WARN, body, attributes);
}

void OtelLogger::error(const String& body, const Dictionary& attributes) {
    emit(SEV_ERROR, body, attributes);
}

void OtelLogger::fatal(const String& body, const Dictionary& attributes) {
    emit(SEV_FATAL, body, attributes);
}

}  // namespace godot
