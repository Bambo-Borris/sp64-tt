cmake_minimum_required(VERSION 3.21)
project(sp-64-unpacker LANGUAGES CXX)

add_subdirectory(external)
add_executable(sp-64-unpacker src/main.cpp)
target_link_libraries(sp-64-unpacker PRIVATE spdlog SFML::Graphics)
target_compile_features(sp-64-unpacker PRIVATE cxx_std_17)
target_compile_options(sp-64-unpacker PRIVATE /W4 /WX /permissive-)
if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET sp-64-unpacker POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:sp-64-unpacker> $<TARGET_FILE_DIR:sp-64-unpacker> COMMAND_EXPAND_LISTS)
endif()

install(TARGETS sp-64-unpacker)
