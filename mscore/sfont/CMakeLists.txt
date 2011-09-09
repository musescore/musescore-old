#=============================================================================
#  MusE
#  Linux Music Editor
#  $Id:$
#
#  Copyright (C) 2009 by Werner Schweer and others
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

include (${PROJECT_SOURCE_DIR}/cmake/gch.cmake)

add_executable(sfconvert
   sfconvert.cpp sfont.cpp
   ${PROJECT_BINARY_DIR}/all.h
   ${PCH}
   )

target_link_libraries(sfconvert
   libmscore
   msynth
   zarchive
   ${QT_LIBRARIES}
   vorbis
   vorbisenc
   ogg
   sndfile
   z
   rt
   )

set_target_properties (
      sfconvert
      PROPERTIES
         COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h -g -Wall -Wextra -Winvalid-pch"
      )

ADD_DEPENDENCIES(sfconvert mops1)
ADD_DEPENDENCIES(sfconvert mops2)

