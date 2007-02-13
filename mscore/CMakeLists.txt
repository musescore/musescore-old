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

# support for mingw32 cross compiling
#  (incomplete, does not work now)
#
OPTION(CROSS_MINGW32 "Cross compile Win32 using mingw32" OFF)
IF (UNIX)
      IF(CROSS_MINGW32)
            set(CMAKE_CXX_COMPILER i586-mingw32msvc-g++)
            set(CMAKE_CXX_LINK_EXECUTABLE i586-mingw32msvc-ld)
      ENDIF(CROSS_MINGW32)
ENDIF (UNIX)

# set(CMAKE_BUILD_TYPE DEBUG)
# set(CMAKE_BUILD_TYPE RELEASE)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)

# The Mscore version number.
SET(Mscore_VERSION_MAJOR 0)
SET(Mscore_VERSION_MINOR 4)
SET(Mscore_VERSION_PATCH "0")
SET(Mscore_VERSION       "0.4")
SET(Mscore_VERSION_FULL  "0.4.0")
SET(Mscore_INSTALL_NAME  "mscore-0.4")

include ( ${PROJECT_SOURCE_DIR}/cmake/UsePkgConfig1.cmake )
include ( ${PROJECT_SOURCE_DIR}/cmake/TargetDoc.cmake)

# for debugging the make system uncomment next line:
#  set(CMAKE_VERBOSE_MAKEFILE ON)

set(QT_USE_QTXML TRUE)
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

if (APPLE)
	message("Disabling ALSA support due to OS X build.")
else (APPLE)
      PKGCONFIG (alsa 0.9.0 ALSA_INCDIR ALSA_LIBDIR ALSA_LIB ALSA_CPP )
if (NOT ALSA_INCDIR)
      message(FATAL_ERROR "Fatal error: ALSA >= 1.0.0 required")
else (NOT ALSA_INCDIR)
	message("Alsa detected.")
      set ( USE_ALSA 1 )
endif (NOT ALSA_INCDIR)
endif (APPLE)

##
## find jack >= 0.98.0
##

PKGCONFIG (jack 0.98.0 JACK_INCDIR JACK_LIBDIR JACK_LIB JACK_CPP )
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

add_custom_target(all.h.pch
   ALL
   COMMAND cp ${PROJECT_SOURCE_DIR}/mscore/all.h ${PROJECT_BINARY_DIR}/mscore/all.h
   COMMAND ${CMAKE_CXX_COMPILER}
     -g -Wextra -fno-exceptions -Wall
     -I${QT_INCLUDE_DIR}
     -I${QT_QTCORE_INCLUDE_DIR}
     -I${QT_QTGUI_INCLUDE_DIR}
     -o all.h.pch all.h
   DEPENDS ${PROJECT_SOURCE_DIR}/mscore/all.h
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/mscore
   )

subdirs(fluid mscore share doc)

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

