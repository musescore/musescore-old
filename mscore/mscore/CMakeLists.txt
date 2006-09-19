#=============================================================================
#  MuseScore
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

add_custom_target(all.h.pch
   ALL
   COMMAND cp ${PROJECT_SOURCE_DIR}/mscore/all.h ${PROJECT_BINARY_DIR}/mscore/all.h
   COMMAND ${CMAKE_CXX_COMPILER} 
     -g -Wextra -fno-exceptions -Wall
     -I${QT_INCLUDE_DIR}
     -I${QT_QTCORE_INCLUDE_DIR}
     -I${QT_QTGUI_INCLUDE_DIR}
     -o all.h.pch all.h
   DEPENDS ${PROJECT_SOURCE_DIR}/mscore/all.h
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/mscore
   )

set(CMAKE_CXX_FLAGS 
   "-g -Wall -Wextra -Winvalid-pch -fno-exceptions"
   )

include_directories(
      ${CMAKE_CURRENT_BINARY_DIR}
      )

QT4_WRAP_UI (ui_headers
      barline.ui
      chord.ui
      chordrest.ui
      editinstrument.ui
      editstyle.ui
      edittempo.ui
      element.ui
      hairpin.ui
      instrdialog.ui
      measure.ui
      measuresdialog.ui
      note.ui
      pad.ui
      page.ui
      pagesettings.ui
      partedit.ui
      playpanel.ui
      prefsdialog.ui
      segment.ui
      text.ui
      textpalette.ui
      textstyle.ui
      )

QT4_WRAP_CPP (mocs
      canvas.h
      editinstrument.h
      editstyle.h
      edittempo.h
      instrdialog.h
      listedit.h
      mscore.h
      navigator.h
      pad.h
      pagesettings.h
      palette.h
      partedit.h
      playpanel.h
      preferences.h
      preview.h
      score.h
      seq.h
      textpalette.h
      textstyle.h
      )

QT4_ADD_RESOURCES (qrc_files mscore.qrc)

add_executable ( mscore
      ${qrc_files}
      ${ui_headers}
      ${mocs}

#      barlinebase.cpp
#      chordbase.cpp
#      chordrestbase.cpp
#      editinstrumentbase.cpp
#      editstylebase.cpp
#      edittempobase.cpp
#      elementbase.cpp
#      hairpinbase.cpp
#      instrdialogbase.cpp
#      measurebase.cpp
#      measuresdialogbase.cpp
#      notebase.cpp
#      padbase.cpp
#      pagebase.cpp
#      pagesettingsbase.cpp
#      parteditbase.cpp
#      playpanelbase.cpp
#      prefsdialogbase.cpp
#      segmentbase.cpp
#      textbase.cpp
#      textpalettebase.cpp
#      textstylebase.cpp

      alsa.cpp
      barline.cpp
      beam.cpp
      binreloc.cpp
      bracket.cpp
      canvas.cpp
      chord.cpp
      chordrest.cpp
      clef.cpp
      cmd.cpp
      dynamics.cpp
      edit.cpp
      editinstrument.cpp
      editstyle.cpp
      edittempo.cpp
      element.cpp
      exportxml.cpp
      file.cpp
      fluid.cpp
      hairpin.cpp
      icons.cpp
      importxml.cpp
      instrdialog.cpp
      jackaudio.cpp
      key.cpp
      keyb.cpp
      layout.cpp
      layoutbreak.cpp
      line.cpp
      listedit.cpp
      measure.cpp
      menus.cpp
      midi.cpp
      mscore.cpp
      mtime.cpp
      navigate.cpp
      navigator.cpp
      note.cpp
      ottava.cpp
      pad.cpp
      page.cpp
      pagesettings.cpp
      palette.cpp
      part.cpp
      partedit.cpp
      pedal.cpp
      playpanel.cpp
      preferences.cpp
      preview.cpp
      rest.cpp
      score.cpp
      segment.cpp
      select.cpp
      seq.cpp
      sig.cpp
      slur.cpp
      staff.cpp
      style.cpp
      symbols.cpp
      system.cpp
      tempo.cpp
      text.cpp
      textelement.cpp
      textpalette.cpp
      textstyle.cpp
      trill.cpp
      tuplet.cpp
      undo.cpp
      utils.cpp
      xml.cpp
      timesig.cpp
      )

set_target_properties( mscore
      PROPERTIES COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/mscore/all.h"
      )

target_link_libraries(mscore
      ${QT_LIBRARIES}
      ${ALSA_LIB}
      ${JACK_LIB}
      )

install_targets ( /bin mscore )
