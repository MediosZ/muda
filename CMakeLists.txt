# project
cmake_minimum_required(VERSION 3.20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# build targets:
option(MUDA_BUILD_EXAMPLE "build muda examples. if you want to see how to use muda, you could enable this option." ON)
option(MUDA_BUILD_TEST "build muda test. if you're the developer, you could enable this option." OFF)

# short cut
option(MUDA_DEV "build muda example and unit test. if you're the developer, you could enable this option." OFF)
option(MUDA_FORCE_CHECK "turn on muda runtime check for all mode (Debug/RelWithDebInfo/Release)" OFF)
option(MUDA_WITH_CHECK "turn on muda runtime check when mode != Release" ON)
option(MUDA_WITH_COMPUTE_GRAPH "turn on muda compute graph" OFF)
option(MUDA_WITH_NVTX3 "turn on nividia tools extension library" OFF)

if(MUDA_DEV)
  set(MUDA_BUILD_EXAMPLE ON)
  set(MUDA_PLAYGROUND ON)
  set(MUDA_BUILD_TEST ON)
endif()

# to remove warning
if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
  set(CMAKE_CUDA_ARCHITECTURES 75)
endif()

project(muda LANGUAGES CXX CUDA)

find_package(CUDAToolkit REQUIRED)

enable_language(CUDA)

# muda
add_library(muda INTERFACE)
target_compile_options(muda INTERFACE
  $<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>
  $<$<COMPILE_LANGUAGE:CUDA>:--extended-lambda>
  $<$<COMPILE_LANGUAGE:CUDA>:-rdc=true>
)
# disable warning
target_compile_options(muda INTERFACE
  $<$<COMPILE_LANGUAGE:CUDA>:-Xcudafe --display_error_number --diag-suppress 20012>
)
file(GLOB_RECURSE MUDA_HEADER_FILES "${PROJECT_SOURCE_DIR}/src/*.h" "${PROJECT_SOURCE_DIR}/src/*.inl" "${PROJECT_SOURCE_DIR}/src/*.cuh")

target_sources(muda PUBLIC ${MUDA_HEADER_FILES})
target_compile_features(muda INTERFACE cxx_std_17)
target_link_libraries(muda INTERFACE CUDA::cudart)
target_include_directories(muda INTERFACE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
target_include_directories(muda INTERFACE "${PROJECT_SOURCE_DIR}/src/")
source_group(TREE "${PROJECT_SOURCE_DIR}/src" PREFIX "Header Files" FILES ${MUDA_HEADER_FILES})



if(MUDA_FORCE_CHECK)
  target_compile_definitions(muda INTERFACE "-DMUDA_CHECK_ON=1")
else()
  if(MUDA_WITH_CHECK)
    target_compile_definitions(muda INTERFACE 
    $<$<CONFIG:Debug>:MUDA_CHECK_ON=1>
    $<$<CONFIG:RelWithDebInfo>:MUDA_CHECK_ON=1>)
  else()
    target_compile_definitions(muda INTERFACE "-DMUDA_CHECK_ON=0")
  endif()
endif()
if (MUDA_WITH_COMPUTE_GRAPH)
  target_compile_definitions(muda INTERFACE "-DMUDA_COMPUTE_GRAPH_ON=1")
else()
  target_compile_definitions(muda INTERFACE "-DMUDA_COMPUTE_GRAPH_ON=0")
endif()
if (MUDA_WITH_NVTX3)
  target_compile_definitions(muda INTERFACE "-DMUDA_NVTX3_ON=1")
else()
  target_compile_definitions(muda INTERFACE "-DMUDA_NVTX3_ON=0")
endif()


if(MUDA_BUILD_EXAMPLE)
  add_subdirectory("example")
endif()

if(MUDA_BUILD_TEST)
  add_subdirectory("test")
endif()