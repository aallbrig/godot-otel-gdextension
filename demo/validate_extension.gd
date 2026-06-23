#!/usr/bin/env -S godot --headless
## Validate that the OTel extension is properly installed and accessible.
extends SceneTree

func _ready():
	print("\n=== OTel Extension Validation ===\n")

	# Check 1: gdextension file exists
	var gdext_path = "res://addons/otel/otel.gdextension"
	var gdext_exists = ResourceLoader.exists(gdext_path)
	print("✓ gdextension manifest: %s" % ("EXISTS" if gdext_exists else "MISSING"))

	# Check 2: Binary exists
	var binary_path = "addons/otel/bin/libotel.linux.template_debug.x86_64.so"
	var file = FileAccess.open(binary_path, FileAccess.READ)
	var binary_exists = file != null
	print("✓ Compiled binary: %s" % ("EXISTS" if binary_exists else "MISSING"))

	# Check 3: Classes available
	var classes = [
		"OtelInit",
		"OtelTracerProvider",
		"OtelTracer",
		"OtelSpan",
		"OtelMeterProvider",
		"OtelMeter",
		"OtelCounter",
		"OtelHistogram",
		"OtelLoggerProvider",
		"OtelLogger",
	]

	var available_count = 0
	for cls in classes:
		if ClassDB.can_instantiate(cls):
			available_count += 1
			print("  ✓ %s" % cls)
		else:
			print("  ✗ %s (NOT FOUND)" % cls)

	print("")
	print("Classes available: %d/%d" % [available_count, classes.size()])

	if available_count == classes.size():
		print("\n✅ Extension is properly installed and all classes are available!\n")
		quit(0)
	else:
		print("\n❌ Extension not fully loaded. Open in editor to load.\n")
		quit(1)
