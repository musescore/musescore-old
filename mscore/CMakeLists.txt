#=============================================================================
#  Mscore
#  Linux Music Score Editor
#  $Id:$
#
#  Copyright (C) 2002-2007 by Werner Schweer and others
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
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6)

# cmake 2.4.3 does not install all.h and does not link against qtsvg

# set(CMAKE_BUILD_TYPE DEBUG)
# set(CMAKE_BUILD_TYPE RELEASE)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)

# The Mscore version number.
SET(Mscore_VERSION_MAJOR  "0")
SET(Mscore_VERSION_MINOR  "7")
SET(Mscore_VERSION_PATCH  "0")

SET(Mscore_VERSION       "${Mscore_VERSION_MAJOR}.${Mscore_VERSION_MINOR}")
SET(Mscore_VERSION_FULL  "${Mscore_VERSION}.${Mscore_VERSION_PATCH}")
SET(Mscore_INSTALL_NAME  "mscore-${Mscore_VERSION}")

include ( ${PROJECT_SOURCE_DIR}/cmake/UsePkgConfig1.cmake )
include ( ${PROJECT_SOURCE_DIR}/cmake/TargetDoc.cmake)

# for debugging the make system uncomment next line:
set(CMAKE_VERBOSE_MAKEFILE ON)

##
##  look for Qt4
##

set(QT_MIN_VERSION "4.3.0")
set(QT_USE_QTXML TRUE)
set(QT_USE_QTSVG TRUE)

find_package(Qt4 4.3)
if (NOT QT4_FOUND)
      message(FATAL_ERROR "Fatal error: QT (version >= 4.3.0) required.\n"
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
      set (USE_ALSA 0)
else (APPLE OR MINGW)
      PKGCONFIG (alsa 1.0.0 ALSA_INCDIR ALSA_LIBDIR ALSA_LIB ALSA_CPP )
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
      set (USE_JACK 0)
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

subdirs(awl fluid mscore share doc demos)

include_directories(
   ${PROJECT_SOURCE_DIR}
   ${PROJECT_SOURCE_DIR}/mscore
   ${PROJECT_BINARY_DIR}
   ${PROJECT_BINARY_DIR}/mscore
   ${ALSA_INCDIR}
   ${JACK_INCDIR}
   )

#
# create package target
#
include (InstallRequiredSystemLibraries)

  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MuseScore is a full featured WYSIWYG score editor")
  SET(CPACK_PACKAGE_VENDOR "Werner Schweer and Others")
  SET(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/COPYING")
  SET(CPACK_RESOURCE_FILE_LICENSE    "${PROJECT_SOURCE_DIR}/COPYING")

  SET(CPACK_PACKAGE_VERSION_MAJOR "${Mscore_VERSION_MAJOR}")
  SET(CPACK_PACKAGE_VERSION_MINOR "${Mscore_VERSION_MINOR}")
  SET(CPACK_PACKAGE_VERSION_PATCH "${Mscore_VERSION_PATCH}")
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "MuseScore ${Mscore_VERSION_MAJOR}.${Mscore_VERSION_MINOR}")

  IF(MINGW)
      SET(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/mscore/data/mscore.bmp")
      SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin/mscore.exe")
      SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} MuseScore score typesetter")
      SET(CPACK_NSIS_HELP_LINK "http://mscore.sourceforge.net")
      SET(CPACK_NSIS_URL_INFO_ABOUT "http://mscore.sourceforge.net")
      SET(CPACK_NSIS_CONTACT "we@seh.de")
      SET(CPACK_NSIS_MODIFY_PATH ON)
  ELSE(MINGW)
      SET(CPACK_STRIP_FILES "mscore")
      SET(CPACK_SOURCE_STRIP_FILES "")
  ENDIF(MINGW)

  SET(CPACK_SOURCE_PACKAGE_FILE_NAME "mscore")
  SET(CPACK_PACKAGE_FILE_NAME     "${CPACK_SOURCE_PACKAGE_FILE_NAME}-${Mscore_VERSION_FULL}")
  SET(CPACK_PACKAGE_EXECUTABLES   "mscore" "MuseScore")

  set(DEBIAN_PACKAGE_NAME         "mscore")
  set(DEBIAN_PACKAGE_VERSION      "${Mscore_VERSION_FULL}-0ubuntu0.1")
  set(DEBIAN_PACKAGE_ARCHITECTURE "i386")
  set(DEBIAN_PACKAGE_DEPENDS      "libqt4-core (>= 4.3), libqt4-gui (>= 4.3)")
  set(DEBIAN_PACKAGE_MAINTAINER   "ws@seh.de")
  set(CPACK_PACKAGE_CONTACT       "ws@seh.de")
if (MINGW)
  set(CPACK_GENERATOR             "NSIS")
else (MINGW)
  set(CPACK_GENERATOR             "DEB;TBZ2")
endif (MINGW)
  set(CPACK_DEB "on")

  include (CPack)
message("Depends2: ${DEBIAN_PACKAGE_DEPENDS}")


add_custom_target(lupdate
   COMMAND ${PROJECT_SOURCE_DIR}/gen-qt-projectfile ${PROJECT_SOURCE_DIR} > mscore.pro
   COMMAND lupdate -noobsolete ${PROJECT_BINARY_DIR}/mscore.pro
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
   )

add_custom_target(lrelease
   COMMAND ${PROJECT_SOURCE_DIR}/gen-qt-projectfile ${PROJECT_SOURCE_DIR} > mscore.pro
   COMMAND lrelease ${PROJECT_BINARY_DIR}/mscore.pro
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
   )

