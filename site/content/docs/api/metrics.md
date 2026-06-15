---
title: Metrics API
weight: 2
---

Metrics are numeric measurements aggregated over time. godot-otel exposes **counters** (monotonically increasing) and **histograms** (value distributions).

## OtelMeterProvider

```gdscript
OtelMeterProvider.get_instance() -> OtelMeterProvider
```

| Method | Return | Description |
|--------|--------|-------------|
| `get_instance()` | `OtelMeterProvider` | Static. Returns the singleton. |
| `get_meter(name, version)` | `OtelMeter` | Returns a Meter for the named instrumentation scope. |

## OtelMeter

```gdscript
var meter := OtelMeterProvider.get_instance().get_meter("game", "1.0.0")
```

| Method | Return | Description |
|--------|--------|-------------|
| `create_int_counter(name, description, unit)` | `OtelCounter` | Creates a monotonic uint64 counter. |
| `create_double_histogram(name, description, unit)` | `OtelHistogram` | Creates a double histogram. |

**Unit** follows the [UCUM](https://ucum.org/) convention: `"{deaths}"`, `"ms"`, `"By"`, `"1"`.

## OtelCounter

A monotonically increasing counter. Use for things that only go up: events, completions, errors.

```gdscript
var deaths := meter.create_int_counter("player.deaths", "Times player died", "{deaths}")
deaths.add(1, {"cause": "spike_trap", "level": "floor_3"})
```

| Method | Return | Description |
|--------|--------|-------------|
| `add(value, attributes)` | `void` | Add `value` (≥ 0) with attribute `Dictionary`. |

## OtelHistogram

Records a distribution of values. Use for latency, sizes, durations.

```gdscript
var frame_ms := meter.create_double_histogram("frame.time", "Frame render time", "ms")

func _process(delta: float) -> void:
    frame_ms.record(delta * 1000.0, {"preset": current_quality})
```

| Method | Return | Description |
|--------|--------|-------------|
| `record(value, attributes)` | `void` | Record one observation with attribute `Dictionary`. |

## Attribute Dictionary

Both instruments accept a `Dictionary` of attributes. Supported value types:

| GDScript type | OTel attribute type |
|---------------|---------------------|
| `String` | `string` |
| `int` | `int64` |
| `float` | `double` |
| `bool` | `bool` |
