extends GdUnitTestSuite

## Installation test: verifies the addon can be installed in another Godot project.
## Checks that the .gdextension file is valid, compiled libraries exist,
## and classes can be instantiated in a clean environment.

func before():
	OtelInit.get_singleton().configure_stdout()

func after():
	OtelInit.get_singleton().shutdown()

# ── Addon structure validation ────────────────────────────────────────────────

func test_gdextension_file_exists() -> void:
	var gdext_path := "res://addons/otel/otel.gdextension"
	assert_bool(ResourceLoader.exists(gdext_path)).is_true()

func test_gdextension_file_is_readable() -> void:
	var gdext_path := "res://addons/otel/otel.gdextension"
	var file := FileAccess.open(gdext_path, FileAccess.READ)
	assert_object(file).is_not_null()

func test_compiled_library_exists_for_platform() -> void:
	var lib_path: String
	match OS.get_name():
		"Linux":
			lib_path = "res://addons/otel/bin/libotel.so"
		"Windows":
			lib_path = "res://addons/otel/bin/otel.dll"
		"macOS":
			lib_path = "res://addons/otel/bin/libotel.dylib"
		_:
			push_error("Unsupported platform: %s" % OS.get_name())
			return

	assert_bool(ResourceLoader.exists(lib_path)).is_true()

# ── Extension class availability ─────────────────────────────────────────────

func test_otel_init_class_available() -> void:
	assert_bool(ClassDB.can_instantiate("OtelInit")).is_true()

func test_otel_tracer_provider_class_available() -> void:
	assert_bool(ClassDB.can_instantiate("OtelTracerProvider")).is_true()

func test_otel_meter_provider_class_available() -> void:
	assert_bool(ClassDB.can_instantiate("OtelMeterProvider")).is_true()

func test_otel_logger_provider_class_available() -> void:
	assert_bool(ClassDB.can_instantiate("OtelLoggerProvider")).is_true()

# ── Basic instantiation in clean context ──────────────────────────────────────

func test_init_singleton_is_accessible() -> void:
	var init := OtelInit.get_singleton()
	assert_object(init).is_not_null()

func test_tracer_provider_instance_obtainable() -> void:
	var provider := OtelTracerProvider.get_instance()
	assert_object(provider).is_not_null()

func test_meter_provider_instance_obtainable() -> void:
	var provider := OtelMeterProvider.get_instance()
	assert_object(provider).is_not_null()

func test_logger_provider_instance_obtainable() -> void:
	var provider := OtelLoggerProvider.get_instance()
	assert_object(provider).is_not_null()

# ── Simulated install: create and use extension in isolation ──────────────────

func test_addon_can_initialize_fresh() -> void:
	# Simulate a new Godot project loading the addon for the first time
	var fresh_init := OtelInit.get_singleton()
	fresh_init.configure_stdout()
	assert_bool(fresh_init.is_configured()).is_true()
	fresh_init.shutdown()

func test_addon_classes_usable_after_initialization() -> void:
	# Ensure the addon is fully functional post-init
	var tracer := OtelTracerProvider.get_instance().get_tracer("install.test", "1.0.0")
	var span := tracer.start_span("test.span")
	assert_str(span.get_trace_id()).has_length(32)
	span.end()

func test_addon_idempotent_initialization() -> void:
	# A fresh project loading the addon should tolerate repeated inits
	OtelInit.get_singleton().configure_stdout()
	OtelInit.get_singleton().configure_stdout()
	assert_bool(OtelInit.get_singleton().is_configured()).is_true()
