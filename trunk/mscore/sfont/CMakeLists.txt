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

add_custom_command(
   OUTPUT ${PROJECT_BINARY_DIR}/all.h
   COMMAND ${CMAKE_COMMAND}
   ARGS -E copy ${PROJECT_SOURCE_DIR}/all.h ${PROJECT_BINARY_DIR}/all.h
   DEPENDS ${PROJECT_SOURCE_DIR}/all.h
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
   )

add_custom_command(
   OUTPUT ${PROJECT_BINARY_DIR}/all.h.pch
   COMMAND ${CMAKE_CXX_COMPILER}
     -g -Wextra -Wall
     -I"${QT_INCLUDE_DIR}"
     -o all.h.pch all.h
   DEPENDS ${PROJECT_BINARY_DIR}/all.h
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
   )

set_source_files_properties(
      ${PROJECT_BINARY_DIR}/all.h
      ${PROJECT_BINARY_DIR}/all.h.pch
      PROPERTIES generated true
      )

add_executable(sfconvert
   sfconvert.cpp sfont.cpp
   ${PROJECT_BINARY_DIR}/all.h.pch
   )

target_link_libraries(sfconvert
   al
   ${QT_LIBRARIES}
   vorbis
   vorbisenc
   ogg
   sndfile
   )

set_target_properties(sfconvert
   PROPERTIES COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h"
   )

