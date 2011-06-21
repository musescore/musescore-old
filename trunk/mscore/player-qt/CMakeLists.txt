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

include (${PROJECT_SOURCE_DIR}/cmake/gch.cmake)

include_directories(
      ${PROJECT_SOURCE_DIR}
      )

QT4_WRAP_CPP (mocs
     scoreview.h
     )

add_executable(mscoreplay
      ${PROJECT_BINARY_DIR}/all.h
      ${PCH}
      ${qrc_files}
      ${ui_headers}
      ${mocs}
      main.cpp
      )

target_link_libraries(mscoreplay
      ${QT_LIBRARIES}
      libmscore
      zarchive
      omr
      al
      poppler
      al
      )

set_target_properties (
      mscoreplay
      PROPERTIES
      COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h -g -Wall -Wextra -Winvalid-pch"
      )

ADD_DEPENDENCIES(mscoreplay mops1)
ADD_DEPENDENCIES(mscoreplay mops2)

