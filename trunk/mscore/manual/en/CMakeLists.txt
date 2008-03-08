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

add_custom_command (
      OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/man-en.pdf
      DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/man-en.tex
      COMMAND ${TEXEXEC_PATH}/texexec
      ARGS  --language=en --verbose --batch --pdf
            --path=${CMAKE_CURRENT_SOURCE_DIR}
            --result=man-en.pdf
            ${CMAKE_CURRENT_SOURCE_DIR}/man-en.tex
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      )

add_custom_target ( man-en
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/man-en.pdf
      )

add_dependencies (man-en man-scores)

install (
      FILES
            ${CMAKE_CURRENT_BINARY_DIR}/man-en.pdf
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}doc/
      OPTIONAL
      )

set (extraClean man-en.log man-en.tmp man-en.tuo man-en.tui
   man-en-mpgraph.mp mpgraph.mp )

set_directory_properties( PROPERTIES
   ADDITIONAL_MAKE_CLEAN_FILES "${extraClean}"
   )

configure_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/figurepath.tex.in
   ${CMAKE_CURRENT_BINARY_DIR}/figurepath.tex
   )

