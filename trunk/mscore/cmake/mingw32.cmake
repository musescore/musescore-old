#
#     toolchain file
#

set (CMAKE_SYSTEM_NAME "Windows")

set (CMAKE_C_COMPILER   i586-mingw32msvc-gcc)
set (CMAKE_CXX_COMPILER i586-mingw32msvc-g++)
#set (CMAKE_C_COMPILER /home/ws/.wine/drive_c/MinGW/bin/gcc.exe)
#set (CMAKE_CXX_COMPILER /home/ws/.wine/drive_c/MinGW/bin/g++.exe)

set (CMAKE_FIND_ROOT_PATH /usr/i586-mingw32msvc)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set (QT_INCLUDE_DIR           "/home/ws/.wine/drive_c/Qt/4.4.0/include")
set (QT_QT_INCLUDE_DIR        "/home/ws/.wine/drive_c/Qt/4.4.0/include/Qt")
set (QT_QTCORE_INCLUDE_DIR    "/home/ws/.wine/drive_c/Qt/4.4.0/include/QtCore")
set (QT_QTXML_INCLUDE_DIR     "/home/ws/.wine/drive_c/Qt/4.4.0/include/QtXml")
set (QT_QTGUI_INCLUDE_DIR     "/home/ws/.wine/drive_c/Qt/4.4.0/include/QtGui")
set (QT_QTUITOOLS_INCLUDE_DIR "/home/ws/.wine/drive_c/Qt/4.4.0/include/QtUiTools")
set (QT_LIBRARY_DIR "/home/ws/.wine/drive_c/Qt/4.4.0/lib")

set (QT_INCLUDES ${QT_INCLUDE_DIR} ${QT_QT_INCLUDE_DIR}
     ${QT_QTCORE_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR} ${QT_GUI_INCLUDE_DIR}
     )
set (QT_mingw_LIBRARIES
    QtScript4
    QtSvg4
    QtNetwork4
    QtUiTools
    Qt3Support4
    QtGui4
    QtCore4
    QtXml4
    )

set (WIN32 ON)
set (MINGW ON)
set (CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

