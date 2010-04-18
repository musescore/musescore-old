#=============================================================================
#  Mscore
#  Linux Music Score Editor
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

if (GCC_VISIBILITY)
set(CMAKE_CXX_FLAGS "-fexceptions -g -Wall -Wextra -Winvalid-pch -fvisibility=hidden -fvisibility-inlines-hidden")
else (GCC_VISIBILITY)
set(CMAKE_CXX_FLAGS "-fexceptions -g -Wall -Wextra -Winvalid-pch")
endif (GCC_VISIBILITY)

QT4_WRAP_CPP (mocs
      qtsingleapplication.h
      qtlocalpeer.h
      )

add_library (
  qtsingleapp STATIC
  ${mocs}
  qtsingleapplication.cpp
  qtlocalpeer.cpp
  )

set_target_properties( qtsingleapp
   PROPERTIES COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h"
   )

