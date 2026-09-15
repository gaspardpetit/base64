cmake_minimum_required(VERSION 3.25)
project(base64-cli VERSION 0.1.0 LANGUAGES C)

set(BASE64_CLI_VERSION "${PROJECT_VERSION}" CACHE STRING
    "Version string reported by the base64 command")
set(BASE64_CLI_BUFFER_SIZE "49152" CACHE STRING
    "Streaming input buffer size in bytes (must be divisible by 12)")
option(BASE64_DISABLE_HARDWARE
    "Disable architecture-specific Base64 implementations" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(BASE64_LIBRARY_DIR "${CMAKE_CURRENT_LIST_DIR}/..")

add_library(base64-codec STATIC "${BASE64_LIBRARY_DIR}/base64.c")
target_include_directories(base64-codec PUBLIC "${BASE64_LIBRARY_DIR}")

add_executable(base64 main.c)
target_link_libraries(base64 PRIVATE base64-codec)
target_compile_definitions(base64 PRIVATE
  BASE64_CLI_VERSION="${BASE64_CLI_VERSION}"
  BASE64_CLI_BUFFER_SIZE=${BASE64_CLI_BUFFER_SIZE}
)

if(BASE64_DISABLE_HARDWARE)
  target_compile_definitions(base64-codec PRIVATE BASE64_DISABLE_HARDWARE)
elseif(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
  target_sources(base64-codec PRIVATE "${BASE64_LIBRARY_DIR}/base64_avx2.c")
  set_source_files_properties("${BASE64_LIBRARY_DIR}/base64_avx2.c"
    PROPERTIES COMPILE_OPTIONS "/arch:AVX2")
elseif(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
  target_sources(base64-codec PRIVATE "${BASE64_LIBRARY_DIR}/base64_neon.c")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND
       CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$" AND
       CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_sources(base64-codec PRIVATE "${BASE64_LIBRARY_DIR}/base64_avx2.c")
  set_source_files_properties("${BASE64_LIBRARY_DIR}/base64_avx2.c"
    PROPERTIES COMPILE_OPTIONS "-mavx2")
else()
  target_compile_definitions(base64-codec PRIVATE BASE64_DISABLE_HARDWARE)
endif()

if(MSVC)
  target_compile_options(base64 PRIVATE /W4)
  target_compile_options(base64-codec PRIVATE /W4)
else()
  target_compile_options(base64 PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(base64-codec PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
  find_package(Python3 3.8 COMPONENTS Interpreter REQUIRED)
  add_test(NAME cli-tests
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_LIST_DIR}/tests/test_cli.py"
            "$<TARGET_FILE:base64>")
  add_executable(base64-api-tests tests/test_api.c)
  target_link_libraries(base64-api-tests PRIVATE base64-codec)
  add_test(NAME api-tests COMMAND base64-api-tests)

  add_executable(base64-header-tests tests/test_api.c)
  target_compile_definitions(base64-header-tests PRIVATE BASE64_HEADER_ONLY)
  target_include_directories(base64-header-tests PRIVATE "${BASE64_LIBRARY_DIR}")
  add_test(NAME header-tests COMMAND base64-header-tests)
endif()

install(TARGETS base64 RUNTIME DESTINATION bin)
