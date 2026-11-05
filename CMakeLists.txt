cmake_minimum_required(VERSION 3.21)
project(paintAZ VERSION 1.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 6.12 REQUIRED COMPONENTS Core Gui Widgets)

set(PAINTAZ_SHARED_SOURCES
    mainwindow.cpp widget.cpp newfiledialog.cpp memento.cpp fillspace.cpp
    mainwindow.h widget.h newfiledialog.h memento.h fillspace.h
    mainwindow.ui widget.ui newfiledialog.ui resources.qrc)
qt_add_executable(paintAZ WIN32 MACOSX_BUNDLE main.cpp ${PAINTAZ_SHARED_SOURCES})
target_include_directories(paintAZ PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(paintAZ PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
set_target_properties(paintAZ PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.github.adam-zielonka.paintaz")
if(EMSCRIPTEN)
    # Import JPEG explicitly; PNG/BMP are built into QtGui. TIFF is not bundled.
    qt_import_plugins(paintAZ INCLUDE Qt6::QJpegPlugin)
else()
    include(CTest)
    if(BUILD_TESTING)
        find_package(Qt6 6.12 REQUIRED COMPONENTS Test)
        add_executable(paintAZ-tests tests/desktop/tst_workflows.cpp ${PAINTAZ_SHARED_SOURCES})
        target_include_directories(paintAZ-tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
        target_link_libraries(paintAZ-tests PRIVATE Qt6::Widgets Qt6::Test)
        add_test(NAME desktop-workflows COMMAND paintAZ-tests)
        set_tests_properties(desktop-workflows PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 120)
    endif()
endif()
