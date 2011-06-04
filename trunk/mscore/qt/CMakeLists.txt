#=============================================================================
#  MusescorePlayer
#  $Id:$
#
#  Copyright (C) 2011 by Werner Schweer
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

project(mscoreplay)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)

cmake_policy(SET CMP0003 NEW)

# for debugging the make system uncomment next line:
# set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_INCLUDE_CURRENT_DIR      TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_SKIP_RULE_DEPENDENCY     TRUE)

# The MscorePlayer version number.
SET(MscorePlayer_VERSION_MAJOR  "0")
SET(MscorePlayer_VERSION_MINOR  "0")
SET(MscorePlayer_VERSION_PATCH  "1")

SET(MscorePlayer_VERSION       "${MscorePlayer_VERSION_MAJOR}.${MscorePlayer_VERSION_MINOR}")
SET(MscorePlayer_VERSION_FULL  "${MscorePlayer_VERSION}.${MscorePlayer_VERSION_PATCH}")

##
##  look for Qt4
##

set(QT_MIN_VERSION "4.7.0")
set(QT_USE_QTSVG   TRUE)

find_package(Qt4 "4.7")

if (NOT QT4_FOUND)
      message(FATAL_ERROR "Fatal error: QT (version >= 4.7.0) required."
         " Cmake tries to detect QT4 by searching for 'qmake' in your PATH."
         "If you have QT4 installed, make sure qmake is found in your PATH. "
         "If you compiled QT4 yourself make sure your new qmake ist found"
         " _first_ in your PATH."
         )
else (NOT QT4_FOUND)
      message("Qt lib found at ${QT_LIBRARY_DIR}")
endif (NOT QT4_FOUND)

include(${QT_USE_FILE})

##
## alsa >= 1.0.0
##
## ALSA_LIBRARY, ALSA_INCLUDE_DIR

include (FindALSA)

if (NOT ALSA_FOUND)
      message(FATAL_ERROR "Fatal error: ALSA >= 1.0.0 required")
else (NOT ALSA_FOUND)
      set (USE_ALSA 1)
endif (NOT ALSA_FOUND)

set(CMAKE_CXX_FLAGS         "-g -Wall -W -Winvalid-pch -fvisibility=hidden -fvisibility-inlines-hidden")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DQT_NO_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "-DQT_DEBUG")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath,${QT_LIBRARY_DIR}")

QT4_ADD_RESOURCES(qrc_files mscoreplay.qrc)

include_directories(
      ${PROJECT_SOURCE_DIR}/..
      ${ALSA_INCLUDE_DIR}
      )

add_executable(mscoreplay
      ${qrc_files}
      ${ui_headers}
      ${mocs}
      main.cpp
      scoreview.cpp
      painter.cpp
      scoreproxy.cpp
      )

target_link_libraries(mscoreplay
      ${QT_LIBRARIES}
      ${ALSA_LIBRARY}
      mscore
      al
      osdabzip
      msynth
      mxml
      )

subdirs(../libmscore ../al ../osdabzip ../msynth ../mxml)

