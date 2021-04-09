cmake_minimum_required(VERSION 3.10)

project(game)

find_package(raylib REQUIRED)

set(SOURCE_PREFIX ${CMAKE_CURRENT_LIST_DIR})
set(GAME_SOURCES 
    ${SOURCE_PREFIX}/main.c
)

add_executable(${PROJECT_NAME} "")
target_sources(${PROJECT_NAME} PUBLIC ${GAME_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
