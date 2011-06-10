#//=============================================================================
#//  MuseScore
#//  Music Score Editor/Player
#//  $Id:$
#//
#//  Copyright (C) 2011 Werner Schweer
#//
#//  This program is free software; you can redistribute it and/or modify
#//  it under the terms of the GNU General Public License version 2.
#//
#//  This program is distributed in the hope that it will be useful,
#//  but WITHOUT ANY WARRANTY; without even the implied warranty of
#//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#//  GNU General Public License for more details.
#//
#//  You should have received a copy of the GNU General Public License
#//  along with this program; if not, write to the Free Software
#//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#//=============================================================================

include (${PROJECT_SOURCE_DIR}/cmake/gch.cmake)

add_library(libmscore STATIC
      ${PROJECT_BINARY_DIR}/all.h
      ${PCH}
      accidental.cpp arpeggio.cpp articulation.cpp barline.cpp beam.cpp bend.cpp
      box.cpp bracket.cpp breath.cpp bsp.cpp chord.cpp chordline.cpp chordrest.cpp
      clef.cpp cursor.cpp drumset.cpp durationtype.cpp dynamics.cpp element.cpp
      elementlayout.cpp file.cpp fingering.cpp font.cpp fret.cpp glissando.cpp
      hairpin.cpp harmony.cpp hook.cpp image.cpp instrchange.cpp instrument.cpp
      interval.cpp key.cpp keysig.cpp layoutbreak.cpp layout.cpp line.cpp
      lyrics.cpp measurebase.cpp measure.cpp navigate.cpp note.cpp noteevent.cpp
      ottava.cpp page.cpp pagesettings.cpp part.cpp pedal.cpp pitch.cpp
      pitchspelling.cpp preferences.cpp rendermidi.cpp repeat.cpp repeatlist.cpp
      rest.cpp score.cpp segment.cpp slur.cpp
      spacer.cpp spanner.cpp staff.cpp staffstate.cpp stafftext.cpp stafftype.cpp
      stubs.cpp style.cpp symbol.cpp sym.cpp system.cpp tablature.cpp tempotext.cpp
      text.cpp textframe.cpp textline.cpp timesig.cpp tremolobar.cpp tremolo.cpp
      trill.cpp tuplet.cpp utils.cpp velo.cpp volta.cpp fifo.cpp
      )

set_target_properties(libmscore PROPERTIES
   COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h -g -Wall -Wextra -Winvalid-pch"
   )

