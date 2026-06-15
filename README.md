#godot - otel - gdextension

A [GDExtension](https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/) that exposes the [OpenTelemetry C++ SDK](https://github.com/open-telemetry/opentelemetry-cpp) to GDScript, giving Godot 4 games and tools production-grade observability: distributed traces, metrics, and structured logs exportable to any OTLP-compatible backend (Jaeger, Honeycomb, Grafana, etc.).

```gdscript
func _ready():
    OtelInit.configure_stdout()

    var tracer = OtelTracerProvider.get_instance().get_tracer("my-game", "1.0.0")
    var span   = tracer.start_span("level.load")
    span.set_attribute("level.name", "dungeon_01")
    span.set_attribute("player.id", str(player_id))
#... load level...
    span.end()
```

---

## Why

Godot has no built-in observability. As games grow — multiplayer, live services, procedural content, crash telemetry — you need real tracing, not print statements. This extension wires the industry-standard OpenTelemetry SDK directly into GDScript without a network hop or sidecar.

---

## Quick Start

### 1. Add to your project

```bash
#Copy the addon into your Godot project
cp -r addons/otel /path/to/your-godot-project/addons/
```

Godot will auto-detect `addons/otel/otel.gdextension` and load the extension.

### 2. Initialize in your main scene

```gdscript
extends Node

func _ready():
#Stdout exporter — swap for OTLP in production
    OtelInit.configure_stdout()

func _exit_tree():
    OtelInit.shutdown()
```

### 3. Trace, measure, log

```gdscript
#-- - Traces -- -
var tracer = OtelTracerProvider.get_instance().get_tracer("game", "1.0.0")
var span = tracer.start_span("player.action")
span.set_attribute("action", "attack")
span.set_attribute("damage", 42)
span.end()

#-- - Metrics -- -
var meter = OtelMeterProvider.get_instance().get_meter("game", "1.0.0")
var counter = meter.create_int_counter("player.deaths", "Player death count", "{deaths}")
counter.add(1, {"cause": "spike_trap", "level": "floor_3"})

#-- - Logs -- -
var logger = OtelLoggerProvider.get_instance().get_logger("game")
logger.info("Player joined session", {"player.id": str(id), "session.id": session_id})
```

---

## Building from Source

### Prerequisites

```bash
sudo apt-get install -y cmake clang clang-format clang-tidy ninja-build
#vcpkg for dependency management
git clone https://github.com/microsoft/vcpkg thirdparty/vcpkg
./thirdparty/vcpkg/bootstrap-vcpkg.sh
```

### Build

```bash
task setup      # install remaining dev tools
task build      # cmake + compile → demo/addons/otel/bin/
task test       # run GDUnit4 tests headless
```

---

## C++ SDK Coverage

> Status: **Planned** = API designed, not yet compiled; **Partial** = works but incomplete; **Complete** = full parity with C++ SDK

### Traces (`opentelemetry::trace`)

| C++ SDK Type | GDScript Class | Status | Notes |
|---|---|---|---|
| `TracerProvider` | `OtelTracerProvider` | Planned | Singleton access |
| `Tracer` | `OtelTracer` | Planned | `get_tracer(name, version)` |
| `Span` | `OtelSpan` | Planned | set_attribute, add_event, set_status, end |
| `SpanContext` | `OtelSpanContext` | Planned | trace_id, span_id, is_valid |
| `SpanKind` | `OtelSpan.Kind.*` | Planned | INTERNAL, SERVER, CLIENT, PRODUCER, CONSUMER |
| `StatusCode` | `OtelSpan.Status.*` | Planned | UNSET, OK, ERROR |
| `Scope` (context propagation) | — | — | Not exposed; managed by SDK internally |
| `TraceState` | — | — | Not planned |

### Metrics (`opentelemetry::metrics`)

| C++ SDK Type | GDScript Class | Status | Notes |
|---|---|---|---|
| `MeterProvider` | `OtelMeterProvider` | Planned | Singleton access |
| `Meter` | `OtelMeter` | Planned | `get_meter(name, version)` |
| `Counter<uint64_t>` | `OtelCounter` | Planned | `add(value, attributes)` |
| `Histogram<double>` | `OtelHistogram` | Planned | `record(value, attributes)` |
| `Gauge<double>` | `OtelGauge` | — | Not yet planned |
| `UpDownCounter<int64_t>` | `OtelUpDownCounter` | — | Not yet planned |
| `ObservableCounter` | — | — | Callback-based; GDScript binding complex |
| `ObservableGauge` | — | — | Not planned |

### Logs (`opentelemetry::logs`)

| C++ SDK Type | GDScript Class | Status | Notes |
|---|---|---|---|
| `LoggerProvider` | `OtelLoggerProvider` | Planned | Singleton access |
| `Logger` | `OtelLogger` | Planned | debug/info/warn/error + emit |
| `LogRecord` | — | Planned | Created internally; not directly exposed |
| `Severity` | `OtelLogger.Severity.*` | Planned | TRACE through FATAL |

### Context & Propagation

| C++ SDK Type | GDScript Class | Status | Notes |
|---|---|---|---|
| `Baggage` | `OtelBaggage` | — | Not yet planned |
| `TextMapPropagator` | — | — | Not planned (server-side concern) |
| `W3CTraceContextPropagator` | — | — | Not planned |

### Exporters & SDK Config (`OtelInit`)

| Capability | Status | Notes |
|---|---|---|
| `configure_stdout()` | Planned | OStream exporter for all signals |
| `configure_otlp_http(url)` | — | Not yet implemented |
| `configure_otlp_grpc(endpoint)` | — | Not yet implemented |
| `shutdown()` | Planned | Flush and shutdown all providers |

---

## Project Structure

```
addons/otel/          ← copy this into your Godot project
  otel.gdextension
  bin/                ← compiled shared libraries

src/                  ← C++ GDExtension source
  register_types.*    ← GDExtension entry point
  otel_init.*         ← SDK initialization singleton
  otel_tracer*.*      ← TracerProvider + Tracer wrappers
  otel_span.*         ← Span wrapper
  otel_meter*.*       ← MeterProvider + Meter wrappers
  otel_counter.*      ← Counter instrument
  otel_histogram.*    ← Histogram instrument
  otel_logger*.*      ← LoggerProvider + Logger wrappers

demo/                 ← Godot test project
  addons/otel/        ← extension loaded here
  test/unit/          ← GDUnit4 unit tests
  test/integration/   ← end-to-end OTel signal tests

site/                 ← Hugo documentation + marketing site
  content/
    _index.md         ← landing page
    docs/             ← API docs

thirdparty/
  godot-cpp/          ← git submodule
  opentelemetry-cpp/  ← git submodule
```

---

## License

MIT
