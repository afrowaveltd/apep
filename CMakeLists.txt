cmake_minimum_required(VERSION 3.10)
project(apep VERSION 0.1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)  # For gnu11

# Library sources
set(APEP_SOURCES
    src/apep_caps.c
    src/apep_color.c
    src/apep_text.c
    src/apep_hex.c
    src/apep_util.c
    src/apep_helpers.c
)

# Create static library
add_library(apep STATIC ${APEP_SOURCES})

# Include directories
target_include_directories(apep PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(apep PRIVATE _CRT_DECLARE_NONSTDC_NAMES=1)
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(apep PRIVATE /W4)
else()
    target_compile_options(apep PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optional: Build examples
option(APEP_BUILD_EXAMPLES "Build example programs" ON)

if(APEP_BUILD_EXAMPLES)
    # Example programs
    set(EXAMPLES
        text_error_demo
        hex_error_demo
        log_demo
        show_demo
        helpers_demo
    )

    foreach(example ${EXAMPLES})
        add_executable(apep_${example} examples/${example}.c)
        target_link_libraries(apep_${example} PRIVATE apep)
    endforeach()
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS apep
    EXPORT apepTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/apep
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# Export targets
install(EXPORT apepTargets
    FILE apepTargets.cmake
    NAMESPACE apep::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/apep
)

# Package config
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/apepConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/apepConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/apep
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/apepConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/apepConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/apepConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/apep
)

# Print summary
message(STATUS "APEP version: ${PROJECT_VERSION}")
message(STATUS "Build examples: ${APEP_BUILD_EXAMPLES}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
