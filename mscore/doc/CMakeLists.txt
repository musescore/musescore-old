#=============================================================================
#  Mscore
#  Linux Music Score Editor
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

add_custom_command(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/promenade.pdf
   COMMAND mscore ${PROJECT_SOURCE_DIR}/demos/promenade.msc -o promenade.pdf
   DEPENDS ${PROJECT_SOURCE_DIR}/demos/promenade.msc
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   )

add_custom_command(
   OUTPUT inv1-1.pdf inv1-2.pdf
   COMMAND mscore ${PROJECT_SOURCE_DIR}/demos/inv1.msc -o inv1.pdf
   COMMAND pdftk inv1.pdf burst output inv1-%d.pdf
   DEPENDS ${PROJECT_SOURCE_DIR}/demos/inv1.msc
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   )

add_custom_command(
   OUTPUT inv6-1.pdf inv6-2.pdf
   COMMAND mscore ${PROJECT_SOURCE_DIR}/demos/inv6.msc -o inv6.pdf
   COMMAND pdftk inv6.pdf burst output inv6-%d.pdf
   DEPENDS ${PROJECT_SOURCE_DIR}/demos/inv6.msc
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   )

add_custom_target ( man
      DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/promenade.pdf
      ${CMAKE_CURRENT_BINARY_DIR}/inv1-1.pdf
      ${CMAKE_CURRENT_BINARY_DIR}/inv1-2.pdf
      ${CMAKE_CURRENT_BINARY_DIR}/inv6-1.pdf
      ${CMAKE_CURRENT_BINARY_DIR}/inv6-2.pdf
      )

if (TEXEXEC_PATH)
      subdirs( de en )
endif (TEXEXEC_PATH)
