---
title: Getting Started
weight: 1
---

## Prerequisites

- Godot 4.1 or later
- The compiled extension binary for your platform (see [Building from Source](#building-from-source))

## 1. Install the addon

Copy the `addons/otel/` directory from the release into your Godot project:

```
your-project/
  addons/
    otel/
      otel.gdextension
      bin/
        libotel.linux.template_debug.x86_64.so
```

Godot discovers the extension automatically — no activation step needed.

## 2. Initialize at startup

Create an `autoload` or call `OtelInit` from your main scene's `_ready()`:

```gdscript
extends Node

func _ready():
    # Stdout exporter — great for local dev; see Exporters for production options
    OtelInit.get_singleton().configure_stdout()

func _exit_tree():
    OtelInit.get_singleton().shutdown()
```

> Always call `shutdown()` before your game exits to flush any pending telemetry.

## 3. Emit your first trace

```gdscript
var tracer := OtelTracerProvider.get_instance().get_tracer("my-game", "1.0.0")

func load_level(level_name: String) -> void:
    var span := tracer.start_span("level.load")
    span.set_attribute("level.name", level_name)
    span.set_attribute("player.id", str(Global.player_id))

    _do_load(level_name)  # your existing code

    span.set_status(OtelSpan.STATUS_OK, "")
    span.end()
```

## 4. Add metrics

```gdscript
var meter := OtelMeterProvider.get_instance().get_meter("my-game", "1.0.0")
var deaths := meter.create_int_counter("player.deaths", "How many times the player died", "{deaths}")
var frame_ms := meter.create_double_histogram("frame.time", "Frame render time", "ms")

func _on_player_died(cause: String) -> void:
    deaths.add(1, {"cause": cause, "level": current_level})

func _process(delta: float) -> void:
    frame_ms.record(delta * 1000.0, {"quality": current_quality_preset})
```

## 5. Add structured logs

```gdscript
var logger := OtelLoggerProvider.get_instance().get_logger("my-game")

func _on_session_start(player_id: String) -> void:
    logger.info("session.started", {
        "player.id": player_id,
        "session.id": session_manager.current_id,
        "client.version": ProjectSettings.get_setting("application/config/version"),
        "platform": OS.get_name(),
    })
```

## Building from Source

```bash
git clone --recursive https://github.com/aallbrig/godot-otel-gdextension
cd godot-otel-gdextension

task setup    # install cmake, clang-format, clang-tidy
task build    # cmake configure + compile
task test     # run GDUnit4 headless tests
```

See [`Taskfile.yml`](https://github.com/aallbrig/godot-otel-gdextension/blob/main/Taskfile.yml) for all available tasks.

## Exporters

| Exporter | GDScript call | Use case |
|----------|--------------|----------|
| Stdout (OStream) | `OtelInit.get_singleton().configure_stdout()` | Local dev |
| OTLP HTTP | `OtelInit.get_singleton().configure_otlp_http("http://localhost:4318")` | Production (coming soon) |

## Next steps

- [Traces API →](/docs/api/traces/)
- [Metrics API →](/docs/api/metrics/)
- [Logs API →](/docs/api/logs/)
