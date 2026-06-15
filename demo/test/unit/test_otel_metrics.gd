extends GdUnitTestSuite

## Unit tests for OtelMeterProvider, OtelMeter, OtelCounter, OtelHistogram.

func before():
	OtelInit.get_singleton().configure_stdout()

func after():
	OtelInit.get_singleton().shutdown()

# ── MeterProvider ─────────────────────────────────────────────────────────────

func test_meter_provider_singleton_is_not_null() -> void:
	var provider := OtelMeterProvider.get_instance()
	assert_object(provider).is_not_null()

func test_get_meter_returns_non_null() -> void:
	var meter := OtelMeterProvider.get_instance().get_meter("game.metrics", "1.0.0")
	assert_object(meter).is_not_null()

# ── Counter ───────────────────────────────────────────────────────────────────

func test_create_int_counter_returns_non_null() -> void:
	var meter := OtelMeterProvider.get_instance().get_meter("game", "1.0.0")
	var counter := meter.create_int_counter("player.deaths", "How many times the player died", "{deaths}")
	assert_object(counter).is_not_null()

func test_counter_add_without_attributes() -> void:
	var meter := OtelMeterProvider.get_instance().get_meter("game", "1.0.0")
	var counter := meter.create_int_counter("player.kills", "Kill count", "{kills}")
	counter.add(1, {})

func test_counter_add_with_attributes() -> void:
	var meter := OtelMeterProvider.get_instance().get_meter("game", "1.0.0")
	var counter := meter.create_int_counter("enemy.spawns", "Enemy spawn count", "{spawns}")
	counter.add(5, {"enemy.type": "goblin", "level.id": "floor_3"})

func test_counter_add_multiple_times() -> void:
	var meter := OtelMeterProvider.get_instance().get_meter("game", "1.0.0")
	var counter := meter.create_int_counter("frame.renders", "Frames rendered", "{frames}")
	for i in range(10):
		counter.add(1, {"quality": "high"})

func test_counter_add_zero_is_safe() -> void:
	var meter := OtelMeterProvider.get_instance().get_meter("game", "1.0.0")
	var counter := meter.create_int_counter("noop.counter", "", "{}")
	counter.add(0, {})

func test_counter_reuse_same_instance() -> void:
	var meter := OtelMeterProvider.get_instance().get_meter("game", "1.0.0")
	var counter := meter.create_int_counter("reuse.counter", "Reuse test", "{}")
	counter.add(1, {"phase": "setup"})
	counter.add(2, {"phase": "play"})
	counter.add(1, {"phase": "cleanup"})

# ── Histogram ─────────────────────────────────────────────────────────────────

func test_create_histogram_returns_non_null() -> void:
	var meter := OtelMeterProvider.get_instance().get_meter("game", "1.0.0")
	var hist := meter.create_double_histogram("frame.time", "Time to render a frame", "ms")
	assert_object(hist).is_not_null()

func test_histogram_record_without_attributes() -> void:
	var meter := OtelMeterProvider.get_instance().get_meter("game", "1.0.0")
	var hist := meter.create_double_histogram("physics.step", "Physics step time", "ms")
	hist.record(16.67, {})

func test_histogram_record_with_attributes() -> void:
	var meter := OtelMeterProvider.get_instance().get_meter("game", "1.0.0")
	var hist := meter.create_double_histogram("rpc.duration", "RPC round-trip time", "ms")
	hist.record(45.2, {"rpc.method": "GetPlayerState", "rpc.status": "ok"})

func test_histogram_record_multiple_values() -> void:
	var meter := OtelMeterProvider.get_instance().get_meter("game", "1.0.0")
	var hist := meter.create_double_histogram("ai.think_time", "AI decision time", "ms")
	var values := [0.5, 1.2, 2.8, 15.0, 0.3]
	for v in values:
		hist.record(v, {"ai.type": "pathfinding"})

func test_histogram_record_zero() -> void:
	var meter := OtelMeterProvider.get_instance().get_meter("game", "1.0.0")
	var hist := meter.create_double_histogram("instant.op", "Instant operation", "ms")
	hist.record(0.0, {})
