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

QT4_WRAP_CPP (mocs
     scoreview.h
     runtime.h
     seq.h
     )

QT4_ADD_RESOURCES(qrc_files mscoreplay.qrc)

add_executable(mscoreplay
      ${PROJECT_BINARY_DIR}/all.h
      ${PCH}
      ${qrc_files}
      ${ui_headers}
      ${mocs}
      main.cpp
      sparm.cpp
      scoreview.cpp
      painterqt.cpp
      pa.cpp
      seq.cpp
      )

#      QtDeclarative
#      /home/ws/qt/qt/lib/libQtDeclarative.so
target_link_libraries(mscoreplay
      libmscore
      /home/ws/qt/qt/lib/libQtDeclarative.so
      ${QT_LIBRARIES}
      zarchive
      m-msynth
      portaudio
      z
      rt
      )

set_target_properties (
      mscoreplay
      PROPERTIES
      COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h -g -Wall -Wextra -Winvalid-pch"
      )

ADD_DEPENDENCIES(mscoreplay mops1)
ADD_DEPENDENCIES(mscoreplay mops2)

