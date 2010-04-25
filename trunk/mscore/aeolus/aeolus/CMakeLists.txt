#=============================================================================
#  Mscore
#  Linux Music Score Editor
#  $Id:$
#
#  Copyright (C) 2010 by Werner Schweer and others
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

set(CMAKE_CXX_FLAGS         "-include all.h -g -Wall -Wextra -Winvalid-pch -DVERSION=\"\\\"0.8.4\\\"\" -D_REENTRANT")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DQT_NO_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "-DQT_DEBUG")

add_library (aeolus STATIC
      aeolus.cpp audio.cpp model.cpp slave.cpp addsynth.cpp scales.cpp
	reverb.cpp asection.cpp division.cpp rankwave.cpp rngen.cpp exp2ap.cpp
      lfqueue.cpp
      a_thread.cpp itc_ctrl.cpp itc_ip1q.cpp itc_mesg.cpp p_thread.cpp textmsg.cpp
      tiface.cpp
  )

set_target_properties( fluid
   PROPERTIES COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h"
   )

