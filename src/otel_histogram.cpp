#include "otel_histogram.h"

#include <godot_cpp/core/class_db.hpp>

#include <opentelemetry/metrics/sync_instruments.h>

#include <string>
#include <vector>

namespace godot {

namespace {
std::vector<std::pair<std::string, opentelemetry::common::AttributeValue>>
dict_to_attrs(const Dictionary& dict) {
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
                    key, opentelemetry::nostd::string_view(
                             String(val).utf8().get_data()));
                break;
        }
    }
    return result;
}
}  // namespace

void OtelHistogram::_bind_methods() {
    ClassDB::bind_method(
        D_METHOD("record", "value", "attributes"), &OtelHistogram::record);
}

void OtelHistogram::set_internal_histogram(
    std::shared_ptr<opentelemetry::v1::metrics::Histogram<double>> histogram) {
    _histogram = std::move(histogram);
}

void OtelHistogram::record(double value, const Dictionary& attributes) {
    if (!_histogram) return;
    auto attrs = dict_to_attrs(attributes);
    std::map<std::string, opentelemetry::common::AttributeValue> attr_map(
        attrs.begin(), attrs.end());
    _histogram->Record(value, opentelemetry::common::MakeAttributes(attr_map),
                       opentelemetry::context::RuntimeContext::GetCurrent());
}

}  // namespace godot
