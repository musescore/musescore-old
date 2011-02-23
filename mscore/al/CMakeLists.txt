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

set (al_src
   al.cpp
   dsp.cpp
   xml.cpp
   tempo.cpp
   sig.cpp
   pos.cpp
   fraction.cpp
   )

if (USE_SSE)
      set(al_src ${al_src} dspSSE.cpp dspXMM.cpp)
endif (USE_SSE)

add_library(al STATIC
   ${PROJECT_BINARY_DIR}/all.h
   ${PCH}
   ${al_src}
   )

set_target_properties (
      al
      PROPERTIES
         COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h -g -Wall -Wextra -Winvalid-pch"
      )

set_source_files_properties(dspSSE.cpp
   PROPERTIES COMPILE_FLAGS "-msse -x assembler"
   )

set_source_files_properties(dspXMM.cpp
   PROPERTIES COMPILE_FLAGS "-msse"
   )

ADD_DEPENDENCIES(al mops1)
ADD_DEPENDENCIES(al mops2)

