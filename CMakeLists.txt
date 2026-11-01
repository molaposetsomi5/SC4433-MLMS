
cmake_minimum_required(VERSION 3.20)

project(CS4433_MLMS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build SQLite as a static library
add_library(sqlite3 STATIC
    third_party/sqlite/sqlite3.c
)

target_include_directories(sqlite3 PUBLIC
    third_party/sqlite
)

# Build Argon2 as a static library
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

# Copy database schema into build directory
configure_file(
    sql/schema.sql
    schema.sql
    COPYONLY
)

# Main MLMS application
add_executable(MLMS
    src/main.cpp

    # Domain classes
    src/domain/User.cpp
    src/domain/RoleUsers.cpp

    # Security classes
    src/security/PasswordHasher.cpp

    # Repository classes
    src/repositories/UserRepository.cpp
    src/repositories/sqlite/SQLiteUserRepository.cpp
)

target_include_directories(MLMS PRIVATE
    include
)

# Link required libraries
target_link_libraries(MLMS PRIVATE
    sqlite3
    argon2
    bcrypt
)
