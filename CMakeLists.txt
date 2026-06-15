cmake_minimum_required(VERSION 3.22)
project(godot-otel-gdextension CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(GODOT_CPP_SYSTEM_HEADERS "Treat godot-cpp headers as system headers (suppresses warnings)" ON)
option(BUILD_DEBUG "Build in debug mode (maps to Godot template_debug)" ON)

# ── godot-cpp ────────────────────────────────────────────────────────────────
# Expected as a git submodule at thirdparty/godot-cpp
set(GODOT_CPP_DIR "${CMAKE_SOURCE_DIR}/thirdparty/godot-cpp")
if(NOT EXISTS "${GODOT_CPP_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "godot-cpp not found at ${GODOT_CPP_DIR}. "
        "Run: git submodule update --init --recursive")
endif()
add_subdirectory("${GODOT_CPP_DIR}" godot-cpp EXCLUDE_FROM_ALL)

# ── opentelemetry-cpp ────────────────────────────────────────────────────────
# Expected as a git submodule at thirdparty/opentelemetry-cpp
set(OTEL_CPP_DIR "${CMAKE_SOURCE_DIR}/thirdparty/opentelemetry-cpp")
if(NOT EXISTS "${OTEL_CPP_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "opentelemetry-cpp not found at ${OTEL_CPP_DIR}. "
        "Run: git submodule update --init --recursive")
endif()

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(WITH_ABSEIL OFF CACHE BOOL "" FORCE)
set(WITH_STL ON CACHE BOOL "" FORCE)
set(WITH_OTLP_HTTP ON CACHE BOOL "" FORCE)
set(WITH_OTLP_GRPC OFF CACHE BOOL "" FORCE)
set(WITH_PROMETHEUS OFF CACHE BOOL "" FORCE)
set(WITH_EXAMPLES OFF CACHE BOOL "" FORCE)
set(WITH_BENCHMARK OFF CACHE BOOL "" FORCE)
set(WITH_FUNC_TESTS OFF CACHE BOOL "" FORCE)
set(OPENTELEMETRY_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory("${OTEL_CPP_DIR}" opentelemetry-cpp EXCLUDE_FROM_ALL)

# ── Extension sources ─────────────────────────────────────────────────────────
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)

add_library(godot_otel SHARED ${SOURCES} ${HEADERS})

target_compile_options(godot_otel PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

target_include_directories(godot_otel PRIVATE src/)

target_link_libraries(godot_otel PRIVATE
    godot::cpp
    opentelemetry_api
    opentelemetry_sdk
    opentelemetry_trace
    opentelemetry_metrics
    opentelemetry_logs
    opentelemetry_exporter_ostream_span
    opentelemetry_exporter_ostream_logs
    opentelemetry_exporter_ostream_metrics
    opentelemetry_exporter_otlp_http
)

# ── Output naming (matches .gdextension file) ─────────────────────────────────
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR BUILD_DEBUG)
    set(GODOT_BUILD_TYPE "template_debug")
else()
    set(GODOT_BUILD_TYPE "template_release")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(GODOT_PLATFORM "linux")
    set(LIB_EXT ".so")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(GODOT_PLATFORM "windows")
    set(LIB_EXT ".dll")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(GODOT_PLATFORM "macos")
    set(LIB_EXT ".dylib")
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(GODOT_ARCH "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64")
    set(GODOT_ARCH "arm64")
endif()

set(OUTPUT_NAME "libotel.${GODOT_PLATFORM}.${GODOT_BUILD_TYPE}.${GODOT_ARCH}")
set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/demo/addons/otel/bin")

set_target_properties(godot_otel PROPERTIES
    OUTPUT_NAME "${OUTPUT_NAME}"
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)

message(STATUS "Output → ${OUTPUT_DIR}/${OUTPUT_NAME}${LIB_EXT}")
