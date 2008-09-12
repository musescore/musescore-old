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

#----------------------------------------------------------
#     mscore online doc
#----------------------------------------------------------

if (MINGW)
      set (GENDOC_UTIL "/home/ws/mscore/mscore/trunk/build/gendoc/gendoc")
else (MINGW)
      set (GENDOC_UTIL "${CMAKE_BINARY_DIR}/gendoc/gendoc")
endif (MINGW)

add_custom_command(
      OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/online-doc
      COMMAND rm -rf man/de man/en man/es
      COMMAND mkdir man/de man/en man/es
      COMMAND ${GENDOC_UTIL} -p -s ${CMAKE_SOURCE_DIR}/onlinedoc
      COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/online-doc
      DEPENDS doc-de.xml doc-en.xml doc-es.xml
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      )

add_custom_target(onlinedoc ALL
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/online-doc
      )

add_dependencies(onlinedoc gendoc)


add_dependencies(gendoc)


#----------------------------------------------------------
#     install mscore online doc
#----------------------------------------------------------

install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/man
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}
      PATTERN ".svn" EXCLUDE
      )

install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/pic
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}man
      PATTERN ".svn" EXCLUDE
      )

