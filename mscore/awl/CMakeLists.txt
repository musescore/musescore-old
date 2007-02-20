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

QT4_WRAP_CPP (mocs
      aslider.h
      knob.h
      panknob.h
      volknob.h
      midipanknob.h
      )

set(awl_src
      awl.cpp utils.cpp  ${mocCPP} ${moc_headers}
      )

add_library (
      awl STATIC
      ${mocs}
      aslider.cpp
      knob.cpp
      panknob.cpp
      volknob.cpp
      midipanknob.cpp
      )

set_target_properties( awl
   PROPERTIES COMPILE_FLAGS
      "-include ${PROJECT_BINARY_DIR}/mscore/all.h"
   )

QT4_WRAP_CPP (mocs2
      aslider.h
      knob.h
      panknob.h
      volknob.h
      midipanknob.h
      awlplugin.h
      )

add_library (
      awlplugin SHARED
      ${mocs2}
      awlplugin.cpp
      aslider.cpp
      knob.cpp
      panknob.cpp
      volknob.cpp
      midipanknob.cpp
      )

set_target_properties( awlplugin
      PROPERTIES COMPILE_FLAGS
      "-D_GNU_SOURCE -D_REENTRANT -DHAVE_CONFIG_H -DQT_PLUGIN -DQT_SHARED -DQT_NO_DEBUG -include ${PROJECT_SOURCE_DIR}/mscore/all.h"
      )

target_link_libraries( awlplugin
      ""
      )

