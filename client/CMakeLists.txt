cmake_minimum_required(VERSION 3.13)

project(NetworkClient)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard

file(GLOB_RECURSE ClientFeatureSource "src/*.cpp")

add_library(ClientFeature STATIC
        ${ClientFeatureSource}
)

target_include_directories(ClientFeature PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

#if(WIN32)
    target_link_libraries(ClientFeature PRIVATE ws2_32)
    target_link_libraries(ClientFeature PRIVATE wsock32)
#endif()

add_executable(NetworkClient main.cpp)

#if(WIN32)
target_link_libraries(NetworkClient PRIVATE ws2_32)
target_link_libraries(NetworkClient PRIVATE wsock32)
#endif()

target_include_directories(NetworkClient PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(NetworkClient PUBLIC
        ClientFeature
)