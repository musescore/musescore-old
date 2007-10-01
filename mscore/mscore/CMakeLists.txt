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
      insertmeasuresdialog.ui barline.ui chord.ui chordrest.ui editinstrument.ui editstyle.ui
      edittempo.ui element.ui hairpin.ui instrdialog.ui measure.ui measuresdialog.ui
      note.ui page.ui pagesettings.ui partedit.ui playpanel.ui prefsdialog.ui
      irregular.ui segment.ui text.ui textpalette.ui textstyle.ui timedialog.ui
      symboldialog.ui dynamic.ui tuplet.ui shortcutcapturedialog.ui slurtie.ui
      slur.ui editdrumset.ui editstaff.ui line.ui linesegment.ui
      )

QT4_WRAP_CPP (mocs
      canvas.h editinstrument.h editstyle.h edittempo.h instrdialog.h listedit.h
      mscore.h navigator.h pad.h pagesettings.h palette.h partedit.h playpanel.h
      preferences.h irregular.h preview.h score.h seq.h textpalette.h textstyle.h
      timedialog.h symboldialog.h shortcutcapturedialog.h simplebutton.h
      greendotbutton.h recordbutton.h editdrumset.h editstaff.h
      )

QT4_ADD_RESOURCES (qrc_files mscore.qrc)

if (MINGW)
      set (resource_file ${PROJECT_BINARY_DIR}/resfile.o)
      set (AUDIO pa.cpp)
else (MINGW)
      if (USE_PORTAUDIO)
            set (AUDIO jackaudio.cpp alsa.cpp pa.cpp)
      else (USE_PORTAUDIO)
            set (AUDIO jackaudio.cpp alsa.cpp)
      endif (USE_PORTAUDIO)
endif (MINGW)

add_executable ( mscore
      ${qrc_files}
      ${ui_headers}
      ${mocs}
      ${PROJECT_BINARY_DIR}/all.h
      ${PROJECT_BINARY_DIR}/all.h.pch
      ${resource_file}

      actions.cpp accidental.cpp barline.cpp beam.cpp
      bracket.cpp canvas.cpp chord.cpp chordrest.cpp clef.cpp
      cmd.cpp dynamics.cpp edit.cpp editinstrument.cpp editstyle.cpp
      edittempo.cpp element.cpp exportxml.cpp file.cpp fluid.cpp
      hairpin.cpp icons.cpp importxml.cpp instrdialog.cpp
      key.cpp keyb.cpp layout.cpp layoutbreak.cpp line.cpp listedit.cpp
      measure.cpp menus.cpp midifile.cpp midi.cpp mscore.cpp
      mtime.cpp navigate.cpp navigator.cpp note.cpp ottava.cpp
      pad.cpp page.cpp pagesettings.cpp palette.cpp part.cpp
      partedit.cpp pedal.cpp playpanel.cpp preferences.cpp irregular.cpp
      preview.cpp rest.cpp score.cpp segment.cpp select.cpp
      seq.cpp sig.cpp slur.cpp staff.cpp style.cpp
      sym.cpp sym.h symbol.cpp symbol.h hook.cpp hook.h
      system.cpp tempo.cpp text.cpp textpalette.cpp textstyle.cpp
      timedialog.cpp symboldialog.cpp trill.cpp tuplet.cpp shortcutcapturedialog.cpp
      undo.cpp utils.cpp xml.cpp timesig.cpp lyrics.cpp bsp.cpp
      viewer.cpp volta.cpp simplebutton.cpp image.cpp keyfinder.cpp
      pitchspelling.cpp musedata.cpp keysig.cpp arpeggio.cpp breath.cpp
      editdrumset.cpp editstaff.cpp drumset.cpp tremolo.cpp repeat.cpp
      repeat2.cpp lilypond.cpp exportly.cpp
      ${AUDIO}
      )

if (MINGW)
   add_custom_command(
      OUTPUT ${PROJECT_BINARY_DIR}/resfile.o
      COMMAND wine windres -o ${PROJECT_BINARY_DIR}/resfile.o mscore.rc
      DEPENDS ${PROJECT_SOURCE_DIR}/mscore/data/mscore.rc
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/mscore/data
      )
   set_source_files_properties(
      ${PROJECT_BINARY_DIR}/resfile.o
      PROPERTIES generated true
      )
   # Windows: add -mconsole to get a console window for debug output
   set_target_properties( mscore
      PROPERTIES
         COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h ${QT_DEFINITIONS} -DQT_SVG_LIB -DQT_GUI_LIB -DQT_XML_LIB -DQT_CORE_LIB"
         LINK_FLAGS "${PROJECT_BINARY_DIR}/resfile.o -mwindows -L /home/ws/.wine/drive_c/Qt/4.3.1/lib -L /home/ws/mscore/portaudio/lib"
      )
   target_link_libraries(mscore
      fluid
      awl
      ${QT_mingw_LIBRARIES}
      portaudio
      )
   install_targets ( /bin mscore )
   install_files ( /bin .dll
      /home/ws/.wine/drive_c/Qt/4.3.1/bin/mingwm10.dll
      /home/ws/.wine/drive_c/Qt/4.3.1/bin/QtCore4.dll
      /home/ws/.wine/drive_c/Qt/4.3.1/bin/QtGui4.dll
      /home/ws/.wine/drive_c/Qt/4.3.1/bin/QtXml4.dll
      /home/ws/.wine/drive_c/Qt/4.3.1/bin/QtSvg4.dll
      /home/ws/mscore/portaudio/bin/portaudio.dll
      )
   install_files ( /bin/iconengines .dll
      /home/ws/.wine/drive_c/Qt/4.3.1/plugins/iconengines/qsvg4.dll
      )
   install_files ( /bin/imageformats .dll
      /home/ws/.wine/drive_c/Qt/4.3.1/plugins/imageformats/qjpeg4.dll
      /home/ws/.wine/drive_c/Qt/4.3.1/plugins/imageformats/qmng4.dll
      /home/ws/.wine/drive_c/Qt/4.3.1/plugins/imageformats/qsvg4.dll
      /home/ws/.wine/drive_c/Qt/4.3.1/plugins/imageformats/qtiff4.dll
      )

else (MINGW)
   set_target_properties( mscore
      PROPERTIES
         COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h ${QT_DEFINITIONS}"
      )
   target_link_libraries(mscore
      ${QT_LIBRARIES}
      ${ALSA_LIB}
      ${JACK_LIB}
      ${PORTAUDIO_LIB}
      fluid
      awl
      )
   install( TARGETS mscore RUNTIME DESTINATION bin )
   install( FILES   data/mscore.png DESTINATION /usr/share/icons/hicolor/64x64/apps)

endif (MINGW)

#======================================
#     target smf2xml
#======================================

if (NOT MINGW)
      add_executable ( smf2xml smf2xml.cpp midifile.cpp xml.cpp sig.cpp drumset.cpp)

      target_link_libraries( smf2xml
            ${QT_LIBRARIES}
            )
      set_target_properties( smf2xml
            PROPERTIES COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h"
            )
endif (NOT MINGW)

#======================================
#     target xml2smf
#======================================

if (NOT MINGW)
      add_executable ( xml2smf xml2smf.cpp midifile.cpp xml.cpp sig.cpp drumset.cpp)

      target_link_libraries( xml2smf
            ${QT_LIBRARIES}
            )
      set_target_properties( xml2smf
            PROPERTIES COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h"
            )
endif (NOT MINGW)

