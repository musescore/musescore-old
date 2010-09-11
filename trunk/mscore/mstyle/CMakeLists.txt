#=============================================================================
#  MusE
#  Linux Music Editor
#  $Id:$
#
#  Copyright (C) 2010 by Werner Schweer and others
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

QT4_WRAP_CPP (mocs
      mstyle.h frameshadow.h animation.h animations.h baseengine.h
      menubardata.h menubarengine.h widgetstateengine.h widgetstatedata.h
      genericdata.h toolboxengine.h toolbarengine.h toolbardata.h
      tabbarengine.h tabbardata.h splitterengine.h spinboxengine.h
      spinboxdata.h sliderengine.h sliderdata.h scrollbarengine.h
      scrollbardata.h progressbarengine.h progressbardata.h
      menuengine.h menudata.h mdiwindowengine.h mdiwindowdata.h
      headerviewengine.h headerviewdata.h enabledata.h
      dockseparatorengine.h dockseparatordata.h
      animationdata.h
      )

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

add_library (
      mstyle STATIC
      ${PROJECT_BINARY_DIR}/all.h
      ${PROJECT_BINARY_DIR}/all.h.pch
      ${mocs}
      mstyle.cpp colorscheme.cpp colorutils.cpp tileset.cpp stylehelper.cpp frameshadow.cpp
      animations.cpp animationdata.cpp dockseparatordata.cpp
      dockseparatorengine.cpp enabledata.cpp genericdata.cpp headerviewdata.cpp
      headerviewengine.cpp mdiwindowdata.cpp mdiwindowengine.cpp menubardata.cpp
      menubarengine.cpp menuengine.cpp progressbardata.cpp
      progressbarengine.cpp scrollbardata.cpp scrollbarengine.cpp sliderdata.cpp
      sliderengine.cpp spinboxdata.cpp spinboxengine.cpp splitterengine.cpp
      tabbardata.cpp tabbarengine.cpp toolbardata.cpp toolbarengine.cpp
      toolboxengine.cpp widgetstatedata.cpp widgetstateengine.cpp mconfig.cpp
      )

set_target_properties( mstyle
   PROPERTIES COMPILE_FLAGS
      "-include ${PROJECT_BINARY_DIR}/all.h"
   )
