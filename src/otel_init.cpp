#include "otel_init.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <opentelemetry/exporters/ostream/log_record_exporter_factory.h>
#include <opentelemetry/exporters/ostream/metric_exporter_factory.h>
#include <opentelemetry/exporters/ostream/span_exporter_factory.h>
#include <opentelemetry/logs/provider.h>
#include <opentelemetry/metrics/provider.h>
#include <opentelemetry/sdk/logs/logger_provider_factory.h>
#include <opentelemetry/sdk/logs/simple_log_record_processor_factory.h>
#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h>
#include <opentelemetry/sdk/metrics/meter_provider_factory.h>
#include <opentelemetry/sdk/trace/simple_processor_factory.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
#include <opentelemetry/trace/provider.h>

namespace trace_api = opentelemetry::trace;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace metrics_api = opentelemetry::metrics;
namespace metrics_sdk = opentelemetry::sdk::metrics;
namespace logs_api = opentelemetry::logs;
namespace logs_sdk = opentelemetry::sdk::logs;
namespace ostream_exporter = opentelemetry::exporter::trace;

namespace godot {

OtelInit* OtelInit::_singleton = nullptr;

OtelInit::OtelInit() {
    _singleton = this;
}

OtelInit::~OtelInit() {
    if (_configured) shutdown();
    _singleton = nullptr;
}

OtelInit* OtelInit::get_singleton() {
    return _singleton;
}

void OtelInit::_bind_methods() {
    ClassDB::bind_static_method("OtelInit", D_METHOD("get_singleton"), &OtelInit::get_singleton);
    ClassDB::bind_method(D_METHOD("configure_stdout"), &OtelInit::configure_stdout);
    ClassDB::bind_method(D_METHOD("configure_otlp_http", "endpoint"),
                         &OtelInit::configure_otlp_http);
    ClassDB::bind_method(D_METHOD("shutdown"), &OtelInit::shutdown);
    ClassDB::bind_method(D_METHOD("is_configured"), &OtelInit::is_configured);
}

void OtelInit::configure_stdout() {
    if (_configured) {
        UtilityFunctions::push_warning("OtelInit: already configured; ignoring configure_stdout()");
        return;
    }

    // --- Traces ---
    auto trace_exporter = opentelemetry::exporter::trace::OStreamSpanExporterFactory::Create();
    auto trace_processor =
        trace_sdk::SimpleSpanProcessorFactory::Create(std::move(trace_exporter));
    auto trace_provider =
        trace_sdk::TracerProviderFactory::Create(std::move(trace_processor));
    trace_api::Provider::SetTracerProvider(
        opentelemetry::nostd::shared_ptr<trace_api::TracerProvider>(trace_provider.release()));

    // --- Metrics ---
    auto metrics_exporter =
        opentelemetry::exporter::metrics::OStreamMetricExporterFactory::Create();
    metrics_sdk::PeriodicExportingMetricReaderOptions reader_opts;
    reader_opts.export_interval_millis = std::chrono::milliseconds(5000);
    reader_opts.export_timeout_millis = std::chrono::milliseconds(500);
    auto metrics_reader =
        metrics_sdk::PeriodicExportingMetricReaderFactory::Create(
            std::move(metrics_exporter), reader_opts);
    auto metrics_provider = metrics_sdk::MeterProviderFactory::Create();
    static_cast<metrics_sdk::MeterProvider*>(metrics_provider.get())
        ->AddMetricReader(std::move(metrics_reader));
    metrics_api::Provider::SetMeterProvider(
        opentelemetry::nostd::shared_ptr<metrics_api::MeterProvider>(
            metrics_provider.release()));

    // --- Logs ---
    auto logs_exporter =
        opentelemetry::exporter::logs::OStreamLogRecordExporterFactory::Create();
    auto logs_processor =
        logs_sdk::SimpleLogRecordProcessorFactory::Create(std::move(logs_exporter));
    auto logs_provider = logs_sdk::LoggerProviderFactory::Create(std::move(logs_processor));
    logs_api::Provider::SetLoggerProvider(
        opentelemetry::nostd::shared_ptr<logs_api::LoggerProvider>(logs_provider.release()));

    _configured = true;
    UtilityFunctions::print("OtelInit: configured with stdout exporters");
}

void OtelInit::configure_otlp_http(const String& endpoint) {
    // TODO: implement OTLP HTTP exporter configuration
    UtilityFunctions::push_warning(
        String("OtelInit: configure_otlp_http not yet implemented (endpoint: ") + endpoint + ")");
}

void OtelInit::shutdown() {
    if (!_configured) return;

    // Flush + shutdown trace provider
    auto tp = trace_api::Provider::GetTracerProvider();
    if (auto sdk_tp =
            dynamic_cast<trace_sdk::TracerProvider*>(tp.get())) {
        sdk_tp->ForceFlush();
        sdk_tp->Shutdown();
    }
    trace_api::Provider::SetTracerProvider(
        opentelemetry::nostd::shared_ptr<trace_api::TracerProvider>());

    // Flush + shutdown metrics provider
    auto mp = metrics_api::Provider::GetMeterProvider();
    if (auto sdk_mp =
            dynamic_cast<metrics_sdk::MeterProvider*>(mp.get())) {
        sdk_mp->ForceFlush();
        sdk_mp->Shutdown();
    }
    metrics_api::Provider::SetMeterProvider(
        opentelemetry::nostd::shared_ptr<metrics_api::MeterProvider>());

    // Flush + shutdown logs provider
    auto lp = logs_api::Provider::GetLoggerProvider();
    if (auto sdk_lp =
            dynamic_cast<logs_sdk::LoggerProvider*>(lp.get())) {
        sdk_lp->ForceFlush();
        sdk_lp->Shutdown();
    }
    logs_api::Provider::SetLoggerProvider(
        opentelemetry::nostd::shared_ptr<logs_api::LoggerProvider>());

    _configured = false;
    UtilityFunctions::print("OtelInit: shut down");
}

bool OtelInit::is_configured() const {
    return _configured;
}

}  // namespace godot
