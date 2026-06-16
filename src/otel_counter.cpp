#include "otel_counter.h"

#include <godot_cpp/core/class_db.hpp>

#include <opentelemetry/context/runtime_context.h>
#include <opentelemetry/metrics/sync_instruments.h>

#include <map>
#include <string>
#include <vector>

namespace godot {

namespace {
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

void OtelCounter::_bind_methods() {
    ClassDB::bind_method(D_METHOD("add", "value", "attributes"), &OtelCounter::add);
}

void OtelCounter::set_internal_counter(
    std::shared_ptr<opentelemetry::v1::metrics::Counter<uint64_t>> counter) {
    _counter = std::move(counter);
}

void OtelCounter::add(int64_t value, const Dictionary& attributes) {
    if (!_counter || value < 0) return;
    auto attrs = dict_to_attrs(attributes);
    std::map<std::string, opentelemetry::common::AttributeValue> attr_map(attrs.begin(),
                                                                          attrs.end());
    _counter->Add(static_cast<uint64_t>(value), opentelemetry::common::MakeAttributes(attr_map));
}

}  // namespace godot
