#include "otel_span.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/span_context.h>
#include <opentelemetry/trace/span_id.h>
#include <opentelemetry/trace/trace_id.h>

#include <iomanip>
#include <sstream>

namespace godot {

namespace {
std::string to_hex(const uint8_t* data, size_t len) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) ss << std::setw(2) << static_cast<int>(data[i]);
    return ss.str();
}

// Convert a Godot Dictionary to an OTel attribute map.
// Returns a vector of pairs we can pass via initializer.
std::vector<std::pair<std::string, opentelemetry::common::AttributeValue>> dict_to_attrs(
    const Dictionary& dict) {
    std::vector<std::pair<std::string, opentelemetry::common::AttributeValue>> result;
    Array keys = dict.keys();
    for (int i = 0; i < keys.size(); ++i) {
        std::string key = String(keys[i]).utf8().get_data();
        Variant val = dict[keys[i]];
        switch (val.get_type()) {
            case Variant::BOOL:
                result.emplace_back(key, static_cast<bool>(val));
                break;
            case Variant::INT:
                result.emplace_back(key, static_cast<int64_t>(val));
                break;
            case Variant::FLOAT:
                result.emplace_back(key, static_cast<double>(val));
                break;
            default:
                result.emplace_back(
                    key, opentelemetry::nostd::string_view(String(val).utf8().get_data()));
                break;
        }
    }
    return result;
}
}  // namespace

void OtelSpan::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_attribute", "key", "value"), &OtelSpan::set_attribute);
    ClassDB::bind_method(D_METHOD("set_status", "status_code", "description"),
                         &OtelSpan::set_status);
    ClassDB::bind_method(D_METHOD("add_event", "name", "attributes"), &OtelSpan::add_event);
    ClassDB::bind_method(D_METHOD("end"), &OtelSpan::end);
    ClassDB::bind_method(D_METHOD("get_trace_id"), &OtelSpan::get_trace_id);
    ClassDB::bind_method(D_METHOD("get_span_id"), &OtelSpan::get_span_id);
    ClassDB::bind_method(D_METHOD("is_recording"), &OtelSpan::is_recording);
    ClassDB::bind_method(D_METHOD("is_valid"), &OtelSpan::is_valid);

    BIND_ENUM_CONSTANT(STATUS_UNSET);
    BIND_ENUM_CONSTANT(STATUS_OK);
    BIND_ENUM_CONSTANT(STATUS_ERROR);
}

void OtelSpan::set_internal_span(std::shared_ptr<opentelemetry::v1::trace::Span> span) {
    _span = std::move(span);
}

std::shared_ptr<opentelemetry::v1::trace::Span> OtelSpan::get_internal_span() const {
    return _span;
}

void OtelSpan::set_attribute(const String& key, const Variant& value) {
    if (!_span || _ended) return;
    std::string k = key.utf8().get_data();
    switch (value.get_type()) {
        case Variant::BOOL:
            _span->SetAttribute(k, static_cast<bool>(value));
            break;
        case Variant::INT:
            _span->SetAttribute(k, static_cast<int64_t>(value));
            break;
        case Variant::FLOAT:
            _span->SetAttribute(k, static_cast<double>(value));
            break;
        default:
            _span->SetAttribute(k,
                                opentelemetry::nostd::string_view(String(value).utf8().get_data()));
            break;
    }
}

void OtelSpan::set_status(int status_code, const String& description) {
    if (!_span || _ended) return;
    auto desc = opentelemetry::nostd::string_view(description.utf8().get_data());
    switch (status_code) {
        case STATUS_OK:
            _span->SetStatus(opentelemetry::trace::StatusCode::kOk, desc);
            break;
        case STATUS_ERROR:
            _span->SetStatus(opentelemetry::trace::StatusCode::kError, desc);
            break;
        default:
            _span->SetStatus(opentelemetry::trace::StatusCode::kUnset, desc);
            break;
    }
}

void OtelSpan::add_event(const String& name, const Dictionary& attributes) {
    if (!_span || _ended) return;
    auto attrs = dict_to_attrs(attributes);
    // OTel C++ SDK AddEvent expects an attribute map
    std::map<std::string, opentelemetry::common::AttributeValue> attr_map(attrs.begin(),
                                                                          attrs.end());
    _span->AddEvent(opentelemetry::nostd::string_view(name.utf8().get_data()),
                    opentelemetry::common::SystemTimestamp(std::chrono::system_clock::now()),
                    opentelemetry::common::MakeAttributes(attr_map));
}

void OtelSpan::end() {
    if (!_span || _ended) return;
    _span->End();
    _ended = true;
}

String OtelSpan::get_trace_id() const {
    if (!_span) return String();
    auto ctx = _span->GetContext();
    if (!ctx.IsValid()) return String();
    uint8_t buf[opentelemetry::trace::TraceId::kSize];
    ctx.trace_id().ToLowerBase16(
        opentelemetry::nostd::span<char, opentelemetry::trace::TraceId::kSize * 2>(
            reinterpret_cast<char*>(buf), sizeof(buf)));
    return String(to_hex(buf, sizeof(buf)).c_str());
}

String OtelSpan::get_span_id() const {
    if (!_span) return String();
    auto ctx = _span->GetContext();
    if (!ctx.IsValid()) return String();
    uint8_t buf[opentelemetry::trace::SpanId::kSize];
    ctx.span_id().ToLowerBase16(
        opentelemetry::nostd::span<char, opentelemetry::trace::SpanId::kSize * 2>(
            reinterpret_cast<char*>(buf), sizeof(buf)));
    return String(to_hex(buf, sizeof(buf)).c_str());
}

bool OtelSpan::is_recording() const {
    if (!_span || _ended) return false;
    return _span->IsRecording();
}

bool OtelSpan::is_valid() const { return _span != nullptr && !_ended; }

}  // namespace godot
