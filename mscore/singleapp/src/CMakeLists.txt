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

include (${PROJECT_SOURCE_DIR}/cmake/gch.cmake)

QT4_WRAP_CPP (mocs
      qtsingleapplication.h
      qtlocalpeer.h
      )

add_library (
  qtsingleapp STATIC
  ${PROJECT_BINARY_DIR}/all.h
  ${PCH}
  ${mocs}
  qtsingleapplication.cpp
  qtlocalpeer.cpp
  )

ADD_DEPENDENCIES(qtsingleapp mops1)
ADD_DEPENDENCIES(qtsingleapp mops2)
