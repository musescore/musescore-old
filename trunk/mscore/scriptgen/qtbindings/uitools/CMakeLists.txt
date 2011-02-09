#=============================================================================
#  MusE
#  Linux Music Editor
#  $Id:$
#
#  Copyright (C) 2002-2008 by Werner Schweer and others
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

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DQT_NO_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "-DQT_DEBUG")

if (STATIC_SCRIPT_BINDINGS)
      set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Winvalid-pch -DQT_STATICPLUGIN")
      set(LIBTYPE "STATIC")
else (STATIC_SCRIPT_BINDINGS)
      set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Winvalid-pch")
      set(LIBTYPE "SHARED")
endif (STATIC_SCRIPT_BINDINGS)

if (GCC_VISIBILITY)
      set(CMAKE_CXX_FLAGS, ${CMAKE_CXX_FLAGS} "-fvisibility=hidden -fvisibility-inlines-hidden")
endif (GCC_VISIBILITY)

include_directories(${QT_INCLUDES})

add_library (
      qtscript_uitools ${LIBTYPE}
      main.cpp
      qtscript_QUiLoader.cpp
      qtscriptshell_QUiLoader.cpp
      )

set_target_properties(qtscript_uitools
      PROPERTIES
      COMPILE_FLAGS
         "-D_GNU_SOURCE -D_REENTRANT -DHAVE_CONFIG_H -DQT_PLUGIN -DQT_NO_DEBUG"
      LINK_FLAGS "-L${QT_LIBRARY_DIR}"
      )

if (STATIC_SCRIPT_BINDINGS)
else (STATIC_SCRIPT_BINDINGS)
   if (MINGW)
      target_link_libraries(
         qtscript_uitools
         ${QT_mingw_LIBRARIES}
         )
      install( TARGETS qtscript_uitools DESTINATION bin/script/ )
   else(MINGW)
      target_link_libraries(
         qtscript_uitools
         QtUiTools
         )
      install( TARGETS qtscript_uitools DESTINATION ${QT_PLUGINS_DIR}/script/ )
   endif(MINGW)
endif (STATIC_SCRIPT_BINDINGS)


