//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "config.h"
#include "xml.h"
#include "score.h"
#include "mscore.h"
#include "preferences.h"
#include "prefsdialog.h"
#include "msynth/synti.h"
#include "seq.h"
#include "note.h"
#include "playpanel.h"
#include "icons.h"
#include "shortcutcapturedialog.h"
#include "scoreview.h"
#include "sym.h"
#include "palette.h"
#include "pa.h"
#include "pm.h"
#include "page.h"
#include "file.h"

extern void writeShortcuts();
extern void readShortcuts();

bool useALSA = false, useJACK = false, usePortaudio = false;

extern bool useFactorySettings;
extern bool externalStyle;
extern QString iconGroup;

static const char* appStyleFile;

//---------------------------------------------------------
//   PeriodItem
//---------------------------------------------------------

struct PeriodItem {
       int time;
       const char* text;
       PeriodItem(const int t, const  char* txt) {
             time = t;
             text = txt;
             }
       };

static PeriodItem updatePeriods[] = {
      PeriodItem(24,      QT_TRANSLATE_NOOP("preferences","Every day")),
      PeriodItem(72,      QT_TRANSLATE_NOOP("preferences","Every 3 days")),
      PeriodItem(7*24,    QT_TRANSLATE_NOOP("preferences","Every week")),
      PeriodItem(2*7*24,  QT_TRANSLATE_NOOP("preferences","Every 2 weeks")),
      PeriodItem(30*24,   QT_TRANSLATE_NOOP("preferences","Every month")),
      PeriodItem(2*30*24, QT_TRANSLATE_NOOP("preferences","Every 2 months")),
      PeriodItem(-1,      QT_TRANSLATE_NOOP("preferences","Never")),
      };

//---------------------------------------------------------
//   appStyleSheet
//---------------------------------------------------------

QString appStyleSheet()
      {
      QString s;
      QFile f(appStyleFile);
      if (f.open(QIODevice::ReadOnly)) {
            s = f.readAll();
            f.close();
            }
      return s;
      }

//---------------------------------------------------------
//   Preferences
//---------------------------------------------------------

Preferences preferences;

Preferences::Preferences()
      {
      init();
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Preferences::init()
      {
      // set fallback defaults:

      bgUseColor         = true;
      fgUseColor         = false;
      bgWallpaper        = QString();
      fgWallpaper        = ":/data/paper5.png";
      fgColor.setRgb(255, 255, 255);
      bgColor.setRgb(0x76, 0x76, 0x6e);

      selectColor[0].setRgb(0, 0, 255);     //blue
      selectColor[1].setRgb(0, 150, 0);     //green
      selectColor[2].setRgb(230, 180, 50);  //yellow
      selectColor[3].setRgb(200, 0, 200);   //purple
      dropColor      = Qt::red;
      defaultColor   = Qt::black;

      enableMidiInput    = true;
      playNotes          = true;

      showNavigator      = true;
      showPlayPanel      = false;
      showStatusBar      = true;
      playPanelPos       = QPoint(100, 300);

#if defined(Q_WS_MAC) || defined(__MINGW32__)
      useAlsaAudio       = false;
      useJackAudio       = false;
      usePortaudioAudio  = true;
      useJackMidi        = false;
#else
      useAlsaAudio       = true;
      useJackAudio       = false;
      usePortaudioAudio  = false;
      useJackMidi        = false;
#endif
      alsaDevice         = "default";
      alsaSampleRate     = 48000;
      alsaPeriodSize     = 1024;
      alsaFragments      = 3;
      portaudioDevice    = -1;
      midiPorts          = 2;
      rememberLastMidiConnections = true;

      layoutBreakColor         = Qt::green;
      antialiasedDrawing       = true;
      sessionStart             = SCORE_SESSION;
      startScore               = ":/data/Promenade_Example.mscx";
      workingDirectory         = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
      showSplashScreen         = true;

      useMidiRemote      = false;
      for (int i = 0; i < MIDI_REMOTES; ++i)
            midiRemote[i].type = MIDI_REMOTE_TYPE_INACTIVE;

      midiExpandRepeats        = true;
      playRepeats              = true;
      instrumentList           = ":/data/instruments.xml";

      alternateNoteEntryMethod = false;
      proximity                = 6;
      autoSave                 = true;
      autoSaveTime             = 2;       // minutes
      pngResolution            = 300.0;
      pngTransparent           = true;
      language                 = "system";

      replaceCopyrightSymbol  = true;
      replaceFractions        = true;

      paperSize               = QPrinter::A4;     // default paper size
      paperWidth              = 1.0;
      paperHeight             = 1.0;
      landscape               = false;
      twosided                = true;
      spatium                 = SPATIUM20;
      mag                     = 1.0;
      tuning                  = 440.0f;
      masterGain              = 0.2;
      chorusGain              = 0.5;
      reverbGain              = 0.5;
      reverbRoomSize          = 0.5;
      reverbDamp              = 0.5;
      reverbWidth             = 1.0;

      defaultPlayDuration     = 300;      // ms
      warnPitchRange          = true;
      followSong              = true;
      importCharset           = "GBK";

      //update
      checkUpdateStartup      = 0;

      useOsc                  = false;
      oscPort                 = 5282;
      appStyleFile            = ":/data/appstyle-dark.css";

      singlePalette           = false;
      };

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Preferences::write()
      {
      dirty = false;
      QSettings s;

      s.setValue("bgUseColor",         bgUseColor);
      s.setValue("fgUseColor",         fgUseColor);
      s.setValue("bgWallpaper",        bgWallpaper);
      s.setValue("fgWallpaper",        fgWallpaper);
      s.setValue("fgColor",            fgColor);
      s.setValue("bgColor",            bgColor);

      s.setValue("selectColor1",       selectColor[0]);
      s.setValue("selectColor2",       selectColor[1]);
      s.setValue("selectColor3",       selectColor[2]);
      s.setValue("selectColor4",       selectColor[3]);
      s.setValue("dropColor",          dropColor);
      s.setValue("defaultColor",       defaultColor);
      s.setValue("enableMidiInput",    enableMidiInput);
      s.setValue("playNotes",          playNotes);

      s.setValue("soundFont",          soundFont);
      s.setValue("lPort",              lPort);
      s.setValue("rPort",              rPort);
      s.setValue("showNavigator",      showNavigator);
      s.setValue("showPlayPanel",      showPlayPanel);
      s.setValue("showStatusBar",      showStatusBar);

      s.setValue("useAlsaAudio",       useAlsaAudio);
      s.setValue("useJackAudio",       useJackAudio);
      s.setValue("useJackMidi",        useJackMidi);
      s.setValue("usePortaudioAudio",  usePortaudioAudio);
      s.setValue("midiPorts",          midiPorts);
      s.setValue("rememberLastMidiConnections", rememberLastMidiConnections);

      s.setValue("alsaDevice",         alsaDevice);
      s.setValue("alsaSampleRate",     alsaSampleRate);
      s.setValue("alsaPeriodSize",     alsaPeriodSize);
      s.setValue("alsaFragments",      alsaFragments);
      s.setValue("portaudioDevice",    portaudioDevice);
      s.setValue("portMidiInput",   portMidiInput);

      s.setValue("layoutBreakColor",   layoutBreakColor);
      s.setValue("antialiasedDrawing", antialiasedDrawing);
      switch(sessionStart) {
            case EMPTY_SESSION:  s.setValue("sessionStart", "empty"); break;
            case LAST_SESSION:   s.setValue("sessionStart", "last"); break;
            case NEW_SESSION:    s.setValue("sessionStart", "new"); break;
            case SCORE_SESSION:  s.setValue("sessionStart", "score"); break;
            }
      s.setValue("startScore",         startScore);
      s.setValue("workingDirectory",   workingDirectory);
      s.setValue("defaultStyle",       defaultStyle);
      s.setValue("showSplashScreen",   showSplashScreen);

      s.setValue("midiExpandRepeats",  midiExpandRepeats);
      s.setValue("playRepeats",        playRepeats);
      s.setValue("instrumentList", instrumentList);

      s.setValue("alternateNoteEntry", alternateNoteEntryMethod);
      s.setValue("proximity",          proximity);
      s.setValue("autoSave",           autoSave);
      s.setValue("autoSaveTime",       autoSaveTime);
      s.setValue("pngResolution",      pngResolution);
      s.setValue("pngTransparent",     pngTransparent);
      s.setValue("language",           language);

      s.setValue("replaceFractions", replaceFractions);
      s.setValue("replaceCopyrightSymbol", replaceCopyrightSymbol);
      s.setValue("paperSize", paperSize);
      s.setValue("paperWidth", paperWidth);
      s.setValue("paperHeight", paperHeight);
      s.setValue("landscape", landscape);
      s.setValue("twosided", twosided);
      s.setValue("spatium", spatium);
      s.setValue("mag", mag);
      s.setValue("tuning", tuning);
      s.setValue("masterGain", masterGain);
      s.setValue("chorusGain", chorusGain);
      s.setValue("reverbGain", reverbGain);
      s.setValue("reverbRoomSize", reverbRoomSize);
      s.setValue("reverbDamp", reverbDamp);
      s.setValue("reverbWidth", reverbWidth);

      s.setValue("defaultPlayDuration", defaultPlayDuration);
      s.setValue("importStyleFile", importStyleFile);
      s.setValue("importCharset", importCharset);
      s.setValue("warnPitchRange", warnPitchRange);
      s.setValue("followSong", followSong);

      s.setValue("useOsc", useOsc);
      s.setValue("oscPort", oscPort);
      s.setValue("style", styleName);
      s.setValue("singlePalette", singlePalette);

      //update
      s.setValue("checkUpdateStartup", checkUpdateStartup);

      s.setValue("useMidiRemote", useMidiRemote);
      for (int i = 0; i < MIDI_REMOTES; ++i) {
            if (midiRemote[i].type != MIDI_REMOTE_TYPE_INACTIVE) {
                  QChar t;
                  if (midiRemote[i].type == MIDI_REMOTE_TYPE_NOTEON)
                        t = QChar('P');
                  else
                        t = QChar('C');
                  s.setValue(QString("remote%1").arg(i),
                     QString("%1%2").arg(t).arg(midiRemote[i].data));
                  }
            }

      s.beginGroup("PlayPanel");
      s.setValue("pos", playPanelPos);
      s.endGroup();

      writeShortcuts();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Preferences::read()
      {
      QSettings s;

      bgUseColor      = s.value("bgUseColor", true).toBool();
      fgUseColor      = s.value("fgUseColor", false).toBool();
      bgWallpaper     = s.value("bgWallpaper").toString();
      fgWallpaper     = s.value("fgWallpaper", ":/data/paper5.png").toString();
      fgColor         = s.value("fgColor", QColor(255, 255, 255)).value<QColor>();
      bgColor         = s.value("bgColor", QColor(0x76, 0x76, 0x6e)).value<QColor>();

      selectColor[0]  = s.value("selectColor1", QColor(Qt::blue)).value<QColor>();     //blue
      selectColor[1]  = s.value("selectColor2", QColor(0, 150, 0)).value<QColor>();    //green
      selectColor[2]  = s.value("selectColor3", QColor(230, 180, 50)).value<QColor>(); //yellow
      selectColor[3]  = s.value("selectColor4", QColor(200, 0, 200)).value<QColor>();  //purple

      defaultColor    = s.value("defaultColor", QColor(Qt::black)).value<QColor>();
      dropColor       = s.value("dropColor",    QColor(Qt::red)).value<QColor>();

      enableMidiInput = s.value("enableMidiInput", true).toBool();
      playNotes       = s.value("playNotes", true).toBool();
      lPort           = s.value("lPort").toString();
      rPort           = s.value("rPort").toString();

      soundFont       = s.value("soundFont", mscoreGlobalShare+"/sound/TimGM6mb.sf2").toString();
      if (soundFont == ":/data/piano1.sf2") {
            // silently change to new default sound font
            soundFont = mscoreGlobalShare + "/sound/TimGM6mb.sf2";
            }
      showNavigator   = s.value("showNavigator", true).toBool();
      showStatusBar   = s.value("showStatusBar", true).toBool();
      showPlayPanel   = s.value("showPlayPanel", false).toBool();

#if defined(Q_WS_MAC) || defined(__MINGW32__)
      useAlsaAudio       = s.value("useAlsaAudio", false).toBool();
      useJackAudio       = s.value("useJackAudio", false).toBool();
      useJackMidi        = s.value("useJackMidi",  false).toBool();
      usePortaudioAudio  = s.value("usePortaudioAudio", true).toBool();
#else
      useAlsaAudio       = s.value("useAlsaAudio", true).toBool();
      useJackAudio       = s.value("useJackAudio", false).toBool();
      useJackMidi        = s.value("useJackMidi",  false).toBool();
      usePortaudioAudio  = s.value("usePortaudioAudio", false).toBool();
#endif

      alsaDevice         = s.value("alsaDevice", "default").toString();
      alsaSampleRate     = s.value("alsaSampleRate", 48000).toInt();
      alsaPeriodSize     = s.value("alsaPeriodSize", 1024).toInt();
      alsaFragments      = s.value("alsaFragments", 3).toInt();
      portaudioDevice    = s.value("portaudioDevice", -1).toInt();
      portMidiInput      = s.value("portMidiInput", "").toString();
      layoutBreakColor   = s.value("layoutBreakColor", QColor(Qt::green)).value<QColor>();
      antialiasedDrawing = s.value("antialiasedDrawing", true).toBool();

      QString path = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
      workingDirectory   = s.value("workingDirectory", path).toString();
      defaultStyle       = s.value("defaultStyle").toString();

      showSplashScreen         = s.value("showSplashScreen", true).toBool();
      midiExpandRepeats        = s.value("midiExpandRepeats", true).toBool();
      playRepeats              = s.value("playRepeats", true).toBool();
      alternateNoteEntryMethod = s.value("alternateNoteEntry", false).toBool();
      midiPorts                = s.value("midiPorts", 2).toInt();
      rememberLastMidiConnections = s.value("rememberLastMidiConnections", true).toBool();
      proximity                = s.value("proximity", 6).toInt();
      autoSave                 = s.value("autoSave", true).toBool();
      autoSaveTime             = s.value("autoSaveTime", 2).toInt();
      pngResolution            = s.value("pngResolution", 300.0).toDouble();
      pngTransparent           = s.value("pngTransparent", true).toBool();
      language                 = s.value("language", "system").toString();

      replaceFractions = s.value("replaceFractions", true).toBool();
      replaceCopyrightSymbol = s.value("replaceCopyrightSymbol", true).toBool();
      paperSize              = QPrinter::PageSize(s.value("paperSize", QPrinter::A4).toInt());
      paperWidth             = s.value("paperWidth", 1.0).toDouble();
      paperHeight            = s.value("paperHeight", 1.0).toDouble();
      landscape              = s.value("landscape", false).toBool();
      twosided               = s.value("twosided", true).toBool();
      spatium                = s.value("spatium", SPATIUM20).toDouble();
      mag                    = s.value("mag", 1.0).toDouble();
      tuning                 = s.value("tuning", 440.0).toDouble();
      masterGain             = s.value("masterGain", 0.2).toDouble();
      chorusGain             = s.value("chorusGain", 0.5).toDouble();
      reverbGain             = s.value("reverbGain", 0.5).toDouble();
      reverbRoomSize         = s.value("reverbRoomSize", 0.5).toDouble();
      reverbDamp             = s.value("reverbDamp", 0.5).toDouble();
      reverbWidth            = s.value("reverbWidth", 1.0).toDouble();

      defaultPlayDuration    = s.value("defaultPlayDuration", 300).toInt();
      importStyleFile        = s.value("importStyleFile", "").toString();
      importCharset          = s.value("importCharset", "GBK").toString();
      warnPitchRange         = s.value("warnPitchRange", true).toBool();
      followSong             = s.value("followSong", true).toBool();

      useOsc                 = s.value("useOsc", false).toBool();
      oscPort                = s.value("oscPort", 5282).toInt();
      styleName              = s.value("style", "dark").toString();
      if (styleName == "light") {
            iconGroup = "icons/";
            appStyleFile = ":/data/appstyle.css";
            globalStyle  = 1;
            }
      else {
            iconGroup = "icons-dark/";
            appStyleFile = ":/data/appstyle-dark.css";
            globalStyle  = 0;
            }
      singlePalette          = s.value("singlePalette", false).toBool();

      checkUpdateStartup = s.value("checkUpdateStartup", UpdateChecker::defaultPeriod()).toInt();
      if (checkUpdateStartup == 0) {
            checkUpdateStartup = UpdateChecker::defaultPeriod();
            }

      QString ss(s.value("sessionStart", "score").toString());
      if (ss == "last")
            sessionStart = LAST_SESSION;
      else if (ss == "new")
            sessionStart = NEW_SESSION;
      else if (ss == "score")
            sessionStart = SCORE_SESSION;
      else if (ss == "empty")
            sessionStart = EMPTY_SESSION;

      startScore     = s.value("startScore", ":/data/Promenade_Example.mscx").toString();
      instrumentList = s.value("instrumentList", ":/data/instruments.xml").toString();

      useMidiRemote  = s.value("useMidiRemote", false).toBool();
      for (int i = 0; i < MIDI_REMOTES; ++i) {
            QString data = s.value(QString("remote%1").arg(i)).toString();
            if (data.isEmpty())
                  midiRemote[i].type = MIDI_REMOTE_TYPE_INACTIVE;
            else {
                  midiRemote[i].data = data.mid(1).toInt();
                  if (data[0] == QChar('P')) {
                        midiRemote[i].type = MIDI_REMOTE_TYPE_NOTEON;
                        }
                  else if (data[0] == QChar('C')) {
                        midiRemote[i].type = MIDI_REMOTE_TYPE_CTRL;
                        }
                  }
            }

      s.beginGroup("PlayPanel");
      playPanelPos = s.value("pos", QPoint(100, 300)).toPoint();
      s.endGroup();

      readShortcuts();
      }

//---------------------------------------------------------
//   preferences
//---------------------------------------------------------

void MuseScore::startPreferenceDialog()
      {
      if (!preferenceDialog) {
            preferenceDialog = new PreferenceDialog(this);
            connect(preferenceDialog, SIGNAL(preferencesChanged()),
               SLOT(preferencesChanged()));
            }
      preferenceDialog->show();
      }

//---------------------------------------------------------
//   PreferenceDialog
//---------------------------------------------------------

PreferenceDialog::PreferenceDialog(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      startWithButton->setIcon(*icons[fileOpen_ICON]);
      instrumentListButton->setIcon(*icons[fileOpen_ICON]);
      defaultStyleButton->setIcon(*icons[fileOpen_ICON]);
      workingDirectoryButton->setIcon(*icons[fileOpen_ICON]);
      bgWallpaperSelect->setIcon(*icons[fileOpen_ICON]);
      fgWallpaperSelect->setIcon(*icons[fileOpen_ICON]);
      sfOpenButton->setIcon(*icons[fileOpen_ICON]);
      styleFileButton->setIcon(*icons[fileOpen_ICON]);
      shortcutsChanged        = false;

#ifndef USE_JACK
      jackDriver->setVisible(false);
#endif
#ifndef USE_ALSA
      alsaDriver->setVisible(false);
#endif
#ifndef USE_PORTAUDIO
      portaudioDriver->setVisible(false);
#endif
#ifndef USE_PORTMIDI
      portmidiDriverInput->setVisible(false);
#endif

      QButtonGroup* fgButtons = new QButtonGroup(this);
      fgButtons->setExclusive(true);
      fgButtons->addButton(fgColorButton);
      fgButtons->addButton(fgWallpaperButton);
      connect(fgColorButton, SIGNAL(toggled(bool)), SLOT(fgClicked(bool)));

      QButtonGroup* bgButtons = new QButtonGroup(this);
      bgButtons->setExclusive(true);
      bgButtons->addButton(bgColorButton);
      bgButtons->addButton(bgWallpaperButton);

      updateValues(&preferences);

      connect(buttonBox,          SIGNAL(clicked(QAbstractButton*)), SLOT(buttonBoxClicked(QAbstractButton*)));
      connect(fgWallpaperSelect,  SIGNAL(clicked()), SLOT(selectFgWallpaper()));
      connect(bgWallpaperSelect,  SIGNAL(clicked()), SLOT(selectBgWallpaper()));
      connect(workingDirectoryButton, SIGNAL(clicked()), SLOT(selectWorkingDirectory()));
      connect(defaultStyleButton,     SIGNAL(clicked()), SLOT(selectDefaultStyle()));
      connect(instrumentListButton,   SIGNAL(clicked()), SLOT(selectInstrumentList()));
      connect(startWithButton,        SIGNAL(clicked()), SLOT(selectStartWith()));
      connect(playPanelCur,   SIGNAL(clicked()), SLOT(playPanelCurClicked()));
      connect(shortcutList,   SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(defineShortcutClicked()));
      connect(resetShortcut,  SIGNAL(clicked()), SLOT(resetShortcutClicked()));
      connect(clearShortcut,  SIGNAL(clicked()), SLOT(clearShortcutClicked()));
      connect(defineShortcut, SIGNAL(clicked()), SLOT(defineShortcutClicked()));
      connect(resetToDefault, SIGNAL(clicked()), SLOT(resetAllValues()));

      connect(paperHeight, SIGNAL(valueChanged(double)), SLOT(paperSizeChanged(double)));
      connect(paperWidth,  SIGNAL(valueChanged(double)), SLOT(paperSizeChanged(double)));
      connect(pageGroup,   SIGNAL(activated(int)), SLOT(pageFormatSelected(int)));
      connect(landscape,   SIGNAL(toggled(bool)), SLOT(landscapeToggled(bool)));

      recordButtons = new QButtonGroup(this);
      recordButtons->setExclusive(false);
      recordButtons->addButton(recordRewind, RMIDI_REWIND);
      recordButtons->addButton(recordTogglePlay,   RMIDI_TOGGLE_PLAY);
      recordButtons->addButton(recordPlay,   RMIDI_PLAY);
      recordButtons->addButton(recordStop,   RMIDI_STOP);
      recordButtons->addButton(rcr2,         RMIDI_NOTE1);
      recordButtons->addButton(rcr3,         RMIDI_NOTE2);
      recordButtons->addButton(rcr4,         RMIDI_NOTE4);
      recordButtons->addButton(rcr5,         RMIDI_NOTE8);
      recordButtons->addButton(rcr6,         RMIDI_NOTE16);
      recordButtons->addButton(rcr7,         RMIDI_NOTE32);
      recordButtons->addButton(rcr8,         RMIDI_NOTE64);
      recordButtons->addButton(rcr9,         RMIDI_REST);
      recordButtons->addButton(rcr10,        RMIDI_DOT);
      recordButtons->addButton(rcr11,        RMIDI_DOTDOT);
      recordButtons->addButton(rcr12,        RMIDI_TIE);
      recordButtons->addButton(recordEditMode, RMIDI_NOTE_EDIT_MODE);

      connect(recordButtons,          SIGNAL(buttonClicked(int)), SLOT(recordButtonClicked(int)));
      connect(midiRemoteControlClear, SIGNAL(clicked()), SLOT(midiRemoteControlClearClicked()));
      connect(sfOpenButton,           SIGNAL(clicked()), SLOT(selectSoundFont()));
      updateRemote();
      }

//---------------------------------------------------------
//   recordButtonClicked
//---------------------------------------------------------

void PreferenceDialog::recordButtonClicked(int val)
      {
      foreach(QAbstractButton* b, recordButtons->buttons()) {
            b->setChecked(recordButtons->id(b) == val);
            }
      mscore->setMidiRecordId(val);
      }

//---------------------------------------------------------
//   updateRemote
//---------------------------------------------------------

void PreferenceDialog::updateRemote()
      {
      rewindActive->setChecked(preferences.midiRemote[RMIDI_REWIND].type != -1);
      togglePlayActive->setChecked(preferences.midiRemote[RMIDI_TOGGLE_PLAY].type   != -1);
      playActive->setChecked(preferences.midiRemote[RMIDI_PLAY].type         != -1);
      stopActive->setChecked(preferences.midiRemote[RMIDI_STOP].type         != -1);
      rca2->setChecked(preferences.midiRemote[RMIDI_NOTE1].type        != -1);
      rca3->setChecked(preferences.midiRemote[RMIDI_NOTE2].type        != -1);
      rca4->setChecked(preferences.midiRemote[RMIDI_NOTE4].type        != -1);
      rca5->setChecked(preferences.midiRemote[RMIDI_NOTE8].type        != -1);
      rca6->setChecked(preferences.midiRemote[RMIDI_NOTE16].type       != -1);
      rca7->setChecked(preferences.midiRemote[RMIDI_NOTE32].type       != -1);
      rca8->setChecked(preferences.midiRemote[RMIDI_NOTE64].type      != -1);
      rca9->setChecked(preferences.midiRemote[RMIDI_DOT].type         != -1);
      rca10->setChecked(preferences.midiRemote[RMIDI_DOTDOT].type      != -1);
      rca11->setChecked(preferences.midiRemote[RMIDI_REST].type        != -1);
      rca12->setChecked(preferences.midiRemote[RMIDI_TIE].type        != -1);
      editModeActive->setChecked(preferences.midiRemote[RMIDI_NOTE_EDIT_MODE].type != -1);

      int id = mscore->midiRecordId();
      recordRewind->setChecked(id == RMIDI_REWIND);
      recordTogglePlay->setChecked(id == RMIDI_TOGGLE_PLAY);
      recordPlay->setChecked(id == RMIDI_PLAY);
      recordStop->setChecked(id == RMIDI_STOP);
      rcr2->setChecked(id       == RMIDI_NOTE1);
      rcr3->setChecked(id       == RMIDI_NOTE2);
      rcr4->setChecked(id       == RMIDI_NOTE4);
      rcr5->setChecked(id       == RMIDI_NOTE8);
      rcr6->setChecked(id       == RMIDI_NOTE16);
      rcr7->setChecked(id       == RMIDI_NOTE32);
      rcr8->setChecked(id       == RMIDI_NOTE64);
      rcr9->setChecked(id       == RMIDI_REST);
      rcr10->setChecked(id      == RMIDI_DOT);
      rcr11->setChecked(id      == RMIDI_DOTDOT);
      rcr12->setChecked(id      == RMIDI_TIE);
      recordEditMode->setChecked(id == RMIDI_NOTE_EDIT_MODE);
      }

//---------------------------------------------------------
//   updateValues
//---------------------------------------------------------

void PreferenceDialog::updateValues(Preferences* p)
      {
      rcGroup->setChecked(p->useMidiRemote);
      fgWallpaper->setText(p->fgWallpaper);
      bgWallpaper->setText(p->bgWallpaper);

      bgColorButton->setChecked(p->bgUseColor);
      bgWallpaperButton->setChecked(!p->bgUseColor);
      fgColorButton->setChecked(p->fgUseColor);
      fgWallpaperButton->setChecked(!p->fgUseColor);

      if (p->bgUseColor) {
            bgColorLabel->setColor(p->bgColor);
            bgColorLabel->setPixmap(0);
            }
      else {
            bgColorLabel->setPixmap(new QPixmap(bgWallpaper->text()));
            }

      if (p->fgUseColor) {
            fgColorLabel->setColor(p->fgColor);
            fgColorLabel->setPixmap(0);
            }
      else {
            fgColorLabel->setPixmap(new QPixmap(fgWallpaper->text()));
            }

      replaceFractions->setChecked(p->replaceFractions);
      replaceCopyrightSymbol->setChecked(p->replaceCopyrightSymbol);

      enableMidiInput->setChecked(p->enableMidiInput);
      playNotes->setChecked(p->playNotes);

      //Update
      checkUpdateStartup->clear();
      int curPeriodIdx = 0;

      for(unsigned i = 0; i < sizeof(updatePeriods)/sizeof(*updatePeriods); ++i) {
            checkUpdateStartup->addItem(qApp->translate("preferences", updatePeriods[i].text), i);
            if (updatePeriods[i].time == p->checkUpdateStartup)
                  curPeriodIdx = i;
            }
      checkUpdateStartup->setCurrentIndex(curPeriodIdx);

      if (seq->isRunning()) {
            QList<QString> sl = seq->inputPorts();
            int idx = 0;
            for (QList<QString>::iterator i = sl.begin(); i != sl.end(); ++i, ++idx) {
                  jackRPort->addItem(*i);
                  jackLPort->addItem(*i);
                  if (p->rPort == *i)
                        jackRPort->setCurrentIndex(idx);
                  if (p->lPort == *i)
                        jackLPort->setCurrentIndex(idx);
                  }
            }
      else {
            jackRPort->setEnabled(false);
            jackLPort->setEnabled(false);
            }

      soundFont->setText(p->soundFont);
      navigatorShow->setChecked(p->showNavigator);
      playPanelShow->setChecked(p->showPlayPanel);
      playPanelX->setValue(p->playPanelPos.x());
      playPanelY->setValue(p->playPanelPos.y());

      alsaDriver->setChecked(p->useAlsaAudio);
      jackDriver->setChecked(p->useJackAudio);
      portaudioDriver->setChecked(p->usePortaudioAudio);
      useJackMidi->setChecked(p->useJackMidi);

      alsaDevice->setText(p->alsaDevice);

      int index = alsaSampleRate->findText(QString("%1").arg(p->alsaSampleRate));
      alsaSampleRate->setCurrentIndex(index);
      index = alsaPeriodSize->findText(QString("%1").arg(p->alsaPeriodSize));
      alsaPeriodSize->setCurrentIndex(index);

      alsaFragments->setValue(p->alsaFragments);
      drawAntialiased->setChecked(p->antialiasedDrawing);
      switch(p->sessionStart) {
            case EMPTY_SESSION:  emptySession->setChecked(true); break;
            case LAST_SESSION:   lastSession->setChecked(true); break;
            case NEW_SESSION:    newSession->setChecked(true); break;
            case SCORE_SESSION:  scoreSession->setChecked(true); break;
            }
      sessionScore->setText(p->startScore);
      workingDirectory->setText(p->workingDirectory);
      showSplashScreen->setChecked(p->showSplashScreen);
      expandRepeats->setChecked(p->midiExpandRepeats);
      instrumentList->setText(p->instrumentList);

      midiPorts->setValue(p->midiPorts);
      rememberLastMidiConnections->setChecked(p->rememberLastMidiConnections);
      proximity->setValue(p->proximity);
      autoSave->setChecked(p->autoSave);
      autoSaveTime->setValue(p->autoSaveTime);
      pngResolution->setValue(p->pngResolution);
      pngTransparent->setChecked(p->pngTransparent);
      for (int i = 0; i < language->count(); ++i) {
            if (language->itemText(i).startsWith(p->language)) {
                  language->setCurrentIndex(i);
                  break;
                  }
            }
      //
      // initialize local shortcut table
      //    we need a deep copy to be able to rewind all
      //    changes on "Abort"
      //
      foreach(Shortcut* s, shortcuts) {
            Shortcut* ns = new Shortcut(*s);
            ns->action   = 0;
            localShortcuts[s->xml] = ns;
            }
      updateSCListView();

      //
      // initialize portaudio
      //
#ifdef USE_PORTAUDIO
      if (usePortaudio) {
            Portaudio* audio = static_cast<Portaudio*>(seq->getDriver());

            QStringList apis = audio->apiList();
            portaudioApi->addItems(apis);
            portaudioApi->setCurrentIndex(audio->currentApi());

            QStringList devices = audio->deviceList(audio->currentApi());
            portaudioDevice->addItems(devices);
            portaudioDevice->setCurrentIndex(audio->currentDevice());

            connect(portaudioApi, SIGNAL(activated(int)), SLOT(portaudioApiActivated(int)));
#ifdef USE_PORTMIDI
            PortMidiDriver* midiDriver = static_cast<PortMidiDriver*>(audio->mididriver());
            if(midiDriver){
                QStringList midiInputs = midiDriver->deviceInList();
                int curMidiInIdx = 0;
                for(int i = 0; i < midiInputs.size(); ++i) {
                      portMidiInput->addItem(midiInputs.at(i), i);
                      if (midiInputs.at(i) == p->portMidiInput)
                            curMidiInIdx = i;
                      }
                portMidiInput->setCurrentIndex(curMidiInIdx);
                }
#endif
            }
#endif

#ifndef HAS_MIDI
      enableMidiInput->setEnabled(false);
      rcGroup->setEnabled(false);
#endif
      pageGroup->clear();
      for (int i = 0; true; ++i) {
            if (paperSizes[i].name == 0)
                  break;
            pageGroup->addItem(QString(paperSizes[i].name));
            }
      //
      // score settings
      //
      bool mm = true;
      const char* suffix = mm ? "mm" : "in";
      pageGroup->setCurrentIndex(p->paperSize);
      paperWidth->setSuffix(suffix);
      paperHeight->setSuffix(suffix);
      paperWidth->blockSignals(true);
      paperHeight->blockSignals(true);

      double pw = p->paperWidth;
      double ph = p->paperHeight;
      if (p->paperSize != QPrinter::Custom) {
            pw = paperSizes[p->paperSize].w;
            ph = paperSizes[p->paperSize].h;
            }
      if (p->landscape) {
            paperWidth->setValue(ph * INCH);
            paperHeight->setValue(pw * INCH);
            }
      else {
            paperWidth->setValue(pw * INCH);
            paperHeight->setValue(ph * INCH);
            }

      paperWidth->blockSignals(false);
      paperHeight->blockSignals(false);

      twosided->setChecked(p->twosided);
      spatiumEntry->setValue(p->spatium * INCH);
      scale->setValue(p->mag);

      landscape->setChecked(p->landscape);

      defaultPlayDuration->setValue(p->defaultPlayDuration);
      importStyleFile->setText(p->importStyleFile);
      useImportBuildinStyle->setChecked(p->importStyleFile.isEmpty());
      useImportStyleFile->setChecked(!p->importStyleFile.isEmpty());

      importCharsetList->clear();
      QList<QByteArray> charsets = QTextCodec::availableCodecs();
      qSort(charsets.begin(), charsets.end());
      int idx = 0;
      foreach (QByteArray charset, charsets) {
            importCharsetList->addItem(charset);
            if (charset == p->importCharset)
                  importCharsetList->setCurrentIndex(idx);
            idx++;
            }

      warnPitchRange->setChecked(p->warnPitchRange);

      language->clear();
      int curIdx = 0;
      for(int i = 0; i < mscore->languages().size(); ++i) {
            language->addItem(mscore->languages().at(i).name, i);
            if (mscore->languages().at(i).key == p->language)
                  curIdx = i;
            }
      language->setCurrentIndex(curIdx);

      oscServer->setChecked(p->useOsc);
      oscPort->setValue(p->oscPort);

      styleName->setCurrentIndex(p->globalStyle);

      sfChanged = false;
      }

//---------------------------------------------------------
//   portaudioApiActivated
//---------------------------------------------------------

#ifdef USE_PORTAUDIO
void PreferenceDialog::portaudioApiActivated(int idx)
      {
      Portaudio* audio = static_cast<Portaudio*>(seq->getDriver());
      QStringList devices = audio->deviceList(idx);
      portaudioDevice->clear();
      portaudioDevice->addItems(devices);
      }
#else
void PreferenceDialog::portaudioApiActivated(int)  {}
#endif

//---------------------------------------------------------
//   ShortcutITem
//---------------------------------------------------------

bool ShortcutItem::operator<(const QTreeWidgetItem& item) const
      {

      const QTreeWidget * pTree =treeWidget ();
      int column   = pTree ? pTree->sortColumn() : 0;
      return QString::localeAwareCompare(text(column).toLower(), item.text(column).toLower()) > 0;
      }

//---------------------------------------------------------
//   updateSCListView
//---------------------------------------------------------

void PreferenceDialog::updateSCListView()
      {
      shortcutList->clear();
      foreach (Shortcut* s, localShortcuts) {
            if (!s)
                  continue;
            ShortcutItem* newItem = new ShortcutItem;
            newItem->setText(0, s->descr);
            if (s->icon != -1)
                  newItem->setIcon(0, *icons[s->icon]);
            if(!s->key.isEmpty())
                newItem->setText(1, s->key.toString(QKeySequence::NativeText));
            else{
                QList<QKeySequence> list = QKeySequence::keyBindings(s->standardKey);
                if (list.size() > 0)
                    newItem->setText(1, list.at(0).toString(QKeySequence::NativeText));
                }
            newItem->setData(0, Qt::UserRole, s->xml);
            shortcutList->addTopLevelItem(newItem);
            }
      shortcutList->resizeColumnToContents(0);
      }

//---------------------------------------------------------
//   resetShortcutClicked
//    reset all shortcuts to buildin defaults
//---------------------------------------------------------

void PreferenceDialog::resetShortcutClicked()
      {
      printf("resetShortcutClicked\n");
      }

//---------------------------------------------------------
//   clearShortcutClicked
//---------------------------------------------------------

void PreferenceDialog::clearShortcutClicked()
      {
      QTreeWidgetItem* active = shortcutList->currentItem();
      if (active) {
            QString str = active->data(0, Qt::UserRole).toString();
            if (!str.isEmpty()) {
                  Shortcut* s = localShortcuts[str];
                  s->key = 0;
                  active->setText(1, s->key.toString(QKeySequence::NativeText));
                  }
            }
      }

//---------------------------------------------------------
//   defineShortcutClicked
//---------------------------------------------------------

void PreferenceDialog::defineShortcutClicked()
      {
      QTreeWidgetItem* active = shortcutList->currentItem();
      if(active){
          QString str = active->data(0, Qt::UserRole).toString();
          if (str.isEmpty())
                return;
          Shortcut* s = localShortcuts[str];
          ShortcutCaptureDialog sc(s, localShortcuts, this);
          if (sc.exec()) {
                s->key = sc.getKey();
                active->setText(1, s->key.toString(QKeySequence::NativeText));
                shortcutsChanged = true;
                }
//        clearButton->setEnabled(true);
          }
      }

//---------------------------------------------------------
//   selectFgWallpaper
//---------------------------------------------------------

void PreferenceDialog::selectFgWallpaper()
      {
      QString s = QFileDialog::getOpenFileName(
         this,                            // parent
         tr("Choose Notepaper"),          // caption
         fgWallpaper->text(),             // dir
         tr("Images (*.jpg *.gif *.png)") // filter
         );
      if (!s.isNull())
            fgWallpaper->setText(s);
      }

//---------------------------------------------------------
//   selectBgWallpaper
//---------------------------------------------------------

void PreferenceDialog::selectBgWallpaper()
      {
      QString s = QFileDialog::getOpenFileName(
         this,
         tr("Choose Background Wallpaper"),
         bgWallpaper->text(),
         tr("Images (*.jpg *.gif *.png)")
         );
      if (!s.isNull())
            bgWallpaper->setText(s);
      }

//---------------------------------------------------------
//   selectWorkingDirectory
//---------------------------------------------------------

void PreferenceDialog::selectWorkingDirectory()
      {
      QString s = QFileDialog::getExistingDirectory(
         this,
         tr("Choose Working Directory"),
         workingDirectory->text()
         );
      if (!s.isNull())
            workingDirectory->setText(s);
      }

//---------------------------------------------------------
//   selectDefaultStyle
//---------------------------------------------------------

void PreferenceDialog::selectDefaultStyle()
      {
      QString s = QFileDialog::getExistingDirectory(
         this,
         tr("Choose Default Style"),
         defaultStyle->text()
         );
      if (!s.isNull())
            defaultStyle->setText(s);
      }

//---------------------------------------------------------
//   selectInstrumentList
//---------------------------------------------------------

void PreferenceDialog::selectInstrumentList()
      {
      QString s = QFileDialog::getOpenFileName(
         this,
         tr("Choose Instrument List"),
         instrumentList->text(),
         tr("Instrument List (*.xml)")
         );
      if (!s.isNull())
            instrumentList->setText(s);
      }

//---------------------------------------------------------
//   selectStartWith
//---------------------------------------------------------

void PreferenceDialog::selectStartWith()
      {
      QString s = QFileDialog::getOpenFileName(
         this,
         tr("Choose Starting Score"),
         sessionScore->text(),
         tr("MuseScore Files (*.mscz *.mscx *.msc);;All (*)")
         );
      if (!s.isNull())
            sessionScore->setText(s);
      }

//---------------------------------------------------------
//   fgClicked
//---------------------------------------------------------

void PreferenceDialog::fgClicked(bool id)
      {
      fgWallpaper->setEnabled(!id);
      fgWallpaperSelect->setEnabled(!id);

      if (id) {
            // fgColorLabel->setColor(p->fgColor);
            fgColorLabel->setPixmap(0);
            }
      else {
            fgColorLabel->setPixmap(new QPixmap(fgWallpaper->text()));
            }
      }

//---------------------------------------------------------
//   bgClicked
//---------------------------------------------------------

void PreferenceDialog::bgClicked(bool id)
      {
      bgWallpaper->setEnabled(!id);
      bgWallpaperSelect->setEnabled(!id);

      if (id) {
            // bgColorLabel->setColor(p->bgColor);
            bgColorLabel->setPixmap(0);
            }
      else {
            bgColorLabel->setPixmap(new QPixmap(bgWallpaper->text()));
            }
      }

//---------------------------------------------------------
//   buttonBoxClicked
//---------------------------------------------------------

void PreferenceDialog::buttonBoxClicked(QAbstractButton* button)
      {
      switch(buttonBox->standardButton(button)) {
            case QDialogButtonBox::Apply:
                  apply();
                  break;
            case QDialogButtonBox::Ok:
                  apply();
            case QDialogButtonBox::Cancel:
            default:
                  hide();
                  break;
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void PreferenceDialog::apply()
      {
      preferences.useMidiRemote  = rcGroup->isChecked();
      preferences.fgWallpaper    = fgWallpaper->text();
      preferences.bgWallpaper    = bgWallpaper->text();
      preferences.fgColor        = fgColorLabel->color();
      preferences.bgColor        = bgColorLabel->color();

      preferences.bgUseColor     = bgColorButton->isChecked();
      preferences.fgUseColor     = fgColorButton->isChecked();
      preferences.enableMidiInput = enableMidiInput->isChecked();
      preferences.playNotes      = playNotes->isChecked();
      if (preferences.lPort != jackLPort->currentText()
         || preferences.rPort != jackRPort->currentText()) {
            // TODO: change ports
            preferences.lPort       = jackLPort->currentText();
            preferences.rPort       = jackRPort->currentText();
            }
      preferences.soundFont          = soundFont->text();
      preferences.showNavigator      = navigatorShow->isChecked();
      preferences.showPlayPanel      = playPanelShow->isChecked();
      preferences.playPanelPos       = QPoint(playPanelX->value(), playPanelY->value());
      preferences.antialiasedDrawing = drawAntialiased->isChecked();

      if (
         (preferences.useAlsaAudio != alsaDriver->isChecked())
         || (preferences.useJackAudio != jackDriver->isChecked())
         || (preferences.usePortaudioAudio != portaudioDriver->isChecked())
         || (preferences.useJackMidi != useJackMidi->isChecked())
         || (preferences.alsaDevice != alsaDevice->text())
         || (preferences.alsaSampleRate != alsaSampleRate->currentText().toInt())
         || (preferences.alsaPeriodSize != alsaPeriodSize->currentText().toInt())
         || (preferences.alsaFragments != alsaFragments->value())
            ) {
            seq->stop();
#ifndef __MINGW32__
            while(!seq->isStopped())
                  usleep(50000);
#endif
            seq->exit();
            preferences.useAlsaAudio       = alsaDriver->isChecked();
            preferences.useJackAudio       = jackDriver->isChecked();
            preferences.usePortaudioAudio  = portaudioDriver->isChecked();
            preferences.useJackMidi        = useJackMidi->isChecked();
            preferences.alsaDevice         = alsaDevice->text();
            preferences.alsaSampleRate     = alsaSampleRate->currentText().toInt();
            preferences.alsaPeriodSize     = alsaPeriodSize->currentText().toInt();
            preferences.alsaFragments      = alsaFragments->value();
            if (!seq->init()) {
                  printf("sequencer init failed\n");
                  }
            }

#ifdef USE_PORTAUDIO
      Portaudio* audio = static_cast<Portaudio*>(seq->getDriver());
      preferences.portaudioDevice = audio->deviceIndex(portaudioApi->currentIndex(),
         portaudioDevice->currentIndex());
#endif

#ifdef USE_PORTMIDI
      preferences.portMidiInput = portMidiInput->currentText();
#endif

      if (lastSession->isChecked())
            preferences.sessionStart = LAST_SESSION;
      else if (newSession->isChecked())
            preferences.sessionStart = NEW_SESSION;
      else if (scoreSession->isChecked())
            preferences.sessionStart = SCORE_SESSION;
      else if (emptySession->isChecked())
            preferences.sessionStart = EMPTY_SESSION;
      preferences.startScore         = sessionScore->text();
      preferences.workingDirectory   = workingDirectory->text();
      preferences.showSplashScreen   = showSplashScreen->isChecked();
      preferences.midiExpandRepeats  = expandRepeats->isChecked();
      preferences.instrumentList     = instrumentList->text();

      preferences.midiPorts          = midiPorts->value();
      preferences.rememberLastMidiConnections = rememberLastMidiConnections->isChecked();
      preferences.proximity          = proximity->value();
      preferences.autoSave           = autoSave->isChecked();
      preferences.autoSaveTime       = autoSaveTime->value();
      preferences.pngResolution      = pngResolution->value();
      preferences.pngTransparent     = pngTransparent->isChecked();
      converterDpi                   = preferences.pngResolution;

      if (shortcutsChanged) {
            shortcutsChanged = false;
            foreach(Shortcut* s, localShortcuts) {
                  Shortcut* os = shortcuts[s->xml];
                  if (os->key != s->key) {
                        os->key = s->key;
                        if (os->action)
                              os->action->setShortcut(s->key);
                        }
                  }
            }
      int lang = language->itemData(language->currentIndex()).toInt();
      QString l = lang == 0 ? "system" : mscore->languages().at(lang).key;
      bool languageChanged = l != preferences.language;
      preferences.language = l;

      preferences.replaceFractions       = replaceFractions->isChecked();
      preferences.replaceCopyrightSymbol = replaceCopyrightSymbol->isChecked();

      //update
      int periodIndex = checkUpdateStartup->currentIndex();
      int t = updatePeriods[periodIndex].time;
      preferences.checkUpdateStartup = t;

      bool mmUnit = true;
      double f  = mmUnit ? 1.0/INCH : 1.0;
      preferences.twosided    = twosided->isChecked();
      preferences.spatium     = spatiumEntry->value() / INCH;
      preferences.mag         = scale->value();
      preferences.landscape   = landscape->isChecked();
      preferences.paperSize   = QPrinter::PageSize(pageGroup->currentIndex());
      preferences.paperHeight = paperHeight->value() * f;
      preferences.paperWidth  = paperWidth->value()  * f;

      preferences.defaultPlayDuration = defaultPlayDuration->value();

      if (useImportStyleFile->isChecked())
            preferences.importStyleFile = importStyleFile->text();
      else
            preferences.importStyleFile.clear();

      preferences.importCharset = importCharsetList->currentText();
      preferences.warnPitchRange = warnPitchRange->isChecked();

      preferences.useOsc  = oscServer->isChecked();
      preferences.oscPort = oscPort->value();
      if (styleName->currentIndex() == 0) {
            iconGroup = "icons-dark/";
            appStyleFile = ":/data/appstyle-dark.css";
            preferences.styleName = "dark";
            preferences.globalStyle = 0;
            }
      else {
            iconGroup = "icons/";
            appStyleFile = ":/data/appstyle.css";
            preferences.styleName = "light";
            preferences.globalStyle = 1;
            }

      if (languageChanged) {
            setMscoreLocale(preferences.language);
            mscore->update();
            }

      qApp->setStyleSheet(appStyleSheet());
      genIcons();

      emit preferencesChanged();
      preferences.write();
      mscore->startAutoSave();
      }

//---------------------------------------------------------
//   playPanelCurClicked
//---------------------------------------------------------

void PreferenceDialog::playPanelCurClicked()
      {
      PlayPanel* w = mscore->getPlayPanel();
      if (w == 0)
            return;
      QPoint s(w->pos());
      playPanelX->setValue(s.x());
      playPanelY->setValue(s.y());
      }

//---------------------------------------------------------
//   writeShortcuts
//---------------------------------------------------------

void writeShortcuts()
      {
      QSettings s;
      s.beginGroup("Shortcuts");

      int n = 0;
      foreach(Shortcut* shortcut, shortcuts) {
            for (unsigned i = 0;; ++i) {
                  if (MuseScore::sc[i].xml == shortcut->xml) {
                        if (MuseScore::sc[i].key != shortcut->key) {
                              QString tag("sc[%1]");
                              s.setValue(tag.arg(n), shortcut->xml);
                              tag = "seq[%1]";
                              s.setValue(tag.arg(n), shortcut->key.toString(QKeySequence::PortableText));
                              ++n;
                              }
                        break;
                        }
                  }
            }
      s.setValue("n", n);
      s.endGroup();
      }

//---------------------------------------------------------
//   readShortcuts
//---------------------------------------------------------

void readShortcuts()
      {
      if (useFactorySettings)
            return;
      QSettings s;
      s.beginGroup("Shortcuts");
      int n = s.value("n", 0).toInt();

      for (int i = 0; i < n; ++i) {
            QString tag("sc[%1]");
            QString name = s.value(tag.arg(i)).toString();
            tag = "seq[%1]";
            QString seq = s.value(tag.arg(i)).toString();
            Shortcut* s = shortcuts.value(name);
            if (s)
                  s->key = QKeySequence::fromString(seq, QKeySequence::PortableText);
            else
                  printf("MuseScore:readShortCuts: unknown tag <%s>\n", qPrintable(name));
            }
      s.endGroup();
      }

//---------------------------------------------------------
//   getShortcut
//---------------------------------------------------------

Shortcut* getShortcut(const char* id)
      {
      Shortcut* s = shortcuts.value(id);
      if (s == 0) {
            printf("internal error: shortcut <%s> not found\n", id);
            return 0;
            }
      return s;
      }

//---------------------------------------------------------
//   getAction
//    returns action for shortcut
//---------------------------------------------------------

QAction* getAction(const char* id)
      {
      Shortcut* s = getShortcut(id);
      return getAction(s);
      }

QAction* getAction(Shortcut* s)
      {
      if (s == 0)
            return 0;
      if (s->action == 0) {
            QAction* a = new QAction(s->xml, 0); // mscore);
            s->action  = a;
            a->setData(s->xml);
            if(!s->key.isEmpty())
                a->setShortcut(s->key);
            else
                a->setShortcuts(s->standardKey);
            a->setShortcutContext(s->context);
            if (!s->help.isEmpty()) {
                  a->setToolTip(s->help);
                  a->setWhatsThis(s->help);
                  }
            else {
                  a->setToolTip(s->descr);
                  a->setWhatsThis(s->descr);
                  }
            if (s->standardKey != QKeySequence::UnknownKey) {
                  QList<QKeySequence> kl = a->shortcuts();
                  if (!kl.isEmpty()) {
                        QString s(a->toolTip());
                        s += " (";
                        for (int i = 0; i < kl.size(); ++i) {
                              if (i)
                                    s += ",";
                              s += kl[i].toString(QKeySequence::NativeText);
                              }
                        s += ")";
                        a->setToolTip(s);
                        }
                  }
            else if (!s->key.isEmpty()) {
                  a->setToolTip(a->toolTip() +
                        " (" + s->key.toString(QKeySequence::NativeText) + ")" );
                  }
            if (!s->text.isEmpty())
                  a->setText(s->text);
            if (s->icon != -1)
                  a->setIcon(*icons[s->icon]);
            }
      return s->action;
      }

//---------------------------------------------------------
//   resetAllValues
//---------------------------------------------------------

void PreferenceDialog::resetAllValues()
      {
      Preferences prefs;
      updateValues(&prefs);

      shortcutsChanged = true;
      foreach(Shortcut* sc, localShortcuts)
            delete sc;
      localShortcuts.clear();
      for (unsigned i = 0;; ++i) {
            if (MuseScore::sc[i].xml == 0)
                  break;
            localShortcuts[MuseScore::sc[i].xml] = new Shortcut(MuseScore::sc[i]);
            }
      updateSCListView();
      }

//---------------------------------------------------------
//   paperSizeChanged
//---------------------------------------------------------

void PreferenceDialog::paperSizeChanged(double)
      {
      pageGroup->setCurrentIndex(paperSizeNameToIndex("Custom"));
      }

//---------------------------------------------------------
//   landscapeToggled
//---------------------------------------------------------

void PreferenceDialog::landscapeToggled(bool /*flag*/)
      {
      }

//---------------------------------------------------------
//   pageFormatSelected
//---------------------------------------------------------

void PreferenceDialog::pageFormatSelected(int pf)
      {
      paperWidth->blockSignals(true);
      paperHeight->blockSignals(true);

      double pw = paperWidth->value();
      double ph = paperHeight->value();
      if (pf != QPrinter::Custom) {
            pw = paperSizes[pf].w;
            ph = paperSizes[pf].h;
            }
      if (landscape->isChecked()) {
            paperWidth->setValue(ph * INCH);
            paperHeight->setValue(pw * INCH);
            }
      else {
            paperWidth->setValue(pw * INCH);
            paperHeight->setValue(ph * INCH);
            }

      paperWidth->blockSignals(false);
      paperHeight->blockSignals(false);
      }

//---------------------------------------------------------
//   styleFileButtonClicked
//---------------------------------------------------------

void PreferenceDialog::styleFileButtonClicked()
      {
      QString fn = QFileDialog::getOpenFileName(
         0, QWidget::tr("MuseScore: Load Style"),
         QString("."),
            QWidget::tr("MuseScore Styles (*.mss);;"
            "All Files (*)"
            )
         );
      if (fn.isEmpty())
            return;
      importStyleFile->setText(fn);
      }

//---------------------------------------------------------
//   midiRemoteControlClearClicked
//---------------------------------------------------------

void PreferenceDialog::midiRemoteControlClearClicked()
      {
      for (int i = 0; i < MIDI_REMOTES; ++i)
            preferences.midiRemote[i].type = MIDI_REMOTE_TYPE_INACTIVE;
      updateRemote();
      }

//---------------------------------------------------------
//   selectSoundFont
//---------------------------------------------------------

void PreferenceDialog::selectSoundFont()
      {
      QString s = ::getSoundFont(soundFont->text());
      soundFont->setText(s);
      }

