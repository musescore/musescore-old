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

project(gendoc)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6)

set(CMAKE_BUILD_TYPE DEBUG)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)

# for debugging the make system uncomment next line:
# set(CMAKE_VERBOSE_MAKEFILE ON)

##
##  look for Qt4
##

set(QT_MIN_VERSION "4.3.0")
set(QT_USE_QTXML TRUE)

find_package(Qt4 4.3)
if (NOT QT4_FOUND)
      message(FATAL_ERROR "Fatal error: QT (version >= 4.3.0) required.\n"
         "Cmake tries to detect QT4 by searching for 'qmake' in your PATH\n"
         "If you have QT4 installed, make sure qmake is found in your PATH.\n"
         "If you compiled QT4 yourself make sure your new qmake ist found\n"
         "_first_ in your PATH.\n"
         )
      endif (NOT QT4_FOUND)

include(${QT_USE_FILE})

# include_directories(
#   ${PROJECT_SOURCE_DIR}
#   )

set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DQT_NO_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "-DQT_DEBUG")

add_executable(gendoc gendoc.cpp xml.cpp)

target_link_libraries(gendoc ${QT_LIBRARIES})

