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

set(CMAKE_CXX_FLAGS         "-g -Wall -Wextra -Winvalid-pch -fno-exceptions")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DQT_NO_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "-DQT_DEBUG")

add_custom_command(
   OUTPUT ${PROJECT_BINARY_DIR}/all.h
   COMMAND cp ${PROJECT_SOURCE_DIR}/all.h ${PROJECT_BINARY_DIR}/all.h
   DEPENDS ${PROJECT_SOURCE_DIR}/all.h
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
   )

add_custom_command(
   OUTPUT ${PROJECT_BINARY_DIR}/all.h.pch
   COMMAND ${CMAKE_CXX_COMPILER}
     -g -Wextra -fno-exceptions -Wall
     -I${QT_INCLUDE_DIR}
     -I${QT_QTCORE_INCLUDE_DIR}
     -I${QT_QTGUI_INCLUDE_DIR}
     -o all.h.pch all.h
   DEPENDS ${PROJECT_BINARY_DIR}/all.h
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
   )

set_source_files_properties(
      ${PROJECT_BINARY_DIR}/all.h
      ${PROJECT_BINARY_DIR}/all.h.pch
      PROPERTIES generated true
      )

include_directories(
      ${CMAKE_CURRENT_BINARY_DIR}
      ${PROJECT_SOURCE_DIR}/fluid
      ${PROJECT_SOURCE_DIR}/awl
      ${PROJECT_SOURCE_DIR}
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
      page.ui
      pagesettings.ui
      partedit.ui
      playpanel.ui
      prefsdialog.ui
      irregular.ui
      segment.ui
      text.ui
      textpalette.ui
      textstyle.ui
      timedialog.ui
      symboldialog.ui
      dynamic.ui
      tuplet.ui
      shortcutcapturedialog.ui
      slurtie.ui
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
      irregular.h
      preview.h
      score.h
      seq.h
      textpalette.h
      textstyle.h
      timedialog.h
      symboldialog.h
      shortcutcapturedialog.h
      simplebutton.h greendotbutton.h recordbutton.h
      )

QT4_ADD_RESOURCES (qrc_files mscore.qrc)

add_executable ( mscore
      ${qrc_files}
      ${ui_headers}
      ${mocs}
      ${PROJECT_BINARY_DIR}/all.h
      ${PROJECT_BINARY_DIR}/all.h.pch

      actions.cpp
      accidental.cpp
      alsa.cpp
      barline.cpp
      beam.cpp
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
      midifile.cpp
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
      irregular.cpp
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
      sym.cpp sym.h
      symbol.cpp symbol.h
      hook.cpp hook.h
      system.cpp
      tempo.cpp
      text.cpp
      textpalette.cpp
      textstyle.cpp
      timedialog.cpp
      symboldialog.cpp
      trill.cpp
      tuplet.cpp
      shortcutcapturedialog.cpp
      undo.cpp
      utils.cpp
      xml.cpp
      timesig.cpp
      lyrics.cpp
      bsp.cpp
      viewer.cpp
      volta.cpp
      simplebutton.cpp
      image.cpp
      )

if (CROSS_MINGW)
   set_target_properties( mscore
      PROPERTIES
         COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h"
         LINK_FLAGS "-L /home/ws/.wine/drive_c/MinGW/lib"
      )
else (CROSS_MINGW)
   set_target_properties( mscore
      PROPERTIES
         COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h"
      )
endif (CROSS_MINGW)

target_link_libraries(mscore
      ${QT_LIBRARIES}
      ${ALSA_LIB}
      ${JACK_LIB}
      fluid
      awl
      )

#======================================
#     target smf2xml
#======================================

add_executable ( smf2xml smf2xml.cpp midifile.cpp xml.cpp)

target_link_libraries( smf2xml
      ${QT_LIBRARIES}
      )
set_target_properties( smf2xml
      PROPERTIES COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h"
      )

install_targets ( /bin mscore )

