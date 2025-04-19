cmake_minimum_required(VERSION 3.25)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

project(${PROJECT_NAME})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(FetchContent)

FetchContent_Declare(
    ktsignal
    GIT_REPOSITORY https://github.com/kin4stat/ktsignal.git
    GIT_TAG f08a19d9a00ad5284871bf44178d835da7b26ebf
)

FetchContent_Declare(
    kthook
    GIT_REPOSITORY https://github.com/kin4stat/kthook.git
    GIT_TAG 21ca70481273d1efcc98ca49063178e46277009b
)

FetchContent_Declare(
    rakhook
    GIT_REPOSITORY https://github.com/imring/RakHook.git
    GIT_TAG 625fd17897395997feb56ae293c7c07dbe49d26b
)

FetchContent_Declare(
    sampapi
    GIT_REPOSITORY https://github.com/BlastHackNet/SAMP-API.git
    GIT_TAG 6d4db99ab41f19d1a6a7c6cd48f5878bd1e14b62
)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG 913151caa8905370229e361486b10c890aeaed45
)

FetchContent_MakeAvailable(ktsignal kthook rakhook sampapi imgui)

include_directories(${imgui_SOURCE_DIR})
include_directories(${imgui_SOURCE_DIR}/backends)

aux_source_directory(${imgui_SOURCE_DIR} ${PROJECT_NAME}_LIST)
list(APPEND ${PROJECT_NAME}_LIST ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp)
list(APPEND ${PROJECT_NAME}_LIST ${imgui_SOURCE_DIR}/backends/imgui_impl_dx9.cpp)

file(GLOB_RECURSE PROJECT_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.h)
file(GLOB_RECURSE PROJECT_SOURCE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.hpp)
file(GLOB_RECURSE PROJECT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_library(${PROJECT_NAME} SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCE_HEADERS} ${PROJECT_SOURCES})

target_sources(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_LIST})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
    SUFFIX ".asi"
)

if (NOT "${DXSDK_DIR}" STREQUAL "")
    target_include_directories(${PROJECT_NAME} PUBLIC ${DXSDK_DIR}/Include)
    target_link_directories(${PROJECT_NAME} PUBLIC ${DXSDK_DIR}/Lib/x86)
elseif (NOT "$ENV{DXSDK_DIR}" STREQUAL "")
    target_include_directories(${PROJECT_NAME} PUBLIC $ENV{DXSDK_DIR}/Include)
    target_link_directories(${PROJECT_NAME} PUBLIC $ENV{DXSDK_DIR}/Lib/x86)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source)

target_link_libraries(${PROJECT_NAME} PUBLIC d3d9 d3dx9 kthook rakhook sampapi)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)