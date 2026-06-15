extends GdUnitTestSuite

## Unit tests for OtelSpan attribute setting, events, and status.

var _tracer: OtelTracer

func before():
	OtelInit.get_singleton().configure_stdout()
	_tracer = OtelTracerProvider.get_instance().get_tracer("test.span", "1.0.0")

func after():
	OtelInit.get_singleton().shutdown()

func test_set_string_attribute() -> void:
	var span := _tracer.start_span("attr.string")
	span.set_attribute("service.name", "my-game")
	span.end()

func test_set_int_attribute() -> void:
	var span := _tracer.start_span("attr.int")
	span.set_attribute("player.level", 42)
	span.end()

func test_set_float_attribute() -> void:
	var span := _tracer.start_span("attr.float")
	span.set_attribute("player.health", 87.5)
	span.end()

func test_set_bool_attribute() -> void:
	var span := _tracer.start_span("attr.bool")
	span.set_attribute("level.boss_defeated", true)
	span.end()

func test_set_multiple_attributes() -> void:
	var span := _tracer.start_span("attr.multiple")
	span.set_attribute("player.id", "p-123")
	span.set_attribute("player.level", 5)
	span.set_attribute("player.score", 9500.0)
	span.set_attribute("level.completed", true)
	span.end()

func test_set_status_ok() -> void:
	var span := _tracer.start_span("status.ok")
	span.set_status(OtelSpan.STATUS_OK, "")
	span.end()

func test_set_status_error() -> void:
	var span := _tracer.start_span("status.error")
	span.set_status(OtelSpan.STATUS_ERROR, "Physics collision exploded")
	span.end()

func test_set_status_unset() -> void:
	var span := _tracer.start_span("status.unset")
	span.set_status(OtelSpan.STATUS_UNSET, "")
	span.end()

func test_add_event_with_attributes() -> void:
	var span := _tracer.start_span("event.test")
	span.add_event("player.jumped", {
		"jump.height": 2.5,
		"jump.type": "double",
	})
	span.end()

func test_add_multiple_events() -> void:
	var span := _tracer.start_span("event.multiple")
	span.add_event("combat.started", {"enemy.type": "goblin"})
	span.add_event("combat.hit", {"damage": 15, "crit": false})
	span.add_event("combat.ended", {"result": "victory", "loot.gold": 42})
	span.end()

func test_add_event_with_empty_attributes() -> void:
	var span := _tracer.start_span("event.empty_attrs")
	span.add_event("checkpoint.reached", {})
	span.end()

func test_set_attribute_after_end_is_safe() -> void:
	var span := _tracer.start_span("attr.after_end")
	span.end()
	# Should not crash — silently ignored
	span.set_attribute("key", "value")

func test_add_event_after_end_is_safe() -> void:
	var span := _tracer.start_span("event.after_end")
	span.end()
	# Should not crash — silently ignored
	span.add_event("late.event", {})

func test_is_valid_false_for_ended_span() -> void:
	var span := _tracer.start_span("validity.test")
	assert_bool(span.is_valid()).is_true()
	span.end()
	assert_bool(span.is_valid()).is_false()
