cmake_minimum_required(VERSION 3.16)

project(sqterm VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_definitions(-gdwarf-4)

if(NOT DEFINED CMAKE_PREFIX_PATH)
    message(STATUS "CMAKE_PREFIX_PATH not set, using system Qt")
endif()

find_package(Qt5 REQUIRED COMPONENTS Core Widgets SerialPort)

add_library(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC 
        "include/"
)

# crunch for AUTOMOC
file(GLOB_RECURSE HDR_FILES "include/*.h")

qt5_add_resources(RESOURCES_RC
    "rc/qemuterminal.qrc"
)

target_sources(${PROJECT_NAME}
    PRIVATE 
        ${HDR_FILES}
        "src/cli_parser.cpp"
        "src/cli_widget.cpp"
        "src/settings.cpp"
        "src/terminal.cpp"
    INTERFACE 
        ${RESOURCES_RC}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE 
        Qt5::Core
        Qt5::Widgets
        Qt5::SerialPort
)

if (BUILD_EXAMPLE)
    # example sqterm_app
    add_executable(${PROJECT_NAME}_app)

    target_include_directories(${PROJECT_NAME}_app
        PUBLIC 
            "example/dark_style/"
    )

    target_sources(${PROJECT_NAME}_app
        PRIVATE 
            "example/dark_style/darkstyle.qrc"
            "example/dark_style/DarkStyle.cpp"
            "example/main.cpp"
    )

    target_link_libraries(${PROJECT_NAME}_app
        PRIVATE 
            sqterm
            Qt5::Core
            Qt5::Widgets
            Qt5::SerialPort
    )
endif()
