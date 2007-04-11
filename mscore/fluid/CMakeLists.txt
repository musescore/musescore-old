#=============================================================================
#  Mscore
#  Linux Music Score Editor
#  $Id:$
#
#  Copyright (C) 2007 by Werner Schweer and others
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

add_library (fluid
  synth.cpp
  settings.cpp
  voice.cpp
  chan.cpp
  defsfont.cpp
  chorus.cpp
  hash.cpp
  list.cpp
  ramsfont.cpp
  conv.cpp
  gen.cpp
  mod.cpp
  rev.cpp
  tuning.cpp
  sys.cpp
  )

set_target_properties( fluid
   PROPERTIES COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h"
   )

