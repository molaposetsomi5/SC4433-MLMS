cmake_minimum_required(VERSION 3.16)
project(MLMS VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(MLMS_BUILD_TESTS "Build the GoogleTest suite" ON)
option(MLMS_COVERAGE "Build with gcov coverage flags" OFF)

# ---------------------------------------------------------------- dependencies
find_package(SQLite3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)

# ---------------------------------------------------------------- warnings / coverage
if(MSVC)
  set(MLMS_WARNINGS /W4)
else()
  set(MLMS_WARNINGS -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wno-sign-conversion)
endif()

if(MLMS_COVERAGE AND NOT MSVC)
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)
endif()

# ---------------------------------------------------------------- core library
# Everything except main() goes into a library so the tests can link it.
add_library(mlms_core
  src/domain/DateTime.cpp
  src/domain/Types.cpp
  src/domain/User.cpp
  src/domain/LatePenaltyPolicy.cpp
  src/data/Database.cpp
  src/data/Schema.cpp
  src/data/SqliteUserRepository.cpp
  src/data/SqliteCourseRepository.cpp
  src/data/SqliteAssessmentRepositories.cpp
  src/data/SqliteAuditRepository.cpp
  src/security/PasswordHasher.cpp
  src/services/PermissionPolicy.cpp
  src/services/AuditService.cpp
  src/services/AuthService.cpp
  src/services/UserManagementService.cpp
  src/services/CourseManagementService.cpp
  src/services/DeadlineResolver.cpp
  src/services/AssignmentService.cpp
  src/services/ExtensionService.cpp
  src/services/SubmissionService.cpp
  src/services/MarkingService.cpp
  src/services/ModerationService.cpp
  src/services/ReleaseService.cpp
  src/app/AppContext.cpp
  src/app/DemoData.cpp
  src/ui/Console.cpp
  src/ui/ConsoleUI.cpp
)
target_include_directories(mlms_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(mlms_core PUBLIC SQLite::SQLite3 PkgConfig::SODIUM)
target_compile_options(mlms_core PRIVATE ${MLMS_WARNINGS})

# ---------------------------------------------------------------- executables
add_executable(mlms src/main.cpp)
target_link_libraries(mlms PRIVATE mlms_core)
target_compile_options(mlms PRIVATE ${MLMS_WARNINGS})

add_executable(mlms_perf tools/perf_test.cpp)
target_link_libraries(mlms_perf PRIVATE mlms_core)

install(TARGETS mlms RUNTIME DESTINATION bin)

# ---------------------------------------------------------------- tests
if(MLMS_BUILD_TESTS)
  enable_testing()
  find_package(GTest QUIET)
  if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    add_library(GTest::gtest_main ALIAS gtest_main)
  endif()

  add_executable(mlms_tests
    tests/TestApp.cpp
    tests/test_datetime.cpp
    tests/test_late_penalty.cpp
    tests/test_users_and_auth.cpp
    tests/test_admin.cpp
    tests/test_submission.cpp
    tests/test_marking.cpp
    tests/test_moderation_release.cpp
    tests/test_audit_and_integrity.cpp
    tests/test_system_console.cpp
    tests/test_class_extension.cpp
  )
  target_include_directories(mlms_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
  target_link_libraries(mlms_tests PRIVATE mlms_core GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(mlms_tests)
endif()
