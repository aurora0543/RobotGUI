# RobotGUI/CMakeLists.txt
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_library(RobotGUI STATIC
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    svgviewer.cpp
    svgviewer.h
    resources.qrc
    patientdatabase.cpp
    patientdatabase.h
    ../src/core/face_recognizer.cpp
    ../include/core/face_recognizer.h
)



target_link_libraries(RobotGUI PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Svg
    Qt6::SvgWidgets
    Qt6::Sql
    Qt6::Xml
    Qt6::Network
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)

target_include_directories(RobotGUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
    ${OpenCV_INCLUDE_DIRS} 
    ${Qt6Multimedia_INCLUDE_DIRS}
)
