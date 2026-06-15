#pragma once

#include <godot_cpp/core/class_db.hpp>

void initialize_otel_module(godot::ModuleInitializationLevel p_level);
void uninitialize_otel_module(godot::ModuleInitializationLevel p_level);
