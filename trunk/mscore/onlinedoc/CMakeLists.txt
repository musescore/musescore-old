#=============================================================================
#  Mscore
#  Linux Music Score Editor
#  $Id:$
#
#  Copyright (C) 2008 by Werner Schweer and others
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
      OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/man
      COMMAND rm -rf man
      COMMAND mkdir man man/de man/en
      COMMAND ${CMAKE_BINARY_DIR}/gendoc/gendoc -p -s ${CMAKE_SOURCE_DIR}/onlinedoc
      DEPENDS doc-de.xml doc-en.xml
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      )

add_custom_target(onlinedoc ALL
      COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/onlinedoc
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/man
      )

add_dependencies(onlinedoc gendoc)


install(
      DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/man
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}
      PATTERN ".svn" EXCLUDE
      )

install(
      DIRECTORY ${CMAKE_SOURCE_DIR}/../web/pic
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}/man
      PATTERN ".svn" EXCLUDE
      )

