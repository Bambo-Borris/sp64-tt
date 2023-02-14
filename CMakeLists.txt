cmake_minimum_required(VERSION 3.21)
project(sp-64-unpacker LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG master)
FetchContent_MakeAvailable(SFML)

add_executable(sp-64-unpacker src/main.cpp)
target_link_libraries(sp-64-unpacker PRIVATE SFML::Graphics)
target_compile_features(sp-64-unpacker PRIVATE cxx_std_17)
if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET sp-64-unpacker POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:sp-64-unpacker> $<TARGET_FILE_DIR:sp-64-unpacker> COMMAND_EXPAND_LISTS)
endif()

install(TARGETS sp-64-unpacker)
