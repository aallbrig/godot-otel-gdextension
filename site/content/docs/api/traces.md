---
title: Traces API
weight: 1
---

Traces represent the lifecycle of an operation. A trace is a tree of **spans** — each span is one unit of work with a name, timestamps, attributes, and status.

## OtelTracerProvider

Singleton accessor for the global `TracerProvider`.

```gdscript
OtelTracerProvider.get_instance() -> OtelTracerProvider
```

### Methods

| Method | Return | Description |
|--------|--------|-------------|
| `get_instance()` | `OtelTracerProvider` | Static. Returns the singleton. |
| `get_tracer(name, version)` | `OtelTracer` | Returns a tracer for the named instrumentation scope. |

**Instrumentation scope** — `name` should identify the library or subsystem (e.g. `"game.physics"`), `version` is its version string.

## OtelTracer

```gdscript
var tracer := OtelTracerProvider.get_instance().get_tracer("game.physics", "1.0.0")
```

### Methods

| Method | Return | Description |
|--------|--------|-------------|
| `start_span(name)` | `OtelSpan` | Creates and starts a new root span. |
| `start_span_with_parent(name, parent)` | `OtelSpan` | Creates a child span of `parent`. Inherits `trace_id`. |

## OtelSpan

```gdscript
var span := tracer.start_span("physics.step")
span.set_attribute("body.count", 128)
span.end()
```

### Methods

| Method | Return | Description |
|--------|--------|-------------|
| `set_attribute(key, value)` | `void` | Set a span attribute. `value` may be `String`, `int`, `float`, or `bool`. |
| `set_status(status_code, description)` | `void` | Set terminal status. |
| `add_event(name, attributes)` | `void` | Add a timestamped event with attributes `Dictionary`. |
| `end()` | `void` | End the span. Required — spans not ended are never exported. |
| `get_trace_id()` | `String` | 32-character lowercase hex trace ID. |
| `get_span_id()` | `String` | 16-character lowercase hex span ID. |
| `is_recording()` | `bool` | `true` until `end()` is called. |
| `is_valid()` | `bool` | `false` if the span is ended or was never initialised. |

### OtelSpan.Status enum

| Constant | Value | Meaning |
|----------|-------|---------|
| `STATUS_UNSET` | 0 | Default — no explicit status |
| `STATUS_OK` | 1 | Operation succeeded |
| `STATUS_ERROR` | 2 | Operation failed |

### Example: error span

```gdscript
var span := tracer.start_span("asset.load")
span.set_attribute("asset.path", "res://player.tres")

var result := ResourceLoader.load("res://player.tres")
if result == null:
    span.set_status(OtelSpan.STATUS_ERROR, "ResourceLoader returned null")
    span.add_event("error", {"asset.path": "res://player.tres"})
else:
    span.set_status(OtelSpan.STATUS_OK, "")

span.end()
```
