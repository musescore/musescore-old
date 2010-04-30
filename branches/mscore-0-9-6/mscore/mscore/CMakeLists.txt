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

if (GCC_VISIBILITY)
set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Winvalid-pch -fvisibility=hidden -fvisibility-inlines-hidden")
else (GCC_VISIBILITY)
set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Winvalid-pch")
endif (GCC_VISIBILITY)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DQT_NO_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "-DQT_DEBUG")

if (NOT MINGW)
#
#     set library search path for runtime linker to load the same
#     qt libraries as we used at compile time
#
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath,${QT_LIBRARY_DIR}")
endif (NOT MINGW)


SET_SOURCE_FILES_PROPERTIES(revision.h PROPERTIES GENERATED TRUE)

add_custom_command(
   OUTPUT ${PROJECT_BINARY_DIR}/all.h
   COMMAND cp ${PROJECT_SOURCE_DIR}/all.h ${PROJECT_BINARY_DIR}/all.h
   DEPENDS ${PROJECT_SOURCE_DIR}/all.h
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
   )

add_custom_command(
   OUTPUT ${PROJECT_BINARY_DIR}/all.h.pch
   COMMAND ${CMAKE_CXX_COMPILER}
     -g -Wextra -Wall
     -I"${QT_INCLUDE_DIR}"
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
      ${PROJECT_SOURCE_DIR}
      ${PROJECT_SOURCE_DIR}/scriptdebug
      ${PROJECT_SOURCE_DIR}/osdabzip
      ${PROJECT_SOURCE_DIR}/fluid
      )

QT4_WRAP_UI (ui_headers
      insertmeasuresdialog.ui barline.ui chord.ui chordrest.ui editinstrument.ui editstyle.ui
      edittempo.ui element.ui hairpin.ui instrdialog.ui measure.ui measuresdialog.ui
      note.ui page.ui pagesettings.ui partedit.ui playpanel.ui prefsdialog.ui
      measureproperties.ui segment.ui text.ui textpalette.ui timedialog.ui
      symboldialog.ui dynamic.ui tuplet.ui shortcutcapturedialog.ui slurtie.ui
      slur.ui editdrumset.ui editstaff.ui line.ui linesegment.ui
      voltaproperties.ui chordproperties.ui restproperties.ui
      jumpproperties.ui markerproperties.ui boxproperties.ui
      instrwizard.ui timesigwizard.ui newwizard.ui aboutbox.ui
      chordedit.ui transposedialog.ui tempoproperties.ui importmidi.ui
      lineproperties.ui excerptsdialog.ui lyrics.ui stafftext.ui
      imageproperties.ui tupletdialog.ui tupletproperties.ui
      glissandoprop.ui articulation.ui metaedit.ui palette.ui textproperties.ui
      dynamicproperties.ui textline.ui slurproperties.ui inspector.ui
      select.ui synthcontrol.ui splitstaff.ui cellproperties.ui beam.ui keyedit.ui
      harmonyedit.ui selectinstr.ui tremolo.ui
      )

QT4_WRAP_CPP (mocs
      scoreview.h editinstrument.h editstyle.h edittempo.h instrdialog.h inspector.h
      mscore.h navigator.h pagesettings.h palette.h partedit.h playpanel.h
      preferences.h measureproperties.h preview.h score.h seq.h textpalette.h textstyle.h
      timedialog.h symboldialog.h shortcutcapturedialog.h simplebutton.h
      greendotbutton.h recordbutton.h editdrumset.h editstaff.h
      voltaproperties.h chordproperties.h restproperties.h
      jumpproperties.h markerproperties.h boxproperties.h newwizard.h
      transposedialog.h chordedit.h tempotext.h importmidi.h textline.h
      excerptsdialog.h stafftext.h image.h tuplet.h glissando.h
      script.h articulation.h metaedit.h magbox.h voiceselector.h
      text.h sccursor.h dynamics.h undo.h textproperties.h
      slurproperties.h select.h synthcontrol.h drumroll.h pianoroll.h piano.h ruler.h
      pianoscene.h splitstaff.h scoretab.h keyedit.h keycanvas.h harmonyedit.h
      harmonycanvas.h updatechecker.h
      )

QT4_ADD_RESOURCES (qrc_files mscore.qrc)

set (AUDIO "")
if (MINGW)
      set (resource_file ${PROJECT_BINARY_DIR}/resfile.o)
else (MINGW)
      if (USE_ALSA)
            set (AUDIO ${AUDIO} alsa.cpp mididriver.cpp)
      endif (USE_ALSA)
endif (MINGW)

if (USE_PORTAUDIO)
      set (AUDIO ${AUDIO} pa.cpp)
endif (USE_PORTAUDIO)

if (USE_PORTMIDI)
      set (AUDIO ${AUDIO} pm.cpp)
endif (USE_PORTMIDI)

if (USE_JACK)
      set (AUDIO ${AUDIO} jackaudio.cpp jackweakapi.cpp)
endif (USE_JACK)

if (APPLE)
	set (ExecutableName mscore MACOSX_BUNDLE)
	set (MACOSX_BUNDLE_INFO_STRING MuseScore prerelease for Mac OSX)
	set (MACOSX_BUNDLE_ICON_FILE mscore.icns)
	set (MACOSX_BUNDLE_GUI_IDENTIFIER net.sourceforge.mscore)
	set (MACOSX_BUNDLE_LONG_VERSION_STRING ${Mscore_VERSION_FULL})
	set (MACOSX_BUNDLE_BUNDLE_NAME MuseScore)
	set (MACOSX_BUNDLE_SHORT_VERSION_STRING ${Mscore_VERSION})
	set (MACOSX_BUNDLE_BUNDLE_VERSION ${Mscore_VERSION_FULL})
	set (MACOSX_BUNDLE_COPYRIGHT musescore.org)
else (APPLE)
	set (ExecutableName mscore)
endif (APPLE)


add_executable ( ${ExecutableName}
      ${qrc_files}
      ${ui_headers}
      ${mocs}
      ${PROJECT_BINARY_DIR}/all.h
      ${PROJECT_BINARY_DIR}/all.h.pch
      ${resource_file}

      actions.cpp accidental.cpp barline.cpp beam.cpp
      bracket.cpp scoreview.cpp chord.cpp chordrest.cpp clef.cpp
      cmd.cpp dynamics.cpp edit.cpp editinstrument.cpp editstyle.cpp
      edittempo.cpp element.cpp exportxml.cpp file.cpp
      hairpin.cpp icons.cpp importxml.cpp instrdialog.cpp
      key.cpp keyb.cpp layout.cpp layoutbreak.cpp line.cpp inspector.cpp
      measure.cpp menus.cpp midifile.cpp importmidi.cpp mscore.cpp
      navigate.cpp navigator.cpp note.cpp ottava.cpp
      page.cpp pagesettings.cpp palette.cpp part.cpp
      partedit.cpp pedal.cpp playpanel.cpp preferences.cpp measureproperties.cpp
      preview.cpp rest.cpp score.cpp segment.cpp select.cpp
      seq.cpp slur.cpp staff.cpp style.cpp instrtemplate.cpp boxproperties.cpp
      sym.cpp sym.h symbol.cpp symbol.h hook.cpp hook.h
      system.cpp text.cpp textline.cpp textpalette.cpp textstyle.cpp
      timedialog.cpp symboldialog.cpp trill.cpp tuplet.cpp shortcutcapturedialog.cpp
      undo.cpp utils.cpp xml.cpp timesig.cpp lyrics.cpp bsp.cpp
      volta.cpp simplebutton.cpp image.cpp keyfinder.cpp
      pitchspelling.cpp musedata.cpp keysig.cpp arpeggio.cpp breath.cpp
      glissando.cpp lilypond.cpp exportly.cpp box.cpp measurebase.cpp
      editdrumset.cpp editstaff.cpp drumset.cpp tremolo.cpp repeat.cpp
      voltaproperties.cpp chordproperties.cpp restproperties.cpp
      newwizard.cpp transposedialog.cpp bb.cpp
      harmony.cpp chordedit.cpp plugins.cpp tempotext.cpp
      excerptsdialog.cpp excerpt.cpp stafftext.cpp instrument.cpp durationtype.cpp
      event.cpp rendermidi.cpp articulation.cpp metaedit.cpp
      fifo.cpp spacer.cpp magbox.cpp voiceselector.cpp capella.cpp
      scscore.cpp sccursor.cpp scchord.cpp scnote.cpp scpart.cpp sctext.cpp scmeasure.cpp scpageformat.cpp
      exportaudio.cpp repeatlist.cpp velo.cpp exportmidi.cpp textproperties.cpp
      screst.cpp scharmony.cpp slurproperties.cpp synthcontrol.cpp
      canonical.cpp drumroll.cpp pianoroll.cpp piano.cpp ruler.cpp pianoscene.cpp
      splitstaff.cpp scoretab.cpp keyedit.cpp harmonyedit.cpp updatechecker.cpp
      importove.cpp ove.cpp interval.cpp
      ${AUDIO}
      )

if (STATIC_SCRIPT_BINDINGS)
      target_link_libraries(mscore
            qtscript_core
            qtscript_gui
            qtscript_network
            qtscript_uitools
            qtscript_xml)
endif (STATIC_SCRIPT_BINDINGS)

if (APPLE)
      set_target_properties (mscore PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/packaging/MacOSXBundleInfo.plist.in)
endif(APPLE)

if (MINGW)
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
   # Windows: add -mconsole to LINK_FLAGS to get a console window for debug output
   if(CMAKE_BUILD_TYPE MATCHES "Debug")
     set_target_properties( mscore
        PROPERTIES
           COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h ${QT_DEFINITIONS} -DQT_SVG_LIB -DQT_GUI_LIB -DQT_XML_LIB -DQT_CORE_LIB -DQT_SCRIPT_LIB"
           LINK_FLAGS "${PROJECT_BINARY_DIR}/resfile.o -mwindows -mconsole -L ${CROSSQT}/lib"
        )
   else(CMAKE_BUILD_TYPE MATCHES "Debug")
     set_target_properties( mscore
          PROPERTIES
             COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h ${QT_DEFINITIONS} -DQT_SVG_LIB -DQT_GUI_LIB -DQT_XML_LIB -DQT_CORE_LIB -DQT_SCRIPT_LIB"
             LINK_FLAGS "-Wl,-S ${PROJECT_BINARY_DIR}/resfile.o -mwindows -L ${CROSSQT}/lib"
          )
   endif(CMAKE_BUILD_TYPE MATCHES "Debug")

   target_link_libraries(mscore
      fluid
      al
      awl
      osdabzip
      rtf2html
      qtsingleapp
      ${QT_mingw_LIBRARIES}
      portaudio
      portmidi
      winmm
      )

   if (HAS_AUDIOFILE)
      target_link_libraries(mscore libsndfile-1)
   endif (HAS_AUDIOFILE)

   install( TARGETS mscore RUNTIME DESTINATION bin )

   install_files ( /bin .dll
      ${CROSS}/bin/mingwm10.dll
      ${CROSS}/bin/libgcc_s_dw2-1.dll
      ${CROSS}/bin/portaudio.dll
      ${CROSS}/bin/libsndfile-1.dll
      ${CROSSQT}/bin/QtCore4.dll
      ${CROSSQT}/bin/QtGui4.dll
      ${CROSSQT}/bin/QtXml4.dll
      ${CROSSQT}/bin/QtSvg4.dll
      ${CROSSQT}/bin/QtScript4.dll
      ${CROSSQT}/bin/QtScriptTools4.dll
      ${CROSSQT}/bin/QtNetwork4.dll
      )
   install_files ( /bin/iconengines .dll
      ${CROSSQT}/plugins/iconengines/qsvgicon4.dll
      )
   install_files ( /bin/imageformats .dll
      ${CROSSQT}/plugins/imageformats/qjpeg4.dll
      ${CROSSQT}/plugins/imageformats/qmng4.dll
      ${CROSSQT}/plugins/imageformats/qsvg4.dll
      ${CROSSQT}/plugins/imageformats/qtiff4.dll
      )

else (MINGW)
   set_target_properties( mscore
      PROPERTIES
         COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h ${QT_DEFINITIONS}"
      )
   target_link_libraries(mscore
      ${QT_LIBRARIES}
      ${QT_QTSCRIPT_LIBRARY_RELEASE}
      ${QT_QTSCRIPT_TOOLS_LIBRARY_RELEASE}
      ${ALSA_LIB}
      ${PORTAUDIO_LIB}
      al
      awl
      osdabzip
      rtf2html
      qtsingleapp
      fluid
      z
      )

   if (HAS_AUDIOFILE)
       target_link_libraries(mscore sndfile)
   endif (HAS_AUDIOFILE)

   if (APPLE)
     if (USE_PORTMIDI)
       target_link_libraries(mscore portmidi)
     endif (USE_PORTMIDI)
     target_link_libraries(mscore ${OsxFrameworks})
   endif (APPLE)

   if (QT45)
     target_link_libraries(mscore ${QT_QTSCRIPT_TOOLS_LIBRARY_RELEASE} )
   endif (QT45)

   if (APPLE)
     install (TARGETS mscore BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX})
     install (FILES data/mscore.icns DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME})
     install (FILES data/musescoreDocument.icns DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME})
   else (APPLE)
     install( TARGETS mscore RUNTIME DESTINATION bin )
     install( FILES   data/mscore.png DESTINATION share/pixmaps)
     install( FILES   data/mscore.xpm DESTINATION share/pixmaps)
   endif (APPLE)
endif (MINGW)

if (NOT MINGW AND NOT APPLE)
# command line utilities are not build for windows
#
#======================================
#     target smf2xml
#======================================

   add_executable(
      smf2xml
      ${ui_headers}
      smf2xml.cpp midifile.cpp event.cpp drumset.cpp xml.cpp
      )
   target_link_libraries(smf2xml al ${QT_LIBRARIES} )
   set_target_properties( smf2xml
      PROPERTIES COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h"
      )

#======================================
#     target xml2smf
#======================================

   add_executable(
      xml2smf
      ${ui_headers}
      xml2smf.cpp midifile.cpp event.cpp drumset.cpp
      )
   target_link_libraries(xml2smf al ${QT_LIBRARIES} )
   set_target_properties( xml2smf
      PROPERTIES COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h"
      )

#======================================
#     target genft
#     create font table
#======================================

   add_executable(
      genft
      ${ui_headers}
      genft.cpp
      )
   target_link_libraries(genft al ${QT_LIBRARIES} -lfreetype )
   set_target_properties(genft
      PROPERTIES COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h -I/usr/include/freetype2"
      )
endif (NOT MINGW AND NOT APPLE)

