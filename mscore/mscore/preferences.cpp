//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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
#include "synti.h"
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

extern void writeShortcuts();
extern void readShortcuts();

bool useALSA = false, useJACK = false, usePortaudio = false;

extern bool useFactorySettings;

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
  PeriodItem(7*24,      QT_TRANSLATE_NOOP("preferences","Every week")),
  PeriodItem(2*7*24,      QT_TRANSLATE_NOOP("preferences","Every 2 weeks")),
  PeriodItem(30*24,      QT_TRANSLATE_NOOP("preferences","Every month")),
  PeriodItem(2*30*24,      QT_TRANSLATE_NOOP("preferences","Every 2 months")),
  PeriodItem(-1,      QT_TRANSLATE_NOOP("preferences","Never")),
};

//---------------------------------------------------------
//   appStyleSheet
//---------------------------------------------------------

QString appStyleSheet()
      {
      QFont fff;
      if (preferences.applicationFont.isEmpty())
            fff = QApplication::font();
      else
            fff.fromString(preferences.applicationFont);
      return QString(
      "* { font-size: %1pt; font-family: \"%2\"}\n"
//      "PaletteBoxButton  { font-size: 8px; background-color: rgb(215, 215, 215) }\n"
//      "PaletteBoxButton  { background-color: rgb(215, 215, 215) }\n"
//      "PaletteBox        { background-color: rgb(230, 230, 230) }\n"
      "PlayPanel QLabel#posLabel   { font-size: 28pt; font-family: \"San Serif\" }\n"
      "PlayPanel QLabel#timeLabel      { font-size: 28pt; font-family: \"San Serif\" }\n"
      "SynthControl QLabel#titleLabel  { font-size: 24pt; font-family: \"San Serif\" }\n"
      "ChordEdit QLabel#chordLabel { font-size: 24pt; font-family: \"San Serif\" }\n"
      "PlayPanel QLabel#tempoLabel { font-size: 10pt; font-family: \"San Serif\" }\n"
      "PlayPanel QLabel#relTempo   { font-size: 10pt; font-family: \"San Serif\" }\n"
      "AboutBoxDialog QLabel#titleLabel { font-size: 28pt  }\n")
         .arg(fff.pointSize())
         .arg(fff.family());
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

      //selectColor[0] = Qt::blue;
      //selectColor[1] = Qt::green;
      //selectColor[2] = Qt::yellow;
      //selectColor[3] = Qt::magenta;
      selectColor[0].setRgb(0, 0, 255);     //blue
      selectColor[1].setRgb(0, 150, 0);     //green
      selectColor[2].setRgb(230, 180, 50);  //yellow
      selectColor[3].setRgb(200, 0, 200);   //purple
      dropColor      = Qt::red;
      defaultColor   = Qt::black;

      enableMidiInput    = true;
      playNotes          = true;

      lPort              = "";
      rPort              = "";
      showNavigator      = true;
      showPlayPanel      = false;
      showWebPanel       = true;
      showStatusBar      = true;

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
      portMidiInput      = "";
      midiPorts          = 2;
      rememberLastMidiConnections = true;

      layoutBreakColor         = Qt::green;
      antialiasedDrawing       = true;
      sessionStart             = SCORE_SESSION;
      startScore               = ":/data/Promenade_Example.mscx";
      workingDirectory         = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
      showSplashScreen         = true;

      rewind.type              = -1;
      play.type                = -1;
      stop.type                = -1;
      len1.type                = -1;
      len2.type                = -1;
      len4.type                = -1;
      len8.type                = -1;
      len16.type               = -1;
      len32.type               = -1;
      len3.type                = -1;
      len6.type                = -1;
      len12.type               = -1;
      len24.type               = -1;

      midiExpandRepeats        = true;
      playRepeats              = true;
      instrumentList           = ":/data/instruments.xml";

      alternateNoteEntryMethod = false;
      proximity                = 6;
      autoSave                 = true;
      autoSaveTime             = 2;       // minutes
      pngScreenShot            = false;
      language                 = "system";
      iconWidth                = 24;
      iconHeight               = 24;
      noteEntryIconWidth       = ICON_WIDTH;
      noteEntryIconHeight      = ICON_HEIGHT;
      applicationFont          = "";
      style                    = "";

      replaceCopyrightSymbol  = true;
      replaceFractions        = true;

      paperSize               = QPrinter::A4;     // default paper size
      paperWidth              = 1.0;
      paperHeight             = 1.0;
      landscape               = false;
      twosided                = true;
      spatium                 = SPATIUM20;
      tuning                  = 440.0f;
      masterGain              = 0.2;
      chorusGain              = 0.5;
      reverbGain              = 0.2;
      reverbRoomSize          = 0.5;
      reverbDamp              = 0.5;
      reverbWidth             = 1.0;

      defaultPlayDuration     = 300;      // ms
      warnPitchRange          = true;
      followSong              = true;
      importCharset           = "GBK";

      //update
      checkUpdateStartup      = 0;
      
      firstStartWeb = true;
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
      s.setValue("showWebPanel",       showWebPanel);
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
//      s.setValue("lastSaveDirectory",  lastSaveDirectory);
//      s.setValue("lastSaveCopyDirectory",  lastSaveCopyDirectory);
      s.setValue("showSplashScreen",   showSplashScreen);

      s.setValue("midiExpandRepeats",  midiExpandRepeats);
      s.setValue("playRepeats",        playRepeats);
      s.setValue("instrumentList", instrumentList);

      s.setValue("alternateNoteEntry", alternateNoteEntryMethod);
      s.setValue("proximity",          proximity);
      s.setValue("autoSave",           autoSave);
      s.setValue("autoSaveTime",       autoSaveTime);
      s.setValue("pngScreenShot",      pngScreenShot);
      s.setValue("language",           language);
      s.setValue("iconHeight",          iconHeight);
      s.setValue("iconWidth",           iconWidth);
      s.setValue("noteEntryIconHeight", noteEntryIconHeight);
      s.setValue("noteEntryIconWidth",  noteEntryIconWidth);
      s.setValue("applicationFont", applicationFont);
      s.setValue("style", style);

      s.setValue("replaceFractions", replaceFractions);
      s.setValue("replaceCopyrightSymbol", replaceCopyrightSymbol);
      s.setValue("paperSize", paperSize);
      s.setValue("paperWidth", paperWidth);
      s.setValue("paperHeight", paperHeight);
      s.setValue("landscape", landscape);
      s.setValue("twosided", twosided);
      s.setValue("spatium", spatium);
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
      
      s.setValue("firstStartWeb", firstStartWeb);

      //update
      s.setValue("checkUpdateStartup", checkUpdateStartup);

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
      defaultSoundfont = mscoreGlobalShare+"sound/TimGM6mb.sf2";
      soundFont       = s.value("soundFont", defaultSoundfont).toString();
      if (soundFont == ":/data/piano1.sf2") {
            // silently change to new default sound font
            soundFont = defaultSoundfont;
            }
      showNavigator   = s.value("showNavigator", true).toBool();
      showStatusBar   = s.value("showStatusBar", true).toBool();
      showPlayPanel   = s.value("showPlayPanel", false).toBool();
      showWebPanel    = s.value("showWebPanel", true).toBool();

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

      showSplashScreen         = s.value("showSplashScreen", true).toBool();
      midiExpandRepeats        = s.value("midiExpandRepeats", true).toBool();
      playRepeats              = s.value("playRepeats", true).toBool();
      alternateNoteEntryMethod = s.value("alternateNoteEntry", false).toBool();
      midiPorts                = s.value("midiPorts", 2).toInt();
      rememberLastMidiConnections = s.value("rememberLastMidiConnections", true).toBool();
      proximity                = s.value("proximity", 6).toInt();
      autoSave                 = s.value("autoSave", true).toBool();
      autoSaveTime             = s.value("autoSaveTime", 2).toInt();
      pngScreenShot            = s.value("pngScreenShot", false).toBool();
      language                 = s.value("language", "system").toString();
      iconHeight               = s.value("iconHeight", 24).toInt();
      iconWidth                = s.value("iconHeight", 24).toInt();
      noteEntryIconHeight      = s.value("noteEntryIconHeight", ICON_HEIGHT).toInt();
      noteEntryIconWidth       = s.value("noteEntryIconWidth", ICON_WIDTH).toInt();
      applicationFont          = s.value("applicationFont", "").toString();
      style                    = s.value("style", "").toString();

      replaceFractions = s.value("replaceFractions", true).toBool();
      replaceCopyrightSymbol = s.value("replaceCopyrightSymbol", true).toBool();
      paperSize              = QPrinter::PageSize(s.value("paperSize", QPrinter::A4).toInt());
      paperWidth             = s.value("paperWidth", 1.0).toDouble();
      paperHeight            = s.value("paperHeight", 1.0).toDouble();
      landscape              = s.value("landscape", false).toBool();
      twosided               = s.value("twosided", true).toBool();
      spatium                = s.value("spatium", SPATIUM20).toDouble();
      tuning                 = s.value("tuning", 440.0).toDouble();
      masterGain             = s.value("masterGain", 0.2).toDouble();
      chorusGain             = s.value("chorusGain", 0.5).toDouble();
      reverbGain             = s.value("reverbGain", 0.2).toDouble();
      reverbRoomSize         = s.value("reverbRoomSize", 0.5).toDouble();
      reverbDamp             = s.value("reverbDamp", 0.5).toDouble();
      reverbWidth            = s.value("reverbWidth", 1.0).toDouble();

      defaultPlayDuration    = s.value("defaultPlayDuration", 300).toInt();
      importStyleFile        = s.value("importStyleFile", "").toString();
      importCharset          = s.value("importCharset", "GBK").toString();
      warnPitchRange         = s.value("warnPitchRange", true).toBool();
      followSong             = s.value("followSong", true).toBool();
      
      firstStartWeb = s.value("firstStartWeb", true).toBool();

      checkUpdateStartup = s.value("checkUpdateStartup", UpdateChecker::defaultPeriod()).toInt();
      if (checkUpdateStartup == 0){
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
      shortcutsChanged        = false;

#ifndef USE_JACK
      jackDriver->setEnabled(false);
#endif
#ifndef USE_ALSA
      alsaDriver->setEnabled(false);
#endif
#ifndef USE_PORTAUDIO
      portaudioDriver->setEnabled(false);
#endif
#ifndef USE_PORTMIDI
      // portmidiDriverInput->setEnabled(false);
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

      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(buttonBoxClicked(QAbstractButton*)));
      connect(fgWallpaperSelect,  SIGNAL(clicked()), SLOT(selectFgWallpaper()));
      connect(bgWallpaperSelect,  SIGNAL(clicked()), SLOT(selectBgWallpaper()));
      connect(workingDirectoryButton, SIGNAL(clicked()), SLOT(selectWorkingDirectory()));
      connect(instrumentListButton,   SIGNAL(clicked()), SLOT(selectInstrumentList()));
      connect(startWithButton,        SIGNAL(clicked()), SLOT(selectStartWith()));
      connect(shortcutList, SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(defineShortcutClicked()));
      connect(resetShortcut, SIGNAL(clicked()), SLOT(resetShortcutClicked()));
      connect(clearShortcut, SIGNAL(clicked()), SLOT(clearShortcutClicked()));
      connect(defineShortcut, SIGNAL(clicked()), SLOT(defineShortcutClicked()));
      connect(resetToDefault, SIGNAL(clicked()), SLOT(resetAllValues()));

      connect(paperHeight, SIGNAL(valueChanged(double)), SLOT(paperSizeChanged(double)));
      connect(paperWidth,  SIGNAL(valueChanged(double)), SLOT(paperSizeChanged(double)));
      connect(pageGroup,   SIGNAL(activated(int)), SLOT(pageFormatSelected(int)));
      connect(landscape,   SIGNAL(toggled(bool)), SLOT(landscapeToggled(bool)));

      connect(styleFileButton, SIGNAL(clicked()), SLOT(styleFileButtonClicked()));
      }

//---------------------------------------------------------
//   updateValues
//---------------------------------------------------------

void PreferenceDialog::updateValues(Preferences* p)
      {
      fgWallpaper->setText(p->fgWallpaper);
      bgWallpaper->setText(p->bgWallpaper);

      fgColorLabel->setColor(p->fgColor);
      bgColorLabel->setColor(p->bgColor);

      selectColorLabel1->setColor(p->selectColor[0]);
      selectColorLabel2->setColor(p->selectColor[1]);
      selectColorLabel3->setColor(p->selectColor[2]);
      selectColorLabel4->setColor(p->selectColor[3]);
      selectColorDefault->setColor(p->defaultColor);
      selectColorDrop->setColor(p->dropColor);

      bgColorButton->setChecked(p->bgUseColor);
      bgWallpaperButton->setChecked(!p->bgUseColor);
      fgColorButton->setChecked(p->fgUseColor);
      fgWallpaperButton->setChecked(!p->fgUseColor);

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

      navigatorShow->setChecked(p->showNavigator);
      playPanelShow->setChecked(p->showPlayPanel);
      webPanelShow->setChecked(p->showWebPanel);

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
      alternateInput->setChecked(p->alternateNoteEntryMethod);

      midiPorts->setValue(p->midiPorts);
      rememberLastMidiConnections->setChecked(p->rememberLastMidiConnections);
      proximity->setValue(p->proximity);
      autoSave->setChecked(p->autoSave);
      autoSaveTime->setValue(p->autoSaveTime);
      pngScreenShot->setChecked(p->pngScreenShot);
      for (int i = 0; i < language->count(); ++i) {
            if (language->itemText(i).startsWith(p->language)) {
                  language->setCurrentIndex(i);
                  break;
                  }
            }
      iconHeight->setValue(p->iconHeight);
      iconWidth->setValue(p->iconWidth);
      noteEntryIconHeight->setValue(p->noteEntryIconHeight);
      noteEntryIconWidth->setValue(p->noteEntryIconWidth);
      {
      QFont ff;
      ff.fromString(p->applicationFont);
      applicationFont->setCurrentFont(ff);
      applicationFontSize->setValue(ff.pointSize());
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

      QStringList sl = QStyleFactory::keys();
      styleCombo->addItem(tr("default"));
      styleCombo->addItems(sl);
      int idx = 1;
      foreach(const QString& s, sl) {
            if (s == p->style)
                  break;
            ++idx;
            }
      if (idx > sl.size())
            idx = 0;
      styleCombo->setCurrentIndex(idx);


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

      landscape->setChecked(p->landscape);

      defaultPlayDuration->setValue(p->defaultPlayDuration);
      importStyleFile->setText(p->importStyleFile);
      useImportBuildinStyle->setChecked(p->importStyleFile.isEmpty());
      useImportStyleFile->setChecked(!p->importStyleFile.isEmpty());

      importCharsetList->clear();
      QList<QByteArray> charsets = QTextCodec::availableCodecs();
      qSort(charsets.begin(), charsets.end());
      idx = 0;
      foreach (QByteArray charset, charsets) {
    	  importCharsetList->addItem(charset);
    	  if(charset == p->importCharset) {
    		  importCharsetList->setCurrentIndex(idx);
    	  }
    	  idx++;
      }

      warnPitchRange->setChecked(p->warnPitchRange);

      language->clear();
      int curIdx = 0;
      for(int i = 0; i < languages.size(); ++i) {
            language->addItem(languages.at(i).name, i);
            if (languages.at(i).key == p->language)
                  curIdx = i;
            }
      language->setCurrentIndex(curIdx);

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
      QTreeWidgetItem* active = shortcutList->currentItem();
      if (!active)
            return;
      QString str = active->data(0, Qt::UserRole).toString();
      if (str.isEmpty())
            return;
      Shortcut* shortcut = localShortcuts[str];

      for (unsigned i = 0;; ++i) {
            if (MuseScore::sc[i].xml == shortcut->xml) {
                  shortcut->key = MuseScore::sc[i].key;
                  active->setText(1, shortcut->key.toString(QKeySequence::NativeText));
                  shortcutsChanged = true;
                  break;
                  }
            }
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
      fgColorLabel->setEnabled(id);
      fgWallpaper->setEnabled(!id);
      fgWallpaperSelect->setEnabled(!id);
      }

//---------------------------------------------------------
//   bgClicked
//---------------------------------------------------------

void PreferenceDialog::bgClicked(bool id)
      {
      bgColorLabel->setEnabled(id);
      bgWallpaper->setEnabled(!id);
      bgWallpaperSelect->setEnabled(!id);
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
      preferences.selectColor[0] = selectColorLabel1->color();
      preferences.selectColor[1] = selectColorLabel2->color();
      preferences.selectColor[2] = selectColorLabel3->color();
      preferences.selectColor[3] = selectColorLabel4->color();
      preferences.dropColor      = selectColorDrop->color();
      preferences.defaultColor   = selectColorDefault->color();
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
      preferences.showNavigator      = navigatorShow->isChecked();
      preferences.showPlayPanel      = playPanelShow->isChecked();
      preferences.showWebPanel       = webPanelShow->isChecked();

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
      preferences.alternateNoteEntryMethod = alternateInput->isChecked();

      preferences.midiPorts          = midiPorts->value();
      preferences.rememberLastMidiConnections = rememberLastMidiConnections->isChecked();
      preferences.proximity          = proximity->value();
      preferences.autoSave           = autoSave->isChecked();
      preferences.autoSaveTime       = autoSaveTime->value();
      preferences.pngScreenShot      = pngScreenShot->isChecked();

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
      QString l = lang == 0 ? "system" : languages.at(lang).key;
      bool languageChanged = l != preferences.language;
      preferences.language = l;

      preferences.iconHeight          = iconHeight->value();
      preferences.iconWidth           = iconWidth->value();
      preferences.noteEntryIconHeight = noteEntryIconHeight->value();
      preferences.noteEntryIconWidth  = noteEntryIconWidth->value();
      QFont fff = applicationFont->currentFont();
      fff.setPointSize(applicationFontSize->value());
      preferences.applicationFont     = fff.toString();

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

      if (languageChanged) {
            setMscoreLocale(preferences.language);
            mscore->update();
            }

      qApp->setStyleSheet(appStyleSheet());

      if (styleCombo->currentIndex() != 0) {
            QString s = styleCombo->currentText();
            QApplication::setStyle(s);
            preferences.style = s;
            }
      else
            preferences.style = QString();
      genIcons();

      emit preferencesChanged();
      preferences.write();
      if (sfChanged) {
            if (!seq->isRunning()) {
                  // try to start sequencer
                  seq->init();
                  }
            if (seq->isRunning()) {
                  sfChanged = false;
                  Synth* synth = seq->getDriver()->getSynth();
                  if (synth)
                        synth->loadSoundFont(preferences.soundFont);
                  }
            }
      mscore->startAutoSave();
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


