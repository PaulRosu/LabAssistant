cmake_minimum_required(VERSION 3.16)

project(LabAssistant VERSION 2.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Add optimization flags similar to qmake
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# Ensure proper Windows configuration
if(WIN32)
    set(CMAKE_WIN32_EXECUTABLE TRUE)
endif()

# Find Qt packages
find_package(Qt6 REQUIRED COMPONENTS 
    Core
    Gui
    Widgets
    Charts
    Concurrent
    Network
    SvgWidgets
)

# Add version and company definitions like in qmake
set(APP_VERSION "${PROJECT_VERSION}")
set(APP_PRODUCT "LabAssistant INTERNAL USE ONLY")
set(APP_COMPANY "Marquardt")
set(APP_COPYRIGHT "2025")

# SingleApplication sources (directly in root)
set(SINGLE_APPLICATION_SOURCES
    singleapplication.cpp
    singleapplication_p.cpp
)

set(SINGLE_APPLICATION_HEADERS
    singleapplication.h
    singleapplication_p.h
)

# Create SingleApplication library
add_library(SingleApplication STATIC
    ${SINGLE_APPLICATION_SOURCES}
    ${SINGLE_APPLICATION_HEADERS}
)

# Configure SingleApplication
target_compile_definitions(SingleApplication PUBLIC
    QAPPLICATION_CLASS=QApplication
    QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
)

target_link_libraries(SingleApplication PUBLIC
    Qt6::Core
    Qt6::Network
    Qt6::Widgets
)

if(WIN32)
    target_link_libraries(SingleApplication PUBLIC advapi32)
endif()

target_include_directories(SingleApplication PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Global compile definitions
add_compile_definitions(
    APP_VERSION="${APP_VERSION}"
    APP_PRODUCT="${APP_PRODUCT}"
    APP_COMPANY="${APP_COMPANY}"
    APP_COPYRIGHT="${APP_COPYRIGHT}"
    QAPPLICATION_CLASS=QApplication
    QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
)

# Ensure proper UI file handling
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Data_Reader
    ${CMAKE_CURRENT_SOURCE_DIR}/Data_Reader/data
)

# Include SingleApplication
#add_subdirectory(SingleApplication)

# Include Data_Reader
add_subdirectory(Data_Reader)

# Main application sources
set(PROJECT_SOURCES
    ZS2_Decoder.cpp
    callout.cpp
    dataloader.cpp
    haptic_serie.cpp
    main.cpp
    mainwindow.cpp
    messagereceiver.cpp
    qcompressor.cpp
    view_utilities.cpp
    view.cpp
    view_event_handlers.cpp
    view_callouts.cpp
    view_old_special_chart_constructors.cpp
)

set(PROJECT_HEADERS
    ZS2_Decoder.h
    callout.h
    dataloader.h
    haptic_serie.h
    mainwindow.h
    messagereceiver.h
    qcompressor.h
    view.h
    view_utilities.h
)

set(PROJECT_FORMS
    mainwindow.ui
)

# Resources
set(PROJECT_RESOURCES
    resources.qrc
)

# Create main executable
add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    ${PROJECT_FORMS}
    ${PROJECT_RESOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SingleApplication
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Charts
    Qt6::Concurrent
    Qt6::Network
    Qt6::SvgWidgets
    Data_Reader
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        z
        OpenGL32
        advapi32
    )
endif()

# Set Windows-specific properties
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
        LINK_FLAGS "-mwindows"
    )
endif()

# Add debug configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE QT_QML_DEBUG)
endif()
