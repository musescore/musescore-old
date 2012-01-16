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

#======================================
#     target smf2xml
#======================================

add_executable(
      smf2xml
      smf2xml.cpp
      ../mscore/midifile.cpp
      ../libmscore/drumset.cpp
      ../libmscore/xml.cpp
      ../libmscore/event.cpp
      ../libmscore/fraction.cpp
      ../libmscore/sig.cpp
      )
target_link_libraries(smf2xml ${QT_LIBRARIES} )
set_target_properties (
      smf2xml
      PROPERTIES
         COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h -g -Wall -Wextra -Winvalid-pch"
      )

#======================================
#     target xml2smf
#======================================

add_executable(
      xml2smf
      xml2smf.cpp
      ../mscore/midifile.cpp
      ../libmscore/drumset.cpp
      ../libmscore/event.cpp
      ../libmscore/fraction.cpp
      ../libmscore/xml.cpp
      ../libmscore/sig.cpp
      )
target_link_libraries(xml2smf ${QT_LIBRARIES} )
set_target_properties (
      xml2smf
      PROPERTIES
         COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h -g -Wall -Wextra -Winvalid-pch"
      )

ADD_DEPENDENCIES(xml2smf mops1)
ADD_DEPENDENCIES(xml2smf mops2)
ADD_DEPENDENCIES(smf2xml mops1)
ADD_DEPENDENCIES(smf2xml mops2)

