#=============================================================================
#  MuseSynth
#  Music Software Synthesizer
#  $Id:$
#
#  Copyright (C) 2011 Werner Schweer
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2
#  as published by the Free Software Foundation and appearing in
#  the file LICENSE.GPL
#=============================================================================

add_library(msynth2
      msynth.cpp voice.cpp channel.cpp instrument.cpp
      )

add_executable(mstest
      ms.cpp
      )

target_link_libraries(mstest
      ${QT_LIBRARIES}
      jack
      sndfile
      msynth2
      )

