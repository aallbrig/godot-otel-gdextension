# Proposal: Seamless Godot OpenTelemetry Integration

## Executive Summary

This proposal outlines how to provide a development experience for Godot + OpenTelemetry that rivals built-in language SDKs. Current friction points: installing the extension, setting up initialization, discovering available classes. This document proposes four mechanisms to eliminate these pain points.

---

## Problem Statement

### Current State

1. **Installation friction**: Copy `addons/otel/` folder to project
2. **Initialization friction**: Manually create autoload script, call `configure_stdout()`
3. **Discovery friction**: No built-in help, examples, or IDE completion
4. **Build friction**: Binary must be pre-compiled for target platform
5. **Verification friction**: No built-in health check or status endpoint

### Target State

1. **One-click installation** via Asset Library
2. **Auto-initialization** — extension self-configures on first load
3. **IDE awareness** — GDScript editor knows all OTel classes and their methods
4. **Cross-platform builds** — CI generates binaries for all platforms; optional local cross-compile
5. **Built-in diagnostics** — `OtelInit.get_status()` returns health report

---

## Proposal 1: Asset Library Packaging

### Goal

Make the extension installable in one click from Godot Asset Library.

### Implementation

1. **Create `asset_registry.txt` manifest** at repo root:

   ```
   title=OpenTelemetry for Godot
   category=Logging
   description=OpenTelemetry C++ SDK wrapper for Godot 4.3+. Instruments traces, metrics, and logs with OTEL standards.
   author=Andrew Allbright
   version=0.1.0
   ```

2. **Register on Asset Library** (`https://godotengine.org/asset-library/`)
   - Category: Logging / Monitoring
   - License: Apache 2.0 (match OTel SDK license)
   - Prebuilt binaries for: Linux x86_64, macOS (intel + arm), Windows x86_64

3. **Distribution Package** (in CI, per-platform):
   ```
   addons/
     otel/
       otel.gdextension
       bin/
         libotel.linux.template_debug.x86_64.so
         libotel.linux.template_release.x86_64.so
         ... (all platforms)
       docs/
         QuickStart.md
         API.md
   ```

### Benefits

- Users find the extension through Godot's native discovery
- One-click install + download all binaries
- No manual folder copying
- Built-in versioning and update mechanism

### Owner

- Implement manifest in repo
- GitHub Actions job: package all platforms, upload to releases
- Manual: submit to Asset Library review

---

## Proposal 2: Auto-Initialization (OtelInit as Built-in Autoload)

### Goal

Eliminate the manual autoload setup step.

### Implementation

**Option A: GDExtension-level autoload registration**

The C++ extension (`otel_init.cpp`) registers itself as a Godot autoload during `gdextension_init()`:

```cpp
void gdextension_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                      const GDExtensionClassLibraryPtr p_library,
                      GDExtension::InitializationLevel p_level) {
  // ... existing code ...

  if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
    // Auto-register OtelInit as an autoload
    auto* sce = SceneTree::get_singleton();
    if (sce && !sce->is_node_in_group("OtelAutoload")) {
      Ref<OtelInit> otel = memnew(OtelInit);
      sce->add_child(otel);
      otel->add_to_group("OtelAutoload");
      otel->configure_stdout(); // Auto-initialize with stdout exporter
    }
  }
}
```

**Option B: Manifest-based auto-initialization**

Add to `otel.gdextension`:

```ini
[auto_init]
enabled=true
initialize_with=stdout
```

Parse this in C++ and self-initialize based on manifest config.

### Benefits

- Zero manual setup; extension works on first load
- `OtelInit.get_instance()` always available in any script
- Backwards-compatible: manual initialization still works if desired

### Trade-offs

- Less explicit (may surprise users)
- **Recommendation**: Make it opt-in via config flag in `.gdextension`

---

## Proposal 3: IDE Awareness & Code Completion

### Goal

Make GDScript editor aware of OTel classes and provide inline documentation.

### Implementation

**1. Generate `otel.gdscript` with type hints and docstrings**

Auto-generate from C++ bindings:

```gdscript
# auto-generated from C++ _bind_methods()
class_name OtelTracer
extends RefCounted

## Get or create a span for this operation
func start_span(name: String, attributes: Dictionary = {}) -> OtelSpan:
	"""
	Start a new span under this tracer.

	@param name: Operation name (e.g., "http.request", "db.query")
	@param attributes: Initial span attributes (optional)
	@return: Active span
	"""
	pass

## Record a span event
func add_event(name: String, attributes: Dictionary = {}) -> void:
	"""
	Record an event during span execution.

	@param name: Event name
	@param attributes: Event context
	"""
	pass
```

**2. Distribute with extension**

Include in `addons/otel/` alongside `.gdextension` and binaries.

**3. Godot editor integration**

- Editor auto-imports `otel.gdscript` into the project
- GDScript LSP server reads file and provides completions
- Hover on `OtelTracer.start_span()` shows docstring
- Go-to-definition jumps to generated `.gdscript` file

**4. CMake target to auto-generate**

Add to `CMakeLists.txt`:

```cmake
add_custom_target(generate_otel_gdscript
  COMMAND python3 scripts/gen_gdscript_bindings.py
          --input src/
          --output addons/otel/otel.gdscript
  DEPENDS ${OTEL_CPP_SOURCES}
)
add_dependencies(otel_gdextension generate_otel_gdscript)
```

### Benefits

- Full IDE awareness (completions, hover, type checking)
- Offline documentation (no external API reference needed)
- Reduces learning curve for new users

### Trade-offs

- Requires Python script to generate bindings
- Must stay in sync with C++ changes (script should be part of CI)

---

## Proposal 4: Build-On-Demand for Missing Platforms

### Goal

Enable developers targeting unsupported platforms to compile locally without full toolchain knowledge.

### Implementation

**1. Docker build container**

```dockerfile
# Dockerfile.build
FROM godotengine/godot:4.3-dev
RUN apt-get install -y cmake clang-17 libssl-dev libcurl4-openssl-dev
COPY . /src/godot-otel-gdextension
WORKDIR /src/godot-otel-gdextension
RUN task build:release && task build:debug
```

**2. Build script (`build.sh`)**

```bash
#!/bin/bash
# ./build.sh [platform] [template]
# Builds locally or via Docker if unavailable

PLATFORM=${1:-linux}
TEMPLATE=${2:-debug}

if command -v cmake &>/dev/null; then
  # Native build
  task build:${TEMPLATE}
else
  # Fall back to Docker
  docker build -f Dockerfile.build -t godot-otel-builder .
  docker run --rm -v $(pwd):/src godot-otel-builder task build:${TEMPLATE}
fi
```

**3. CI: Build matrix for all platforms**

Expand GitHub Actions to build:

- Linux: x86_64, ARM64 (both debug + release)
- macOS: x86_64, ARM64 (both)
- Windows: x86_64 (both)

Each platform as separate job, upload all binaries to releases page.

**4. Download helper (`scripts/install_binaries.sh`)**

```bash
#!/bin/bash
# Download pre-built binaries for current platform
GODOT_VERSION=$(godot --version | cut -d' ' -f2)
PLATFORM=$(uname -s | tr '[:upper:]' '[:lower:]')
ARCH=$(uname -m)

mkdir -p addons/otel/bin
wget -q \
  https://github.com/aallbright/godot-otel-gdextension/releases/download/v0.1.0/libotel.${PLATFORM}.${ARCH}.so \
  -O addons/otel/bin/libotel.${PLATFORM}.${ARCH}.so

chmod +x addons/otel/bin/*.so
echo "✓ Binaries installed for $PLATFORM/$ARCH"
```

### Benefits

- Developers can build locally with simple commands
- CI automatically provides all platform binaries
- Reduces dependency on pre-built artifacts

---

## Proposal 5: Health Check & Diagnostics

### Goal

Built-in status endpoint so users can verify extension is working.

### Implementation

Add to `OtelInit`:

```gdscript
## Get a status report of the OTel extension
func get_status() -> Dictionary:
	"""
	Returns:
	{
		"initialized": bool,
		"godot_version": string,
		"extension_version": string,
		"exporters": [list of configured exporters],
		"spans_recorded": int,
		"metrics_recorded": int,
		"logs_recorded": int,
		"last_export": float (unix timestamp)
	}
	"""
	pass

## Get extension version
func get_extension_version() -> String:
	pass

## Get Godot version
func get_godot_version() -> String:
	pass
```

Example output:

```gdscript
var status = OtelInit.get_instance().get_status()
print(status)
# {
#   "initialized": true,
#   "godot_version": "4.3-stable",
#   "extension_version": "0.1.0",
#   "exporters": ["stdout", "otlp"],
#   "spans_recorded": 42,
#   "metrics_recorded": 128,
#   "logs_recorded": 3421,
#   "last_export": 1718520000.0
# }
```

Use case: Health check script that runs on game startup

```gdscript
func _ready():
	var status = OtelInit.get_instance().get_status()
	if not status.initialized:
		printerr("OTel extension failed to initialize!")
		get_tree().quit(1)
	else:
		print("✓ OTel healthy: ", status.godot_version, " + ", status.extension_version)
```

### Benefits

- Users can verify extension is loaded and working
- Operators can monitor extension health in production
- Reduces debugging time

### Implementation Status

- `get_godot_version()` — Already implemented ✓
- `get_extension_version()` — Already implemented ✓
- `get_status()` — Needs implementation

---

## Recommended Rollout (MVP → Full)

### Phase 1 (MVP) — _Current_

- [x] Core C++ implementation (10 classes)
- [x] GDUnit4 test suite
- [x] GitHub Actions CI
- [ ] Pre-commit quality gates in CI (started)
- [ ] `get_status()` method

### Phase 2 (Polish)

- [ ] Asset Library packaging + registration
- [ ] Auto-initialization via C++ (opt-in)
- [ ] `build.sh` script for local compilation
- [ ] Cross-platform CI builds (all platforms + architectures)

### Phase 3 (DX)

- [ ] Generated `otel.gdscript` type stubs
- [ ] Diagnostics test scene (in demo project)
- [ ] QuickStart guide + API docs
- [ ] Example integrations (HTTP client, game metrics, error tracking)

### Phase 4 (Community)

- [ ] Asset Library listing
- [ ] YouTube tutorial
- [ ] Community Discord / GitHub Discussions
- [ ] Third-party exporter plugins (Grafana, Datadog, etc.)

---

## Success Metrics

1. **Installation time**: < 2 minutes (Asset Library download + copy)
2. **First trace**: < 5 minutes (auto-init + tutorial)
3. **IDE support**: Full code completion + hover docs
4. **Cross-platform**: Binaries available for ≥ 3 platforms
5. **Community**: ≥ 100 GitHub stars, ≥ 10 community PRs

---

## Open Questions

1. **Auto-initialization safety**: Should it be opt-in or default? Trade-off: convenience vs. explicitness
2. **Exporter pluggability**: Support third-party exporters (e.g., Grafana Loki) as separate GDExtensions?
3. **Performance monitoring**: Should extension self-report metrics (memory, CPU)? Or leave to user code?
4. **macOS signing**: How to handle notarization for Apple Silicon Macs?

---

## Conclusion

This proposal outlines a path to make Godot + OpenTelemetry as frictionless as native language SDKs. By implementing these five mechanisms, we eliminate the main barriers to adoption:

- **Installation** → Asset Library (1-click)
- **Setup** → Auto-initialization (0 manual steps)
- **Learning** → IDE awareness + generated docs (completions in editor)
- **Building** → Build scripts + cross-platform CI (no toolchain knowledge needed)
- **Verification** → Health check endpoint (debugging built-in)

**Next steps**: Prioritize phases 1–2 for v0.2.0 release.
