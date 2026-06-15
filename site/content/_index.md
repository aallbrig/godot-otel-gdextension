---
title: godot-otel
layout: hextra-home
---

{{< hextra/hero-badge >}}
<div class="hx-w-2 hx-h-2 hx-rounded-full hx-bg-primary-400"></div>
<span>Godot 4 · OpenTelemetry C++ SDK</span>
{{< /hextra/hero-badge >}}

<div class="hx-mt-6 hx-mb-6">
{{< hextra/hero-headline >}}
  Production observability&nbsp;for&nbsp;Godot&nbsp;4
{{< /hextra/hero-headline >}}
</div>

<div class="hx-mb-12">
{{< hextra/hero-subtitle >}}
  Traces, metrics, and structured logs — directly in GDScript.
  No sidecar. No print statements. Ship games you can debug in production.
{{< /hextra/hero-subtitle >}}
</div>

<div class="hx-mb-6">
{{< hextra/hero-button text="Get Started →" link="/docs/getting-started/" data-cta="hero-docs" >}}
{{< hextra/hero-button text="View on GitHub" link="https://github.com/aallbrig/godot-otel-gdextension" style="secondary" >}}
</div>

```gdscript
func _ready():
    OtelInit.get_singleton().configure_stdout()

    var tracer = OtelTracerProvider.get_instance().get_tracer("my-game", "1.0.0")
    var span   = tracer.start_span("level.load")
    span.set_attribute("level.name", "dungeon_01")
    span.set_attribute("player.id",  str(player_id))
    # ... load your level ...
    span.end()
```

---

## Why godot-otel?

{{< cards >}}

{{< card title="Real distributed traces" icon="git-branch"
    subtitle="Correlate game events across clients, servers, and services with W3C-standard trace context. See exactly where 200ms went." >}}

{{< card title="OTLP-native" icon="arrow-right"
    subtitle="Export to Honeycomb, Grafana Tempo, Jaeger, or any OpenTelemetry-compatible backend. No vendor lock-in." >}}

{{< card title="Zero GDScript overhead" icon="zap"
    subtitle="The heavy lifting lives in the C++ extension. GDScript calls are thin wrappers — instrumentation doesn't cost frames." >}}

{{< card title="Wide events" icon="layers"
    subtitle="Capture the full context of a player action in a single span. One rich event beats ten narrow ones for debugging." >}}

{{< card title="All three signals" icon="bar-chart"
    subtitle="Traces for request flows, metrics for counters and histograms, structured logs for record-keeping — from a single SDK." >}}

{{< card title="Open source (MIT)" icon="code"
    subtitle="Fork it, extend it, send a PR. The binding layer is designed to grow with the OpenTelemetry C++ SDK." >}}

{{< /cards >}}

---

## Quick install

```bash
# Copy the addon into your Godot project
cp -r addons/otel /path/to/your-project/addons/
```

Godot auto-detects `addons/otel/otel.gdextension`. No plugin activation needed.

**[Full getting-started guide →](/docs/getting-started/)**  {data-cta="body-docs"}

---

## Signals at a glance

| Signal | What to track | GDScript entry point |
|--------|--------------|----------------------|
| Traces | Level loads, combat loops, RPC flows | `OtelTracerProvider.get_instance()` |
| Metrics | Deaths, frame time, server latency | `OtelMeterProvider.get_instance()` |
| Logs | Session events, errors, state transitions | `OtelLoggerProvider.get_instance()` |
