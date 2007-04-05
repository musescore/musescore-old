#=============================================================================
#  Mscore
#  Linux Music Score Editor
#  $Id:$
#
#  Copyright (C) 2002-2006 by Werner Schweer and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

project(mscore)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.1)

# support for mingw cross compiling
#  (incomplete, does not work now)
#
if (UNIX)
      option (CROSS_MINGW "Cross compile using MinGW on linux" OFF)
else (UNIX)
      set (CROSS_MINGW OFF)
endif (UNIX)

if (CROSS_MINGW)
      set (CMAKE_CXX_COMPILER  "i586-mingw32msvc-c++")
      set (CMAKE_COMPILER_IS_GNUCXX 1)
      set (CMAKE_C_COMPILER  "i586-mingw32msvc-gcc")
      set (CMAKE_AR "i586-mingw32msvc-ar")
      set (CMAKE_RANLIB "i586-mingw32msvc-ranlib")
      set (CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "1")
      set (QT_INCLUDE_DIR "/usr/win-qt422/include")
      set (QT_LIBRARY_DIR "/usr/win-qt422/lib")
      set (QT_MKSPECS_DIR "/usr/win-qt422/mkspecs")
      set (WIN32 ON)
      set (MINGW ON)
      set (CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
      set (CMAKE_SYSTEM_LIBRARY_PATH /home/ws/.wine/drive_c/MinGW)
      set (CMAKE_VERBOSE_MAKEFILE ON)
      set (QT_X11_X11_LIBRARY "")
      set (QT_X11_Xext_LIBRARY "")
      set (QT_X11_m_LIBRARY "")
      set (HAVE_PTHREAD OFF)
ENDIF(CROSS_MINGW)

# set(CMAKE_BUILD_TYPE DEBUG)
# set(CMAKE_BUILD_TYPE RELEASE)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)

# The Mscore version number.
SET(Mscore_VERSION_MAJOR 0)
SET(Mscore_VERSION_MINOR 5)
SET(Mscore_VERSION_PATCH "0")
SET(Mscore_VERSION       "0.6")
SET(Mscore_VERSION_FULL  "0.6.0")
SET(Mscore_INSTALL_NAME  "mscore-0.6")

include ( ${PROJECT_SOURCE_DIR}/cmake/UsePkgConfig1.cmake )
include ( ${PROJECT_SOURCE_DIR}/cmake/TargetDoc.cmake)

# for debugging the make system uncomment next line:
# set(CMAKE_VERBOSE_MAKEFILE ON)

set(QT_USE_QTXML TRUE)
set(QT_USE_QTSVG TRUE)
set(QT_USE_QTDESIGNER TRUE)
set(QT_USE_QTNSPLUGIN TRUE)

##
## Just print a notice if this is OS X
##
if (APPLE)
	message("OS X detected.")
else (APPLE)
	if (UNIX)
		message("Unix (probably linux) detected")
	endif(UNIX)

endif (APPLE)

##
##  look for Qt4
##

find_package(Qt4)
if (NOT QT4_FOUND)
      message(FATAL_ERROR "Fatal error: QT (version >= 4.2.0) required.\n"
         "Cmake tries to detect QT4 by searching for 'qmake' in your PATH\n"
         "If you have QT4 installed, make sure qmake is found in your PATH."
         )
      endif (NOT QT4_FOUND)
include(${QT_USE_FILE})

##
## find texexec (ConTeXt)
##

find_path( TEXEXEC_PATH texutil )
if (NOT TEXEXEC_PATH)
      message("texexec from ConTeXt package not found. Documentation will not be build")
endif (NOT TEXEXEC_PATH)


##
## find doxygen
##    TODO

##
## alsa >= 0.9.0
##

if (APPLE OR MINGW)
	message("Disabling ALSA support due to OS X or MINGW build.")
else (APPLE OR MINGW)
      PKGCONFIG (alsa 0.9.0 ALSA_INCDIR ALSA_LIBDIR ALSA_LIB ALSA_CPP )
if (NOT ALSA_INCDIR)
      message(FATAL_ERROR "Fatal error: ALSA >= 1.0.0 required")
else (NOT ALSA_INCDIR)
	message("Alsa detected.")
      set ( USE_ALSA 1 )
endif (NOT ALSA_INCDIR)
endif (APPLE OR MINGW)

##
## find jack >= 0.98.0
##

if (MINGW)
      message("no JACK in MINGW build")
else (MINGW)
      PKGCONFIG (jack 0.98.0 JACK_INCDIR JACK_LIBDIR JACK_LIB JACK_CPP )
endif (MINGW)

if (JACK_INCDIR)
	message("Jack detected")
      set ( USE_JACK 1 )
endif (JACK_INCDIR)

#
# produce config.h file
#
configure_file (
      ${PROJECT_SOURCE_DIR}/config.h.in
      ${PROJECT_BINARY_DIR}/config.h
      )

subdirs(awl fluid mscore share doc)

include_directories(
   ${PROJECT_SOURCE_DIR}
   ${PROJECT_SOURCE_DIR}/mscore
   ${PROJECT_BINARY_DIR}
   ${PROJECT_BINARY_DIR}/mscore
   ${ALSA_INCDIR}
   ${JACK_INCDIR}
   )

#
# If the cmake version includes cpack, use it
#
IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")

  IF(EXISTS "${CMAKE_ROOT}/Modules/InstallRequiredSystemLibraries.cmake")
      SET(CMAKE_INSTALL_MFC_LIBRARIES 1)
      INCLUDE(InstallRequiredSystemLibraries)
  ENDIF(EXISTS "${CMAKE_ROOT}/Modules/InstallRequiredSystemLibraries.cmake")

  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MuseScore is a full featured WYSIWYG score editor")
  SET(CPACK_PACKAGE_VENDOR "Werner Schweer and Others")
  SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
  SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
  SET(CPACK_PACKAGE_VERSION_MAJOR "${Mscore_VERSION_MAJOR}")
  SET(CPACK_PACKAGE_VERSION_MINOR "${Mscore_VERSION_MINOR}")
  SET(CPACK_PACKAGE_VERSION_PATCH "${Mscore_VERSION_PATCH}")
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
  SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${Mscore_INSTALL_NAME}")

  SET(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})

  SET(CPACK_PACKAGE_FILE_NAME "${CPACK_SOURCE_PACKAGE_FILE_NAME}-${CPACK_SYSTEM_NAME}")
  SET(CPACK_STRIP_FILES "bin/mscore")
  SET(CPACK_PACKAGE_EXECUTABLES "mscore" "MuseScore")
  INCLUDE(CPack)
ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")

add_custom_target(lupdate
   COMMAND ${PROJECT_SOURCE_DIR}/gen-qt-projectfile ${PROJECT_SOURCE_DIR} > mscore.pro
   COMMAND lupdate -noobsolete ${PROJECT_BINARY_DIR}/mscore.pro
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
   )


