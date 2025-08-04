cmake_minimum_required(VERSION 3.13)

project(NetworkServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_compile_options(-Wall -Wextra)

file(GLOB_RECURSE ServerFeatureSource "src/*.cpp")

add_library(ServerFeature STATIC
    ${ServerFeatureSource}
)

#if(WIN32)
    target_link_libraries(ServerFeature PRIVATE ws2_32)
    target_link_libraries(ServerFeature PRIVATE wsock32)
    target_link_libraries(ServerFeature PRIVATE Psapi)
    target_link_libraries(ServerFeature PRIVATE Shlwapi)
    target_link_libraries(ServerFeature PRIVATE Mf)
    target_link_libraries(ServerFeature PRIVATE Mfplat)
    target_link_libraries(ServerFeature PRIVATE Mfreadwrite)
    target_link_libraries(ServerFeature PRIVATE Mfuuid)
#endif()
target_include_directories(ServerFeature PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(NetworkServer main.cpp)

target_include_directories(NetworkServer PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

#if(WIN32)
    target_link_libraries(NetworkServer PRIVATE ws2_32)
    target_link_libraries(NetworkServer PRIVATE wsock32)
    target_link_libraries(NetworkServer PRIVATE Psapi)
#endif()


target_link_libraries(NetworkServer PUBLIC
    ServerFeature
)