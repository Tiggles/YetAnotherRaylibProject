cmake_minimum_required(VERSION 3.10)

project(game)

# Add you sources to this variable definition
set(GAME_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.c
)

add_executable(${PROJECT_NAME} "")

find_package(raylib CONFIG REQUIRED)

target_sources(${PROJECT_NAME} PUBLIC ${GAME_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${raylib_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${raylib_LIBRARIES})
