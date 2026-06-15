# CLAUDE.md — godot-otel-gdextension

Inherits global defaults from `~/CLAUDE.md`. Project-specific overrides and context below.

---

## Project Context

A Godot 4 GDExtension wrapping the OpenTelemetry C++ SDK. Exposes OTel traces, metrics, and logs to GDScript.

- **Language**: C++17 (extension), GDScript 2.0 (tests + demo)
- **Build system**: CMake (primary), Taskfile for developer tasks
- **Godot version**: 4.6.x (binary at `/home/aallbright/bin/godot`)
- **Test framework**: GDUnit4 (`demo/addons/gdunit4/`)
- **Reference**: O'Reilly "Learning OpenTelemetry" — Ted Young & Austin Parker

---

## File Layout

```
src/             C++ GDExtension source (headers + implementations)
addons/otel/     Distributable addon (gdextension file + compiled bins)
demo/            Godot project used for testing
  addons/otel/   Extension loaded from here; bin/ has compiled .so
  test/unit/     GDUnit4 unit tests (test_otel_*.gd)
  test/integration/  End-to-end OTel signal flow tests
site/            Hugo documentation + marketing site
thirdparty/      git submodules: godot-cpp, opentelemetry-cpp
```

---

## C++ Conventions

- All GDExtension wrapper classes live in the `godot` namespace
- All class headers: `#pragma once`, minimal includes (forward-declare where possible)
- All OTel internals held as `std::shared_ptr` or `std::unique_ptr`
- Map Godot `String` → `std::string` via `.utf8().get_data()`
- Map Godot `Dictionary` → attribute arrays using a helper `dict_to_attrs()`
- Never expose raw OTel pointers to GDScript; always wrap in RefCounted subclasses
- Use `GDCLASS(ClassName, ParentClass)` macro for every wrapper class
- Bind all public GDScript-visible methods in `_bind_methods()`

### OTel C++ Header Locations (after cmake configure)

- API: `thirdparty/opentelemetry-cpp/api/include/`
- SDK: `thirdparty/opentelemetry-cpp/sdk/include/`
- Exporters: `thirdparty/opentelemetry-cpp/exporters/*/include/`
- godot-cpp: `thirdparty/godot-cpp/include/`

---

## GDScript Test Conventions

- All test files: `extends GdUnitTestSuite`, filename `test_otel_<feature>.gd`
- `before()` calls `OtelInit.configure_stdout()` to avoid null provider
- Always call `span.end()` to avoid leaks; test that is_recording() returns false after
- Assert trace_id is 32 hex chars, span_id is 16 hex chars
- Use GDUnit4 assert API: `assert_object()`, `assert_str()`, `assert_bool()`, `assert_int()`

---

## Common Task Commands

```bash
task setup        # install cmake, clang-format, clang-tidy, GDUnit4
task build        # cmake configure + compile → demo/addons/otel/bin/
task test         # run GDUnit4 headless
task test:unit    # unit tests only
task lint         # clang-format --dry-run + clang-tidy
task format       # clang-format -i (in-place)
task ci           # lint + build + test + site:build
task dev          # start tmuxinator session
task site:dev     # Hugo dev server at localhost:1320
```

---

## Do Not

- Do not expose raw `opentelemetry::*` types in GDScript-visible method signatures
- Do not use `GDREGISTER_ABSTRACT_CLASS` for anything that GDScript needs to instantiate
- Do not hard-code paths to the compiled library — use CMake output variables
- Do not add `_bind_methods()` for methods not intended to be called from GDScript
