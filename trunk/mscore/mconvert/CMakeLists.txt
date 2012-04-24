#=============================================================================
#  MuseScore
#  Music Composition & Notation
#  $Id:$
#
#  Copyright (C) 2011 Werner Schweer
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2
#  as published by the Free Software Foundation and appearing in
#  the file LICENSE.GPL
#=============================================================================

set(CMAKE_VERBOSE_MAKEFILE ON)

include (${PROJECT_SOURCE_DIR}/cmake/gch.cmake)

include_directories(
      ${PROJECT_BINARY_DIR}
      ${PROJECT_SOURCE_DIR}
      )

QT4_ADD_RESOURCES(qrc_files mconvert.qrc)

add_executable(mconvert
      ${PROJECT_BINARY_DIR}/all.h
      ${PCH}
      ${qrc_files}
      mconvert.cpp
      )

target_link_libraries(mconvert
      libmscore
      msynth
      ${QT_LIBRARIES}
      zarchive
      z
      )

set_target_properties (
      mconvert
      PROPERTIES
      COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h -g -Wall -Wextra -Winvalid-pch"
      )

ADD_DEPENDENCIES(mscoreplay mops1)
ADD_DEPENDENCIES(mscoreplay mops2)

