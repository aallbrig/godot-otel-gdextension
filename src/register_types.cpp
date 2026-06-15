#include "register_types.h"

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include <gdextension_interface.h>

#include "otel_counter.h"
#include "otel_histogram.h"
#include "otel_init.h"
#include "otel_logger.h"
#include "otel_logger_provider.h"
#include "otel_meter.h"
#include "otel_meter_provider.h"
#include "otel_span.h"
#include "otel_tracer.h"
#include "otel_tracer_provider.h"

using namespace godot;

void initialize_otel_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) return;

    ClassDB::register_class<OtelInit>();
    ClassDB::register_class<OtelTracerProvider>();
    ClassDB::register_class<OtelTracer>();
    ClassDB::register_class<OtelSpan>();
    ClassDB::register_class<OtelMeterProvider>();
    ClassDB::register_class<OtelMeter>();
    ClassDB::register_class<OtelCounter>();
    ClassDB::register_class<OtelHistogram>();
    ClassDB::register_class<OtelLoggerProvider>();
    ClassDB::register_class<OtelLogger>();
}

void uninitialize_otel_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) return;
}

extern "C" {
GDExtensionBool GDE_EXPORT gdextension_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                                            const GDExtensionClassLibraryPtr p_library,
                                            GDExtensionInitialization* r_initialization) {
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_otel_module);
    init_obj.register_terminator(uninitialize_otel_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}
