cmake_minimum_required(VERSION 3.22)
project(godot-otel-gdextension CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(GODOT_CPP_SYSTEM_HEADERS "Treat godot-cpp headers as system headers (suppresses warnings)" ON)
option(BUILD_DEBUG "Build in debug mode (maps to Godot template_debug)" ON)

# ── godot-cpp ────────────────────────────────────────────────────────────────
# Uses local submodule if present; otherwise FetchContent (CI-friendly).
include(FetchContent)

set(GODOT_CPP_VERSION "godot-4.3-stable" CACHE STRING "godot-cpp git tag")
set(GODOT_CPP_DIR "${CMAKE_SOURCE_DIR}/thirdparty/godot-cpp")

if(EXISTS "${GODOT_CPP_DIR}/CMakeLists.txt")
    message(STATUS "Using local godot-cpp submodule at ${GODOT_CPP_DIR}")
    add_subdirectory("${GODOT_CPP_DIR}" godot-cpp EXCLUDE_FROM_ALL)
else()
    message(STATUS "Fetching godot-cpp @ ${GODOT_CPP_VERSION}")
    FetchContent_Declare(
        godot-cpp
        GIT_REPOSITORY https://github.com/godotengine/godot-cpp.git
        GIT_TAG        "${GODOT_CPP_VERSION}"
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(godot-cpp)
endif()

# ── opentelemetry-cpp ────────────────────────────────────────────────────────
# Uses local submodule if present; otherwise FetchContent (CI-friendly).
set(OTEL_CPP_VERSION "v1.18.0" CACHE STRING "opentelemetry-cpp git tag")
set(OTEL_CPP_DIR "${CMAKE_SOURCE_DIR}/thirdparty/opentelemetry-cpp")

set(BUILD_TESTING           OFF CACHE BOOL "" FORCE)
set(WITH_ABSEIL             OFF CACHE BOOL "" FORCE)
set(WITH_STL                ON  CACHE BOOL "" FORCE)
set(WITH_OTLP_HTTP          OFF CACHE BOOL "" FORCE)
set(WITH_OTLP_GRPC          OFF CACHE BOOL "" FORCE)
set(WITH_PROMETHEUS         OFF CACHE BOOL "" FORCE)
set(WITH_EXAMPLES           OFF CACHE BOOL "" FORCE)
set(WITH_BENCHMARK          OFF CACHE BOOL "" FORCE)
set(WITH_FUNC_TESTS         OFF CACHE BOOL "" FORCE)
set(WITH_ZPAGES             OFF CACHE BOOL "" FORCE)
set(OPENTELEMETRY_INSTALL   OFF CACHE BOOL "" FORCE)

if(EXISTS "${OTEL_CPP_DIR}/CMakeLists.txt")
    message(STATUS "Using local opentelemetry-cpp submodule at ${OTEL_CPP_DIR}")
    add_subdirectory("${OTEL_CPP_DIR}" opentelemetry-cpp EXCLUDE_FROM_ALL)
else()
    message(STATUS "Fetching opentelemetry-cpp @ ${OTEL_CPP_VERSION}")
    FetchContent_Declare(
        opentelemetry-cpp
        GIT_REPOSITORY https://github.com/open-telemetry/opentelemetry-cpp.git
        GIT_TAG        "${OTEL_CPP_VERSION}"
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(opentelemetry-cpp)
endif()

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
