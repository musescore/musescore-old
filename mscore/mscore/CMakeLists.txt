#=============================================================================
#  MuseScore
#  Linux Music Score Editor
#  $Id:$
#
#  Copyright (C) 2002-2007 by Werner Schweer and others
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

set(CMAKE_CXX_FLAGS         "-g -Wall -Wextra -Winvalid-pch -fno-exceptions -fno-rtti")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DQT_NO_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "-DQT_DEBUG")

add_custom_target(update_revision_target
  ${CMAKE_CURRENT_SOURCE_DIR}/updaterevision
  COMMENT "Updating revision.h" VERBATIM)

SET_SOURCE_FILES_PROPERTIES(revsion.h PROPERTIES GENERATED TRUE)

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
      ${PROJECT_SOURCE_DIR}/awl
      ${PROJECT_SOURCE_DIR}/scriptdebug
      ${PROJECT_SOURCE_DIR}/osdabzip
      ${PROJECT_SOURCE_DIR}
      )
if (NOT USE_GLOBAL_FLUID)
      include_directories(${PROJECT_SOURCE_DIR}/fluid)
endif (NOT USE_GLOBAL_FLUID)


QT4_WRAP_UI (ui_headers
      insertmeasuresdialog.ui barline.ui chord.ui chordrest.ui editinstrument.ui editstyle.ui
      edittempo.ui element.ui hairpin.ui instrdialog.ui measure.ui measuresdialog.ui
      note.ui page.ui pagesettings.ui partedit.ui playpanel.ui prefsdialog.ui
      measureproperties.ui segment.ui text.ui textpalette.ui textstyle.ui timedialog.ui
      symboldialog.ui dynamic.ui tuplet.ui shortcutcapturedialog.ui slurtie.ui
      slur.ui editdrumset.ui editstaff.ui line.ui linesegment.ui
      voltaproperties.ui chordproperties.ui restproperties.ui repeatproperties.ui
      jumpproperties.ui markerproperties.ui boxproperties.ui
      instrwizard.ui timesigwizard.ui newwizard.ui aboutbox.ui
      chordedit.ui transposedialog.ui tempoproperties.ui importmidi.ui
      lineproperties.ui excerptsdialog.ui lyrics.ui stafftext.ui
      imageproperties.ui tupletdialog.ui tupletproperties.ui
      glissandoprop.ui
      )

QT4_WRAP_CPP (mocs
      canvas.h editinstrument.h editstyle.h edittempo.h instrdialog.h listedit.h
      mscore.h navigator.h pagesettings.h palette.h partedit.h playpanel.h
      preferences.h measureproperties.h preview.h score.h seq.h textpalette.h textstyle.h
      timedialog.h symboldialog.h shortcutcapturedialog.h simplebutton.h
      greendotbutton.h recordbutton.h editdrumset.h editstaff.h
      voltaproperties.h chordproperties.h restproperties.h repeatproperties.h
      jumpproperties.h markerproperties.h boxproperties.h newwizard.h
      transposedialog.h chordedit.h tempotext.h importmidi.h textline.h
      excerptsdialog.h stafftext.h image.h tuplet.h glissando.h
      script.h
      )

QT4_ADD_RESOURCES (qrc_files mscore.qrc)

if (MINGW)
      set (resource_file ${PROJECT_BINARY_DIR}/resfile.o)
      set (AUDIO pa.cpp)
else (MINGW)
      set (AUDIO "")
      if (USE_PORTAUDIO)
            set (AUDIO ${AUDIO} pa.cpp)
      endif (USE_PORTAUDIO)
      if (USE_JACK)
            set (AUDIO ${AUDIO} jackaudio.cpp)
      endif (USE_JACK)
      if (USE_ALSA)
            set (AUDIO ${AUDIO} alsa.cpp midiseq.cpp rtctimer.cpp
               thread.cpp mididriver.cpp)
      endif (USE_ALSA)
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
      measure.cpp menus.cpp midifile.cpp importmidi.cpp mscore.cpp
      navigate.cpp navigator.cpp note.cpp ottava.cpp
      page.cpp pagesettings.cpp palette.cpp part.cpp
      partedit.cpp pedal.cpp playpanel.cpp preferences.cpp measureproperties.cpp
      preview.cpp rest.cpp score.cpp segment.cpp select.cpp
      seq.cpp sig.cpp slur.cpp staff.cpp style.cpp
      sym.cpp sym.h symbol.cpp symbol.h hook.cpp hook.h
      system.cpp tempo.cpp text.cpp textline.cpp textpalette.cpp textstyle.cpp
      timedialog.cpp symboldialog.cpp trill.cpp tuplet.cpp shortcutcapturedialog.cpp
      undo.cpp utils.cpp xml.cpp timesig.cpp lyrics.cpp bsp.cpp
      viewer.cpp volta.cpp simplebutton.cpp image.cpp keyfinder.cpp
      pitchspelling.cpp musedata.cpp keysig.cpp arpeggio.cpp breath.cpp
      glissando.cpp
      editdrumset.cpp editstaff.cpp drumset.cpp tremolo.cpp repeat.cpp
      lilypond.cpp exportly.cpp repeatflag.cpp
      voltaproperties.cpp chordproperties.cpp restproperties.cpp
      repeatproperties.cpp box.cpp measurebase.cpp
      instrtemplate.cpp boxproperties.cpp
      newwizard.cpp revision.cpp transposedialog.cpp bb.cpp
      harmony.cpp chordedit.cpp plugins.cpp tempotext.cpp
      excerptsdialog.cpp excerpt.cpp stafftext.cpp instrument.cpp durationtype.cpp
      event.cpp rendermidi.cpp
      ${AUDIO}
      )

if (MINGW)
   set(QT4 "/home/ws/.wine/drive_c/Qt/4.4.0")

   add_custom_command(
      OUTPUT ${PROJECT_BINARY_DIR}/resfile.o
      COMMAND wrc -i mscore.rc -o ${PROJECT_BINARY_DIR}/mscore.res
      COMMAND wine /home/ws/.wine/drive_c/MingW/bin/windres.exe ${PROJECT_BINARY_DIR}/mscore.res -o ${PROJECT_BINARY_DIR}/resfile.o
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
         COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h ${QT_DEFINITIONS} -DQT_SVG_LIB -DQT_GUI_LIB -DQT_XML_LIB -DQT_CORE_LIB -DQT_SCRIPT_LIB"
         LINK_FLAGS "-Wl,-S ${PROJECT_BINARY_DIR}/resfile.o -mwindows -L ${QT4}/lib -L /home/ws/mscore/portaudio/lib"
      )
   target_link_libraries(mscore
      fluid
      awl
      osdabzip
      ${QT_mingw_LIBRARIES}
      portaudio
      )
   if (BUILD_SCRIPT_DEBUG)
         target_link_libraries(mscore qtscript_debug)
   endif (BUILD_SCRIPT_DEBUG)

   install(TARGETS mscore RUNTIME DESTINATION bin)

   install_files ( /bin .dll
      ${QT4}/bin/mingwm10.dll
      ${QT4}/bin/QtCore4.dll
      ${QT4}/bin/QtGui4.dll
      ${QT4}/bin/QtXml4.dll
      ${QT4}/bin/QtSvg4.dll
      ${QT4}/bin/QtScript4.dll
      ${QT4}/bin/QtNetwork4.dll
      /home/ws/mscore/portaudio/bin/portaudio.dll
      )
   install_files ( /bin/iconengines .dll
      /home/ws/.wine/drive_c/Qt/4.4.0/plugins/iconengines/qsvgicon4.dll
      )
   install_files ( /bin/imageformats .dll
      ${QT4}/plugins/imageformats/qjpeg4.dll
      ${QT4}/plugins/imageformats/qmng4.dll
      ${QT4}/plugins/imageformats/qsvg4.dll
      ${QT4}/plugins/imageformats/qtiff4.dll
      )

else (MINGW)
   set_target_properties( mscore
      PROPERTIES
         COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h ${QT_DEFINITIONS}"
      )
   target_link_libraries(mscore
      ${QT_LIBRARIES}
      ${QT_QTSCRIPT_LIBRARY_RELEASE}
      ${ALSA_LIB}
      ${JACK_LIB}
      ${PORTAUDIO_LIB}
      awl
      osdabzip
      )
   if (USE_GLOBAL_FLUID)
         target_link_libraries(mscore ${FLUID_LIB} )
   else (USE_GLOBAL_FLUID)
         target_link_libraries(mscore fluid )
   endif (USE_GLOBAL_FLUID)

   if (BUILD_SCRIPT_DEBUG)
         target_link_libraries(mscore qtscript_debug)
   endif (BUILD_SCRIPT_DEBUG)


   install( TARGETS mscore RUNTIME DESTINATION bin )
   install( FILES   data/mscore.png DESTINATION share/pixmaps)
   install( FILES   data/mscore.xpm DESTINATION share/pixmaps)

endif (MINGW)

#======================================
#     target smf2xml
#======================================

if (NOT MINGW)
      add_executable(
            smf2xml smf2xml.cpp midifile.cpp event.cpp xml.cpp sig.cpp drumset.cpp
            )

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
      add_executable(
            xml2smf xml2smf.cpp midifile.cpp event.cpp xml.cpp sig.cpp drumset.cpp
            )

      target_link_libraries( xml2smf
            ${QT_LIBRARIES}
            )
      set_target_properties( xml2smf
            PROPERTIES COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h"
            )
endif (NOT MINGW)

add_dependencies(mscore update_revision_target)
