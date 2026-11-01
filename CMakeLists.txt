cmake_minimum_required(VERSION 3.20)

project(CS4433_MLMS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build SQLite as a static library
add_library(sqlite3 STATIC
    third_party/sqlite/sqlite3.c
)

# Make SQLite header available to the application
target_include_directories(sqlite3 PUBLIC
    third_party/sqlite
)

# Copy the database schema into the build directory
configure_file(
    sql/schema.sql
    schema.sql
    COPYONLY
)

# Main MLMS application
add_executable(MLMS
    src/main.cpp
)

# Link SQLite to the application
target_link_libraries(MLMS PRIVATE sqlite3)