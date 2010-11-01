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

install(FILES
      leadsheet.mss
      chords.xml
      stdchords.xml
      jazzchords.xml
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}styles
      )

if (APPLE)

install(FILES
      appstyle-mac.css
      RENAME appstyle.css
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}styles
      )

install(FILES
      appstyle-mac-dark.css
      RENAME appstyle-dark.css
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}styles
      )

else (APPLE)

install(FILES
      appstyle.css
      appstyle-dark.css
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}styles
      )

endif (APPLE)


