extends GdUnitTestSuite

## Unit tests for OtelLoggerProvider and OtelLogger.

func before():
	OtelInit.get_singleton().configure_stdout()

func after():
	OtelInit.get_singleton().shutdown()

# ── LoggerProvider ────────────────────────────────────────────────────────────

func test_logger_provider_singleton_is_not_null() -> void:
	var provider := OtelLoggerProvider.get_instance()
	assert_object(provider).is_not_null()

func test_get_logger_returns_non_null() -> void:
	var logger := OtelLoggerProvider.get_instance().get_logger("game.logger")
	assert_object(logger).is_not_null()

# ── Logger convenience methods ────────────────────────────────────────────────

func test_logger_trace() -> void:
	var logger := OtelLoggerProvider.get_instance().get_logger("test")
	logger.trace("Entering combat loop", {})

func test_logger_debug() -> void:
	var logger := OtelLoggerProvider.get_instance().get_logger("test")
	logger.debug("Pathfinding node count", {"node.count": 128})

func test_logger_info() -> void:
	var logger := OtelLoggerProvider.get_instance().get_logger("test")
	logger.info("Player connected", {"player.id": "p-001", "session.id": "s-xyz"})

func test_logger_warn() -> void:
	var logger := OtelLoggerProvider.get_instance().get_logger("test")
	logger.warn("High frame time detected", {"frame.ms": 45.2, "target.ms": 16.67})

func test_logger_error() -> void:
	var logger := OtelLoggerProvider.get_instance().get_logger("test")
	logger.error("Failed to load asset", {"asset.path": "res://missing.png", "error.code": 2})

func test_logger_fatal() -> void:
	var logger := OtelLoggerProvider.get_instance().get_logger("test")
	logger.fatal("Unrecoverable state: save data corrupted", {"save.path": "/tmp/save.dat"})

# ── Logger emit with explicit severity ───────────────────────────────────────

func test_logger_emit_info() -> void:
	var logger := OtelLoggerProvider.get_instance().get_logger("test")
	logger.emit(OtelLogger.SEV_INFO, "Explicit info emit", {"key": "value"})

func test_logger_emit_warn() -> void:
	var logger := OtelLoggerProvider.get_instance().get_logger("test")
	logger.emit(OtelLogger.SEV_WARN, "Explicit warn emit", {})

# ── Wide-event pattern (per O'Reilly Learning OTel) ──────────────────────────

func test_wide_event_player_session() -> void:
	var logger := OtelLoggerProvider.get_instance().get_logger("game.session")
	logger.info("player.session.started", {
		"player.id": "p-42",
		"player.level": 7,
		"player.class": "warrior",
		"session.id": "sess-abc123",
		"session.region": "us-west-2",
		"client.version": "1.2.3",
		"client.platform": "linux",
		"server.instance": "game-server-07",
	})

func test_wide_event_combat_outcome() -> void:
	var logger := OtelLoggerProvider.get_instance().get_logger("game.combat")
	logger.info("combat.outcome", {
		"player.id": "p-42",
		"enemy.type": "dragon",
		"enemy.level": 15,
		"combat.result": "victory",
		"combat.duration_ms": 12450,
		"player.hp_before": 100,
		"player.hp_after": 23,
		"loot.gold": 500,
		"loot.items": 3,
	})

func test_logger_with_empty_attributes() -> void:
	var logger := OtelLoggerProvider.get_instance().get_logger("test")
	logger.info("Simple message with no attributes", {})

func test_multiple_loggers_for_different_scopes() -> void:
	var physics_log := OtelLoggerProvider.get_instance().get_logger("game.physics")
	var ai_log := OtelLoggerProvider.get_instance().get_logger("game.ai")
	var net_log := OtelLoggerProvider.get_instance().get_logger("game.network")

	physics_log.debug("Physics tick", {"delta": 0.016})
	ai_log.debug("AI decision made", {"agent.id": "npc-007", "decision": "patrol"})
	net_log.info("Packet received", {"size_bytes": 512, "latency_ms": 3})
