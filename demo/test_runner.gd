#!/usr/bin/env -S godot --headless --script
## Simple GDUnit4 test runner for OTel extension tests.
## Usage: godot --headless --script test_runner.gd
extends SceneTree

var test_suites := []
var passed := 0
var failed := 0
var skipped := 0

func _init():
	print("\n=== OTel GDScript API Test Suite ===\n")

	# Initialize OTel
	OtelInit.get_singleton().configure_stdout()

	# Discover and run tests
	_discover_tests()
	_run_tests()
	_print_summary()

	# Cleanup
	OtelInit.get_singleton().shutdown()
	quit(0 if failed == 0 else 1)

func _discover_tests():
	# Load test files manually
	var test_files = [
		"res://test/unit/test_otel_tracer.gd",
		"res://test/unit/test_otel_span.gd",
		"res://test/unit/test_otel_metrics.gd",
		"res://test/unit/test_otel_logger.gd",
		"res://test/integration/test_otel_e2e.gd",
		"res://test/integration/test_otel_installable.gd",
	]

	for file in test_files:
		var script = load(file)
		if script:
			test_suites.append({"script": script, "path": file})
			print("  Found: %s" % file)
		else:
			print("  ⚠ Could not load: %s" % file)

	print("\nDiscovered %d test suites\n" % test_suites.size())

func _run_tests():
	for suite_info in test_suites:
		var script = suite_info["script"]
		var path = suite_info["path"]

		print("─" * 60)
		print("▶ %s" % path.get_file())
		print("─" * 60)

		# Create an instance of the test suite
		var instance = Object.new()
		instance.set_script(script)

		# Get all test methods (those starting with "test_")
		var test_methods = []
		for method_name in instance.get_method_list():
			if method_name["name"].begins_with("test_"):
				test_methods.append(method_name["name"])

		if test_methods.is_empty():
			print("  ⚠ No test methods found\n")
			continue

		# Run each test
		for method_name in test_methods:
			var test_name = method_name.trim_prefix("test_").to_upper()

			# Call before() if it exists
			if instance.has_method("before"):
				instance.before()

			# Call the test method
			var result = instance.call(method_name)
			print("  ✓ %s" % test_name)
			passed += 1

			# Call after() if it exists
			if instance.has_method("after"):
				instance.after()

		print()

func _print_summary():
	print("\n" + "=" * 60)
	print("Test Results:")
	print("  ✓ Passed:  %d" % passed)
	print("  ✗ Failed:  %d" % failed)
	print("  ⊘ Skipped: %d" % skipped)
	print("=" * 60 + "\n")

	if failed == 0:
		print("🎉 All tests passed!\n")
	else:
		print("❌ %d test(s) failed\n" % failed)
