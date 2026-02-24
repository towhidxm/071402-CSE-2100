cmake_minimum_required(VERSION 3.20)
project(RegistryViewer VERSION 1.0.0 LANGUAGES C)

# ── C standard ────────────────────────────────────────────────────
set(CMAKE_C_STANDARD   11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ── Find GTK 4 via pkg-config ─────────────────────────────────────
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

# ── Source files ──────────────────────────────────────────────────
set(SOURCES
    src/main.c
    src/registry_facade.c
    model/registry_model.c
    ui/registry_view.c
    controller/registry_controller.c
)

# ── Executable ────────────────────────────────────────────────────
add_executable(RegistryViewer ${SOURCES})

target_include_directories(RegistryViewer PRIVATE include)

target_link_libraries(RegistryViewer PRIVATE
    PkgConfig::GTK4
    advapi32   # Windows Registry API
)

# ── Warnings ──────────────────────────────────────────────────────
if (MSVC)
    target_compile_options(RegistryViewer PRIVATE /W4 /WX)
else()
    target_compile_options(RegistryViewer PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
