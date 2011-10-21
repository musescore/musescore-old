# Add more folders to ship with the application, here
folder_01.source = qml/mplayer
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

# symbian:TARGET.UID3 = 0xEE36C57B

# Smart Installer package's UID   xxx
# This UID is from the protected range and therefore the package will
# fail to install if self-signed. By default qmake uses the unprotected
# range value if unprotected UID is defined for the application and
# 0x2002CCCF value if protected UID is given to the application
#symbian:DEPLOYMENT.installer_header = 0x2002CCCF

# Allow network access on Symbian
# symbian:TARGET.CAPABILITY += NetworkServices

# If your application uses the Qt Mobility libraries, uncomment the following
# lines and add the respective components to the MOBILITY variable.
# CONFIG += mobility
# MOBILITY +=

# Add dependency to symbian components
# CONFIG += qtquickcomponents

INCLUDEPATH += ..

CONFIG += precompile_header

PRECOMPILED_HEADER = ../all.h

QT += xml declarative

LIBS += -lOpenSLES

QMAKE_CXXFLAGS += -include ../all.h

RESOURCES += mplayer.qrc

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += main.cpp                              \
    ../libmscore/accidental.cpp                  \
    ../libmscore/arpeggio.cpp                    \
    ../libmscore/articulation.cpp                \
    ../libmscore/barline.cpp                     \
    ../libmscore/beam.cpp                        \
    ../libmscore/bend.cpp                        \
    ../libmscore/box.cpp                         \
    ../libmscore/bracket.cpp                     \
    ../libmscore/breath.cpp                      \
    ../libmscore/bsp.cpp                         \
    ../libmscore/check.cpp                       \
    ../libmscore/chord.cpp                       \
    ../libmscore/chordline.cpp                   \
    ../libmscore/chordlist.cpp                   \
    ../libmscore/chordrest.cpp                   \
    ../libmscore/clef.cpp                        \
    ../libmscore/cleflist.cpp                    \
    ../libmscore/cmd.cpp                         \
    ../libmscore/drumset.cpp                     \
    ../libmscore/dsp.cpp                         \
    ../libmscore/durationtype.cpp                \
    ../libmscore/dynamic.cpp                     \
    ../libmscore/edit.cpp                        \
    ../libmscore/element.cpp                     \
    ../libmscore/elementlayout.cpp               \
    ../libmscore/excerpt.cpp                     \
    ../libmscore/fifo.cpp                        \
    ../libmscore/fingering.cpp                   \
    ../libmscore/fraction.cpp                    \
    ../libmscore/fret.cpp                        \
    ../libmscore/glissando.cpp                   \
    ../libmscore/hairpin.cpp                     \
    ../libmscore/harmony.cpp                     \
    ../libmscore/hook.cpp                        \
    ../libmscore/icon.cpp                        \
    ../libmscore/image.cpp                       \
    ../libmscore/iname.cpp                       \
    ../libmscore/init.cpp                        \
    ../libmscore/input.cpp                       \
    ../libmscore/instrchange.cpp                 \
    ../libmscore/instrtemplate.cpp               \
    ../libmscore/instrument.cpp                  \
    ../libmscore/interval.cpp                    \
    ../libmscore/key.cpp                         \
    ../libmscore/keyfinder.cpp                   \
    ../libmscore/keysig.cpp                      \
    ../libmscore/lasso.cpp                       \
    ../libmscore/layoutbreak.cpp                 \
    ../libmscore/layout.cpp                      \
    ../libmscore/line.cpp                        \
    ../libmscore/lyrics.cpp                      \
    ../libmscore/measurebase.cpp                 \
    ../libmscore/measure.cpp                     \
    ../libmscore/mscore.cpp                      \
    ../libmscore/navigate.cpp                    \
    ../libmscore/note.cpp                        \
    ../libmscore/noteevent.cpp                   \
    ../libmscore/ossia.cpp                       \
    ../libmscore/ottava.cpp                      \
    ../libmscore/page.cpp                        \
    ../libmscore/part.cpp                        \
    ../libmscore/pedal.cpp                       \
    ../libmscore/pitch.cpp                       \
    ../libmscore/pitchspelling.cpp               \
    ../libmscore/pos.cpp                         \
    ../libmscore/rendermidi.cpp                  \
    ../libmscore/repeat.cpp                      \
    ../libmscore/repeatlist.cpp                  \
    ../libmscore/rest.cpp                        \
    ../libmscore/revisions.cpp                   \
    ../libmscore/score.cpp                       \
    ../libmscore/scorefile.cpp                   \
    ../libmscore/segment.cpp                     \
    ../libmscore/segmentlist.cpp                 \
    ../libmscore/select.cpp                      \
    ../libmscore/shadownote.cpp                  \
    ../libmscore/sig.cpp                         \
    ../libmscore/slur.cpp                        \
    ../libmscore/spacer.cpp                      \
    ../libmscore/spanner.cpp                     \
    ../libmscore/staff.cpp                       \
    ../libmscore/staffstate.cpp                  \
    ../libmscore/stafftext.cpp                   \
    ../libmscore/stafftype.cpp                   \
    ../libmscore/stem.cpp                        \
    ../libmscore/style.cpp                       \
    ../libmscore/symbol.cpp                      \
    ../libmscore/sym.cpp                         \
    ../libmscore/system.cpp                      \
    ../libmscore/tablature.cpp                   \
    ../libmscore/tempo.cpp                       \
    ../libmscore/tempotext.cpp                   \
    ../libmscore/text.cpp                        \
    ../libmscore/textframe.cpp                   \
    ../libmscore/textline.cpp                    \
    ../libmscore/timesig.cpp                     \
    ../libmscore/tremolobar.cpp                  \
    ../libmscore/tremolo.cpp                     \
    ../libmscore/trill.cpp                       \
    ../libmscore/tuplet.cpp                      \
    ../libmscore/undo.cpp                        \
    ../libmscore/utils.cpp                       \
    ../libmscore/velo.cpp                        \
    ../libmscore/volta.cpp                       \
    ../libmscore/xml.cpp                         \
    ../m-msynth/chan.cpp                         \
    ../m-msynth/chorus.cpp                       \
    ../m-msynth/conv.cpp                         \
    ../m-msynth/fluid.cpp                        \
    ../m-msynth/gen.cpp                          \
    ../m-msynth/mod.cpp                          \
    ../m-msynth/piano.cpp                        \
    ../m-msynth/rev.cpp                          \
    ../m-msynth/sfont.cpp                        \
    ../zarchive/zarchive.cpp                     \
    seq.cpp \
    scoreview.cpp \
    painterqt.cpp \
    ../libmscore/event.cpp \
    ../m-msynth/seq_event.cpp \
    ../m-msynth/synth_voice.cpp \
    ../m-msynth/synth_dsp.cpp \
    sparm.cpp \
    androidaudio.cpp

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    android/src/eu/licentia/necessitas/industrius/QtSurface.java \
    android/src/eu/licentia/necessitas/industrius/QtLayout.java \
    android/src/eu/licentia/necessitas/industrius/QtApplication.java \
    android/src/eu/licentia/necessitas/industrius/QtActivity.java \
    android/src/eu/licentia/necessitas/ministro/IMinistroCallback.aidl \
    android/src/eu/licentia/necessitas/ministro/IMinistro.aidl \
    android/src/eu/licentia/necessitas/mobile/QtSensors.java \
    android/src/eu/licentia/necessitas/mobile/QtMediaPlayer.java \
    android/src/eu/licentia/necessitas/mobile/QtFeedback.java \
    android/src/eu/licentia/necessitas/mobile/QtSystemInfo.java \
    android/src/eu/licentia/necessitas/mobile/QtAndroidContacts.java \
    android/src/eu/licentia/necessitas/mobile/QtLocation.java \
    android/src/eu/licentia/necessitas/mobile/QtCamera.java \
    android/res/values/libs.xml \
    android/res/values/strings.xml \
    android/res/drawable-ldpi/icon.png \
    android/res/drawable-hdpi/icon.png \
    android/res/drawable-mdpi/icon.png \
    android/AndroidManifest.xml \
    mplayer.qml

HEADERS += \
    scoreview.h \
    seq.h \
    painterqt.h


