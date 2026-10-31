cmake_minimum_required(VERSION 3.21)
project(paintAZ LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 6.10 REQUIRED COMPONENTS Core Gui Widgets)
qt_add_executable(paintAZ
    main.cpp mainwindow.cpp widget.cpp newfiledialog.cpp memento.cpp fillspace.cpp
    mainwindow.h widget.h newfiledialog.h memento.h fillspace.h
    mainwindow.ui widget.ui newfiledialog.ui resources.qrc)
target_include_directories(paintAZ PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(paintAZ PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
if(EMSCRIPTEN)
    # Import JPEG explicitly; PNG/BMP are built into QtGui. TIFF is not bundled.
    qt_import_plugins(paintAZ INCLUDE Qt6::QJpegPlugin)
endif()
