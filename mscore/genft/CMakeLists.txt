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

set(CMAKE_VERBOSE_MAKEFILE OFF)

include (${PROJECT_SOURCE_DIR}/cmake/gch.cmake)

include_directories(
      ${PROJECT_BINARY_DIR}
      ${PROJECT_SOURCE_DIR}
      )

add_executable(
      genft
      ${ui_headers}
      genft.cpp
      ../libmscore/xml.cpp
      ../libmscore/fraction.cpp
      )

target_link_libraries(genft ${QT_LIBRARIES} -lfreetype)
set_target_properties(genft PROPERTIES COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h -I/usr/include/freetype2 -g -Wall -Wextra -Winvalid-pch")


ADD_DEPENDENCIES(mtest mops1)
ADD_DEPENDENCIES(mtest mops2)

