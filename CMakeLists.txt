cmake_minimum_required(VERSION 3.16)

project(RobotGUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable automatic MOC, UIC, RCC
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Svg SvgWidgets Sql Xml Network)

# 项目源码
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    resources.qrc
    svgviewer.cpp
    svgviewer.h
    patientdatabase.cpp
    patientdatabase.h
    globals.cpp
    globals.h
)

# 创建可执行文件（适用于 Qt6）
qt_add_executable(RobotGUI
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

# 链接 Qt6 库
target_link_libraries(RobotGUI PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Svg
    Qt6::SvgWidgets
    Qt6::Sql
    Qt6::Xml
    Qt6::Network
)

# 加入头文件目录（非必须，但有时能避免找不到头文件）
target_include_directories(RobotGUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 如果你要打包/部署，可以保留安装规则
include(GNUInstallDirs)
install(TARGETS RobotGUI
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Qt6 需要 finalize
qt_finalize_executable(RobotGUI)
