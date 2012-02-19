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

QT4_ADD_RESOURCES(qrc_files mtest.qrc)


add_executable(mtest
      ${PROJECT_BINARY_DIR}/all.h
      ${PCH}
      ${qrc_files}
      ${ui_headers}
      ${mocs}
      mtest.cpp
      testnote.cpp
      testhairpin.cpp
      testmidi.cpp
      mcursor.cpp
      testutils.cpp
      testparts.cpp
      ../mscore/exportmidi.cpp
      ../mscore/importmidi.cpp
      ../mscore/midifile.cpp
      )

target_link_libraries(mtest
      libmscore
      msynth
      ${QT_LIBRARIES}
      zarchive
      z
      rt
      )

set_target_properties (
      mtest
      PROPERTIES
      COMPILE_FLAGS "-D TESTROOT=\\\"${PROJECT_SOURCE_DIR}\\\" -include ${PROJECT_BINARY_DIR}/all.h -g -Wall -Wextra -Winvalid-pch"
      LINK_FLAGS    "-g"
      )

ADD_DEPENDENCIES(mtest mops1)
ADD_DEPENDENCIES(mtest mops2)

