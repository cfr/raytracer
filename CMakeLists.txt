cmake_minimum_required(VERSION 3.28)

project(raytracer
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "An elementary raytracer"
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(glm CONFIG QUIET)

if(NOT glm_FOUND)
    message(STATUS "GLM not found on system. Fetching from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.3
    )
    FetchContent_MakeAvailable(glm)
else()
    message(STATUS "Using system-provided GLM.")
endif()

add_executable(raytracer
    src/raytracer.cpp
)

target_include_directories(raytracer PRIVATE src)
target_link_libraries(raytracer PRIVATE glm::glm)
target_compile_features(raytracer PRIVATE cxx_std_23)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(ARCH_NATIVE -march=native)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    set(ARCH_NATIVE -mcpu=native)
endif()

target_compile_options(raytracer PRIVATE
    -Wall -Wextra
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:${ARCH_NATIVE} -funroll-loops>
)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported)
if(ipo_supported)
    set_property(TARGET raytracer PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

