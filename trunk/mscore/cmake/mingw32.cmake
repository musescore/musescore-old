#
#     toolchain file
#

set (CMAKE_SYSTEM_NAME "Windows")

set (CROSS /home/ws/mingw)
set (CROSSQT ${CROSS}/Qt/4.4.0)

set (CMAKE_C_COMPILER     ${CROSS}/bin/i386-mingw32-gcc)
set (CMAKE_CXX_COMPILER   ${CROSS}/bin/i386-mingw32-g++)
set (CMAKE_FIND_ROOT_PATH ${CROSS}/i386-mingw32)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set (QT_INCLUDE_DIR           ${CROSSQT}/include)
set (QT_QT_INCLUDE_DIR        ${CROSSQT}/include/Qt)
set (QT_QTCORE_INCLUDE_DIR    ${CROSSQT}/include/QtCore)
set (QT_QTXML_INCLUDE_DIR     ${CROSSQT}/include/QtXml)
set (QT_QTGUI_INCLUDE_DIR     ${CROSSQT}/include/QtGui)
set (QT_QTNETWORK_INCLUDE_DIR ${CROSSQT}/include/QtNetwork)
set (QT_QTUITOOLS_INCLUDE_DIR ${CROSSQT}/include/QtUiTools)
set (QT_LIBRARY_DIR           ${CROSSQT}/lib)

set (QT_MOC_EXECUTABLE        "/usr/qt4/bin/moc")
set (QT_UIC_EXECUTABLE        "/usr/qt4/bin/uic")
set (QT_RCC_EXECUTABLE        "/usr/qt4/bin/rcc")
set (QT_QTCORE_LIBRARY        "mops")

set (QT_INCLUDES ${QT_INCLUDE_DIR} ${QT_QT_INCLUDE_DIR}
     ${QT_QTCORE_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR} ${QT_GUI_INCLUDE_DIR}
     ${QT_QTNETWORK_INCLUDE_DIR}
     )
set (QT_mingw_LIBRARIES
    QtScript4
    QtSvg4
    QtUiTools
    QtUiToolsd
    QtGui4
    QtCore4
    QtXml4
    QtNetwork4
    )

#    Qt3Support4

set (WIN32 ON)
set (MINGW ON)
set (CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

