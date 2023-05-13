cmake_minimum_required(VERSION 3.22)
project(sp64-tt LANGUAGES CXX)

add_subdirectory(external)
add_executable(sp64-tt  src/main.cpp
                        src/App.cpp
                        src/Unpacker.cpp)
target_link_libraries(sp64-tt PRIVATE spdlog SFML::Graphics ImGui-SFML::ImGui-SFML)
target_compile_features(sp64-tt PRIVATE cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang)")
    target_compile_options(sp64-tt PRIVATE -Werror -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(sp64-tt PRIVATE /W4 /WX /permissive-)
endif()

if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET sp64-tt POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:sp64-tt> $<TARGET_FILE_DIR:sp64-tt> COMMAND_EXPAND_LISTS)
endif()

install(TARGETS sp64-tt)
