cmake_minimum_required(VERSION 3.25)
project(AsiProject)

set(USE_KTHOOK ON)
set(USE_RAKHOOK ON)
set(USE_SAMPAPI ON)
set(USE_PLUGINSDK OFF)
set(USE_IMGUI OFF)
set(USE_DXSDK OFF)
set(USE_CURL OFF)
set(USE_JSON OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

file(GLOB_RECURSE PROJECT_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
file(GLOB_RECURSE PROJECT_SOURCE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
file(GLOB_RECURSE PROJECT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCE_HEADERS} ${PROJECT_SOURCES})

include(FetchContent)

if (USE_KTHOOK)
    FetchContent_Declare(xbyak GIT_REPOSITORY https://github.com/herumi/xbyak.git GIT_TAG ce083a0dcc306c1717685a81f577a4e050193919 SOURCE_DIR ${CMAKE_BINARY_DIR}/xbyak)
    FetchContent_Declare(ktsignal GIT_REPOSITORY https://github.com/kin4stat/ktsignal.git GIT_TAG f08a19d9a00ad5284871bf44178d835da7b26ebf SOURCE_DIR ${CMAKE_BINARY_DIR}/ktsignal)
    FetchContent_Declare(kthook GIT_REPOSITORY https://github.com/kin4stat/kthook.git GIT_TAG 21ca70481273d1efcc98ca49063178e46277009b SOURCE_DIR ${CMAKE_BINARY_DIR}/kthook)
 
    FetchContent_MakeAvailable(xbyak ktsignal kthook)
 
    target_link_libraries(${PROJECT_NAME} PUBLIC kthook)
endif ()

if (USE_RAKHOOK)
    FetchContent_Declare(rakhook GIT_REPOSITORY https://github.com/imring/RakHook.git GIT_TAG 625fd17897395997feb56ae293c7c07dbe49d26b SOURCE_DIR ${CMAKE_BINARY_DIR}/rakhook)

    FetchContent_MakeAvailable(rakhook)

    target_link_libraries(${PROJECT_NAME} PUBLIC rakhook)
endif ()

if (USE_SAMPAPI)
    FetchContent_Declare(SAMPAPI GIT_REPOSITORY https://github.com/BlastHackNet/SAMP-API.git GIT_TAG 6d4db99ab41f19d1a6a7c6cd48f5878bd1e14b62 SOURCE_DIR ${CMAKE_BINARY_DIR}/sampapi)

    FetchContent_MakeAvailable(sampapi)

    target_link_libraries(${PROJECT_NAME} PUBLIC sampapi)
endif ()

if (USE_PLUGINSDK)
    set(PSDK_EXAMPLES OFF)
    set(PSDK_INSTALL  OFF)
    set(PSDK_GTASA    ON)

    FetchContent_Declare(plugin-sdk GIT_REPOSITORY https://github.com/imring/plugin-sdk.git GIT_TAG 81b18b744b64db3058f48344c144591c9030205b SOURCE_DIR ${CMAKE_BINARY_DIR}/pluginsdk)

    FetchContent_MakeAvailable(plugin-sdk)

    target_link_libraries(${PROJECT_NAME} PUBLIC pluginsdk::gtasa)
endif ()

if (USE_IMGUI)
    if (NOT USE_DXSDK)
	message(FATAL_ERROR "Can't use imgui without USE_DXSDK!")
    endif ()

    FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG 913151caa8905370229e361486b10c890aeaed45 SOURCE_DIR ${CMAKE_BINARY_DIR}/imgui)

    FetchContent_MakeAvailable(imgui)

    include_directories(${CMAKE_BINARY_DIR}/imgui)
    include_directories(${CMAKE_BINARY_DIR}/imgui/backends)

    aux_source_directory(${CMAKE_BINARY_DIR}/imgui ${PROJECT_NAME}_LIST)
    list(APPEND ${PROJECT_NAME}_LIST "${CMAKE_BINARY_DIR}/imgui/backends/imgui_impl_win32.cpp")
    list(APPEND ${PROJECT_NAME}_LIST "${CMAKE_BINARY_DIR}/imgui/backends/imgui_impl_dx9.cpp")

    target_sources(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_LIST})
endif ()

if (USE_DXSDK)
    if(NOT "${DXSDK_DIR}" STREQUAL "")
        target_include_directories( ${PROJECT_NAME} PUBLIC ${DXSDK_DIR}/Include)
        target_link_directories(${PROJECT_NAME} PUBLIC ${DXSDK_DIR}/Lib/x86)
    elseif(NOT "$ENV{DXSDK_DIR}" STREQUAL "")
        target_include_directories( ${PROJECT_NAME} PUBLIC $ENV{DXSDK_DIR}/Include)
        target_link_directories( ${PROJECT_NAME} PUBLIC $ENV{DXSDK_DIR}/Lib/x86)
    endif()

    target_link_libraries(${PROJECT_NAME} PUBLIC d3d9 d3dx9)
endif ()

if (USE_CURL)
    include_directories(${CMAKE_SOURCE_DIR}/3rdparty/curl/include)
    target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/3rdparty/curl/lib)

    target_link_libraries(${PROJECT_NAME} PRIVATE libcurl_a.lib Ws2_32.lib Crypt32.lib Wldap32.lib Normaliz.lib)

    add_definitions(-DCURL_STATICLIB)
endif ()

if (USE_JSON)
    include_directories(${CMAKE_SOURCE_DIR}/3rdparty/json)
endif ()

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME} SUFFIX ".asi")
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)