//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: mscore.cpp,v 1.105 2006/09/15 09:34:57 wschweer Exp $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#include <fenv.h>

#include "config.h"
#include "mscore.h"
#include "canvas.h"
#include "style.h"
#include "score.h"
#include "instrdialog.h"
#include "preferences.h"
#include "icons.h"
#include "textstyle.h"
#include "xml.h"
#include "seq.h"
#include "icons.h"
#include "tempo.h"
#include "sym.h"
#include "padids.h"
#include "pagesettings.h"
#include "listedit.h"
#include "editstyle.h"
#include "playpanel.h"
#include "page.h"
#include "partedit.h"
#include "layout.h"
#include "palette.h"
#include "part.h"
#include "drumset.h"
#include "instrtemplate.h"
#include "note.h"
#include "staff.h"
#include "driver.h"
#include "harmony.h"
#include "magbox.h"
#include "voiceselector.h"

#ifdef STATIC_SCRIPT_BINDINGS
Q_IMPORT_PLUGIN(com_trolltech_qt_gui_ScriptPlugin)
Q_IMPORT_PLUGIN(com_trolltech_qt_core_ScriptPlugin)
Q_IMPORT_PLUGIN(com_trolltech_qt_network_ScriptPlugin)
Q_IMPORT_PLUGIN(com_trolltech_qt_uitools_ScriptPlugin)
Q_IMPORT_PLUGIN(com_trolltech_qt_xml_ScriptPlugin)
#endif

QString dataPath;
int division = 480;     // 480 midi ticks represent a quarter note

QPaintDevice* pdev;
double PDPI, DPI, DPMM;
double SPATIUM;

QTextStream cout(stdout);
QTextStream eout(stderr);

QString mscoreGlobalShare;

static QStringList recentScores;

Shortcut* midiActionMap[128];
QMap<QString, Shortcut*> shortcuts;
bool converterMode = false;
double converterDpi = 300;

static const char* outFileName;
static const char* styleFile;
static QString localeName;
bool useFactorySettings = false;

//---------------------------------------------------------
//   NoteButton
//---------------------------------------------------------

NoteButton::NoteButton(QWidget* parent)
   : QToolButton(parent)
      {
      }

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize NoteButton::sizeHint() const
      {
      int w = preferences.noteEntryIconWidth;
      int h = preferences.noteEntryIconHeight;
      return QSize(w, h);
      }

//---------------------------------------------------------
// cmdInsertMeasure
//---------------------------------------------------------

void MuseScore::cmdInsertMeasures()
      {
	if (cs) {
		insertMeasuresDialog = new InsertMeasuresDialog;
		insertMeasuresDialog->show();
            }
      }

//---------------------------------------------------------
// InsertMeasuresDialog
//---------------------------------------------------------

InsertMeasuresDialog::InsertMeasuresDialog(QWidget* parent)
   : QDialog(parent)
      {
	setupUi(this);
      }

//---------------------------------------------------------
// Insert Measure -->   accept
//---------------------------------------------------------

void InsertMeasuresDialog::accept()
      {
	int n = insmeasures->value();
	if (mscore->currentScore())
            mscore->currentScore()->cmdInsertMeasures(n);
	done(1);
      }

//---------------------------------------------------------
//   getSharePath
//---------------------------------------------------------

static QString getSharePath()
      {
#ifdef __MINGW32__
      QDir dir(QCoreApplication::applicationDirPath() + QString("/../" INSTALL_NAME));
      return dir.absolutePath() + "/";
#else
      return QString( INSTPREFIX "/share/" INSTALL_NAME);
#endif
      }

//---------------------------------------------------------
//   printVersion
//---------------------------------------------------------

static void printVersion(const char* prog)
      {
      extern int revision;

#ifdef MSCORE_UNSTABLE
      cout << prog << ": Linux Music Score Editor\nUnstable Prerelease for Version " << VERSION
           << "; Build " << revision << endl;
#else
      cout << prog << ": Linux Music Score Editor; Version " << VERSION
           << "  Build " << revision << endl;
#endif
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void MuseScore::closeEvent(QCloseEvent* ev)
      {
      foreach(Score* score, scoreList) {
            if (checkDirty(score)) {
                  ev->ignore();
                  return;
                  }
            }
      saveScoreList();
      writeSettings();
      if (pageListEdit)
            pageListEdit->writeSettings();

      seq->stop();
#ifndef __MINGW32__
      while(!seq->isStopped())
            usleep(50000);
#endif
      seq->exit();
      ev->accept();
      if (preferences.dirty)
            preferences.write();
      QSettings s;
      s.setValue("lastSaveDirectory",  preferences.lastSaveDirectory);
      s.setValue("lastSaveCopyDirectory",  preferences.lastSaveCopyDirectory);

      //
      // close all toplevel windows
      //
      foreach(QWidget* w, qApp->topLevelWidgets()) {
            if (w != this)
                  w->close();
            }
      }

//---------------------------------------------------------
//   preferencesChanged
//---------------------------------------------------------

void MuseScore::preferencesChanged()
      {
      for (int i = 0; i < tab->count(); ++i) {
            Canvas* canvas = static_cast<Canvas*>(tab->widget(i));
            if (preferences.bgUseColor)
                  canvas->setBackground(preferences.bgColor);
            else {
                  QPixmap* pm = new QPixmap(preferences.bgWallpaper);
                  canvas->setBackground(pm);
                  }
            if (preferences.fgUseColor)
                  canvas->setForeground(preferences.fgColor);
            else {
                  QPixmap* pm = new QPixmap(preferences.fgWallpaper);
                  if (pm == 0 || pm->isNull())
                        printf("no valid pixmap %s\n", preferences.fgWallpaper.toLatin1().data());
                  canvas->setForeground(pm);
                  }
            }

      transportTools->setEnabled(!noSeq);
      playId->setEnabled(!noSeq);

      getAction("midi-on")->setEnabled(preferences.enableMidiInput);
      _statusBar->setShown(preferences.showStatusBar);
      }

//---------------------------------------------------------
//   MuseScore
//---------------------------------------------------------

MuseScore::MuseScore()
   : QMainWindow()
      {
      setIconSize(QSize(preferences.iconWidth, preferences.iconHeight));
      setWindowTitle(QString("MuseScore"));
      setAcceptDrops(true);
      cs                    = 0;
      se                    = 0;    // script engine
      debugger              = 0;
      instrList             = 0;
      playPanel             = 0;
      preferenceDialog      = 0;
      measuresDialog        = 0;
      insertMeasuresDialog  = 0;
      iledit                = 0;
      pageListEdit          = 0;
      measureListEdit       = 0;
      symbolDialog          = 0;
      clefPalette           = 0;
      keyPalette            = 0;
      timePalette           = 0;
      barPalette            = 0;
      fingeringPalette      = 0;
      linePalette           = 0;
      bracketPalette        = 0;
      dynamicsPalette       = 0;
      pageSettings          = 0;
      noteAttributesPalette = 0;
      accidentalsPalette    = 0;
      layoutBreakPalette    = 0;
      paletteBox            = 0;
      _midiinEnabled        = true;
      _speakerEnabled       = true;
      newWizard             = 0;
      drumPalette           = 0;
      drumset               = 0;
      lastOpenPath          = preferences.workingDirectory;
      _textTools            = 0;

      _positionLabel = new QLabel;
      _positionLabel->setText("001:01:000");
      _positionLabel->setAutoFillBackground(true);
      QPalette p(_positionLabel->palette());
      p.setColor(QPalette::Window, QColor(176, 190, 242));
      _positionLabel->setPalette(p);

      _modeText = new QLabel;
      _modeText->setAutoFillBackground(true);
      p.setColor(QPalette::Window, QColor(176, 190, 242));
      _modeText->setPalette(p);
      _statusBar = new QStatusBar;
      _statusBar->addPermanentWidget(_modeText, 0);
      _statusBar->addPermanentWidget(_positionLabel, 0);

      setStatusBar(_statusBar);

      QAction* a;

      // otherwise unused actions:
      //   must be added somewere to work

      QActionGroup* ag = new QActionGroup(this);
      ag->setExclusive(false);
      QStringList sl;
      sl << "page-prev" << "page-next" << "page-top" << "page-end"
         << "add-slur" << "add-hairpin" << "add-hairpin-reverse"
         << "escape" << "delete" << "rest" << "pitch-up" << "pitch-down"
         << "rest-1" << "rest-2" << "rest-4" << "rest-8"
         << "pitch-up-octave" << "pitch-down-octave"
         << "move-up" << "move-down" << "up-chord" << "down-chord"
         << "top-chord" << "bottom-chord" << "next-chord" << "prev-chord"
         << "select-next-chord" << "select-prev-chord" << "select-next-measure" << "select-prev-measure"
         << "select-begin-line" << "select-end-line"
         << "select-begin-score" << "select-end-score"
         << "select-staff-above" << "select-staff-below"
         << "next-measure" << "prev-measure" << "print" << "undo" << "redo"
         << "append-measure" << "append-measures" << "insert-measure" << "insert-measures"
         << "insert-hbox" << "insert-vbox" << "append-hbox" << "append-vbox"
         << "duplet" << "triplet" << "quadruplet" << "quintuplet" << "sextuplet"
         << "septuplet" << "octuplet" << "nonuplet" << "tuplet-dialog"
         << "note-c" << "note-d" << "note-e" << "note-f" << "note-g"
         << "note-a" << "note-b"
         << "chord-c" << "chord-d" << "chord-e" << "chord-f" << "chord-g"
         << "chord-a" << "chord-b"
         << "stretch+" << "stretch-"
         << "instruments" << "clefs" << "keys" << "symbols" << "times" << "dynamics"
         << "title-text" << "subtitle-text" << "composer-text" << "poet-text" << "chord-text"
         << "rehearsalmark-text" << "copyright-text"
         << "lyrics" << "fingering" << "system-text" << "staff-text" << "tempo"
         << "cut" << "copy" << "paste"
         << "file-open" << "file-new" << "file-save" << "file-save-as"
         << "file-save-a-copy" << "file-reload" << "file-close"
         << "quit"
         << "toggle-statusbar" << "note-input" << "pitch-spell"
         << "rewind" << "play" << "pause" <<"repeat"
         << "play-next-measure" << "play-next-chord" << "play-prev-measure" << "play-prev-chord"
         << "seek-begin" << "seek-end"
         << "load-style" << "save-style" << "select-all" << "transpose" << "concert-pitch"
         << "reset-beammode"
         << "clef-violin" << "clef-bass"
	   << "add-staccato" << "add-trill"
         << "voice-x12" << "voice-x13" << "voice-x14" << "voice-x23" << "voice-x24" << "voice-x34"
         << "repeat-cmd"
         << "edit-meta"
         << "harmony-properties"
         << "system-break" << "page-break"
         << "edit-element"
         << "mag" << "reset-positions" << "inspector" << "script-debug"
         ;

      foreach(const QString& s, sl) {
            QAction* a = getAction(s.toLatin1().data());
            addAction(a);
            ag->addAction(a);
            }

      connect(ag, SIGNAL(triggered(QAction*)), SLOT(cmd(QAction*)));

      QWidget* mainWindow = new QWidget;
      layout = new QVBoxLayout;
      mainWindow->setLayout(layout);
      layout->setMargin(0);
      layout->setSpacing(0);
      tab = new QTabWidget;
      tab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      removeTabButton = new QToolButton;
      removeTabButton->setIcon(QIcon(QPixmap(":/data/tab_remove.png")));
      tab->setCornerWidget(removeTabButton);
      connect(removeTabButton, SIGNAL(clicked()), SLOT(removeTab()));

      layout->addWidget(tab);

      connect(tab, SIGNAL(currentChanged(int)), SLOT(setCurrentScore(int)));

      QAction* whatsThis = QWhatsThis::createAction(this);

      //---------------------------------------------------
      //    Transport Action
      //---------------------------------------------------

#ifdef HAS_MIDI
      a  = getAction("midi-on");
      a->setCheckable(true);
      a->setEnabled(preferences.enableMidiInput);
      a->setChecked(_midiinEnabled);
      connect(a, SIGNAL(triggered(bool)), SLOT(midiinToggled(bool)));
#endif

      a = getAction("sound-on");
      a->setCheckable(true);
      a->setEnabled(preferences.playNotes);
      a->setChecked(_speakerEnabled);
      connect(a, SIGNAL(triggered(bool)), SLOT(speakerToggled(bool)));

      a = getAction("play");
      a->setCheckable(true);
      a = getAction("pause");
      a->setCheckable(true);
      a = getAction("repeat");
      a->setCheckable(true);
      a->setChecked(true);

      //---------------------------------------------------
      //    File Action
      //---------------------------------------------------

      //---------------------
      //    Tool Bar
      //---------------------

      fileTools = addToolBar(tr("File Operations"));
      fileTools->setObjectName("file-operations");
      fileTools->addAction(getAction("file-new"));
      fileTools->addAction(getAction("file-open"));
      fileTools->addAction(getAction("file-save"));

      fileTools->addAction(getAction("print"));
      fileTools->addAction(whatsThis);
      fileTools->addSeparator();
      fileTools->addAction(getAction("undo"));
      fileTools->addAction(getAction("redo"));
      fileTools->addSeparator();

      transportTools = addToolBar(tr("Transport Tools"));
      transportTools->setObjectName("transport-tools");
      transportTools->addAction(getAction("sound-on"));
#ifdef HAS_MIDI
      transportTools->addAction(getAction("midi-on"));
#endif
      transportTools->addSeparator();
      transportTools->addAction(getAction("rewind"));
      transportTools->addAction(getAction("pause"));
      transportTools->addAction(getAction("play"));
      transportTools->addSeparator();
      a = getAction("repeat");
      a->setChecked(preferences.playRepeats);
      transportTools->addAction(a);

      fileTools->addAction(getAction("mag"));

      mag = new MagBox;
      connect(mag, SIGNAL(magChanged(int)), SLOT(magChanged(int)));
      fileTools->addWidget(mag);
      addToolBarBreak();

      cpitchTools = addToolBar(tr("Concert Pitch"));
      cpitchTools->setObjectName("pitch-tools");
      cpitchTools->addAction(getAction("concert-pitch"));

      //-------------------------------
      //    Note Entry Tool Bar
      //-------------------------------

      entryTools = addToolBar(tr("Note Entry"));
      entryTools->setObjectName("entry-tools");
      entryTools->setIconSize(QSize(preferences.noteEntryIconWidth, preferences.noteEntryIconHeight));

      a = getAction("note-input");
      a->setCheckable(true);
      entryTools->addAction(a);

      QStringList sl1;
      sl1 << "pad-note-64" << "pad-note-32" << "pad-note-16" << "pad-note-8"
         << "pad-note-4" << "pad-note-2" << "pad-note-1" << "note-breve" << "note-longa"
         << "pad-dot"
         << "pad-dotdot" << "tie" << "pad-rest" << "pad-sharp2" << "pad-sharp"
         << "pad-nat" << "pad-flat"  <<"pad-flat2";

      foreach(const QString& s, sl1) {
            NoteButton* nb = new NoteButton;
            QAction* a = getAction(s.toLatin1().data());
            if (s != "tie")
                  a->setCheckable(true);
            ag->addAction(a);
            nb->setDefaultAction(a);
            entryTools->addWidget(nb);
            // entryTools->addAction(a);
            if (s == "tie" || s == "pad-rest")
                  entryTools->addSeparator();
            }

      sl1.clear();
      sl1 << "pad-appoggiatura" << "pad-acciaccatura" << "pad-grace4" <<"pad-grace16" << "pad-grace32"
          << "beam-start" << "beam-mid" << "no-beam" << "beam32" << "auto-beam"
          << "show-invisible" << "show-frames";

      foreach(const QString& s, sl1) {
            QAction* a = getAction(s.toLatin1().data());
            a->setCheckable(true);
            ag->addAction(a);
            }

      a = getAction("flip");
      ag->addAction(a);
      entryTools->addAction(a);
      entryTools->addSeparator();

      VoiceSelector* vw = new VoiceSelector;
      entryTools->addWidget(vw);
      connect(vw, SIGNAL(triggered(QAction*)), SLOT(cmd(QAction*)));

      //---------------------
      //    Menus
      //---------------------

      QMenuBar* mb = menuBar();

      //---------------------
      //    Menu File
      //---------------------

      QMenu* menuFile = mb->addMenu(tr("&File"));
      menuFile->setObjectName("File");

      menuFile->addAction(getAction("file-new"));
      menuFile->addAction(getAction("file-open"));
      openRecent = menuFile->addMenu(fileOpenIcon, tr("Open &Recent"));
      connect(openRecent, SIGNAL(aboutToShow()), SLOT(openRecentMenu()));
      connect(openRecent, SIGNAL(triggered(QAction*)), SLOT(selectScore(QAction*)));
      menuFile->addSeparator();
      menuFile->addAction(getAction("file-save"));
      menuFile->addAction(getAction("file-save-as"));
      menuFile->addAction(getAction("file-save-a-copy"));
      menuFile->addSeparator();
      menuFile->addAction(getAction("file-reload"));
      menuFile->addSeparator();
      menuFile->addAction(getAction("file-close"));

      menuFile->addSeparator();
      menuFile->addAction(tr("Parts..."), this, SLOT(startExcerptsDialog()));
      menuFile->addAction(getAction("print"));
      menuFile->addSeparator();
      menuFile->addAction(getAction("quit"));

      //---------------------
      //    Menu Edit
      //---------------------

      menuEdit = mb->addMenu(tr("&Edit"));
      menuEdit->setObjectName("Edit");
      menuEdit->addAction(getAction("undo"));
      menuEdit->addAction(getAction("redo"));

      menuEdit->addSeparator();

      menuEdit->addAction(getAction("cut"));
      menuEdit->addAction(getAction("copy"));
      a = getAction("paste");
      a->setEnabled(false);
      menuEdit->addAction(a);
      selectionChanged(0);
      menuEdit->addSeparator();
      menuEdit->addAction(getAction("select-all"));
      menuEdit->addSeparator();

      QMenu* menuVoices = new QMenu(tr("Voices"));
      menuVoices->addAction(getAction("voice-x12"));
      menuVoices->addAction(getAction("voice-x13"));
      menuVoices->addAction(getAction("voice-x14"));
      menuVoices->addAction(getAction("voice-x23"));
      menuVoices->addAction(getAction("voice-x24"));
      menuVoices->addAction(getAction("voice-x34"));
      menuEdit->addMenu(menuVoices);

      menuEdit->addSeparator();
      menuEdit->addAction(getAction("edit-meta"));
      menuEdit->addSeparator();
      menuEdit->addAction(getAction("inspector"));
      menuEdit->addSeparator();
      menuEdit->addAction(tr("Preferences..."), this, SLOT(startPreferenceDialog()));

      //---------------------
      //    Menu Create
      //---------------------

      menuCreate = genCreateMenu(mb);
      mb->setObjectName("Create");
      mb->addMenu(menuCreate);

      //---------------------
      //    Menu Notes
      //---------------------

      menuNotes = mb->addMenu(qApp->translate("MenuNotes", "Notes"));
      menuNotes->setObjectName("Notes");

      menuNotes->addAction(getAction("note-input"));
      menuNotes->addAction(getAction("pitch-spell"));
      menuNotes->addSeparator();

      QMenu* menuAddPitch = new QMenu(tr("Add Note"));
      for (int i = 0; i < 7; ++i) {
            char buffer[8];
            sprintf(buffer, "note-%c", "cdefgab"[i]);
            a = getAction(buffer);
            menuAddPitch->addAction(a);
            }
      menuNotes->addMenu(menuAddPitch);

      for (int i = 0; i < 7; ++i) {
            char buffer[8];
            sprintf(buffer, "chord-%c", "cdefgab"[i]);
            a = getAction(buffer);
            menuAddPitch->addAction(a);
            }

      QMenu* menuAddInterval = new QMenu(tr("Add Interval"));
      for (int i = 1; i < 10; ++i) {
            char buffer[16];
            sprintf(buffer, "interval%d", i);
            a = getAction(buffer);
            ag->addAction(a);
            menuAddInterval->addAction(a);
            }
      menuAddInterval->addSeparator();
      for (int i = 2; i < 10; ++i) {
            char buffer[16];
            sprintf(buffer, "interval-%d", i);
            a = getAction(buffer);
            ag->addAction(a);
            menuAddInterval->addAction(a);
            }
      menuNotes->addMenu(menuAddInterval);

      QMenu* menuNtole = new QMenu(tr("Tuplets"));
      menuNtole->addAction(getAction("duplet"));
      menuNtole->addAction(getAction("triplet"));
      menuNtole->addAction(getAction("quadruplet"));
      menuNtole->addAction(getAction("quintuplet"));
      menuNtole->addAction(getAction("sextuplet"));
      menuNtole->addAction(getAction("septuplet"));
      menuNtole->addAction(getAction("octuplet"));
      menuNtole->addAction(getAction("nonuplet"));
      menuNtole->addAction(getAction("tuplet-dialog"));
      menuNotes->addMenu(menuNtole);

      menuNotes->addSeparator();
      menuNotes->addAction(getAction("transpose"));
      a = getAction("concert-pitch");
      a->setCheckable(true);
      menuNotes->addAction(a);

      //---------------------
      //    Menu Layout
      //---------------------

      menuLayout = mb->addMenu(tr("&Layout"));
      menuLayout->setObjectName("Layout");

      menuLayout->addAction(tr("Page Settings..."), this, SLOT(showPageSettings()));

      menuLayout->addAction(getAction("reset-positions"));
      menuLayout->addAction(getAction("stretch+"));
      menuLayout->addAction(getAction("stretch-"));

      menuLayout->addAction(getAction("reset-stretch"));
      menuLayout->addAction(getAction("reset-beammode"));
      menuLayout->addAction(tr("Breaks..."), this, SLOT(showLayoutBreakPalette()));

      //---------------------
      //    Menu Style
      //---------------------

      menuStyle = mb->addMenu(tr("&Style"));
      menuStyle->setObjectName("Style");
      menuStyle->addAction(tr("Edit Style..."), this, SLOT(editStyle()));
      menuStyle->addAction(tr("Edit Text Style..."), this, SLOT(editTextStyle()));
      menuStyle->addSeparator();
      menuStyle->addAction(getAction("load-style"));
      menuStyle->addAction(getAction("save-style"));

      //---------------------
      //    Menu Display
      //---------------------

      menuDisplay = mb->addMenu(tr("&Display"));
      menuDisplay->setObjectName("Display");

      a = getAction("toggle-palette");
      a->setCheckable(true);
      connect(a, SIGNAL(toggled(bool)), SLOT(showPalette(bool)));
      menuDisplay->addAction(a);

      playId = getAction("toggle-playpanel");
      playId->setCheckable(true);
      connect(playId, SIGNAL(toggled(bool)), SLOT(showPlayPanel(bool)));
      menuDisplay->addAction(playId);

      navigatorId = getAction("toggle-navigator");
      navigatorId->setCheckable(true);
      connect(navigatorId, SIGNAL(toggled(bool)), SLOT(showNavigator(bool)));
      menuDisplay->addAction(navigatorId);

      a = getAction("toggle-mixer");
      a->setCheckable(true);
      connect(a, SIGNAL(toggled(bool)), SLOT(showMixer(bool)));
      menuDisplay->addAction(a);

      menuDisplay->addSeparator();

      transportId = getAction("toggle-transport");
      transportId->setCheckable(true);
      transportId->setChecked(true);
      menuDisplay->addAction(transportId);
      connect(transportId, SIGNAL(toggled(bool)), transportTools, SLOT(setVisible(bool)));

      inputId = getAction("toggle-noteinput");
      inputId->setCheckable(true);
      inputId->setChecked(true);
      menuDisplay->addAction(inputId);
      connect(inputId, SIGNAL(toggled(bool)), entryTools, SLOT(setVisible(bool)));

      a = getAction("toggle-statusbar");
      a->setCheckable(true);
      a->setChecked(preferences.showStatusBar);
      menuDisplay->addAction(a);

      menuDisplay->addSeparator();
      menuDisplay->addAction(getAction("show-invisible"));
      menuDisplay->addAction(getAction("show-frames"));

      //---------------------
      //    Menu Help
      //---------------------

      mb->addSeparator();
      QMenu* menuHelp = mb->addMenu(tr("&Help"));
      menuHelp->setObjectName("Help");

      menuHelp->addAction(tr("Local Manual"),  this, SLOT(helpBrowser()), Qt::Key_F1);
      menuHelp->addAction(tr("Online Manual"), this, SLOT(helpBrowser1()));
      menuHelp->addAction(tr("&About"),   this, SLOT(about()));
      menuHelp->addAction(tr("About&Qt"), this, SLOT(aboutQt()));
      menuHelp->addSeparator();

      a = getAction("script-debug");
      a->setCheckable(true);
      a->setChecked(scriptDebug);
      menuHelp->addAction(a);
#ifndef HAS_SCRIPT_DEBUG
      a->setEnabled(false);
#endif

      menuHelp->addAction(whatsThis);

      setCentralWidget(mainWindow);

      loadInstrumentTemplates(preferences.instrumentList);
      preferencesChanged();
      connect(seq, SIGNAL(started()), SLOT(seqStarted()));
      connect(seq, SIGNAL(stopped()), SLOT(seqStopped()));
      loadScoreList();

      QClipboard* cb = QApplication::clipboard();
      connect(cb, SIGNAL(dataChanged()), SLOT(clipboardChanged()));
      connect(cb, SIGNAL(selectionChanged()), SLOT(clipboardChanged()));
      autoSaveTimer = new QTimer(this);
      autoSaveTimer->setSingleShot(true);
      connect(autoSaveTimer, SIGNAL(timeout()), this, SLOT(autoSaveTimerTimeout()));
      startAutoSave();
      }

//---------------------------------------------------------
//   startAutoSave
//---------------------------------------------------------

void MuseScore::startAutoSave()
      {
      if (preferences.autoSave) {
            int t = preferences.autoSaveTime * 60 * 1000;
            autoSaveTimer->start(t);
            }
      else
            autoSaveTimer->stop();
      }

//---------------------------------------------------------
//   autoSaveTimerTimeout
//---------------------------------------------------------

void MuseScore::autoSaveTimerTimeout()
      {
      foreach(Score* s, scoreList) {
            if (s->dirty() && !s->created()) {
                  printf("auto save <%s>\n", qPrintable(s->name()));
                  s->saveFile(true);
                  }
            }
      if (preferences.autoSave) {
            int t = preferences.autoSaveTime * 60 * 1000;
            autoSaveTimer->start(t);
            }
      }

//---------------------------------------------------------
//   helpBrowser
//    show local help
//---------------------------------------------------------

void MuseScore::helpBrowser()
      {
      QString lang(localeName.left(2));
      if (debugMode) {
            printf("open manual for language <%s>\n", qPrintable(lang));
            }

      QFileInfo mscoreHelp(mscoreGlobalShare + QString("man/MuseScore-") + lang + QString(".pdf"));
      if (!mscoreHelp.isReadable()) {
            if (debugMode) {
                  printf("cannot open doc <%s>\n", qPrintable(mscoreHelp.filePath()));
                  }
            mscoreHelp.setFile(mscoreGlobalShare + QString("man/MuseScore-en.pdf"));
            if (!mscoreHelp.isReadable()) {
                  QString info(tr("MuseScore online manual not found at: "));
                  info += mscoreHelp.filePath();
                  QMessageBox::critical(this, tr("MuseScore: Open Help"), info);
                  return;
                  }
            }
      QString p = mscoreHelp.filePath();
      p = p.replace(" ", "%20");    // HACK: why does'nt fromLocalFile() do this?
      QUrl url(QUrl::fromLocalFile(p));
      QDesktopServices::openUrl(url);
      }

//---------------------------------------------------------
//   helpBrowser1
//    show online help
//---------------------------------------------------------

void MuseScore::helpBrowser1()
      {
      QString lang(localeName.left(2));
      if (debugMode)
            printf("open online manual for language <%s>\n", qPrintable(lang));
      QString help("http://musescore.org/en/handbook");
      if (lang == "de")
            help = "http://musescore.org/de/handbuch";
      else if (lang == "es")
            help = "http://musescore.org/es/manual";
      else if (lang == "fr")
            help = "http://musescore.org/fr/manuel";
      QUrl url(help);
      QDesktopServices::openUrl(url);
      }

//---------------------------------------------------------
//   aboutQt
//---------------------------------------------------------

void MuseScore::aboutQt()
      {
      QMessageBox::aboutQt(this, QString("MuseScore"));
      }

//---------------------------------------------------------
//   selectScore
//---------------------------------------------------------

void MuseScore::selectScore(QAction* action)
      {
      QString a = action->data().toString();
      if (!a.isEmpty()) {
            Score* score = new Score(defaultStyle);
            score->addViewer(new Canvas);
            score->read(a);
            appendScore(score);
            setCurrentScore(score);
            }
      }

//---------------------------------------------------------
//   selectionChanged
//---------------------------------------------------------

void MuseScore::selectionChanged(int state)
      {
      getAction("cut")->setEnabled(state);
      getAction("copy")->setEnabled(state);
      }

//---------------------------------------------------------
//   appendScore
//    append score to project list
//---------------------------------------------------------

void MuseScore::appendScore(Score* score)
      {
      if (score->getViewer().isEmpty())
            score->addViewer(new Canvas);

      connect(score, SIGNAL(dirtyChanged(Score*)), SLOT(dirtyChanged(Score*)));
      connect(score, SIGNAL(stateChanged(int)), SLOT(changeState(int)));

      for (int i = 0; i < scoreList.size(); ++i) {
            if (scoreList[i]->filePath() == score->filePath()) {
                  removeTab(i);
                  scoreList.insert(i, score);
                  tab->blockSignals(true);
                  tab->insertTab(i, score->canvas(), score->name());
                  tab->blockSignals(false);
                  return;
                  }
            }

      scoreList.push_back(score);

      tab->blockSignals(true);
      tab->addTab(score->canvas(), score->name());
      tab->blockSignals(false);

      recentScores.removeAll(score->filePath());
      recentScores.prepend(score->filePath());
      }

//---------------------------------------------------------
//   updateTabNames
//---------------------------------------------------------

void MuseScore::updateTabNames()
      {
      for (int i = 0; i < tab->count(); ++i) {
            Canvas* canvas = static_cast<Canvas*>(tab->widget(i));
            tab->setTabText(i, canvas->score()->name());
            }
      }

//---------------------------------------------------------
//   usage
//---------------------------------------------------------

static void usage(const char* prog, const char*)
      {
      printVersion(prog);
      fprintf(stderr, "usage: %s flags scorefile\n   Flags:\n", prog);
      fprintf(stderr, "   -v        print version\n"
        "   -d        debug mode\n"
        "   -D        enable plugin script debugger\n"
        "   -s        no internal synthesizer\n"
        "   -m        no midi\n"
        "   -L        layout debug\n"
        "   -I        dump midi input\n"
        "   -O        dump midi output\n"
        "   -o file   export to 'file'; format depends on file extension\n"
        "   -r dpi    set output resolution for image export\n"
        "   -S style  load style file\n"
        "   -F        use factory settings\n"
        );
      }

//---------------------------------------------------------
//   editTextStyle
//---------------------------------------------------------

void MuseScore::editTextStyle()
      {
      TextStyleDialog dialog(this, cs);
      dialog.exec();
      }

static const int RECENT_LIST_SIZE = 10;

//---------------------------------------------------------
//   saveScoreList
//---------------------------------------------------------

void MuseScore::saveScoreList()
      {
      QSettings settings;
      for (int i = 0; i < RECENT_LIST_SIZE; ++i)
            settings.setValue(QString("recent-%1").arg(i), recentScores.value(i));

      settings.setValue("scores", scoreList.size());
      settings.setValue("currentScore", scoreList.indexOf(cs));
      int idx = 0;
      foreach(Score* s, scoreList) {
            settings.setValue(QString("score-%1").arg(idx), s->fileInfo()->absoluteFilePath());
            ++idx;
            }
      }

//---------------------------------------------------------
//   loadScoreList
//    read list of "Recent Scores"
//---------------------------------------------------------

void MuseScore::loadScoreList()
      {
      if (useFactorySettings)
            return;
      QSettings s;
      for (int i = 0; i < RECENT_LIST_SIZE; ++i)
            recentScores.append(s.value(QString("recent-%1").arg(i),"").toString());
      }

//---------------------------------------------------------
//   openRecentMenu
//---------------------------------------------------------

void MuseScore::openRecentMenu()
      {
      openRecent->clear();
      foreach(QString s, recentScores) {
            if (s.isEmpty())
                  break;
            QFileInfo fi(s);
            QAction* action = openRecent->addAction(fi.completeBaseName());
            action->setData(s);
            }
      }

//---------------------------------------------------------
//   setCurrentScore
//---------------------------------------------------------

void MuseScore::setCurrentScore(int idx)
      {
      setCurrentScore(scoreList[idx]);
      }

//---------------------------------------------------------
//   setCurrentScore
//---------------------------------------------------------

void MuseScore::setCurrentScore(Score* score)
      {
      bool enable = score != 0;
      if (paletteBox)
            paletteBox->setEnabled(enable);
      transportTools->setEnabled(enable);
      cpitchTools->setEnabled(enable);
      mag->setEnabled(enable);
      entryTools->setEnabled(enable);

      QList<QObject*> ol = menuBar()->children();
      foreach(QObject* o, ol) {
            QMenu* menu = qobject_cast<QMenu*>(o);
            if (!menu)
                  continue;
            QString s(menu->objectName());
            if (s == "File" || s == "Help" || s == "Edit")
                  continue;
            menu->setEnabled(enable);
            }
      if (!enable) {
            changeState(STATE_DISABLED);
            return;
            }

      int idx = scoreList.indexOf(score);
      if (tab->currentIndex() != idx) {
            tab->blockSignals(true);
            tab->setCurrentIndex(idx);
            tab->blockSignals(false);
            }
      if (cs) {
            cs->setXoff(cs->canvas()->xoffset());
            cs->setYoff(cs->canvas()->yoffset());
            disconnect(cs, SIGNAL(selectionChanged(int)), this, SLOT(selectionChanged(int)));
            disconnect(cs, SIGNAL(posChanged(int)), this, SLOT(setPos(int)));
            }
      cs = score;
      cs->canvas()->setFocus(Qt::OtherFocusReason);

      getAction("undo")->setEnabled(!cs->undoEmpty());
      getAction("redo")->setEnabled(!cs->redoEmpty());
      getAction("file-save")->setEnabled(cs->isSavable());
      getAction("show-invisible")->setChecked(cs->showInvisible());
      getAction("show-frames")->setChecked(cs->showFrames());
      if (cs->magIdx() == MAG_FREE)
            mag->setMag(cs->mag());
      else
            mag->setMagIdx(cs->magIdx());
      double m = getMag(cs->canvas());
      cs->canvas()->setMag(m);
      cs->canvas()->setOffset(cs->xoff(), cs->yoff());


      navigatorId->setChecked(cs->canvas()->navigatorVisible());

      setWindowTitle("MuseScore: " + cs->name());
      seq->setScore(cs);
      seq->initInstruments();
      if (playPanel)
            playPanel->setScore(cs);

      cs->setLayoutAll(true);
      cs->end();
      QAction* a = getAction("concert-pitch");
      a->setChecked(cs->styleB(ST_concertPitch));

      connect(cs, SIGNAL(selectionChanged(int)), SLOT(selectionChanged(int)));
      connect(cs, SIGNAL(posChanged(int)), SLOT(setPos(int)));
      setPos(cs->inputPos());
      changeState(cs->state());
      }

//---------------------------------------------------------
//   dragEnterEvent
//---------------------------------------------------------

void MuseScore::dragEnterEvent(QDragEnterEvent* event)
      {
      const QMimeData* data = event->mimeData();
      if (data->hasUrls()) {
            QList<QUrl>ul = event->mimeData()->urls();
            foreach(const QUrl& u, ul) {
                  if (debugMode)
                        printf("drag Url: %s\n", qPrintable(u.toString()));
                  if (u.scheme() == "file") {
                        QFileInfo fi(u.path());
                        event->acceptProposedAction();
                        break;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void MuseScore::dropEvent(QDropEvent* event)
      {
      const QMimeData* data = event->mimeData();
      if (data->hasUrls()) {
            Score* lastScore = 0;
            foreach(const QUrl& u, event->mimeData()->urls()) {
                  if (u.scheme() == "file") {
                        Score* score = new Score(defaultStyle);
                        score->addViewer(new Canvas);
                        score->read(u.path());
                        appendScore(score);
                        lastScore = score;
                        }
                  }
            setCurrentScore(lastScore);
            event->acceptProposedAction();
            }
      }

//---------------------------------------------------------
//   showPageSettings
//---------------------------------------------------------

void MuseScore::showPageSettings()
      {
      if (pageSettings == 0) {
            pageSettings = new PageSettings();
            connect(pageSettings, SIGNAL(pageSettingsChanged()), SLOT(pageSettingsChanged()));
            }
      pageSettings->setScore(cs);
      pageSettings->show();
      pageSettings->raise();
      }

//---------------------------------------------------------
//   pageSettingsChanged
//---------------------------------------------------------

void MuseScore::pageSettingsChanged()
      {
      cs->canvas()->updateNavigator(true);
      cs->setLayoutAll(true);
      cs->end();
      }

//---------------------------------------------------------
//   startPageListEditor
//---------------------------------------------------------

void MuseScore::startPageListEditor()
      {
      if (!cs)
            return;
      if (pageListEdit == 0)
            pageListEdit = new PageListEditor(this);
      pageListEdit->updateList(cs);
      pageListEdit->show();
      }

//---------------------------------------------------------
//   showElementContext
//---------------------------------------------------------

void MuseScore::showElementContext(Element* el)
      {
      startPageListEditor();
      pageListEdit->setElement(el);
      }

//---------------------------------------------------------
//   editStyle
//---------------------------------------------------------

void MuseScore::editStyle()
      {
      EditStyle es(cs, this);
      es.exec();
      }

//---------------------------------------------------------
//   showPlayPanel
//---------------------------------------------------------

void MuseScore::showPlayPanel(bool visible)
      {
      if (cs == 0 || noSeq)
            return;

      if (playPanel == 0) {
            playPanel = new PlayPanel(this);
            connect(playPanel, SIGNAL(volChange(float)),    seq, SLOT(setVolume(float)));
            connect(playPanel, SIGNAL(relTempoChanged(int)),seq, SLOT(setRelTempo(int)));
            connect(playPanel, SIGNAL(posChange(int)),      seq, SLOT(seek(int)));
            connect(playPanel, SIGNAL(closed()),                 SLOT(closePlayPanel()));

            playPanel->setVolume(seq->volume());
            playPanel->setTempo(cs->tempomap->tempo(0));
            playPanel->setRelTempo(cs->tempomap->relTempo());
            playPanel->setEndpos(seq->getEndTick());
            playPanel->setScore(cs);
            }
      playPanel->setShown(visible);
      playId->setChecked(visible);
      }

//---------------------------------------------------------
//   closePlayPanel
//---------------------------------------------------------

void MuseScore::closePlayPanel()
      {
      playId->setChecked(false);
      }

//---------------------------------------------------------
//   cmdAppendMeasures
//---------------------------------------------------------

void MuseScore::cmdAppendMeasures()
      {
      if (cs) {
		if (measuresDialog == 0)
                  measuresDialog = new MeasuresDialog;
            measuresDialog->show();
            }
      }

//---------------------------------------------------------
//   MeasuresDialog
//---------------------------------------------------------

MeasuresDialog::MeasuresDialog(QWidget* parent)
   : QDialog(parent)
	{
      setupUi(this);
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void MeasuresDialog::accept()
	{
	int n = measures->value();
      if (mscore->currentScore())
            mscore->currentScore()->cmdAppendMeasures(n);
      done(1);
	}

//---------------------------------------------------------
//   midiinToggled
//---------------------------------------------------------

void MuseScore::midiinToggled(bool val)
      {
      _midiinEnabled = val;
      }

//---------------------------------------------------------
//   midiinEnabled
//---------------------------------------------------------

bool MuseScore::midiinEnabled() const
      {
      return preferences.enableMidiInput && _midiinEnabled;
      }

//---------------------------------------------------------
//   midiNoteReceived
//---------------------------------------------------------

void MuseScore::midiNoteReceived(int pitch, bool chord)
      {
      QWidget* w = QApplication::activeModalWidget();
      if (cs && w == 0)
            cs->midiNoteReceived(pitch, chord);
      }

//---------------------------------------------------------
//   speakerToggled
//---------------------------------------------------------

void MuseScore::speakerToggled(bool val)
      {
      _speakerEnabled = val;
      }

//---------------------------------------------------------
//   playEnabled
//---------------------------------------------------------

bool MuseScore::playEnabled() const
      {
      return preferences.playNotes && _speakerEnabled;
      }

//---------------------------------------------------------
//   removeTab
//---------------------------------------------------------

void MuseScore::removeTab()
      {
      int n = scoreList.size();
      for (int i = 0; i < n; ++i) {
            if (scoreList[i] == cs) {
                  removeTab(i);
                  break;
                  }
            }
      }

void MuseScore::removeTab(int i)
      {
      int n = scoreList.size();
      if (n <= 0)
            return;
      Score* score = scoreList[i];
      if (checkDirty(score))
            return;
      scoreList.removeAt(i);
      tab->blockSignals(true);
      tab->removeTab(i);
      tab->blockSignals(false);
      cs = 0;
      if (i >= (n-1))
            i = 0;
      if (scoreList.isEmpty())
            score = 0;
      else
            score = scoreList[i];
      setCurrentScore(score);
      }

//---------------------------------------------------------
//   setLocale
//---------------------------------------------------------

void setMscoreLocale(QString localeName)
      {
      static QList<QTranslator*> translatorList;

      foreach(QTranslator* t, translatorList) {
            qApp->removeTranslator(t);
            delete t;
            }

      if (debugMode)
            printf("configured localeName <%s>\n", qPrintable(localeName));
      if (localeName.toLower() == "system") {
            localeName = QLocale::system().name();
            if (debugMode)
                  printf("real localeName <%s>\n", qPrintable(localeName));
            }

      QTranslator* translator = new QTranslator;
      QString lp = mscoreGlobalShare + "locale/" + QString("mscore_") + localeName;
      if (debugMode)
            printf("load translator <%s>\n", qPrintable(lp));

      if (!translator->load(lp) && debugMode)
            printf("load translator <%s> failed\n", qPrintable(lp));
      else {
            qApp->installTranslator(translator);
            translatorList.append(translator);
            }

      QString resourceDir;
#ifdef __MINGW32__
      resourceDir = mscoreGlobalShare + "locale/";
#else
      resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
      QTranslator* qtTranslator = new QTranslator;
      if (debugMode)
            printf("load translator <qt_%s> from <%s>\n",
               qPrintable(localeName), qPrintable(resourceDir));

      if (!qtTranslator->load(QLatin1String("qt_") + localeName, resourceDir) && debugMode)
            printf("load translator <qt_%s> failed\n", qPrintable(localeName));
      else {
            qApp->installTranslator(qtTranslator);
            translatorList.append(qtTranslator);
            }

      //
      // initialize shortcut hash table
      //
      shortcuts.clear();
      for (unsigned i = 0;; ++i) {
            if (MuseScore::sc[i].xml == 0)
                  break;
            shortcuts[MuseScore::sc[i].xml] = new Shortcut(MuseScore::sc[i]);
            }
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
//      feclearexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
//      feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
      QSettings::setDefaultFormat(QSettings::IniFormat);

      for (int i = 0; i < 128; ++i)
            midiActionMap[i] = 0;

      Harmony::initHarmony();
      QApplication app(argc, argv);
      QCoreApplication::setOrganizationName("MusE");
      QCoreApplication::setOrganizationDomain("muse.org");
      QCoreApplication::setApplicationName("MuseScore");
      qApp->setWindowIcon(windowIcon);

      dataPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
      QDir dataPathDir;
      dataPathDir.mkpath(dataPath);

      setDefaultStyle();

      int c;
      while ((c = getopt(argc, argv, "vdLsmiIOo:r:S:DF")) != EOF) {
            switch (c) {
                  case 'v':
                        printVersion(argv[0]);
                        return 0;
                   case 'd':
                        debugMode = true;
                        break;
                  case 'L':
                        layoutDebug = true;
                        break;
                  case 's':
                        noSeq = true;
                        break;
                  case 'm':
                        noMidi = true;
                        break;
                  case 'i':
                  case 'I':
                        midiInputTrace = true;
                        break;
                  case 'O':
                        midiOutputTrace = true;
                        break;
                  case 'o':
                        converterMode = true;
                        outFileName = optarg;
                        break;
                  case 'r':
                        converterDpi = atof(optarg);
                        break;
                  case 'S':
                        styleFile = optarg;
                        break;
                  case 'D':
                        scriptDebug = true;
                        break;
                  case 'F':
                        useFactorySettings = true;
                        break;

                  default:
                        usage(argv[0], "bad argument");
                        return -1;
                  }
            }
      argc -= optind;
      ++argc;
/**/
      mscoreGlobalShare = getSharePath();
      if (debugMode)
            printf("global share: <%s>\n", qPrintable(mscoreGlobalShare));

      //
      // set translator before preferences are read to get
      //    translations for all shortcuts
      //
      if (useFactorySettings)
            localeName = "system";
      else {
            QSettings s;
            localeName = s.value("language", "system").toString();
            }

      setMscoreLocale(localeName);

      if (!useFactorySettings)
            preferences.read();

      QSplashScreen* sc = 0;
      if (!converterMode && preferences.showSplashScreen) {
            QPixmap pm(":/data/splash.jpg");
            sc = new QSplashScreen(pm);
            sc->setWindowFlags(Qt::FramelessWindowHint);
#if 0
            QSize s(pm.size());
            QBitmap bm(s);
            bm.clear();
            QPainter p;
            p.begin(&bm);
            p.setBrush(Qt::SolidPattern);
//            p.drawRoundRect(QRect(QPoint(0, 0), s), s.height()/6, s.width()/6);
            p.drawRoundRect(QRect(QPoint(0, 0), s), s.height()/20, s.width()/20);
            p.end();
            sc->setMask(bm);
#endif
            sc->show();
            app.processEvents();
            }

      qApp->setStyleSheet(appStyleSheet());
      if (!preferences.style.isEmpty())
            QApplication::setStyle(preferences.style);

      //
      //  load internal fonts
      //
      if (-1 == QFontDatabase::addApplicationFont(":/fonts/mscore-20.ttf")) {
            fprintf(stderr, "Mscore: fatal error: cannot load internal font\n");
            if (!debugMode)
                  exit(-1);
            }
      if (-1 == QFontDatabase::addApplicationFont(":/fonts/mscore1-20.ttf")) {
            fprintf(stderr, "Mscore: fatal error: cannot load internal font\n");
            if (!debugMode)
                  exit(-1);
            }

      seq = new Seq();
      if (converterMode)
            noSeq = true;
      else {
            if (!noSeq) {
                  if (!seq->init()) {
                        printf("sequencer init failed\n");
                        noSeq = true;
                        }
                  }
            }
      //
      // avoid font problems by overriding the environment
      //    fall back to "C" locale
      //

#ifndef __MINGW32__
      setenv("LANG", "C", 1);
#endif
      QLocale::setDefault(QLocale(QLocale::C));

      pdev = new QPrinter(QPrinter::HighResolution);
      QWidget wi(0);

      PDPI = wi.logicalDpiX();         // physical resolution
      DPI  = pdev->logicalDpiX();      // logical drawing resolution


      // sanity checks for DPI and PDPI

      if (DPI == 0)           // this happens on windows if there is no printer installed
            DPI = 1200.0;
      if (PDPI == 0)
            PDPI = 300.0;
      DPMM = DPI / INCH;      // dots/mm

      if (debugMode) {
            printf("DPI %f(%d) PDPI %f(%d) DPMM %f\n",
               DPI, pdev->physicalDpiX(),
               PDPI, wi.physicalDpiX(),
               DPMM);
            QStringList sl(QCoreApplication::libraryPaths());
            foreach(const QString& s, sl)
                  printf("LibraryPath: <%s>\n", qPrintable(s));
            }

      // rastral size of font is 20pt = 20/72 inch = 20*DPI/72 dots
      //   staff has 5 lines = 4 * _spatium
      _spatium    = SPATIUM20  * DPI;     // 20.0 / 72.0 * DPI / 4.0;
      _spatiumMag = 1.0;

      initSymbols();
      genIcons();
      initDrumset();

      mscore = new MuseScore();
      gscore = new Score(defaultStyle);
      mscore->readSettings();

      //-------------------------------
      //  load scores
      //-------------------------------

      Score* currentScore = 0;
      bool scoreCreated = false;
      if (argc < 2) {
            switch (preferences.sessionStart) {
                  case LAST_SESSION:
                        {
                        QSettings settings;
                        int n = settings.value("scores", 0).toInt();
                        int c = settings.value("currentScore", 0).toInt();
                        for (int i = 0; i < n; ++i) {
                              QString s = settings.value(QString("score-%1").arg(i),"").toString();
                              Score* score = new Score(defaultStyle);
                              score->addViewer(new Canvas);
                              scoreCreated = true;
                              score->read(s);
                              mscore->appendScore(score);
                              if (i == c)
                                    currentScore = score;
                              }
                        }
                        break;
                  case EMPTY_SESSION:
                  case NEW_SESSION:
                        break;
                  case SCORE_SESSION:
                        Score* score = new Score(defaultStyle);
                        score->addViewer(new Canvas);
                        scoreCreated = true;
                        score->read(preferences.startScore);
                        mscore->appendScore(score);
                        currentScore = score;
                        break;
                  }
            }
      else {
            while (argc > 1) {
                  QString name = QString::fromLocal8Bit(argv[optind++]);
                  --argc;
                  if (!name.isEmpty()) {
                        Score* score = new Score(defaultStyle);
                        score->addViewer(new Canvas);
                        scoreCreated = true;
                        if (!score->read(name)) {
                              QMessageBox::warning(0,
                                    QWidget::tr("MuseScore"),
                                    QWidget::tr("reading file <")
                                       + name + "> failed: " +
                                    QString(strerror(errno)),
                                    QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
                              }
                        else
                              mscore->appendScore(score);
                        currentScore = score;
                        }
                  }
            }

      if (!scoreCreated && preferences.sessionStart != EMPTY_SESSION) {
            // start with empty score:
            Score* score = new Score(defaultStyle);
            score->addViewer(new Canvas);
            score->fileInfo()->setFile(mscore->createDefaultName());
            score->setCreated(true);
            mscore->appendScore(score);
            currentScore = score;
            }
      if (mscore->noScore())
            currentScore = 0;
      mscore->setCurrentScore(currentScore);

      mscore->showPlayPanel(preferences.showPlayPanel);
      if (mscore->getPlayPanel())
            mscore->getPlayPanel()->move(preferences.playPanelPos);

      if (converterMode) {
            QString fn(outFileName);
            Score* cs = mscore->currentScore();
            if (styleFile) {
                  QFile f(styleFile);
                  if (f.open(QIODevice::ReadOnly))
                        cs->loadStyle(&f);
                  }
            cs->layout()->doLayout();

            bool rv = true;
            if (fn.endsWith(".msc")) {
                  QFileInfo fi(fn);
                  try {
                        cs->saveFile(fi, false);
                        }
                  catch(QString) {
                        rv = false;
                        }
                  }
            else if (fn.endsWith(".mscz")) {
                  QFileInfo fi(fn);
                  try {
                        cs->saveCompressedFile(fi, false);
                        }
                  catch(QString) {
                        rv = false;
                        }
                  }
            else if (fn.endsWith(".xml"))
                  rv = cs->saveXml(fn);
            else if (fn.endsWith(".mxl"))
                  rv = cs->saveMxl(fn);
            else if (fn.endsWith(".mid"))
                  rv = cs->saveMidi(fn);
            else if (fn.endsWith(".pdf"))
                  rv = cs->savePsPdf(fn, QPrinter::PdfFormat);
            else if (fn.endsWith(".ps"))
                  rv = cs->savePsPdf(fn, QPrinter::PostScriptFormat);
            else if (fn.endsWith(".png"))
                  rv = cs->savePng(fn);
            else if (fn.endsWith(".svg"))
                  rv = cs->saveSvg(fn);
            else if (fn.endsWith(".ly"))
                  rv = cs->saveLilypond(fn);
            else {
                  fprintf(stderr, "dont know how to convert to %s\n", outFileName);
                  rv = false;
                  }
            exit(rv ? 0 : -1);
            }
      mscore->loadPlugins();
      mscore->show();
      if (sc)
            sc->finish(mscore);
      if (debugMode)
            printf("start event loop...\n");
      return qApp->exec();
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void MuseScore::cmd(QAction* a)
      {
      static QString lastCmd;

      QString cmd(a->data().toString());
      Shortcut* sc = getShortcut(cmd.toAscii().data());
      if (sc == 0) {
            printf("unknown action <%s>\n", qPrintable(cmd));
            return;
            }
      if (cs && (sc->state & cs->state()) == 0) {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: invalid command"),
               QString("command %1 not valid in current state").arg(cmd),
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            return;
            }
      if (cmd == "repeat-cmd") {
            cmd = lastCmd;
            if (cmd.isEmpty())
                  return;
            }
      else
            lastCmd = cmd;
      if (cmd == "instruments")
            editInstrList();
      else if (cmd == "clefs")
            clefMenu();
      else if (cmd == "keys")
            keyMenu();
      else if (cmd == "symbols")
            symbolMenu();
      else if (cmd == "times")
            timeMenu();
      else if (cmd == "dynamics")
            dynamicsMenu();
      else if (cmd == "file-open")
            loadFile();
      else if (cmd == "file-save")
            saveFile();
      else if (cmd == "file-reload") {
            if (cs && !cs->created() && !checkDirty(cs)) {
                  Score* score = new Score(defaultStyle);
                  score->addViewer(new Canvas);
                  score->read(cs->filePath());
                  // hack: so we don't get another checkDirty in appendScore
                  cs->setDirty(false);
                  appendScore(score);
                  setCurrentScore(score);
                  }
            }
      else if (cmd == "file-close")
            removeTab(scoreList.indexOf(cs));
      else if (cmd == "file-save-as") {
            if (cs)
                  cs->saveAs(false);
            }
      else if (cmd == "file-save-a-copy") {
            if (cs)
                  cs->saveAs(true);
            }
      else if (cmd == "file-new")
            newFile();
      else if (cmd == "quit")
            close();
      else if (cmd == "fingering")
            fingeringMenu();
      else if (cmd == "toggle-statusbar") {
            preferences.showStatusBar = a->isChecked();
            _statusBar->setShown(preferences.showStatusBar);
            preferences.write();
            }
      else if (cmd == "append-measures")
            cmdAppendMeasures();
      else if (cmd == "insert-measures")
            cmdInsertMeasures();
      else if (cmd == "inspector")
            startPageListEditor();
      else if (cmd == "script-debug") {
            scriptDebug = a->isChecked();
            }
      else {
            if (cs)
                  cs->cmd(cmd);
            }
      }

//---------------------------------------------------------
//   clipboardChanged
//---------------------------------------------------------

void MuseScore::clipboardChanged()
      {
      const QMimeData* ms = QApplication::clipboard()->mimeData();
      if (ms == 0)
            return;
      QStringList formats = ms->formats();

      bool flag = ms->hasFormat(mimeSymbolFormat)
            ||    ms->hasFormat(mimeStaffListFormat)
            ||    ms->hasFormat(mimeMeasureListFormat)
            ||    ms->hasFormat(mimeSymbolListFormat);
      // TODO: depends on selection state
      getAction("paste")->setEnabled(flag);
      }

//---------------------------------------------------------
//   changeState
//    score state has changed
//---------------------------------------------------------

void MuseScore::changeState(int val)
      {
//      printf("changeState %d\n", val);

      foreach (Shortcut* s, shortcuts) {
            if (!s->action)
                  continue;
            if (strcmp(s->xml, "undo") == 0)
                  s->action->setEnabled(cs && !cs->undoEmpty());
            else if (strcmp(s->xml, "redo") == 0)
                  s->action->setEnabled(cs && !cs->redoEmpty());
            else if (strcmp(s->xml, "cut") == 0)
                  s->action->setEnabled(cs && cs->selection()->state());
            else if (strcmp(s->xml, "copy") == 0)
                  s->action->setEnabled(cs && cs->selection()->state());
            else
                  s->action->setEnabled(s->state & val);
            if (val == STATE_DISABLED) {
                  const char* names[] = { "file-open", "file-new", "quit", 0 };
                  for (const char** p = names; *p; ++p) {
                        if (strcmp(*p, s->xml) == 0)
                              s->action->setEnabled(true);
                        }
                  }
            }
      switch(val) {
            case STATE_DISABLED:
                  _modeText->setText(tr("no score"));
                  _modeText->show();
                  if (pageListEdit)
                        pageListEdit->hide();

                  break;
            case STATE_NORMAL:
                  _modeText->hide();
                  break;
            case STATE_NOTE_ENTRY:
                  _modeText->setText(tr("note entry mode"));
                  _modeText->show();
                  break;
            case STATE_EDIT:
                  _modeText->setText(tr("edit mode"));
                  _modeText->show();
                  break;
            case STATE_PLAY:
                  _modeText->setText(tr("play"));
                  _modeText->show();
                  break;
            default:
                  printf("MuseScore::setState: illegal state %d\n", val);
                  break;
            }
      QAction* a = getAction("note-input");
      a->setChecked(val == STATE_NOTE_ENTRY);
      }

//---------------------------------------------------------
//   Shortcut
//---------------------------------------------------------

Shortcut::Shortcut()
      {
      state   = STATE_NORMAL;
      xml     = 0;
      key     = 0;
      context = Qt::WindowShortcut;
      icon    = 0;
      action  = 0;
      translated = false;
      }

Shortcut::Shortcut(int s, const char* name, const char* d, const QKeySequence& k,
   Qt::ShortcutContext cont, const char* txt, const char* h, QIcon* i)
      {
      state   = s;
      xml     = name;
      key     = k;
      context = cont;
      icon    = i;
      action  = 0;
      descr   = d; // qApp->translate("MuseScore", d);
      help    = h; // qApp->translate("MuseScore", h);
      text    = txt; // qApp->translate("MuseScore", txt);
      translated = false;
      }

Shortcut::Shortcut(const Shortcut& c)
      {
      state   = c.state;
      xml     = c.xml;
      key     = c.key;
      context = c.context;
      icon    = c.icon;
      action  = c.action;
      if (c.translated) {
            descr   = c.descr;
            help    = c.help;
            text    = c.text;
            }
      else {
            descr   = qApp->translate("MuseScore", c.descr.toUtf8().data());
            help    = qApp->translate("MuseScore", c.help.toUtf8().data());
            text    = qApp->translate("MuseScore", c.text.toUtf8().data());
            translated = true;
            }
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void MuseScore::writeSettings()
      {
      QSettings settings;
      settings.beginGroup("MainWindow");
      settings.setValue("size", size());
      settings.setValue("pos", pos());
      settings.setValue("showPanel", paletteBox && paletteBox->isVisible());
      settings.setValue("state", saveState());
      settings.endGroup();
      if (paletteBox && paletteBox->dirty())
            paletteBox->write(dataPath + "/" + "mscore-palette.xml");
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void MuseScore::readSettings()
      {
      if (useFactorySettings) {
            resize(QSize(800, 600));
            return;
            }
      QSettings settings;
      settings.beginGroup("MainWindow");
      resize(settings.value("size", QSize(950, 800)).toSize());
      move(settings.value("pos", QPoint(10, 10)).toPoint());
      mscore->showPalette(settings.value("showPanel", "0").toBool());
      restoreState(settings.value("state").toByteArray());
      settings.endGroup();
      }

//---------------------------------------------------------
//   play
//    play note for preferences.defaultPlayDuration
//---------------------------------------------------------

void MuseScore::play(Element* e) const
      {
      if (mscore->playEnabled() && e->type() == NOTE) {
            Note* note = static_cast<Note*>(e);
            Part* part = note->staff()->part();
            play(e, note->pitch() + part->pitchOffset());
            }
      }

void MuseScore::play(Element* e, int pitch) const
      {
      if (mscore->playEnabled() && e->type() == NOTE) {
            Note* note = static_cast<Note*>(e);
            Part* part = note->staff()->part();
            Instrument* i = part->instrument();
            seq->startNote(i->channel[note->subchannel()], pitch, 80,
               preferences.defaultPlayDuration);
            }
      }

//---------------------------------------------------------
//   about
//---------------------------------------------------------

void MuseScore::about()
      {
      AboutBoxDialog ab;
      ab.show();
      ab.exec();
      }

//---------------------------------------------------------
//   AboutBoxDialog
//---------------------------------------------------------

AboutBoxDialog::AboutBoxDialog()
      {
      extern int revision;
      setupUi(this);
#ifdef MSCORE_UNSTABLE
      versionLabel->setText(tr("Unstable Prerelease for Version: ") + VERSION);
#else
      versionLabel->setText(tr("Version: ") + VERSION);
#endif
      revisionLabel->setText(tr("Revision: %1").arg(revision));
      }

//---------------------------------------------------------
//   dirtyChanged
//---------------------------------------------------------

void MuseScore::dirtyChanged(Score* score)
      {
      int idx = scoreList.indexOf(score);
      if (idx == -1) {
            printf("score not in list\n");
            return;
            }
      QString label(score->name());
      if (score->dirty())
            label += "*";
      tab->setTabText(idx, label);
      }

//---------------------------------------------------------
//   magChanged
//---------------------------------------------------------

void MuseScore::magChanged(int idx)
      {
      if (cs) {
            if (idx == MAG_FREE)
                  cs->setMag(mag->getMag(cs->canvas()));
            else
                  cs->setMagIdx(idx);
            }
      }

//---------------------------------------------------------
//   incMag
//---------------------------------------------------------

void MuseScore::incMag()
      {
      if (cs) {
            qreal _mag = cs->canvas()->mag() * 1.7;
            if (_mag > 16.0)
                  _mag = 16.0;
            setMag(_mag);
            }
      }

//---------------------------------------------------------
//   decMag
//---------------------------------------------------------

void MuseScore::decMag()
      {
      if (cs) {
            qreal nmag = cs->canvas()->mag() / 1.7;
            if (nmag < 0.05)
                  nmag = 0.05;
            setMag(nmag);
            }
      }

//---------------------------------------------------------
//   getMag
//---------------------------------------------------------

double MuseScore::getMag(Canvas* canvas) const
      {
      return mag->getMag(canvas);
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void MuseScore::setMag(double d)
      {
      mag->setMag(d);
      mag->setMagIdx(MAG_FREE);
      if (cs)
            cs->setMag(d);
      }

//---------------------------------------------------------
//   setPos
//    set position label
//---------------------------------------------------------

void MuseScore::setPos(int t)
      {
      if (cs == 0 || t < 0)
            return;
      SigList* s = cs->sigmap;
      int bar, beat, tick;
      s->tickValues(t, &bar, &beat, &tick);
      _positionLabel->setText(QString("Bar %1 Beat %2.%3")
         .arg(bar + 1,  3, 10, QLatin1Char(' '))
         .arg(beat + 1, 2, 10, QLatin1Char(' '))
         .arg(tick,     3, 10, QLatin1Char('0'))
         );
      }

