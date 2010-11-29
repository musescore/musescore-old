#=============================================================================
#  MuseScore
#  Linux Music Score Editor
#  $Id:$
#
#  Copyright (C) 2002-2010 by Werner Schweer and others
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
      ${PROJECT_SOURCE_DIR}/bww2mxml
      ${QTSINGLEAPPLICATION_INCLUDE_DIRS}
      )

QT4_WRAP_UI (ui_headers
      insertmeasuresdialog.ui barline.ui chord.ui chordrest.ui editinstrument.ui editstyle.ui
      edittempo.ui element.ui hairpin.ui instrdialog.ui measure.ui measuresdialog.ui
      note.ui page.ui pagesettings.ui mixer.ui playpanel.ui prefsdialog.ui
      measureproperties.ui segment.ui text.ui textpalette.ui timedialog.ui
      symboldialog.ui dynamic.ui tuplet.ui shortcutcapturedialog.ui slurtie.ui
      slur.ui editdrumset.ui editstaff.ui line.ui linesegment.ui
      voltaproperties.ui chordproperties.ui restproperties.ui
      jumpproperties.ui markerproperties.ui boxproperties.ui
      instrwizard.ui timesigwizard.ui newwizard.ui aboutbox.ui
      chordedit.ui transposedialog.ui tempoproperties.ui importmidi.ui
      lineproperties.ui excerptsdialog.ui lyrics.ui stafftext.ui
      imageproperties.ui tupletdialog.ui tupletproperties.ui
      glissandoprop.ui articulation.ui metaedit.ui palette.ui
      dynamicproperties.ui textline.ui slurproperties.ui inspector.ui
      select.ui synthcontrol.ui splitstaff.ui cellproperties.ui beam.ui keyedit.ui
      harmonyedit.ui selectinstr.ui tremolo.ui fretdprops.ui stafftype.ui
      sectionbreak.ui hairpinproperties.ui spanner.ui bend.ui tremolobar.ui
      textproperties.ui
      )

QT4_WRAP_CPP (mocs
      scoreview.h editinstrument.h editstyle.h edittempo.h instrdialog.h inspector.h
      mscore.h navigator.h pagesettings.h palette.h mixer.h playpanel.h
      measureproperties.h preview.h score.h seq.h textpalette.h textstyle.h
      timedialog.h symboldialog.h shortcutcapturedialog.h simplebutton.h
      greendotbutton.h recordbutton.h editdrumset.h editstaff.h selinstrument.h
      voltaproperties.h chordproperties.h restproperties.h
      jumpproperties.h markerproperties.h boxproperties.h newwizard.h
      transposedialog.h chordedit.h tempotext.h importmidi.h textline.h
      excerptsdialog.h stafftext.h image.h tuplet.h glissando.h
      script.h articulation.h metaedit.h magbox.h voiceselector.h
      text.h sccursor.h dynamics.h undo.h textproperties.h
      slurproperties.h select.h synthcontrol.h drumroll.h pianoroll.h piano.h ruler.h
      pianoview.h drumview.h splitstaff.h scoretab.h keyedit.h keycanvas.h harmonyedit.h
      harmonycanvas.h updatechecker.h fret.h fretcanvas.h editstafftype.h
      prefsdialog.h layoutbreak.h hairpin.h bend.h bendcanvas.h tremolobar.h tremolobarcanvas.h
      drumtools.h texteditor.h
      )

if (APPLE)
      QT4_ADD_RESOURCES (qrc_files mscore-apple.qrc)
else (APPLE)
      QT4_ADD_RESOURCES (qrc_files mscore.qrc)
endif (APPLE)


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

if (OMR)
      set(OMR_FILES importpdf.cpp)
endif (OMR)

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
      hairpin.cpp icons.cpp importbww.cpp importxml.cpp instrdialog.cpp
      key.cpp keyb.cpp layout.cpp layoutbreak.cpp line.cpp inspector.cpp
      measure.cpp menus.cpp midifile.cpp importmidi.cpp mscore.cpp
      navigate.cpp navigator.cpp note.cpp ottava.cpp
      page.cpp pagesettings.cpp palette.cpp part.cpp
      mixer.cpp pedal.cpp playpanel.cpp preferences.cpp measureproperties.cpp
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
      canonical.cpp drumroll.cpp pianoroll.cpp piano.cpp ruler.cpp pianoview.cpp drumview.cpp
      splitstaff.cpp scoretab.cpp keyedit.cpp harmonyedit.cpp updatechecker.cpp
      importove.cpp ove.cpp interval.cpp check.cpp tablature.cpp
      fret.cpp stafftype.cpp staffstate.cpp spanner.cpp fingering.cpp
      shadownote.cpp pitch.cpp importgtp.cpp revisions.cpp bend.cpp tremolobar.cpp
      noteevent.cpp chordline.cpp lasso.cpp fotomode.cpp drumtools.cpp
      elementlayout.cpp selinstrument.cpp instrchange.cpp texteditor.cpp
      cursor.cpp textframe.cpp editstafftype.cpp
      ${OMR_FILES}
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
      set_target_properties (mscore
         PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/packaging/MacOSXBundleInfo.plist.in)
endif(APPLE)

if (OSC)
    target_link_libraries(mscore ofqf)
endif (OSC)

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
      al
      awl
      mstyle
      diff_match_patch
      msynth
      bww
      osdabzip
      rtf2html
      ${QTSINGLEAPPLICATION_LIBRARIES}
      fluid
      ${QT_mingw_LIBRARIES}
      portaudio
      portmidi
      winmm
      )

   if (HAS_AUDIOFILE)
      target_link_libraries(mscore libsndfile-1)
   endif (HAS_AUDIOFILE)

   if (OGGVORBIS)
      target_link_libraries(mscore vorbis-0 ogg-0)
   endif (OGGVORBIS)

   if (AEOLUS)
      target_link_libraries(mscore aeolus)
   endif (AEOLUS)

   if (OMR)
      target_link_libraries(mscore omr poppler)
   endif (OMR)

   install( TARGETS mscore RUNTIME DESTINATION bin )

   install_files ( /bin .dll
      ${CROSS}/bin/mingwm10.dll
      ${CROSS}/bin/libgcc_s_dw2-1.dll
      ${CROSS}/lib/portaudio.dll
      ${CROSS}/lib/libsndfile-1.dll
      ${CROSS}/lib/libogg-0.dll
      ${CROSS}/lib/libvorbis-0.dll
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
      diff_match_patch
      al
      awl
      mstyle
      msynth
      bww
      osdabzip
      rtf2html
      ${QTSINGLEAPPLICATION_LIBRARIES}
      fluid
      z
      dl
      )

   if (NOT APPLE)
       target_link_libraries(mscore rt)
   endif (NOT APPLE)

   if (HAS_AUDIOFILE)
       target_link_libraries(mscore sndfile)
   endif (HAS_AUDIOFILE)

   if (OGGVORBIS)
      target_link_libraries(mscore vorbis ogg)
   endif (OGGVORBIS)

   if (AEOLUS)
      target_link_libraries(mscore aeolus)
   endif (AEOLUS)

   if (OMR)
      target_link_libraries(mscore omr poppler)
   endif (OMR)

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
     install( FILES   data/icons/mscore.xpm DESTINATION share/pixmaps)
     install( FILES   data/icons-dark/mscore.xpm DESTINATION share/pixmaps)
   endif (APPLE)
endif (MINGW)

if (NOT MINGW AND NOT APPLE)
# command line utilities are not build for windows/apple
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

#======================================
#     install icons if not embedded
#======================================

# if (NOT EMBED_ICONS)
   install (FILES
      data/greendot.svg
      data/darkgreendot.svg
      data/recordOn.svg
      data/recordOff.svg
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}/icons
      )
   install (FILES
      data/icons/text_top.svg
      data/icons/text_bottom.svg
      data/icons/text_vcenter.svg
      data/icons/minus.svg
      data/icons/flip.svg
      data/icons/undo.svg
      data/icons/redo.svg
      data/icons/midiin.svg
      data/icons/speaker.svg
      data/icons/start.svg
      data/icons/play.svg
      data/icons/sbeam.svg
      data/icons/mbeam.svg
      data/icons/nbeam.svg
      data/icons/beam32.svg
      data/icons/beam64.svg
      data/icons/abeam.svg
      data/icons/fbeam1.svg
      data/icons/fbeam2.svg
      data/icons/fileopen.svg
      data/icons/filenew.svg
      data/icons/filesave.svg
      data/icons/filesaveas.svg
      data/icons/exit.svg
      data/icons/viewmag.svg
      data/icons/repeat.svg
      data/icons/noteentry.svg
      data/icons/grace4.svg
      data/icons/grace16.svg
      data/icons/grace32.svg
      data/icons/keyboard.svg
      data/icons/tie.svg
      data/icons/acciaccatura.svg
      data/icons/appoggiatura.svg
      data/icons/brevis.svg
      data/icons/clef.svg
      data/icons/dfermata.svg
      data/icons/dlongfermata.svg
      data/icons/dmarcato.svg
      data/icons/dotdot.svg
      data/icons/dot.svg
      data/icons/downbow.svg
      data/icons/downmordent.svg
      data/icons/downprall.svg
      data/icons/dportato.svg
      data/icons/dshortfermata.svg
      data/icons/dstaccatissimo.svg
      data/icons/dverylongfermata.svg
      data/icons/espressivo.svg
      data/icons/flatflat.svg
      data/icons/flat.svg
      data/icons/longaUp.svg
      data/icons/mordent.svg
      data/icons/natural.svg
      data/icons/note16.svg
      data/icons/note2.svg
      data/icons/note32.svg
      data/icons/note4.svg
      data/icons/note64.svg
      data/icons/note8.svg
      data/icons/note.svg
      data/icons/ouvert.svg
      data/icons/plus.svg
      data/icons/plusstop.svg
      data/icons/popping.svg
      data/icons/prallmordent.svg
      data/icons/prall.svg
      data/icons/prallprall.svg
      data/icons/quartrest.svg
      data/icons/reverseturn.svg
      data/icons/sforzatoaccent.svg
      data/icons/sforzato.svg
      data/icons/sharp.svg
      data/icons/sharpsharp.svg
      data/icons/slapping.svg
      data/icons/snappizzicato.svg
      data/icons/staccato.svg
      data/icons/tapping.svg
      data/icons/tenuto.svg
      data/icons/thumb.svg
      data/icons/trill.svg
      data/icons/turn.svg
      data/icons/ufermata.svg
      data/icons/ulongfermata.svg
      data/icons/umarcato.svg
      data/icons/upbow.svg
      data/icons/upmordent.svg
      data/icons/uportato.svg
      data/icons/upprall.svg
      data/icons/ushortfermata.svg
      data/icons/ustaccatissimo.svg
      data/icons/uverylongfermata.svg
      data/icons/editcut.svg
      data/icons/editcopy.svg
      data/icons/editpaste.svg
      data/icons/fileprint.svg
      data/icons/text_bold.svg
      data/icons/text_italic.svg
      data/icons/text_under.svg
      data/icons/text_left.svg
      data/icons/text_center.svg
      data/icons/text_right.svg
      data/icons/subscript.svg
      data/icons/superscript.svg
      data/icons/fotomode.svg
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}/icons/icons
      )
   install (FILES
      data/icons-dark/text_top.svg
      data/icons-dark/text_bottom.svg
      data/icons-dark/text_vcenter.svg
      data/icons-dark/minus.svg
      data/icons-dark/flip.svg
      data/icons-dark/undo.svg
      data/icons-dark/redo.svg
      data/icons-dark/midiin.svg
      data/icons-dark/speaker.svg
      data/icons-dark/start.svg
      data/icons-dark/play.svg
      data/icons-dark/sbeam.svg
      data/icons-dark/mbeam.svg
      data/icons-dark/nbeam.svg
      data/icons-dark/beam32.svg
      data/icons-dark/beam64.svg
      data/icons-dark/abeam.svg
      data/icons-dark/fbeam1.svg
      data/icons-dark/fbeam2.svg
      data/icons-dark/fileopen.svg
      data/icons-dark/filenew.svg
      data/icons-dark/filesave.svg
      data/icons-dark/filesaveas.svg
      data/icons-dark/exit.svg
      data/icons-dark/viewmag.svg
      data/icons-dark/repeat.svg
      data/icons-dark/noteentry.svg
      data/icons-dark/grace4.svg
      data/icons-dark/grace16.svg
      data/icons-dark/grace32.svg
      data/icons-dark/keyboard.svg
      data/icons-dark/tie.svg
      data/icons-dark/acciaccatura.svg
      data/icons-dark/appoggiatura.svg
      data/icons-dark/brevis.svg
      data/icons-dark/clef.svg
      data/icons-dark/dfermata.svg
      data/icons-dark/dlongfermata.svg
      data/icons-dark/dmarcato.svg
      data/icons-dark/dotdot.svg
      data/icons-dark/dot.svg
      data/icons-dark/downbow.svg
      data/icons-dark/downmordent.svg
      data/icons-dark/downprall.svg
      data/icons-dark/dportato.svg
      data/icons-dark/dshortfermata.svg
      data/icons-dark/dstaccatissimo.svg
      data/icons-dark/dverylongfermata.svg
      data/icons-dark/espressivo.svg
      data/icons-dark/flatflat.svg
      data/icons-dark/flat.svg
      data/icons-dark/longaUp.svg
      data/icons-dark/mordent.svg
      data/icons-dark/natural.svg
      data/icons-dark/note16.svg
      data/icons-dark/note2.svg
      data/icons-dark/note32.svg
      data/icons-dark/note4.svg
      data/icons-dark/note64.svg
      data/icons-dark/note8.svg
      data/icons-dark/note.svg
      data/icons-dark/ouvert.svg
      data/icons-dark/plus.svg
      data/icons-dark/plusstop.svg
      data/icons-dark/popping.svg
      data/icons-dark/prallmordent.svg
      data/icons-dark/prall.svg
      data/icons-dark/prallprall.svg
      data/icons-dark/quartrest.svg
      data/icons-dark/reverseturn.svg
      data/icons-dark/sforzatoaccent.svg
      data/icons-dark/sforzato.svg
      data/icons-dark/sharp.svg
      data/icons-dark/sharpsharp.svg
      data/icons-dark/slapping.svg
      data/icons-dark/snappizzicato.svg
      data/icons-dark/staccato.svg
      data/icons-dark/tapping.svg
      data/icons-dark/tenuto.svg
      data/icons-dark/thumb.svg
      data/icons-dark/trill.svg
      data/icons-dark/turn.svg
      data/icons-dark/ufermata.svg
      data/icons-dark/ulongfermata.svg
      data/icons-dark/umarcato.svg
      data/icons-dark/upbow.svg
      data/icons-dark/upmordent.svg
      data/icons-dark/uportato.svg
      data/icons-dark/upprall.svg
      data/icons-dark/ushortfermata.svg
      data/icons-dark/ustaccatissimo.svg
      data/icons-dark/uverylongfermata.svg
      data/icons-dark/editcut.svg
      data/icons-dark/editcopy.svg
      data/icons-dark/editpaste.svg
      data/icons-dark/fileprint.svg
      data/icons-dark/text_bold.svg
      data/icons-dark/text_italic.svg
      data/icons-dark/text_under.svg
      data/icons-dark/text_left.svg
      data/icons-dark/text_center.svg
      data/icons-dark/text_right.svg
      data/icons-dark/subscript.svg
      data/icons-dark/superscript.svg
      data/icons-dark/fotomode.svg
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}/icons/icons-dark
      )
# endif (NOT EMBED_ICONS)
