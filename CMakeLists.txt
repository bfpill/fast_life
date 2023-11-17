cmake_minimum_required(VERSION 3.0)
project(shadered_game)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(SOURCE_FILES game.cpp game.hpp gl_utils.hpp gl_utils.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} "-framework OpenGL -framework GLUT")

