#=============================================================================
#  Mscore
#  Linux Music Score Editor
#  $Id:$
#
#  Copyright (C) 2002-2008 by Werner Schweer and others
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

# for debugging the make system uncomment next line:
# set(CMAKE_VERBOSE_MAKEFILE ON)

set(USE_GLOBAL_FLUID FALSE)               # use internal fluid lib

set(BUILD_SCRIPT_INTERFACE TRUE)          # this requires qt >= 4.4 && cmake 2.6
set(BUILD_SCRIPT_DEBUG TRUE)
set(STATIC_SCRIPT_BINDINGS FALSE)         # this requires qt 4.5

if (MINGW)
      set(USE_GLOBAL_FLUID FALSE)
endif (MINGW)
if (BUILD_SCRIPT_DEBUG)
      set (HAS_SCRIPT_DEBUG TRUE)
endif (BUILD_SCRIPT_DEBUG)

if (BUILD_SCRIPT_INTERFACE)
      set (HAS_SCRIPT_INTERFACE TRUE)
endif (BUILD_SCRIPT_INTERFACE)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)

# The Mscore version number.
SET(Mscore_VERSION_MAJOR  "0")
SET(Mscore_VERSION_MINOR  "9")
SET(Mscore_VERSION_PATCH  "4")

SET(Mscore_VERSION       "${Mscore_VERSION_MAJOR}.${Mscore_VERSION_MINOR}")
SET(Mscore_VERSION_FULL  "${Mscore_VERSION}.${Mscore_VERSION_PATCH}")

if (MINGW)
      SET(Mscore_INSTALL_NAME  "")
      SET(Mscore_SHARE_NAME    "./")
else (MINGW)
      SET(Mscore_INSTALL_NAME  "mscore-${Mscore_VERSION}/")
      SET(Mscore_SHARE_NAME    "share/")
endif (MINGW)

include ( ${PROJECT_SOURCE_DIR}/cmake/UsePkgConfig1.cmake )
include ( ${PROJECT_SOURCE_DIR}/cmake/TargetDoc.cmake)

##
##  look for Qt4
##

set(QT_MIN_VERSION "4.4.0")
set(QT_USE_QTXML     TRUE)
set(QT_USE_QTSVG     TRUE)
set(QT_USE_QTNETWORK TRUE)
set(QT_USE_QTUITOOLS TRUE)
# only available in cmake 2.5:
set(QT_USE_QTSCRIPT TRUE)

find_package(Qt4 "4.4")

if (NOT QT4_FOUND)
      message(FATAL_ERROR "Fatal error: QT (version >= 4.4.0) required."
         " Cmake tries to detect QT4 by searching for 'qmake' in your PATH."
         "If you have QT4 installed, make sure qmake is found in your PATH. "
         "If you compiled QT4 yourself make sure your new qmake ist found"
         " _first_ in your PATH."
         )
else (NOT QT4_FOUND)
      message("Qt lib found at ${QT_LIBRARY_DIR}")
endif (NOT QT4_FOUND)


# Set QT_QTSCRIPT_INCLUDE_DIR
FIND_PATH(QT_QTSCRIPT_INCLUDE_DIR QtScript
   PATHS
   ${QT_INCLUDE_DIR}/QtScript
   ${QT_LIBRARY_DIR}/QtScript.framework/Headers
   NO_DEFAULT_PATH
   )

# Set QT_QTSCRIPT_LIBRARY (for cmake 2.4)
FIND_LIBRARY(QT_QTSCRIPT_LIBRARY_RELEASE NAMES QtScript QtScript4 PATHS ${QT_LIBRARY_DIR}        NO_DEFAULT_PATH)
FIND_LIBRARY(QT_QTSCRIPT_LIBRARY_DEBUG   NAMES QtScript QtScript_debug QtScriptd4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)

message("QT_LIBRARIES  ${QT_LIBRARIES}")

_QT4_ADJUST_LIB_VARS(QTSCRIPT)

#for cmake 2.4
# QT_MODULE_SETUP(SCRIPT)

include(${QT_USE_FILE})

##
## look for fluidsynth
if (USE_GLOBAL_FLUID)
      PKGCONFIG ( fluidsynth 1.0.0 FLUID_INCDIR FLUID_LIBDIR FLUID_LIB FLUID_CPP )
      if (NOT FLUID_INCDIR)
	      message (FATAL_ERROR "FluidSynth >= 1.0.0 required")
      else (NOT FLUID_INCDIR)
	      message ("FluidSynth detected.")
      endif (NOT FLUID_INCDIR)
endif (USE_GLOBAL_FLUID)

##
## alsa >= 0.9.0
##

if (APPLE OR MINGW)
	message("Disabling ALSA support due to OS X or MINGW build.")
      set (USE_ALSA 0)
      set (HAS_MIDI 1)
else (APPLE OR MINGW)
      PKGCONFIG (alsa 1.0.0 ALSA_INCDIR ALSA_LIBDIR ALSA_LIB ALSA_CPP )
      if (NOT ALSA_INCDIR)
            message(FATAL_ERROR "Fatal error: ALSA >= 1.0.0 required")
      else (NOT ALSA_INCDIR)
	      message("Alsa detected.")
            set (USE_ALSA 1)
            set (HAS_MIDI 1)
      endif (NOT ALSA_INCDIR)
endif (APPLE OR MINGW)

##
## find jack >= 0.98.0
##

if (MINGW)
      set (USE_JACK 0)
else (MINGW)
      PKGCONFIG (jack 0.98.0 JACK_INCDIR JACK_LIBDIR JACK_LIB JACK_CPP )
endif (MINGW)

if (JACK_INCDIR)
	message("Jack detected")
      set (USE_JACK 1)
endif (JACK_INCDIR)

##
## portaudio
##

if (MINGW)
      set ( USE_PORTAUDIO 1 )
      set ( USE_PORTMIDI  1 )
else (MINGW)
   PKGCONFIG (portaudio-2.0 19 PORTAUDIO_INCDIR PORAUDIO_LIBDIR PORTAUDIO_LIB PORTAUDIO_CPP )
   if (PORTAUDIO_INCDIR)
	message("portaudio detected")
      set ( USE_PORTAUDIO 1 )
   else (PORTAUDIO_INCDIR)
      message("optional package portaudio-2.0 Version 19 not found (package portaudio19-dev)\n")
      set ( USE_PORTAUDIO 0 )
   endif (PORTAUDIO_INCDIR)
   set (USE_PORTMIDI 0)
endif (MINGW)

# produce config.h file
#
configure_file (
      ${PROJECT_SOURCE_DIR}/config.h.in
      ${PROJECT_BINARY_DIR}/config.h
      )
if (NOT MINGW)
      install( FILES packaging/mscore.desktop DESTINATION share/applications)
endif (NOT MINGW)

set (subdirs awl osdabzip mscore share manual demos)

if (NOT USE_GLOBAL_FLUID)
      set (subdirs ${subdirs} fluid)
endif (NOT USE_GLOBAL_FLUID)

if (BUILD_SCRIPT_INTERFACE)
      set (subdirs ${subdirs} scriptgen)
endif (BUILD_SCRIPT_INTERFACE)

if (BUILD_SCRIPT_DEBUG)
      set (subdirs ${subdirs} scriptdebug)
endif (BUILD_SCRIPT_DEBUG)

if (MINGW)
      set (subdirs ${subdirs} portmidi)
endif (MINGW)

subdirs(${subdirs})


include_directories(
   ${PROJECT_SOURCE_DIR}
   ${PROJECT_SOURCE_DIR}/mscore
   ${PROJECT_BINARY_DIR}
   ${PROJECT_BINARY_DIR}/mscore
   ${ALSA_INCDIR}
   ${JACK_INCDIR}
   )
if (USE_GLOBAL_FLUID)
      include_directories(${FLUID_INCDIR})
endif (USE_GLOBAL_FLUID)

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
  SET(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/mscore/data/mscore.bmp")

  IF(MINGW)
      SET(CPACK_NSIS_INSTALLED_ICON_NAME "${PROJECT_SOURCE_DIR}/mscore/data/mscore.bmp")
      SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} MuseScore score typesetter")
      SET(CPACK_NSIS_HELP_LINK "http://www.musescore.org")
      SET(CPACK_NSIS_URL_INFO_ABOUT "http://www.musescore.org")
      SET(CPACK_NSIS_CONTACT "ws@seh.de")
      SET(CPACK_NSIS_MODIFY_PATH OFF)
#      SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "")
  ELSE(MINGW)
      SET(CPACK_STRIP_FILES "mscore")
      SET(CPACK_SOURCE_STRIP_FILES "")
  ENDIF(MINGW)

  SET(CPACK_SOURCE_PACKAGE_FILE_NAME "mscore")
  SET(CPACK_PACKAGE_FILE_NAME     "${CPACK_SOURCE_PACKAGE_FILE_NAME}-${Mscore_VERSION_FULL}")
  SET(CPACK_PACKAGE_EXECUTABLES   "mscore" "MuseScore")

  set(CPACK_DEBIAN_PACKAGE_NAME         "mscore")
  set(CPACK_DEBIAN_PACKAGE_VERSION      "${Mscore_VERSION_FULL}-0ubuntu0.1")
  set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")

  if (USE_GLOBAL_FLUID)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS  "libqt4-core (>= 4.4), libqt4-gui (>= 4.4), libfluidsynth-dev (>= 1.0.0)")
  else (USE_GLOBAL_FLUID)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS      "libqt4-core (>= 4.4), libqt4-gui (>= 4.4)")
  endif (USE_GLOBAL_FLUID)

  set(CPACK_DEBIAN_PACKAGE_MAINTAINER   "tsmithe@ubuntu.com")
  set(CPACK_DEBIAN_PACKAGE_SECTION      "devel")
  set(CPACK_DEBIAN_PACKAGE_PRIORITY     "optional")
  set(CPACK_DEBIAN_PACKAGE_RECOMMENDS   "")
  set(CPACK_DEBIAN_PACKAGE_SUGGESTS     "")

  set(CPACK_PACKAGE_CONTACT       "ws@seh.de")

if (MINGW)
  set(CPACK_GENERATOR             "NSIS")
else (MINGW)
  set(CPACK_GENERATOR             "DEB;TBZ2")
endif (MINGW)
  set(CPACK_DEB "on")

  include (CPack)

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

