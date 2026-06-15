extends GdUnitTestSuite

## Unit tests for OtelTracerProvider and OtelTracer.
## Requires the extension to be built and present at addons/otel/bin/.

func before():
	OtelInit.get_singleton().configure_stdout()

func after():
	OtelInit.get_singleton().shutdown()

func test_tracer_provider_singleton_is_not_null() -> void:
	var provider := OtelTracerProvider.get_instance()
	assert_object(provider).is_not_null()

func test_get_tracer_returns_non_null() -> void:
	var tracer := OtelTracerProvider.get_instance().get_tracer("test-scope", "1.0.0")
	assert_object(tracer).is_not_null()

func test_get_tracer_with_empty_version() -> void:
	var tracer := OtelTracerProvider.get_instance().get_tracer("test-scope", "")
	assert_object(tracer).is_not_null()

func test_start_span_returns_valid_span() -> void:
	var tracer := OtelTracerProvider.get_instance().get_tracer("test", "1.0.0")
	var span := tracer.start_span("my.operation")
	assert_object(span).is_not_null()
	assert_bool(span.is_valid()).is_true()
	span.end()

func test_span_is_recording_before_end() -> void:
	var tracer := OtelTracerProvider.get_instance().get_tracer("test", "1.0.0")
	var span := tracer.start_span("recording.test")
	assert_bool(span.is_recording()).is_true()
	span.end()

func test_span_not_recording_after_end() -> void:
	var tracer := OtelTracerProvider.get_instance().get_tracer("test", "1.0.0")
	var span := tracer.start_span("end.test")
	span.end()
	assert_bool(span.is_recording()).is_false()

func test_span_has_32_char_trace_id() -> void:
	var tracer := OtelTracerProvider.get_instance().get_tracer("test", "1.0.0")
	var span := tracer.start_span("traceid.test")
	var tid := span.get_trace_id()
	assert_str(tid).is_not_empty()
	assert_int(tid.length()).is_equal(32)
	span.end()

func test_span_has_16_char_span_id() -> void:
	var tracer := OtelTracerProvider.get_instance().get_tracer("test", "1.0.0")
	var span := tracer.start_span("spanid.test")
	var sid := span.get_span_id()
	assert_str(sid).is_not_empty()
	assert_int(sid.length()).is_equal(16)
	span.end()

func test_child_span_inherits_trace_id() -> void:
	var tracer := OtelTracerProvider.get_instance().get_tracer("test", "1.0.0")
	var parent := tracer.start_span("parent")
	var child := tracer.start_span_with_parent("child", parent)
	assert_str(child.get_trace_id()).is_equal(parent.get_trace_id())
	child.end()
	parent.end()

func test_child_span_has_different_span_id_than_parent() -> void:
	var tracer := OtelTracerProvider.get_instance().get_tracer("test", "1.0.0")
	var parent := tracer.start_span("parent")
	var child := tracer.start_span_with_parent("child", parent)
	assert_str(child.get_span_id()).is_not_equal(parent.get_span_id())
	child.end()
	parent.end()

func test_multiple_tracers_from_same_scope_share_trace_state() -> void:
	var provider := OtelTracerProvider.get_instance()
	var t1 := provider.get_tracer("game.physics", "1.0.0")
	var t2 := provider.get_tracer("game.ai", "1.0.0")
	var s1 := t1.start_span("physics.tick")
	var s2 := t2.start_span_with_parent("ai.decision", s1)
	assert_str(s2.get_trace_id()).is_equal(s1.get_trace_id())
	s2.end()
	s1.end()
