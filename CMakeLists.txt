cmake_minimum_required(VERSION 3.28)

project(aktis
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Ray/Path Tracer"
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

option(USE_PCG "Use the PCG generator instead of std::mt19937(_64)" OFF)

# NOTE: float might be slower because of grazing ray quantization => more BVH visits
option(USE_FLOAT32 "Use 32-bit float instead of double" OFF)

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

if(USE_PCG)
    message(STATUS "Fetching PCG from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        pcg
        GIT_REPOSITORY https://github.com/imneme/pcg-cpp.git
        GIT_TAG v0.98.1
    )
    FetchContent_MakeAvailable(pcg)

    if(NOT TARGET pcg)
        add_library(pcg INTERFACE)
        target_include_directories(pcg INTERFACE ${pcg_SOURCE_DIR}/include)
        add_library(pcg::pcg ALIAS pcg)
    endif()
endif()

add_executable(aktis
    src/main.cpp
)

target_include_directories(aktis PRIVATE src)
target_link_libraries(aktis PRIVATE glm::glm)
target_compile_features(aktis PRIVATE cxx_std_23)

if(USE_FLOAT32)
    target_compile_definitions(aktis PRIVATE USE_FLOAT32)
    message(STATUS "Precision: float32")
else()
    message(STATUS "Precision: double (default)")
endif()

if(USE_PCG)
    target_link_libraries(aktis PRIVATE pcg::pcg)
    target_compile_definitions(aktis PRIVATE USE_PCG)
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(ARCH_NATIVE -march=native)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    set(ARCH_NATIVE -mcpu=native)
endif()

target_compile_options(aktis PRIVATE
    -Wall -Wextra
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:${ARCH_NATIVE} -funroll-loops -fno-math-errno -fno-trapping-math>
)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported)
if(ipo_supported)
    set_property(TARGET aktis PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
