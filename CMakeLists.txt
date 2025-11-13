cmake_minimum_required(VERSION 3.1)
project(nchex)

find_package(Curses REQUIRED)

set(
    SOURCE_FILES

    src/main.c
    src/fileio.c
    src/ui.c
    src/editor.c
)

add_executable(
    ${PROJECT_NAME}
    ${SOURCE_FILES}
)

set(
    INCLUDE_DIRS
    include/
)

target_include_directories(
    ${PROJECT_NAME} PUBLIC
    ${INCLUDE_DIRS}
    ${CURSES_INCLUDE_DIRS}
)

target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    ${CURSES_LIBRARIES}
)

include_directories(${SOURCE_FILES})

target_compile_features(${PROJECT_NAME} PUBLIC c_std_11)

set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
