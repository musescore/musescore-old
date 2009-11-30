//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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
#include "al/tempo.h"
#include "sym.h"
#include "padids.h"
#include "pagesettings.h"
#include "inspector.h"
#include "editstyle.h"
#include "playpanel.h"
#include "page.h"
#include "partedit.h"
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
#include "al/sig.h"
#include "undo.h"
#include "synthcontrol.h"
#include "pianoroll.h"
#include "scoretab.h"

#ifdef STATIC_SCRIPT_BINDINGS
Q_IMPORT_PLUGIN(com_trolltech_qt_gui_ScriptPlugin)
Q_IMPORT_PLUGIN(com_trolltech_qt_core_ScriptPlugin)
Q_IMPORT_PLUGIN(com_trolltech_qt_network_ScriptPlugin)
Q_IMPORT_PLUGIN(com_trolltech_qt_uitools_ScriptPlugin)
Q_IMPORT_PLUGIN(com_trolltech_qt_xml_ScriptPlugin)
#endif

bool debugMode          = false;
bool enableExperimental = false;

QString dataPath;
QPaintDevice* pdev;
double PDPI, DPI, DPMM;
double SPATIUM;

QString mscoreGlobalShare;
static QStringList recentScores;

Shortcut* midiActionMap[128];
QMap<QString, Shortcut*> shortcuts;

bool converterMode = false;
static bool pluginMode = false;
double converterDpi = 300;

static QString outFileName;
static QString pluginName;
static QString styleFile;
static QString localeName;
bool useFactorySettings = false;

QString revision;

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
#ifdef Q_WS_MAC
	  QDir dir(QCoreApplication::applicationDirPath() + QString("/../Resources"));
      return dir.absolutePath() + "/";
#else
      return QString( INSTPREFIX "/share/" INSTALL_NAME);
#endif
#endif
      }

//---------------------------------------------------------
//   printVersion
//---------------------------------------------------------

static void printVersion(const char* prog)
      {
#ifdef MSCORE_UNSTABLE
      printf("%s: Music Score Editor\nUnstable Prerelease for Version %s; Build %s\n",
         prog, VERSION, qPrintable(revision));
#else
     printf("%s: Music Score Editor; Version %s; Build %s\n", prog, VERSION, qPrintable(revision));
#endif
      }

static const int RECENT_LIST_SIZE = 10;

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void MuseScore::closeEvent(QCloseEvent* ev)
      {
      QList<Score*> removeList;
      foreach(Score* score, scoreList) {
            if (score->created() && !score->dirty())
                  removeList.append(score);
            else {
                  if (checkDirty(score)) {
                        ev->ignore();
                        return;
                        }
                  //
                  // if score is still dirty, the the user has discarded the
                  // score and we can remove it from the list
                  //
                  if (score->created() && score->dirty())
                        removeList.append(score);
                  }
            }
      writeSessionFile(true);
      foreach(Score* score, scoreList) {
            if (!score->tmpName().isEmpty()) {
                  QFile f(score->tmpName());
                  f.remove();
                  }
            }

      // remove all new created/not save score so they are
      // note saved as session data

      foreach(Score* score, removeList)
            scoreList.removeAll(score);

      // save score list
      QSettings settings;
      for (int i = 0; i < RECENT_LIST_SIZE; ++i)
            settings.setValue(QString("recent-%1").arg(i), recentScores.value(i));

      settings.setValue("scores", scoreList.size());
      int curScore = scoreList.indexOf(cs);
      if (curScore == -1)  // cs removed if new created and not modified
            curScore = 0;
      settings.setValue("currentScore", curScore);

      int idx = 0;
      foreach(Score* s, scoreList) {
            settings.setValue(QString("score-%1").arg(idx), s->fileInfo()->absoluteFilePath());
            ++idx;
            }

      settings.setValue("lastSaveCopyDirctory", lastSaveCopyDirectory);
      settings.setValue("lastSaveDirectory", lastSaveDirectory);

      if (synthControl)
            synthControl->updatePreferences();

      writeSettings();
      if (inspector)
            inspector->writeSettings();

      seq->stop();
#ifndef __MINGW32__
      while(!seq->isStopped())
            usleep(50000);
#endif
      seq->exit();
      ev->accept();
      if (preferences.dirty)
            preferences.write();

      //
      // close all toplevel windows (on mac it crashes on quit with these lines)
      //
#ifndef Q_WS_MAC
      foreach(QWidget* w, qApp->topLevelWidgets()) {
            if (w != this)
                  w->close();
            }
#endif
      }

//---------------------------------------------------------
//   preferencesChanged
//---------------------------------------------------------

void MuseScore::preferencesChanged()
      {
      for (int i = 0; i < tab1->count(); ++i) {
            Canvas* canvas = static_cast<Canvas*>(tab1->viewer(i));
            if (canvas == 0)
                  continue;
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
      for (int i = 0; i < tab2->count(); ++i) {
            Canvas* canvas = static_cast<Canvas*>(tab2->viewer(i));
            if (canvas == 0)
                  continue;
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
      _undoGroup            = new UndoGroup();
      cs                    = 0;
      cv                    = 0;
      se                    = 0;    // script engine
      debugger              = 0;
      instrList             = 0;
      playPanel             = 0;
      preferenceDialog      = 0;
      measuresDialog        = 0;
      insertMeasuresDialog  = 0;
      iledit                = 0;
      synthControl          = 0;
      inspector             = 0;
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
      pianorollEditor       = 0;
      _splitScreen          = false;
      _horizontalSplit      = true;

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

      _progressBar = 0;

      // otherwise unused actions:
      //   must be added somewere to work

      QActionGroup* ag = new QActionGroup(this);
      ag->setExclusive(false);
      foreach(Shortcut* s, shortcuts) {
            QAction* a = getAction(s);
            ag->addAction(a);
            }
      addActions(ag->actions());
      connect(ag, SIGNAL(triggered(QAction*)), SLOT(cmd(QAction*)));

      QWidget* mainWindow = new QWidget;
      layout = new QVBoxLayout;
      mainWindow->setLayout(layout);
      layout->setMargin(0);
      layout->setSpacing(0);
      splitter = new QSplitter;

      tab1 = new ScoreTab(&scoreList);
      tab1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      connect(tab1, SIGNAL(currentViewerChanged(Viewer*)), SLOT(setCurrentViewer(Viewer*)));
      connect(tab1, SIGNAL(tabCloseRequested(int)), SLOT(removeTab(int)));

      tab2 = new ScoreTab(&scoreList);
      tab2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      connect(tab2, SIGNAL(currentViewerChanged(Viewer*)), SLOT(setCurrentViewer(Viewer*)));
      connect(tab2, SIGNAL(tabCloseRequested(int)), SLOT(removeTab(int)));

      splitter->addWidget(tab1);
      splitter->addWidget(tab2);
      tab2->setVisible(false);
      layout->addWidget(splitter);

      searchDialog = 0;

      QAction* whatsThis = QWhatsThis::createAction(this);

      //---------------------------------------------------
      //    Transport Action
      //---------------------------------------------------

      QAction* a;
#ifdef HAS_MIDI
      a  = getAction("midi-on");
      a->setCheckable(true);
      a->setEnabled(preferences.enableMidiInput);
      a->setChecked(_midiinEnabled);
#endif
      a = getAction("sound-on");
      a->setCheckable(true);
      a->setEnabled(preferences.playNotes);
      a->setChecked(_speakerEnabled);

      getAction("play")->setCheckable(true);
      //getAction("pause")->setCheckable(true);
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

      a = getAction("undo");
      a->setEnabled(false);
      connect(_undoGroup, SIGNAL(canUndoChanged(bool)), a, SLOT(setEnabled(bool)));
      fileTools->addAction(a);

      a = getAction("redo");
      a->setEnabled(false);
      connect(_undoGroup, SIGNAL(canRedoChanged(bool)), a, SLOT(setEnabled(bool)));
      fileTools->addAction(a);

      fileTools->addSeparator();

      transportTools = addToolBar(tr("Transport Tools"));
      transportTools->setObjectName("transport-tools");
      transportTools->addAction(getAction("sound-on"));
#ifdef HAS_MIDI
      transportTools->addAction(getAction("midi-on"));
#endif
      transportTools->addSeparator();
      transportTools->addAction(getAction("rewind"));
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
            nb->setDefaultAction(a);
            entryTools->addWidget(nb);
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
            }

      a = getAction("flip");
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
      menuEdit->addAction(getAction("find"));
      menuEdit->addSeparator();

      menuEdit->addAction(getAction("delete-measures"));
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

      menuNotes = mb->addMenu(qApp->translate("MenuNotes", "&Notes"));
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
            menuAddInterval->addAction(a);
            }
      menuAddInterval->addSeparator();
      for (int i = 2; i < 10; ++i) {
            char buffer[16];
            sprintf(buffer, "interval-%d", i);
            a = getAction(buffer);
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
      menuStyle->addAction(getAction("edit-style"));
      menuStyle->addAction(getAction("edit-text-style"));
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
      menuDisplay->addAction(a);

      playId = getAction("toggle-playpanel");
      playId->setCheckable(true);
      menuDisplay->addAction(playId);

      a = getAction("toggle-navigator");
      a->setCheckable(true);
      menuDisplay->addAction(a);

      a = getAction("toggle-mixer");
      a->setCheckable(true);
      menuDisplay->addAction(a);

      a = getAction("synth-control");
      a->setCheckable(true);
      menuDisplay->addAction(a);

      menuDisplay->addSeparator();
      menuDisplay->addAction(getAction("zoomin"));
      menuDisplay->addAction(getAction("zoomout"));
      menuDisplay->addSeparator();

      a = getAction("toggle-transport");
      a->setCheckable(true);
      a->setChecked(transportTools->isVisible());
      menuDisplay->addAction(a);

      a = getAction("toggle-noteinput");
      a->setCheckable(true);
      a->setChecked(true);
      menuDisplay->addAction(a);

      a = getAction("toggle-statusbar");
      a->setCheckable(true);
      a->setChecked(true);
      menuDisplay->addAction(a);

      menuDisplay->addSeparator();
      a = getAction("split-h");
      a->setCheckable(true);
      a->setChecked(false);
      menuDisplay->addAction(a);
      a = getAction("split-v");
      a->setCheckable(true);
      a->setChecked(false);
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

      menuHelp->addAction(getAction("local-help"));
      menuHelp->addAction(tr("Online Handbook"), this, SLOT(helpBrowser1()));
      menuHelp->addAction(tr("&About"),   this, SLOT(about()));
      menuHelp->addAction(tr("About&Qt"), this, SLOT(aboutQt()));
      menuHelp->addSeparator();

      a = getAction("script-debug");
      a->setCheckable(true);
      a->setChecked(scriptDebug);
      menuHelp->addAction(a);
      a->setEnabled(false);

      menuHelp->addAction(whatsThis);

      setCentralWidget(mainWindow);

      loadInstrumentTemplates(preferences.instrumentList);
      preferencesChanged();
      if (seq) {
            connect(seq, SIGNAL(started()), SLOT(seqStarted()));
            connect(seq, SIGNAL(stopped()), SLOT(seqStopped()));
            }
      loadScoreList();

      showPlayPanel(preferences.showPlayPanel);
      if (getPlayPanel())
            getPlayPanel()->move(preferences.playPanelPos);

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
//   helpBrowser
//    show local help
//---------------------------------------------------------

void MuseScore::helpBrowser()
      {
      QString lang;
      if (localeName.toLower() == "system")
            lang = QLocale::system().name().left(2);
      else
            lang = localeName.left(2);
      if (debugMode)
            printf("open handbook for language <%s>\n", qPrintable(lang));

      QFileInfo mscoreHelp(mscoreGlobalShare + QString("man/MuseScore-") + lang + QString(".pdf"));
      if (!mscoreHelp.isReadable()) {
            if (debugMode) {
                  printf("cannot open doc <%s>\n", qPrintable(mscoreHelp.filePath()));
                  }
            mscoreHelp.setFile(mscoreGlobalShare + QString("man/MuseScore-en.pdf"));
            if (!mscoreHelp.isReadable()) {
                  QString info(tr("MuseScore handbook not found at: \n"));
                  info += mscoreHelp.filePath();
                  info += tr("\n\nFrom the \"Help\" menu try choosing \"Online Handbook\" instead.");
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
      QString lang;
      if (localeName.toLower() == "system")
            lang = QLocale::system().name().left(2);
      else
            lang = localeName.left(2);
      if (debugMode)
            printf("open online handbook for language <%s>\n", qPrintable(lang));
      QString help("http://www.musescore.org/en/handbook");
      if (lang == "de")
            help = QString::fromUtf8("http://www.musescore.org/de/handbuch");
      else if (lang == "es")
            help = QString::fromUtf8("http://www.musescore.org/es/manual");
      else if (lang == "fi")
            help = QString::fromUtf8("http://www.musescore.org/fi/käsikirja");
      else if (lang == "fr")
            help = QString::fromUtf8("http://www.musescore.org/fr/manuel");
      else if (lang == "gl")
            help = QString::fromUtf8("http://www.musescore.org/gl/manual");
      else if (lang == "it")
            help = QString::fromUtf8("http://www.musescore.org/it/manuale");
      else if (lang == "nb")
            help = QString::fromUtf8("http://www.musescore.org/nb/håndbok");
      else if (lang == "nl")
            help = QString::fromUtf8("http://www.musescore.org/nl/handboek");
      else if (lang == "pl")
            help = QString::fromUtf8("http://www.musescore.org/pl/podręcznik");
      else if (lang == "pt")
            help = QString::fromUtf8("http://www.musescore.org/pt-br/manual");
      else if (lang == "ru")
            help = QString::fromUtf8("http://www.musescore.org/ru/cправочник");
      else if (lang == "tr")
            help = QString::fromUtf8("http://www.musescore.org/tr/kullanım");
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
//    "open recent"
//---------------------------------------------------------

void MuseScore::selectScore(QAction* action)
      {
      QString a = action->data().toString();
      if (!a.isEmpty()) {
            Score* score = new Score(defaultStyle);
            score->read(a);
            setCurrentViewer(appendScore(score));
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

int MuseScore::appendScore(Score* score)
      {
      connect(score, SIGNAL(dirtyChanged(Score*)),  SLOT(dirtyChanged(Score*)));
      connect(score, SIGNAL(stateChanged(int)),     SLOT(changeState(int)));
      connect(score, SIGNAL(selectionChanged(int)), SLOT(selectionChanged(int)));
      connect(score, SIGNAL(posChanged(int)),       SLOT(setPos(int)));

      int index = scoreList.size();
      for (int i = 0; i < scoreList.size(); ++i) {
            if (scoreList[i]->filePath() == score->filePath()) {
                  removeTab(i);
                  index = i;
                  break;
                  }
            }
      scoreList.insert(index, score);
      tab1->blockSignals(true);
      tab2->blockSignals(true);
      tab1->insertTab(index, score->name());
      tab2->insertTab(index, score->name());
      _undoGroup->addStack(score->undo());
      tab1->blockSignals(false);
      tab2->blockSignals(false);
      return index;
      }

//---------------------------------------------------------
//   updateRecentScores
//---------------------------------------------------------

void MuseScore::updateRecentScores(Score* score)
      {
      QString path = score->fileInfo()->absoluteFilePath();
      recentScores.removeAll(path);
      recentScores.prepend(path);
      }

//---------------------------------------------------------
//   updateTabNames
//---------------------------------------------------------

void MuseScore::updateTabNames()
      {
      for (int i = 0; i < tab1->count(); ++i) {
            Viewer* viewer = tab1->viewer(i);
            tab1->setTabText(i, viewer->score()->name());
            }
      for (int i = 0; i < tab2->count(); ++i) {
            Viewer* viewer = tab2->viewer(i);
            tab2->setTabText(i, viewer->score()->name());
            }
      }

//---------------------------------------------------------
//   usage
//---------------------------------------------------------

static void usage()
      {
      printVersion("MuseScore");
      fprintf(stderr, "usage: mscore flags scorefile\n   Flags:\n");
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
        "   -p name   execute named plugin\n"
        "   -F        use factory settings\n"
        "   -e        enable experimental features\n"
        );
      exit(-1);
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
      for (int i = RECENT_LIST_SIZE-1; i >= 0; --i) {
            QString path = s.value(QString("recent-%1").arg(i),"").toString();
            if (!path.isEmpty()) {
                  recentScores.removeAll(path);
                  recentScores.prepend(path);
                  }
            }
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
            // QFileInfo fi(s);
            // QAction* action = openRecent->addAction(fi.completeBaseName());
            QAction* action = openRecent->addAction(s);  // show complete path
            action->setData(s);
            }
      }

//---------------------------------------------------------
//   setCurrentView
//---------------------------------------------------------

void MuseScore::setCurrentViewer(int idx)
      {
      setCurrentView(0, idx);
      }

void MuseScore::setCurrentView(int tabIdx, int idx)
      {
      if (idx == -1)
            setCurrentViewer(0);
      else
            (tabIdx ? tab2 : tab1)->setCurrentIndex(idx);
      }

//---------------------------------------------------------
//   setCurrentViewer
//---------------------------------------------------------

void MuseScore::setCurrentViewer(Viewer* viewer)
      {
      cv = viewer;
      cs = viewer ? viewer->score() : 0;

      bool enable = cs != 0;
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
            seq->setScore(0);
            _undoGroup->setActiveStack(0);
            return;
            }

      _undoGroup->setActiveStack(cs->undo());
      viewer->setFocus(Qt::OtherFocusReason);

      getAction("file-save")->setEnabled(cs->isSavable());
      getAction("show-invisible")->setChecked(cs->showInvisible());
      getAction("show-frames")->setChecked(cs->showFrames());
      if (viewer->magIdx() == MAG_FREE)
            mag->setMag(viewer->mag());
      else
            mag->setMagIdx(viewer->magIdx());

      setWindowTitle("MuseScore: " + cs->name());
      if (seq)
            seq->setScore(cs);
      if (playPanel)
            playPanel->setScore(cs);

      QAction* a = getAction("concert-pitch");
      a->setChecked(cs->styleB(ST_concertPitch));

      setPos(cs->inputPos());
      _statusBar->showMessage(cs->filePath(), 2000);
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
                        QFileInfo fi(u.toLocalFile());
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
            int viewer = -1;
            foreach(const QUrl& u, event->mimeData()->urls()) {
                  if (u.scheme() == "file") {
                        Score* score = new Score(defaultStyle);
                        score->read(u.toLocalFile());
                        viewer = appendScore(score);
                        }
                  }
            setCurrentViewer(viewer);
            event->acceptProposedAction();
            }
      }

//---------------------------------------------------------
//   showPageSettings
//---------------------------------------------------------

void MuseScore::showPageSettings()
      {
      if (pageSettings == 0)
            pageSettings = new PageSettings();
      pageSettings->setScore(cs);
      pageSettings->show();
      pageSettings->raise();
      }

//---------------------------------------------------------
//   startInspector
//---------------------------------------------------------

void MuseScore::startInspector()
      {
      if (!cs)
            return;
      if (inspector == 0)
            inspector = new Inspector(this);
      inspector->updateList(cs);
      inspector->show();
      }

//---------------------------------------------------------
//   showElementContext
//---------------------------------------------------------

void MuseScore::showElementContext(Element* el)
      {
      if (el == 0)
            return;
      startInspector();
      inspector->setElement(el);
      }

//---------------------------------------------------------
//   showPlayPanel
//---------------------------------------------------------

void MuseScore::showPlayPanel(bool visible)
      {
      if (cs == 0 || noSeq)
            return;

      if (playPanel == 0) {
            if (!visible)
                  return;
            playPanel = new PlayPanel(this);
            connect(playPanel, SIGNAL(volChange(float)),    seq, SLOT(setMasterVolume(float)));
            connect(playPanel, SIGNAL(relTempoChanged(double,int)),seq, SLOT(setRelTempo(double)));
            connect(playPanel, SIGNAL(posChange(int)),      seq, SLOT(seek(int)));
            connect(playPanel, SIGNAL(closed()),                 SLOT(closePlayPanel()));
            connect(seq,       SIGNAL(masterVolumeChanged(float)), playPanel, SLOT(setVolume(float)));

            playPanel->setVolume(seq->masterVolume());
            playPanel->setTempo(cs->tempomap()->tempo(0));
            playPanel->setRelTempo(cs->tempomap()->relTempo());
            playPanel->setEndpos(seq->getEndTick());
            playPanel->setScore(cs);
            int tick, utick;
            seq->getCurTick(&tick, &utick);
            playPanel->heartBeat(tick, utick);
            playPanel->heartBeat2(seq->getCurTime());
            }
      playPanel->setVisible(visible);
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
      int n = scoreList.indexOf(cs);
      if (n == -1) {
            printf("removeTab: %p not found\n", cs);
            return;
            }
      removeTab(n);
      }

void MuseScore::removeTab(int i)
      {
      Score* score = scoreList.value(i);
      if (score == 0)
            return;

      if (checkDirty(score))
            return;
      if (seq->score() == score)
            seq->setScore(0);

      tab1->blockSignals(true);
      tab1->removeTab(i);
      tab1->blockSignals(false);

      tab2->blockSignals(true);
      tab2->removeTab(i);
      tab2->blockSignals(false);

      scoreList.removeAt(i);

      cs = 0;
      cv = 0;
      int n = scoreList.size();
      if (i >= (n-1))
            i = n-2;
      setCurrentViewer(scoreList.isEmpty() ? 0 : tab1->viewer(i));
      writeSessionFile(false);
      if (!score->tmpName().isEmpty()) {
            QFile f(score->tmpName());
            f.remove();
            }
      delete score;
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
      translatorList.clear();

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
//   loadScores
//    load scores for a new session
//---------------------------------------------------------

static void loadScores(const QStringList& argv)
      {
      int currentViewer = -1;
      bool scoreCreated = false;
      if (argv.isEmpty()) {
            switch (preferences.sessionStart) {
                  case LAST_SESSION:
                        {
                        QSettings settings;
                        int n = settings.value("scores", 0).toInt();
                        int c = settings.value("currentScore", 0).toInt();
                        for (int i = 0; i < n; ++i) {
                              QString s = settings.value(QString("score-%1").arg(i),"").toString();
                              Score* score = new Score(defaultStyle);
                              scoreCreated = true;
                              score->read(s);
                              int viewer = mscore->appendScore(score);
                              if (i == c)
                                    currentViewer = viewer;
                              }
                        }
                        break;
                  case EMPTY_SESSION:
                  case NEW_SESSION:
                        break;
                  case SCORE_SESSION:
                        Score* score = new Score(defaultStyle);
                        scoreCreated = true;
                        score->read(preferences.startScore);
                        currentViewer = mscore->appendScore(score);
                        break;
                  }
            }
      else {
            foreach(const QString& name, argv) {
                  if (name.isEmpty())
                        continue;
                  Score* score = new Score(defaultStyle);
                  scoreCreated = true;
                  if (!score->read(name)) {
                        QMessageBox::warning(0,
                              QWidget::tr("MuseScore"),
                              QWidget::tr("reading file <")
                                 + name + "> failed: " +
                              QString(strerror(errno)),
                              QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
                        }
                  else {
                        currentViewer = mscore->appendScore(score);
                        printf("load %d <%s>\n", currentViewer, qPrintable(name));
                        }
                  }
            }

      if (!scoreCreated && preferences.sessionStart != EMPTY_SESSION) {
            // start with empty score:
            Score* score = new Score(defaultStyle);
            score->fileInfo()->setFile(mscore->createDefaultName());
            score->setCreated(true);
            currentViewer = mscore->appendScore(score);
            }
      if (mscore->noScore())
            currentViewer = 0;
      mscore->setCurrentView(0, currentViewer);
      }

//---------------------------------------------------------
//   processNonGui
//---------------------------------------------------------

static bool processNonGui()
      {
      if (pluginMode) {
            QString pn(pluginName);
            bool res = false;
            if (mscore->loadPlugin(pn)){
                  Score* cs = mscore->currentScore();
                  if (!styleFile.isEmpty()) {
                        QFile f(styleFile);
                        if (f.open(QIODevice::ReadOnly))
                              cs->loadStyle(&f);
                        }
                  cs->doLayout();
                  mscore->pluginTriggered(0);
                  res = true;
                  }
            if (!converterMode)
                  return res;
            }

      if (converterMode) {
            QString fn(outFileName);
            Score* cs = mscore->currentScore();
            if (!styleFile.isEmpty()) {
                  QFile f(styleFile);
                  if (f.open(QIODevice::ReadOnly))
                        cs->loadStyle(&f);
                  }
            cs->doLayout();

            if (fn.endsWith(".mscx")) {
                  QFileInfo fi(fn);
                  try {
                        cs->saveFile(fi, false);
                        }
                  catch(QString) {
                        return false;
                        }
                  return true;
                  }
            if (fn.endsWith(".mscz")) {
                  QFileInfo fi(fn);
                  try {
                        cs->saveCompressedFile(fi, false);
                        }
                  catch(QString) {
                        return false;
                        }
                  return true;
                  }
            if (fn.endsWith(".xml"))
                  return cs->saveXml(fn);
            if (fn.endsWith(".mxl"))
                  return cs->saveMxl(fn);
            if (fn.endsWith(".mid"))
                  return cs->saveMidi(fn);
            if (fn.endsWith(".pdf"))
                  return cs->savePsPdf(fn, QPrinter::PdfFormat);
            if (fn.endsWith(".ps"))
                  return cs->savePsPdf(fn, QPrinter::PostScriptFormat);
            if (fn.endsWith(".png"))
                  return cs->savePng(fn);
            if (fn.endsWith(".svg"))
                  return cs->saveSvg(fn);
            if (fn.endsWith(".ly"))
                  return cs->saveLilypond(fn);
#ifdef HAS_AUDIOFILE
            if (fn.endsWith(".wav"))
                  return cs->saveWav(fn);
            if (fn.endsWith(".ogg"))
                  return cs->saveOgg(fn);
            if (fn.endsWith(".flac"))
                  return cs->saveFlac(fn);
#endif
            else {
                  fprintf(stderr, "dont know how to convert to %s\n", qPrintable(outFileName));
                  return false;
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* av[])
      {
      QFile f(":/revision.h");
      f.open(QIODevice::ReadOnly);
      revision = QString(f.readAll());
      f.close();

      QtSingleApplication* app = new QtSingleApplication("mscore", argc, av);

      QStringList argv =  QCoreApplication::arguments();
      argv.removeFirst();

      for (int i = 0; i < argv.size();) {
            QString s = argv[i];
            if (s[0] != '-') {
                  ++i;
                  continue;
                  }
            switch(s[1].toAscii()) {
                  case 'v':
                        printVersion("MuseScore");
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
                        if (argv.size() - i < 2)
                              usage();
                        outFileName = argv.takeAt(i + 1);
                        break;
                  case 'p':
                        pluginMode = true;
                        if (argv.size() - i < 2)
                              usage();
                        pluginName = argv.takeAt(i + 1);
                        break;
                  case 'r':
                        if (argv.size() - i < 2)
                              usage();
                        converterDpi = argv.takeAt(i + 1).toDouble();
                        break;
                  case 'S':
                        if (argv.size() - i < 2)
                              usage();
                        styleFile = argv.takeAt(i + 1);
                        break;
                  case 'D':
                        scriptDebug = true;
                        break;
                  case 'F':
                        useFactorySettings = true;
                        break;
                  case 'e':
                        enableExperimental = true;
                        break;
                  default:
                        usage();
                  }
            argv.removeAt(i);
            }

      QSettings::setDefaultFormat(QSettings::IniFormat);

      for (int i = 0; i < 128; ++i)
            midiActionMap[i] = 0;

      QCoreApplication::setOrganizationName("MusE");
      QCoreApplication::setOrganizationDomain("muse.org");
      QCoreApplication::setApplicationName("MuseScore");

      if (!converterMode) {
            qApp->setWindowIcon(windowIcon);
            if (!argv.isEmpty()) {
                  int ok = true;
                  foreach(QString message, argv) {
                        QFileInfo fi(message);
                        if (!app->sendMessage(fi.absoluteFilePath())) {
                              ok = false;
                              break;
                              }
                        }
                  if (ok)
                        return 0;
                  }
            else
                  app->sendMessage("");
            }

/**/
      dataPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
      setDefaultStyle();
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
      if (!converterMode && !pluginMode && preferences.showSplashScreen) {
            QPixmap pm(":/data/splash.jpg");
            sc = new QSplashScreen(pm);
            sc->setWindowTitle(QString("MuseScore Startup"));
            sc->setWindowFlags(Qt::FramelessWindowHint);
            sc->show();
            qApp->processEvents();
            }

      if (!useFactorySettings && !converterMode) {
            qApp->setStyleSheet(appStyleSheet());
            if (!preferences.style.isEmpty())
                  QApplication::setStyle(preferences.style);
            }

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
      if (-1 == QFontDatabase::addApplicationFont(":/fonts/MuseJazz.ttf")) {
            fprintf(stderr, "Mscore: fatal error: cannot load internal font MuseJazz.ttf\n");
            if (!debugMode)
                  exit(-1);
            }

      if (converterMode) {
            noSeq = true;
            seq = 0;
            }
      else {
            seq = new Seq();
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

      // sanity check for DPI
      if (DPI == 0) {           // this happens on windows if there is no printer installed
            DPI = PDPI;
            pdev = &wi;   //pdev is used to draw text, if it's qprinter, text is tiny.
            }
      DPMM = DPI / INCH;      // dots/mm

      if (debugMode) {
            printf("printer DPI %f(%d) display PDPI %f(%d) DPMM %f\n",
               DPI, pdev->physicalDpiX(),
               PDPI, wi.physicalDpiX(),
               DPMM);
            QStringList sl(QCoreApplication::libraryPaths());
            foreach(const QString& s, sl)
                  printf("LibraryPath: <%s>\n", qPrintable(s));
            }

      // rastral size of font is 20pt = 20/72 inch = 20*DPI/72 dots
      //   staff has 5 lines = 4 * _spatium
      //   _spatium    = SPATIUM20  * DPI;     // 20.0 / 72.0 * DPI / 4.0;

      initSymbols();
      if (!converterMode)
            genIcons();
      initDrumset();

      gscore = new Score(defaultStyle);
      mscore = new MuseScore();

      if (!(converterMode || pluginMode)) {
            mscore->readSettings();
            QObject::connect(qApp, SIGNAL(messageReceived(const QString&)),
               mscore, SLOT(handleMessage(const QString&)));
            static_cast<QtSingleApplication*>(qApp)->setActivationWindow(mscore, false);
            int files = 0;
            foreach(const QString& name, argv) {
                  if (!name.isEmpty())
                        ++files;
                  }
            //
            // TODO: delete old session backups
            //
            if (files || !mscore->restoreSession(preferences.sessionStart == LAST_SESSION))
                  loadScores(argv);
            }
      else {
            loadScores(argv);
            exit(processNonGui() ? 0 : -1);
            }
      mscore->loadPlugins();
      mscore->writeSessionFile(false);
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
      static QAction* lastCmd;

      QString cmd(a->data().toString());

      if (debugMode)
            printf("MuseScore::cmd <%s>\n", cmd.toAscii().data());

      Shortcut* sc = getShortcut(cmd.toAscii().data());
      if (sc == 0) {
            printf("MuseScore::cmd(): unknown action <%s>\n", qPrintable(cmd));
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
            a = lastCmd;
            if (a == 0)
                  return;
            cmd = a->data().toString();
            }
      else
            lastCmd = a;
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
                  score->read(cs->filePath());
                  // hack: so we don't get another checkDirty in appendScore
                  cs->setDirty(false);
                  setCurrentViewer(appendScore(score));
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
            startInspector();
      else if (cmd == "script-debug") {
            scriptDebug = a->isChecked();
            }
      else if (cmd == "backspace")
            undo();
      else if (cmd == "zoomin")
            incMag();
      else if (cmd == "zoomout")
            decMag();
      else if (cmd == "midi-on")
            midiinToggled(a->isChecked());
      else if (cmd == "sound-on")
            speakerToggled(a->isChecked());
      else if (cmd == "undo")
            undo();
      else if (cmd == "redo")
            redo();
      else if (cmd == "toggle-palette")
            showPalette(a->isChecked());
      else if (cmd == "toggle-playpanel")
            showPlayPanel(a->isChecked());
      else if (cmd == "toggle-navigator")
            showNavigator(a->isChecked());
      else if (cmd == "toggle-mixer")
            showMixer(a->isChecked());
      else if (cmd == "synth-control")
            showSynthControl(a->isChecked());
      else if (cmd == "show-keys")
            ;
      else if (cmd == "toggle-transport")
            transportTools->setVisible(!transportTools->isVisible());
      else if (cmd == "toggle-noteinput")
            entryTools->setVisible(!entryTools->isVisible());
      else if (cmd == "local-help")
            helpBrowser();
      else if (cmd == "follow")
            preferences.followSong = a->isChecked();
      else if (cmd == "split-h")
            splitWindow(true);
      else if (cmd == "split-v")
            splitWindow(false);
      else if (cmd == "mag") {
            if (cv)
                  cv->magCanvas();
            }
      else if (cmd == "page-prev") {
            if (cv)
                  cv->pagePrev();
            }
      else if (cmd == "page-next") {
            if (cv)
                  cv->pageNext();
            }
      else if (cmd == "page-top") {
            if (cv)
                  cv->pageTop();
            }
      else if (cmd == "page-end") {
            if (cv)
                  cv->pageEnd();
            }
      else {
            if (cs)
                  cs->cmd(a);
            else
                  printf("unknown cmd <%s>\n", qPrintable(cmd));
            }
      if (inspector)
            inspector->reloadClicked();
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
                  s->action->setEnabled((s->state & val) && _undoGroup->canUndo());
            else if (strcmp(s->xml, "redo") == 0)
                  s->action->setEnabled((s->state & val) && _undoGroup->canRedo());
            else if (strcmp(s->xml, "cut") == 0)
                  s->action->setEnabled(cs && cs->selection()->state());
            else if (strcmp(s->xml, "copy") == 0)
                  s->action->setEnabled(cs && cs->selection()->state());
            else if (strcmp(s->xml, "synth-control") == 0) {
                  Driver* driver = seq ? seq->getDriver() : 0;
                  s->action->setEnabled(driver && driver->getSynth());
                  }
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
      if (cs && (cs->state() == STATE_SEARCH) && (val != STATE_SEARCH))
            searchDialog->hide();

      switch(val) {
            case STATE_DISABLED:
                  _modeText->setText(tr("no score"));
                  _modeText->show();
                  if (inspector)
                        inspector->hide();
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
            case STATE_SEARCH:
                  if (searchDialog == 0) {
                        searchDialog = new QWidget;
                        searchDialog->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
                        QHBoxLayout* searchDialogLayout = new QHBoxLayout;
                        searchDialog->setLayout(searchDialogLayout);
                        layout->insertWidget(2, searchDialog);

                        QToolButton* searchExit = new QToolButton;
                        searchExit->setIcon(QIcon(":/data/cancel.png"));
                        connect(searchExit, SIGNAL(clicked()), SLOT(endSearch()));
                        searchDialogLayout->addWidget(searchExit);

                        searchDialogLayout->addWidget(new QLabel(tr("Go To: ")));

                        searchCombo = new QComboBox;
                        searchCombo->setEditable(true);
                        searchCombo->setInsertPolicy(QComboBox::InsertAtTop);
                        searchDialogLayout->addWidget(searchCombo);

                        searchDialogLayout->addStretch(10);
                        searchDialog->hide();

                        // ?? connect(tab1, SIGNAL(currentChanged(int)), SLOT(setCurrentViewer(int)));
                        connect(searchCombo, SIGNAL(editTextChanged(const QString&)),
                           SLOT(searchTextChanged(const QString&)));
                        }

                  searchCombo->clearEditText();
                  searchCombo->setFocus();
                  searchDialog->show();
                  _modeText->setText(tr("Search"));
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
      state       = STATE_NORMAL;
      xml         = 0;
      standardKey = QKeySequence::UnknownKey;
      key         = 0;
      context     = Qt::WindowShortcut;
      icon        = 0;
      action      = 0;
      translated  = false;
      }

Shortcut::Shortcut(int s, const char* name, const char* d, const QKeySequence& k,
   Qt::ShortcutContext cont, const char* txt, const char* h, QIcon* i)
      {
      state       = s;
      xml         = name;
      standardKey = QKeySequence::UnknownKey;
      key         = k;
      context     = cont;
      icon        = i;
      action      = 0;
      descr       = qApp->translate("action", d);
      help        = qApp->translate("action", h);
      text        = qApp->translate("action", txt);
      translated  = false;
      }

Shortcut::Shortcut(int s, const char* name, const char* d, QKeySequence::StandardKey sk,
   Qt::ShortcutContext cont, const char* txt, const char* h, QIcon* i)
      {
      state       = s;
      xml         = name;
      standardKey = sk;
      key         = 0;
      context     = cont;
      icon        = i;
      action      = 0;
      descr       = qApp->translate("action", d);
      help        = qApp->translate("action", h);
      text        = qApp->translate("action", txt);
      translated  = false;
      }

Shortcut::Shortcut(const Shortcut& c)
      {
      state       = c.state;
      xml         = c.xml;
      standardKey = c.standardKey;
      key         = c.key;
      context     = c.context;
      icon        = c.icon;
      action      = c.action;
      if (c.translated) {
            descr   = c.descr;
            help    = c.help;
            text    = c.text;
            }
      else {
            descr   = qApp->translate("action", c.descr.toUtf8().data());
            help    = qApp->translate("action", c.help.toUtf8().data());
            text    = qApp->translate("action", c.text.toUtf8().data());
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
      settings.setValue("maximized", isMaximized());
      settings.setValue("showPanel", paletteBox && paletteBox->isVisible());
      settings.setValue("state", saveState());
      settings.setValue("splitScreen", _splitScreen);
      if (_splitScreen) {
            settings.setValue("split", _horizontalSplit);
            settings.setValue("splitter", splitter->saveState());
            }
      settings.endGroup();
      if (paletteBox && paletteBox->dirty()) {
            QDir dir;
            dir.mkpath(dataPath);
            paletteBox->write(dataPath + "/mscore-palette.xml");
            }
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
      resize(settings.value("size", QSize(950, 700)).toSize());
      move(settings.value("pos", QPoint(10, 10)).toPoint());
      if (settings.value("maximized", false).toBool())
            showMaximized();
      mscore->showPalette(settings.value("showPanel", "0").toBool());
      restoreState(settings.value("state").toByteArray());
      if (settings.value("splitScreen", false).toBool()) {
            splitWindow(settings.value("split").toBool());
            QAction* a = getAction(_horizontalSplit ? "split-h" : "split-v");
            a->setChecked(true);
            }
      else
            _splitScreen = false;
      settings.endGroup();
      QAction* a = getAction("toggle-transport");
      a->setChecked(!transportTools->isHidden());
      a = getAction("toggle-noteinput");
      a->setChecked(!entryTools->isHidden());
      }

//---------------------------------------------------------
//   play
//    play note for preferences.defaultPlayDuration
//---------------------------------------------------------

void MuseScore::play(Element* e) const
      {
      if (mscore->playEnabled() && e->type() == NOTE) {
            Note* note = static_cast<Note*>(e);
            play(e, note->ppitch());
            }
      }

void MuseScore::play(Element* e, int pitch) const
      {
      if (mscore->playEnabled() && e->type() == NOTE) {
            Note* note = static_cast<Note*>(e);
            Part* part = note->staff()->part();
            Instrument* i = part->instrument();
            seq->startNote(i->channel[note->subchannel()], pitch, 80,
               preferences.defaultPlayDuration, note->tuning());
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
      tab1->setTabText(idx, label);
      tab2->setTabText(idx, label);
      }

//---------------------------------------------------------
//   magChanged
//---------------------------------------------------------

void MuseScore::magChanged(int idx)
      {
      if (cv)
            cv->setMag(idx, mag->getMag(cv));
      }

//---------------------------------------------------------
//   incMag
//---------------------------------------------------------

void MuseScore::incMag()
      {
      if (cv) {
            qreal _mag = cv->mag() * 1.7;
            if (_mag > 16.0)
                  _mag = 16.0;
            cv->setMag(_mag);
            }
      }

//---------------------------------------------------------
//   decMag
//---------------------------------------------------------

void MuseScore::decMag()
      {
      if (cv) {
            qreal nmag = cv->mag() / 1.7;
            if (nmag < 0.05)
                  nmag = 0.05;
            cv->setMag(nmag);
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
      }

//---------------------------------------------------------
//   setPos
//    set position label
//---------------------------------------------------------

void MuseScore::setPos(int t)
      {
      if (cs == 0 || t < 0)
            return;
      AL::TimeSigMap* s = cs->sigmap();
      int bar, beat, tick;
      s->tickValues(t, &bar, &beat, &tick);
      _positionLabel->setText(QString("Bar %1 Beat %2.%3")
         .arg(bar + 1,  3, 10, QLatin1Char(' '))
         .arg(beat + 1, 2, 10, QLatin1Char(' '))
         .arg(tick,     3, 10, QLatin1Char('0'))
         );
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void MuseScore::undo()
      {
      _undoGroup->undo();
      if (cs)
            cs->endUndoRedo();
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void MuseScore::redo()
      {
      _undoGroup->redo();
      if (cs)
            cs->endUndoRedo();
      }

//---------------------------------------------------------
//   showProgressBar
//---------------------------------------------------------

QProgressBar* MuseScore::showProgressBar()
      {
      if (_progressBar == 0)
            _progressBar = new QProgressBar(this);
      _statusBar->addWidget(_progressBar);
      _progressBar->show();
      return _progressBar;
      }

//---------------------------------------------------------
//   hideProgressBar
//---------------------------------------------------------

void MuseScore::hideProgressBar()
      {
      if (_progressBar)
            _statusBar->removeWidget(_progressBar);
      }

//---------------------------------------------------------
//   searchTextChanged
//---------------------------------------------------------

void MuseScore::searchTextChanged(const QString& s)
      {
      if (cs == 0)
            return;
      cs->search(s);
      }

//---------------------------------------------------------
//   endSearch
//---------------------------------------------------------

void MuseScore::endSearch()
      {
      if (cs)
            cs->setState(STATE_NORMAL);
      }

//---------------------------------------------------------
//   handleMessage
//---------------------------------------------------------

void MuseScore::handleMessage(const QString& message)
      {
      if (message.isEmpty())
            return;
      ((QtSingleApplication*)(qApp))->activateWindow();
      Score* score = new Score(defaultStyle);
      score->read(message);
      setCurrentViewer(appendScore(score));
      lastOpenPath = score->fileInfo()->path();
      }

//---------------------------------------------------------
//   editInPianoroll
//---------------------------------------------------------

void MuseScore::editInPianoroll(Staff* staff)
      {
      if (pianorollEditor == 0)
            pianorollEditor = new PianorollEditor;
      else
            disconnect(pianorollEditor->score(), SIGNAL(selectionChanged(int)), pianorollEditor, SLOT(changeSelection(int)));
      pianorollEditor->setStaff(staff);
      pianorollEditor->show();
      connect(staff->score(), SIGNAL(selectionChanged(int)), pianorollEditor, SLOT(changeSelection(int)));
      }

//---------------------------------------------------------
//   writeSessionFile
//---------------------------------------------------------

void MuseScore::writeSessionFile(bool cleanExit)
      {
      printf("write session file\n");

      QDir dir;
      dir.mkpath(dataPath);
      QFile f(dataPath + "/session");
      if (!f.open(QIODevice::WriteOnly)) {
            printf("cannot create session file <%s>\n", qPrintable(f.fileName()));
            return;
            }
      Xml xml(&f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      xml.tagE(cleanExit ? "clean" : "dirty");
      foreach(Score* score, scoreList) {
            xml.stag("Score");
            xml.tag("created", score->created());
            xml.tag("dirty", score->dirty());
            if (score->tmpName().isEmpty()) {
                  xml.tag("path", score->fileInfo()->absoluteFilePath());
                  }
            else {
                  xml.tag("name", score->fileInfo()->absoluteFilePath());
                  xml.tag("path", score->tmpName());
                  }
            xml.etag();
            }
      int tab = 0;
      int idx = 0;
      for (int i = 0; i < tab1->count(); ++i) {
            Viewer* v = tab1->viewer(i);
            if (v) {
                  if (v == cv) {
                        tab = 0;
                        idx = i;
                        }
                  xml.stag("Viewer");
                  xml.tag("tab", tab);    // 0 instead of "tab" does not work
                  xml.tag("idx", i);
                  if (v->magIdx() == MAG_FREE)
                        xml.tag("mag", v->mag());
                  else
                        xml.tag("magIdx", v->magIdx());
                  xml.tag("x",   v->xoffset() / DPMM);
                  xml.tag("y",   v->yoffset() / DPMM);
                  xml.etag();
                  }
            }
      if (splitScreen()) {
            for (int i = 0; i < tab2->count(); ++i) {
                  Viewer* v = tab2->viewer(i);
                  if (v) {
                        if (v == cv) {
                              tab = 1;
                              idx = i;
                              }
                        xml.stag("Viewer");
                        xml.tag("tab", 1);
                        xml.tag("idx", i);
                        if (v->magIdx() == MAG_FREE)
                              xml.tag("mag", v->mag());
                        else
                              xml.tag("magIdx", v->magIdx());
                        xml.tag("x",   v->xoffset() / DPMM);
                        xml.tag("y",   v->yoffset() / DPMM);
                        xml.etag();
                        }
                  }
            }
      xml.tag("tab", tab);
      xml.tag("idx", idx);
      xml.etag();
      f.close();
      if (cleanExit) {
            // TODO: remove all temporary session backup files
            }
      }

//---------------------------------------------------------
//   removeSessionFile
//    remove temp files and session file
//---------------------------------------------------------

void MuseScore::removeSessionFile()
      {
printf("remove session file\n");
      QFile f(dataPath + "/session");
      if (!f.exists())
            return;
      if (!f.remove()) {
            printf("cannot remove session file <%s>\n", qPrintable(f.fileName()));
            }
      }

//---------------------------------------------------------
//   autoSaveTimerTimeout
//---------------------------------------------------------

void MuseScore::autoSaveTimerTimeout()
      {
      bool sessionChanged = false;
printf("auto save\n");
      foreach(Score* s, scoreList) {
            if (s->autosaveDirty()) {
                  QString tmp = s->tmpName();
                  if (!tmp.isEmpty()) {
                        QFileInfo fi(tmp);
                        // TODO: cannot catch exeption here:
                        cs->saveCompressedFile(fi, true);
                        }
                  else {
                        QDir dir;
                        dir.mkpath(dataPath);
                        QTemporaryFile tf(dataPath + "/scXXXXXX.mscz");
                        tf.setAutoRemove(false);
                        if (!tf.open()) {
                              printf("autoSaveTimerTimeout(): create temporary file failed\n");
                              return;
                              }
                        s->setTmpName(tf.fileName());
                        QFileInfo info(tf.fileName());
                        s->saveCompressedFile(&tf, info, true);
                        tf.close();
                        sessionChanged = true;
                        }
                  s->setAutosaveDirty(false);
                  }
            }
//      if (sessionChanged)
            writeSessionFile(false);
      if (preferences.autoSave) {
            int t = preferences.autoSaveTime * 60 * 1000;
            autoSaveTimer->start(t);
            }
      }

//---------------------------------------------------------
//   restoreSession
//    Restore last session. If "always" is true, then restore
//    last session even on a clean exit else only if last
//    session ended abnormal.
//    Return true if a session was found and restored.
//---------------------------------------------------------

bool MuseScore::restoreSession(bool always)
      {
      QFile f(dataPath + "/session");
      if (!f.exists())
            return false;
      if (!f.open(QIODevice::ReadOnly)) {
            printf("cannot open session file <%s>\n", qPrintable(f.fileName()));
            return false;
            }
      QDomDocument doc;
      int line, column;
      QString err;
      docName = f.fileName();
      if (!doc.setContent(&f, false, &err, &line, &column)) {
            QString error;
            error.sprintf("error reading session file %s at line %d column %d: %s\n",
               qPrintable(docName), line, column, qPrintable(err));
            return false;
            }
      int tab = 0;
      int idx = 0;
      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  /* QString version = e.attribute(QString("version"));
                  QStringList sl  = version.split('.');
                  int v           = sl[0].toInt() * 100 + sl[1].toInt();
                  */
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull();  ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        if (tag == "clean") {
                              if (!always) {
                                    f.close();
                                    return false;
                                    }
                              }
                        else if (tag == "dirty") {
                              QMessageBox::StandardButton b = QMessageBox::question(0,
                                 tr("MuseScore"),
                                 tr("Last Session ended abnormal.\nRestore Session?"),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::Yes
                                 );
                              if (b != QMessageBox::Yes) {
                                    f.close();
                                    return false;
                                    }
                              }
                        else if (tag == "Score") {
                              QString name;
                              bool created = false;
                              bool dirty = false;
                              for (QDomElement eee = ee.firstChildElement(); !eee.isNull();  eee = eee.nextSiblingElement()) {
                                    QString tag(eee.tagName());
                                    QString val(eee.text());
                                    if (tag == "name")
                                          name = val;
                                    else if (tag == "created")
                                          created = val.toInt();
                                    else if (tag == "dirty")
                                          dirty = val.toInt();
                                    else if (tag == "path") {
                                          printf("restore <%s>\n", qPrintable(val));
                                          Score* score = new Score(defaultStyle);
                                          if (!score->read(val)) {
                                                printf("failed to restore <%s>\n", qPrintable(val));
                                                delete score;
                                                f.close();
                                                return false;
                                                }
                                          else {
                                                if (!name.isEmpty()) {
                                                      printf("set name <%s>\n", qPrintable(name));
                                                      score->setName(name);
                                                      }
                                                appendScore(score);
                                                score->setDirty(dirty);
                                                score->setCreated(created);
                                                }
                                          }
                                    else {
                                          domError(eee);
                                          f.close();
                                          return false;
                                          }
                                    }
                              }
                        else if (tag == "Viewer") {
                              double x = .0, y = .0, vmag = .0;
                              int magIdx = MAG_FREE;
                              int tab = 0, idx = 0;
                              for (QDomElement eee = ee.firstChildElement(); !eee.isNull();  eee = eee.nextSiblingElement()) {
                                    QString tag(eee.tagName());
                                    QString val(eee.text());
                                    if (tag == "tab")
                                          tab = val.toInt();
                                    else if (tag == "idx")
                                          idx = val.toInt();
                                    else if (tag == "mag")
                                          vmag = val.toDouble();
                                    else if (tag == "magIdx")
                                          magIdx = val.toInt();
                                    else if (tag == "x")
                                          x = val.toDouble() * DPMM;
                                    else if (tag == "y")
                                          y = val.toDouble() * DPMM;
                                    else {
                                          domError(eee);
                                          f.close();
                                          return false;
                                          }
                                    }
                              if (magIdx != MAG_FREE)
                                    vmag = mag->getMag(cv);
                              (tab == 0 ? tab1 : tab2)->initViewer(idx, vmag, magIdx, x, y);
                              }
                        else if (tag == "tab")
                              tab = ee.text().toInt();
                        else if (tag == "idx")
                              idx = ee.text().toInt();
                        else {
                              domError(ee);
                              f.close();
                              return false;
                              }
                        }
                  }
            else {
                  domError(e);
                  f.close();
                  return false;
                  }
            }
      setCurrentView(tab, idx);
      f.close();
      return true;
      }

//---------------------------------------------------------
//   splitWindow
//---------------------------------------------------------

void MuseScore::splitWindow(bool horizontal)
      {
      if (!_splitScreen) {
            tab2->setVisible(true);
            _splitScreen = true;
            _horizontalSplit = horizontal;
            splitter->setOrientation(_horizontalSplit ? Qt::Horizontal : Qt::Vertical);
            if (!scoreList.isEmpty()) {
                  tab2->setCurrentIndex(0);
                  Score* s = scoreList[0];
                  s->setLayoutAll(true);
                  s->end();
                  setCurrentView(1, 0);
                  }
            }
      else {
            if (_horizontalSplit == horizontal) {
                  _splitScreen = false;
                  tab2->setVisible(false);
                  }
            else {
                  _horizontalSplit = horizontal;
                  QAction* a;
                  if (_horizontalSplit)
                        a = getAction("split-v");
                  else
                        a = getAction("split-h");
                  a->setChecked(false);
                  splitter->setOrientation(_horizontalSplit ? Qt::Horizontal : Qt::Vertical);
                  }
            }
      }

