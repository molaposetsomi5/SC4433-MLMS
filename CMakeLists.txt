
cmake_minimum_required(VERSION 3.20)

project(CS4433_MLMS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(sqlite3 STATIC
    third_party/sqlite/sqlite3.c
)

target_include_directories(sqlite3 PUBLIC
    third_party/sqlite
)

add_library(argon2 STATIC
    third_party/argon2/src/argon2.c
    third_party/argon2/src/core.c
    third_party/argon2/src/blake2/blake2b.c
    third_party/argon2/src/thread.c
    third_party/argon2/src/encoding.c
    third_party/argon2/src/ref.c
)

target_include_directories(argon2 PUBLIC
    third_party/argon2/include
    third_party/argon2/src
)

configure_file(
    sql/schema.sql
    schema.sql
    COPYONLY
)

add_executable(MLMS
    src/main.cpp
    src/domain/User.cpp
    src/domain/RoleUsers.cpp
    src/security/PasswordHasher.cpp
    src/repositories/UserRepository.cpp
    src/repositories/UserManagementRepository.cpp
    src/repositories/sqlite/SQLiteUserRepository.cpp
    src/services/AuthenticationService.cpp
    src/services/Session.cpp
    src/services/AuthorizationService.cpp
    src/services/UserManagementService.cpp
)

target_include_directories(MLMS PRIVATE
    include
)

target_link_libraries(MLMS PRIVATE
    sqlite3
    argon2
    bcrypt
)

add_executable(MLMS_Authentication_Tests
    tests/AuthenticationTest.cpp
    src/domain/User.cpp
    src/domain/RoleUsers.cpp
    src/security/PasswordHasher.cpp
    src/repositories/UserRepository.cpp
    src/services/AuthenticationService.cpp
)

target_include_directories(MLMS_Authentication_Tests PRIVATE
    include
)

target_link_libraries(MLMS_Authentication_Tests PRIVATE
    argon2
    bcrypt
)

add_executable(MLMS_Session_Tests
    tests/SessionTest.cpp
    src/domain/User.cpp
    src/domain/RoleUsers.cpp
    src/services/Session.cpp
)

target_include_directories(MLMS_Session_Tests PRIVATE
    include
)

add_executable(MLMS_Authorization_Tests
    tests/AuthorizationTest.cpp
    src/domain/User.cpp
    src/domain/RoleUsers.cpp
    src/services/AuthorizationService.cpp
)

target_include_directories(MLMS_Authorization_Tests PRIVATE
    include
)

add_executable(MLMS_UserManagement_Tests
    tests/UserManagementTest.cpp
    src/domain/User.cpp
    src/domain/RoleUsers.cpp
    src/security/PasswordHasher.cpp
    src/repositories/UserManagementRepository.cpp
    src/services/AuthorizationService.cpp
    src/services/UserManagementService.cpp
)

target_include_directories(MLMS_UserManagement_Tests PRIVATE
    include
)

target_link_libraries(MLMS_UserManagement_Tests PRIVATE
    argon2
    bcrypt
)

add_executable(MLMS_SQLite_UserManagement_Tests
    tests/SQLiteUserManagementTest.cpp
    src/domain/User.cpp
    src/domain/RoleUsers.cpp
    src/security/PasswordHasher.cpp
    src/repositories/UserRepository.cpp
    src/repositories/UserManagementRepository.cpp
    src/repositories/sqlite/SQLiteUserRepository.cpp
)

target_include_directories(MLMS_SQLite_UserManagement_Tests PRIVATE
    include
)

target_link_libraries(MLMS_SQLite_UserManagement_Tests PRIVATE
    sqlite3
    argon2
    bcrypt
)
