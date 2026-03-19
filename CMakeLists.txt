cmake_minimum_required(VERSION 3.28)

project(CSE167xRaytracer
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "UC San DiegoX CSE167x Computer Graphics Homework 3: Raytracer"
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

include_directories(src)

add_executable(raytracer
    src/parser.hpp
    src/raytracer.cpp
)


target_link_libraries(raytracer PRIVATE glm::glm)
target_compile_features(raytracer PRIVATE cxx_std_23)

target_compile_options(raytracer PRIVATE
    -Wall -Wextra
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:-O3>
)

