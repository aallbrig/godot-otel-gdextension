---
title: Logs API
weight: 3
---

Structured logs are time-stamped records with a severity, a body string, and arbitrary attributes. Unlike print statements, OTel logs carry trace context and are exportable to any backend.

## OtelLoggerProvider

```gdscript
OtelLoggerProvider.get_instance() -> OtelLoggerProvider
```

| Method | Return | Description |
|--------|--------|-------------|
| `get_instance()` | `OtelLoggerProvider` | Static. Returns the singleton. |
| `get_logger(name)` | `OtelLogger` | Returns a Logger for the given instrumentation scope name. |

## OtelLogger

```gdscript
var logger := OtelLoggerProvider.get_instance().get_logger("game.combat")
```

### Convenience methods

| Method | Severity | Description |
|--------|----------|-------------|
| `trace(body, attributes)` | TRACE (1) | Very detailed internal events |
| `debug(body, attributes)` | DEBUG (5) | Diagnostic information |
| `info(body, attributes)` | INFO (9) | Normal operational events |
| `warn(body, attributes)` | WARN (13) | Something unexpected but recoverable |
| `error(body, attributes)` | ERROR (17) | An operation failed |
| `fatal(body, attributes)` | FATAL (21) | Unrecoverable — system should stop |

### emit()

For explicit severity control:

```gdscript
logger.emit(OtelLogger.SEV_INFO, "session.started", {
    "player.id": "p-42",
    "session.id": session_id,
})
```

| Method | Return | Description |
|--------|--------|-------------|
| `emit(severity, body, attributes)` | `void` | Emit with explicit severity int. |

### OtelLogger.Severity enum

| Constant | Value |
|----------|-------|
| `SEV_TRACE` | 1 |
| `SEV_DEBUG` | 5 |
| `SEV_INFO` | 9 |
| `SEV_WARN` | 13 |
| `SEV_ERROR` | 17 |
| `SEV_FATAL` | 21 |

## Wide events pattern

Per *Learning OpenTelemetry* (Young & Parker): prefer one rich log record over many narrow ones. Capture the full user action context in a single emit:

```gdscript
logger.info("combat.outcome", {
    "player.id":          str(player_id),
    "enemy.type":         enemy.type,
    "enemy.level":        enemy.level,
    "combat.result":      "victory",
    "combat.duration_ms": combat_timer.elapsed_ms(),
    "player.hp_before":   player_hp_before,
    "player.hp_after":    player.current_hp,
    "loot.gold":          loot_gold,
    "loot.item_count":    loot_items.size(),
})
```

This gives you a single queryable record with everything you need to understand why the player won — no need to join across multiple events.
