cmake_minimum_required(VERSION 3.10)

project(game)

# Add you sources to this variable definition
set(GAME_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.c
)

# Compiler flags per compiler, first is clang and gcc, next is MS VC
set(COMPILER_FLAGS
   $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
      -Wall -Wextra -O2
   >
   $<$<CXX_COMPILER_ID:MSVC>:
      /W4 /Ot
   >)

add_executable(${PROJECT_NAME} "")

find_package(raylib CONFIG REQUIRED)

target_sources(${PROJECT_NAME} PRIVATE ${GAME_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${raylib_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${raylib_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILER_FLAGS})