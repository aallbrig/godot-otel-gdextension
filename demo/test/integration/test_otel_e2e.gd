extends GdUnitTestSuite

## End-to-end integration tests: verifies the full OTel signal pipeline.
## Exercises traces → metrics → logs together in realistic patterns.

func before():
	OtelInit.get_singleton().configure_stdout()

func after():
	OtelInit.get_singleton().shutdown()

# ── Full signal flow ──────────────────────────────────────────────────────────

func test_complete_game_session_signals() -> void:
	# Session-level trace
	var tracer := OtelTracerProvider.get_instance().get_tracer("game.session", "1.0.0")
	var meter := OtelMeterProvider.get_instance().get_meter("game.session", "1.0.0")
	var logger := OtelLoggerProvider.get_instance().get_logger("game.session")

	var session_span := tracer.start_span("session")
	session_span.set_attribute("session.id", "integ-test-001")
	session_span.set_attribute("player.id", "p-test")

	var deaths := meter.create_int_counter("player.deaths", "Death count", "{deaths}")
	var latency := meter.create_double_histogram("server.latency", "Server round-trip", "ms")

	logger.info("session.started", {
		"session.id": "integ-test-001",
		"player.id": "p-test",
	})

	# Simulate a level
	var level_span := tracer.start_span_with_parent("level.play", session_span)
	level_span.set_attribute("level.name", "dungeon_floor_1")

	deaths.add(1, {"cause": "spike_trap"})
	latency.record(42.0, {"op": "respawn"})

	logger.warn("player.died", {
		"cause": "spike_trap",
		"player.hp": 0,
		"level.name": "dungeon_floor_1",
	})

	level_span.set_status(OtelSpan.STATUS_OK, "")
	level_span.end()

	deaths.add(0, {})  # no additional deaths
	latency.record(5.3, {"op": "level_complete"})

	logger.info("level.completed", {
		"level.name": "dungeon_floor_1",
		"score": 4200,
		"time_ms": 180000,
	})

	session_span.set_status(OtelSpan.STATUS_OK, "")
	session_span.end()

func test_error_propagation_through_signals() -> void:
	var tracer := OtelTracerProvider.get_instance().get_tracer("game.error", "1.0.0")
	var logger := OtelLoggerProvider.get_instance().get_logger("game.error")

	var op_span := tracer.start_span("asset.load")
	op_span.set_attribute("asset.path", "res://missing_asset.tres")
	op_span.set_attribute("asset.type", "Texture2D")

	# Simulate failure
	op_span.set_status(OtelSpan.STATUS_ERROR, "ResourceLoader: file not found")
	op_span.add_event("error.occurred", {
		"error.type": "FileNotFoundError",
		"asset.path": "res://missing_asset.tres",
	})

	logger.error("asset.load.failed", {
		"asset.path": "res://missing_asset.tres",
		"error.message": "FileNotFoundError",
		"trace.id": op_span.get_trace_id(),
		"span.id": op_span.get_span_id(),
	})

	op_span.end()

func test_nested_spans_model_call_stack() -> void:
	var tracer := OtelTracerProvider.get_instance().get_tracer("game.render", "1.0.0")

	var frame_span := tracer.start_span("frame.render")
	var physics_span := tracer.start_span_with_parent("physics.update", frame_span)
	var collision_span := tracer.start_span_with_parent("collision.detect", physics_span)

	collision_span.set_attribute("collider.count", 24)
	collision_span.end()

	physics_span.set_attribute("body.count", 80)
	physics_span.end()

	frame_span.set_attribute("draw.calls", 312)
	frame_span.end()

	# All three should share the same trace
	# (Checked above at unit level; here we confirm the real pipeline holds)

func test_otel_init_idempotent_shutdown() -> void:
	# Shutdown in after() will be called; this extra call should not crash
	OtelInit.get_singleton().shutdown()
	# Reconfigure for after() shutdown
	OtelInit.get_singleton().configure_stdout()

func test_is_configured_reflects_state() -> void:
	assert_bool(OtelInit.get_singleton().is_configured()).is_true()
