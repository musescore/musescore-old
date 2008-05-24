#=============================================================================
#  MusE
#  Linux Music Editor
#  $Id:$
#
#  Copyright (C) 2002-2006 by Werner Schweer and others
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

set(CMAKE_CXX_FLAGS         "-g -Wall -Wextra -Winvalid-pch -fno-exceptions -fno-rtti")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DQT_NO_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "-DQT_DEBUG")

QT4_WRAP_CPP (mocs
      )

add_library (
      qtscript_uitools SHARED
      ${mocs}
      main.cpp
      qtscript_QUiLoader.cpp
      qtscriptshell_QUiLoader.cpp
      )

set_target_properties(qtscript_uitools
      PROPERTIES
      COMPILE_FLAGS
         "-D_GNU_SOURCE -D_REENTRANT -DHAVE_CONFIG_H -DQT_PLUGIN -DQT_SHARED -DQT_NO_DEBUG -include ${PROJECT_SOURCE_DIR}/all.h"
      )

target_link_libraries(qtscript_uitools
      QtUiTools
      )


install( TARGETS qtscript_uitools DESTINATION ${QT_PLUGINS_DIR}/script/ )

