set (CMAKE_SYSTEM_NAME "Windows")

set (CMAKE_C_COMPILER   i586-mingw32msvc-gcc)
set (CMAKE_CXX_COMPILER i586-mingw32msvc-g++)

set (CMAKE_FIND_ROOT_PATH /usr/i586-mingw32msvc /home/ws/mingw-install)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set (QT_INCLUDE_DIR "/home/ws/.wine/drive_c/Qt/4.4.0-beta1/include")
set (QT_LIBRARY_DIR "/home/ws/.wine/drive_c/Qt/4.4.0-beta1/lib")
set (QT_mingw_LIBRARIES QtCore4 QtGui4 QtXml4 QtSvg4 QtScript4)

set (WIN32 ON)
set (MINGW ON)
set (CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

