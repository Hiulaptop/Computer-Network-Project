cmake_minimum_required(VERSION 3.13)

project(NetworkClient)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard
set(IMGUI_DEFINE_MATH_OPERATORS)

include(FetchContent)

FetchContent_Declare(imgui_source
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.92.1
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM
)

FetchContent_MakeAvailable(imgui_source)

add_library(imgui
        ${imgui_source_SOURCE_DIR}/imgui.cpp
        ${imgui_source_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_source_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_source_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_source_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_source_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_source_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)


# ========== Find OpenGL ==========
find_package(OpenGL REQUIRED)
# ========== Find GLFW ==========
FetchContent_Declare(GLFW
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM
)

FetchContent_MakeAvailable(GLFW)

# LINK STB
FetchContent_Declare(stb
    GIT_REPOSITORY https://github.com/nothings/stb
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)

FetchContent_MakeAvailable(stb)

target_link_libraries(imgui PUBLIC glfw)

target_include_directories(imgui PUBLIC
        ${imgui_source_SOURCE_DIR}
)

# ========== Find CURL ==========
include(FetchContent)
FetchContent_Declare(download_curl
        URL https://curl.se/windows/dl-8.15.0_4/curl-8.15.0_4-win64-mingw.zip
        DOWNLOAD_NO_EXTRACT true
)

FetchContent_MakeAvailable(download_curl)

file(ARCHIVE_EXTRACT
        INPUT ${download_curl_SOURCE_DIR}/curl-8.15.0_4-win64-mingw.zip
        DESTINATION ${CMAKE_SOURCE_DIR}/client/external/
)
#
set(CURL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/client/external/curl-8.15.0_4-win64-mingw/include)
set(CURL_LIBRARY ${CMAKE_SOURCE_DIR}/client/external/curl-8.15.0_4-win64-mingw/bin/libcurl-x64.dll)
set(CURL_USE_STATIC_LIBS ON)
set(CURL_NO_CURL_CMAKE ON)
add_definitions(-DCURL_STATICLIB)

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

set_target_properties(CURL::libcurl PROPERTIES
        IMPORTED_LOCATION ${CURL_LIBRARY})

file(GLOB_RECURSE ClientFeatureSource "src/*.cpp")

add_library(ClientFeature STATIC
        ${ClientFeatureSource}
)

target_include_directories(ClientFeature PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${stb_SOURCE_DIR}
)

#if(WIN32)
    target_link_libraries(ClientFeature PRIVATE ws2_32)
    target_link_libraries(ClientFeature PRIVATE wsock32)
    target_link_libraries(ClientFeature PRIVATE Psapi)
    target_link_libraries(ClientFeature PRIVATE Shlwapi)
    target_link_libraries(ClientFeature PRIVATE Mf)
    target_link_libraries(ClientFeature PRIVATE Mfplat)
    target_link_libraries(ClientFeature PRIVATE Mfreadwrite)
    target_link_libraries(ClientFeature PRIVATE Mfuuid)
#endif()
target_link_libraries(ClientFeature PUBLIC
        imgui
        glfw
        OpenGL::GL
        CURL::libcurl
)

add_executable(NetworkClient main.cpp)

#if(WIN32)
target_link_libraries(NetworkClient PRIVATE ws2_32)
target_link_libraries(NetworkClient PRIVATE wsock32)
target_link_libraries(NetworkClient PRIVATE Psapi)
target_link_libraries(NetworkClient PRIVATE Shlwapi)
target_link_libraries(NetworkClient PRIVATE Mf)
target_link_libraries(NetworkClient PRIVATE Mfplat)
target_link_libraries(NetworkClient PRIVATE Mfreadwrite)
target_link_libraries(NetworkClient PRIVATE Mfuuid)
#
#endif()

target_include_directories(NetworkClient PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${stb_SOURCE_DIR}
)

target_link_libraries(NetworkClient PUBLIC
        ClientFeature
)