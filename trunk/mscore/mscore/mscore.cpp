//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: mscore.cpp,v 1.105 2006/09/15 09:34:57 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#include <signal.h>
#include <fenv.h>
#include "config.h"

#include "mscore.h"
#include "canvas.h"
#include "style.h"
#include "score.h"
#include "instrdialog.h"
#include "preferences.h"
#include "config.h"
#include "icons.h"
#include "textstyle.h"
#include "xml.h"
#include "seq.h"
#include "icons.h"
#include "tempo.h"
#include "padstate.h"
#include "pad.h"
#include "sym.h"

#include "data/fileprint.xpm"
#include "data/filenew.xpm"
#include "data/fileopen.xpm"
#include "data/filesave.xpm"
#include "data/undo.xpm"
#include "data/redo.xpm"
#include "data/editcut.xpm"
#include "data/editcopy.xpm"
#include "data/editpaste.xpm"
#include "data/undoS.xpm"
#include "data/redoS.xpm"
#include "data/exitS.xpm"
#include "data/viewmag.xpm"
#include "data/viewmagS.xpm"
#include "data/start.xpm"
#include "data/stop.xpm"
#include "data/play.xpm"
#include "data/midiin.xpm"
#include "data/speaker.xpm"

#include "padids.h"
#include "pad.h"
#include "alsa.h"
#include "pagesettings.h"
#include "listedit.h"
#include "editstyle.h"
#include "playpanel.h"
#include "page.h"

PadState padState;
QString mscoreGlobalShare;
QString language;

const char* magTable[] = {
     "25%", "50%", "75%", "100%", "150%", "200%", "400%", "800%", "1600%",
      QT_TRANSLATE_NOOP("magTable","PgeWidth"),
      QT_TRANSLATE_NOOP("magTable","Page"),
      QT_TRANSLATE_NOOP("magTable","DblPage"),
     };

const char* fileOpenText       = QT_TR_NOOP("Load Score from File");
const char* fileSaveText       = QT_TR_NOOP("Save Score to File");
const char* fileNewText        = QT_TR_NOOP("Create New Score");
const char* filePrintText      = QT_TR_NOOP("Print Score");
const char* infoStartButton    = QT_TR_NOOP("rewind to start position");
const char* infoStopButton     = QT_TR_NOOP("stop sequencer");
const char* infoPlayButton     = QT_TR_NOOP("start sequencer play");

static unsigned int startMag = 3;   // 100%, synchronize with canvas default

QAction* undoAction;
QAction* redoAction;
const char* eventRecordFile;

static bool haveMidi;

struct ProjectList {
      QString name;
      bool loaded;
      bool current;
      };
ProjectList projectList[PROJECT_LIST_LEN];

int appDpiX = 75;
int appDpiY = 75;

//---------------------------------------------------------
//   getSharePath
//---------------------------------------------------------

static QString getSharePath()
      {
      return QString( INSTPREFIX "/share/" INSTALL_NAME);
      }

//---------------------------------------------------------
//   printVersion
//---------------------------------------------------------

static void printVersion(const char* prog)
      {
      fprintf(stderr, "%s: Linux Music Score Editor; Version %s\n", prog, VERSION);
      }

//---------------------------------------------------------
//   quitDoc
//---------------------------------------------------------

void MuseScore::quitDoc()
      {
      if (checkDirty())
            return;
      int n = scoreList.size();
      if (n <= 1) {
            close();
            return;
            }
      for (int i = 0; i < n; ++i) {
            if (scoreList[i] == cs) {
                  std::vector<Score*>::iterator ii = scoreList.begin() + i;
                  scoreList.erase(ii);
                  tab->removeTab(i);
                  cs = 0;
                  if (i >= (n-1))
                        i = 0;
                  setCurrentScore(i);
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void MuseScore::closeEvent(QCloseEvent* /*ev*/)
      {
      if (!checkDirty())
            qApp->quit();
      saveScoreList();
      seq->exit();
      }

//---------------------------------------------------------
//   preferencesChanged
//---------------------------------------------------------

void MuseScore::preferencesChanged()
      {
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
      transportAction->setEnabled(seq->isRunning());
      transportId->setChecked(seq->isRunning());
      transportTools->setShown(seq->isRunning());
      midiinAction->setEnabled(preferences.enableMidiInput);
      }

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

MuseScore::MuseScore()
   : QMainWindow()
      {
      setWindowTitle(QString("MuseScore"));
      cs                    = 0;
      textStyleDialog       = 0;
      editStyleWin          = 0;
      instrList             = 0;
      playPanel             = 0;
      preferenceDialog      = 0;
      measuresDialog        = 0;
      iledit                = 0;
      pageListEdit          = 0;
      measureListEdit       = 0;
      symbolPalette1        = 0;
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
      pad                   = 0;
      _midiinEnabled        = true;
      _speakerEnabled       = true;

      QWidget* mainWindow = new QWidget;
      layout = new QVBoxLayout;
      mainWindow->setLayout(layout);
      layout->setMargin(0);
      layout->setSpacing(0);
      tab    = new TabBar(mainWindow);
      canvas = new Canvas(mainWindow);
      layout->addWidget(tab);
      layout->addWidget(canvas);

      connect(tab, SIGNAL(currentChanged(int)), SLOT(setCurrentScore(int)));
      connect(tab, SIGNAL(doubleClick(int)), SLOT(removeTab(int)));

      QPixmap newIcon(filenew_xpm);
      QPixmap saveIcon(filesave_xpm);
      QPixmap printIcon(fileprint_xpm);
      QPixmap newIconS(filenew_xpm);
      QPixmap openIconS(fileopen_xpm);
      QPixmap openIcon(fileopen_xpm);
      QPixmap saveIconS(filesave_xpm);
      QPixmap printIconS(fileprint_xpm);
      QPixmap undoIconS(undoS_xpm);
      QPixmap undoIcon(undo_xpm);
      QPixmap redoIconS(redoS_xpm);
      QPixmap redoIcon(redo_xpm);
      QPixmap exitIconS(exitS_xpm);
      QPixmap viewmagIcon(viewmag_xpm);
      QPixmap viewmagIconS(viewmagS_xpm);

      QPixmap midiinIcon(midiin_xpm);
      QPixmap speakerIcon(speaker_xpm);

      QPixmap startIcon(start_xpm);
      QPixmap stopIcon(stop_xpm);
      QPixmap playIcon(play_xpm);

      QAction* whatsThis = QWhatsThis::createAction(this);

      //---------------------------------------------------
      //    Transport Action
      //---------------------------------------------------

      midiinAction = new QAction(QIcon(midiinIcon), tr("Enable Midi Input"), this);
      midiinAction->setCheckable(true);
      midiinAction->setEnabled(preferences.enableMidiInput);
      midiinAction->setChecked(_midiinEnabled);
      connect(midiinAction, SIGNAL(triggered(bool)), SLOT(midiinToggled(bool)));

      speakerAction = new QAction(QIcon(speakerIcon), tr("Enable Sound while editing"), this);
      speakerAction->setCheckable(true);
      speakerAction->setEnabled(preferences.playNotes);
      speakerAction->setChecked(_speakerEnabled);
      connect(speakerAction, SIGNAL(triggered(bool)), SLOT(speakerToggled(bool)));

      transportAction = new QActionGroup(this);
      transportAction->setExclusive(true);

      QAction* startAction = new QAction(QIcon(startIcon), tr("Rewind"), transportAction);
      startAction->setWhatsThis(tr(infoStartButton));
      connect(startAction, SIGNAL(triggered()), seq, SLOT(rewindStart()));

      stopAction = new QAction(QIcon(stopIcon), tr("Stop"), transportAction);
      stopAction->setCheckable(true);
      stopAction->setChecked(true);
      stopAction->setWhatsThis(tr(infoStopButton));
      connect(stopAction, SIGNAL(toggled(bool)), this, SLOT(setStop(bool)));

      playAction = new QAction(QIcon(playIcon), tr("Play"), transportAction);
      playAction->setCheckable(true);
      playAction->setWhatsThis(tr(infoPlayButton));
      connect(playAction, SIGNAL(toggled(bool)), this, SLOT(setPlay(bool)));

      transportAction->addAction(startAction);
      transportAction->addAction(stopAction);
      transportAction->addAction(playAction);

      //---------------------------------------------------
      //    File Action
      //---------------------------------------------------

      QAction* fileNewAction = new QAction(QIcon(newIcon), tr("&New"), this);
      fileNewAction->setToolTip(tr(fileNewText));

      QAction* fileOpenAction = new QAction(QIcon(openIcon), tr("&Open"), this);
      fileOpenAction->setToolTip(tr(fileOpenText));

      QAction* fileSaveAction = new QAction(QIcon(saveIcon), tr("&Save"), this);
      fileSaveAction->setToolTip(tr(fileSaveText));

      QAction* filePrintAction = new QAction(QIcon(printIcon), tr("&Print"), this);
      filePrintAction->setToolTip(tr(filePrintText));

      QActionGroup* undoRedo = new QActionGroup(this);

      undoAction = new QAction(QIcon(undoIcon), tr("Und&o"), undoRedo);
      redoAction = new QAction(QIcon(redoIcon), tr("Re&do"), undoRedo);
      undoAction->setWhatsThis(tr("undo last change"));
      undoAction->setStatusTip(tr("undo last change"));
      redoAction->setWhatsThis(tr("redo last undo"));

      connect(redoAction,      SIGNAL(triggered()), SLOT(doRedo()));
      connect(undoAction,      SIGNAL(triggered()), SLOT(doUndo()));

      connect(filePrintAction, SIGNAL(triggered()), SLOT(printFile()));
      connect(fileNewAction,   SIGNAL(triggered()), SLOT(newFile()));
      connect(fileOpenAction,  SIGNAL(triggered()), SLOT(loadFile()));
      connect(fileSaveAction,  SIGNAL(triggered()), SLOT(saveFile()));

      //---------------------
      //    Tool Bar
      //---------------------

      fileTools = new QToolBar(tr("File Operations"));
      addToolBar(fileTools);

      fileTools->addAction(fileNewAction);
      fileTools->addAction(fileOpenAction);
      fileTools->addAction(fileSaveAction);
      fileTools->addAction(filePrintAction);
      fileTools->addAction(whatsThis);
      fileTools->addSeparator();
      fileTools->addAction(undoAction);
      fileTools->addAction(redoAction);
      fileTools->addSeparator();

      transportTools = new QToolBar(tr("Transport Tools"));
      addToolBar(transportTools);
      transportTools->addAction(speakerAction);
      transportTools->addAction(midiinAction);
      transportTools->addSeparator();
      transportTools->addAction(startAction);
      transportTools->addAction(stopAction);
      transportTools->addAction(playAction);

      QAction* magAction = new QAction(QIcon(viewmagIcon), tr("Mag"), fileTools);
      fileTools->addAction(magAction);
      connect(magAction, SIGNAL(triggered()), canvas, SLOT(magCanvas()));

      mag = new QComboBox(fileTools);
      mag->setEditable(true);
      mag->setValidator(new QDoubleValidator(.05, 20.0, 2, mag));
      for (unsigned int i =  0; i < sizeof(magTable)/sizeof(*magTable); ++i) {
            mag->addItem(tr(magTable[i]), i);
            if (i == startMag)
                  mag->setCurrentIndex(i);
            }

      connect(mag, SIGNAL(activated(int)), SLOT(magChanged(int)));
      fileTools->addWidget(mag);
      addToolBarBreak();

      //-------------------------------
      //    Note Entry Tool Bar
      //-------------------------------

      struct {
            int id;
            bool toggleButton;
            QIcon* is;
            const char* tt;         // tool tip
            } ne[] = {
            { PAD_NOTE1,  true, &noteIcon,       "1/1" },
            { PAD_NOTE2,  true, &note2Icon,      "1/2" },
            { PAD_NOTE4,  true, &note4Icon,      "1/4" },
            { PAD_NOTE8,  true, &note8Icon,      "1/8" },
            { PAD_NOTE16, true, &note16Icon,     "1/16" },
            { PAD_NOTE32, true, &note32Icon,     "1/32" },
            { PAD_NOTE64, true, &note64Icon,     "1/64" },
            { PAD_DOT,    true, &dotIcon,        QT_TR_NOOP("dot") },
            { PAD_TIE,    true, &plusIcon,       QT_TR_NOOP("tie") },
            { -1,         false,0,               0 },
            { PAD_REST,   true, &quartrestIcon,  QT_TR_NOOP("rest") },
            { -1,         false, 0,              0 },
            { PAD_SHARP2, true, &sharpsharpIcon, QT_TR_NOOP("double sharp") },
            { PAD_SHARP,  true, &sharpIcon,      QT_TR_NOOP("sharp") },
            { PAD_NAT,    true, &naturalIcon,    QT_TR_NOOP("natural") },
            { PAD_FLAT,   true, &flatIcon,       QT_TR_NOOP("flat") },
            { PAD_FLAT2,  true, &flatflatIcon,   QT_TR_NOOP("double flat") },
            { -1,         false, 0,              0 },
            { PAD_FLIP,   false, &flipIcon,      QT_TR_NOOP("flip stem") },
            };

      entryTools = new QToolBar(tr("Note Entry"));
      entryTools->setIconSize(QSize(ICON_WIDTH, ICON_HEIGHT));
      addToolBar(entryTools);

      int n = 0;
      for (unsigned int i = 0; i < sizeof(ne)/sizeof(*ne); ++i) {
            int id = ne[i].id;
            if (id < 0) {
                  entryTools->addSeparator();
                  continue;
                  }
            entryAction[n] = entryTools->addAction(*ne[i].is, ne[i].tt);
            entryAction[n]->setCheckable(true);
            entryAction[n]->setData(QVariant(id));
            entryAction[n]->setToolTip(ne[i].tt);
            ++n;
            }
      entryTools->addSeparator();
      for (int i = 0; i < VOICES; ++i) {
            QString txt = QString("%1").arg(i+1);
            voiceAction[i] = entryTools->addAction(voiceIcons[i], txt);
            voiceAction[i]->setCheckable(true);
            voiceAction[i]->setData(QVariant(i + PAD_VOICE0));
            voiceAction[i]->setToolTip(QString("voice %1").arg(i+1));
            }

      connect(entryTools, SIGNAL(actionTriggered(QAction*)), SLOT(padTriggered(QAction*)));

      //---------------------
      //    Menus
      //---------------------

      QMenuBar* mb = menuBar();

      //---------------------
      //    Menu File
      //---------------------

      QMenu* menuFile = mb->addMenu(tr("&File"));

      menuFile->addAction(fileNewAction);
      menuFile->addAction(fileOpenAction);
      openRecent = new QMenu(tr("Open &Recent"));
      connect(openRecent, SIGNAL(aboutToShow()), SLOT(openRecentMenu()));
      connect(openRecent, SIGNAL(triggered(QAction*)), SLOT(selectScore(QAction*)));
      menuFile->addMenu(openRecent);
      menuFile->addSeparator();
      menuFile->addAction(fileSaveAction);
      menuFile->addAction(QIcon(saveIcon), tr("Save &As"), this, SLOT(saveAs()), Qt::CTRL + Qt::Key_A);
      menuFile->addSeparator();
      menuFile->addAction(QIcon(saveIcon), tr("Export Midi"),     this, SLOT(exportMidi()));
      menuFile->addAction(QIcon(saveIcon), tr("Export MusicXML"), this, SLOT(exportMusicXml()));
      menuFile->addAction(QIcon(openIcon), tr("Import Midi"),     this, SLOT(importMidi()));
      menuFile->addAction(QIcon(openIcon), tr("Import MusicXML"), this, SLOT(importMusicXml()));
      menuFile->addSeparator();
      menuFile->addAction(filePrintAction);
      menuFile->addSeparator();
      menuFile->addAction(QIcon(exitIconS), tr("&Quit"), this, SLOT(quitDoc()), Qt::CTRL + Qt::Key_Q);

      //---------------------
      //    Menu Edit
      //---------------------

      menuEdit = mb->addMenu(tr("&Edit"));

      //menuEdit->addAction(undoRedo);
      menuEdit->addAction(undoAction);
      menuEdit->addAction(redoAction);

      menuEdit->addSeparator();

      cutId = menuEdit->addAction(QIcon(QPixmap(editcut_xpm)), tr("C&ut"),  canvas, SLOT(cmdCut()));
      cutId->setShortcut(Qt::CTRL + Qt::Key_X);
      copyId = menuEdit->addAction(QIcon(QPixmap(editcopy_xpm)), tr("&Copy"),  canvas, SLOT(cmdCopy()));
      copyId->setShortcut(Qt::CTRL + Qt::Key_C);
      pasteId = menuEdit->addAction(QIcon(QPixmap(editpaste_xpm)), tr("&Paste"), canvas, SLOT(cmdPaste()));
      pasteId->setShortcut(Qt::CTRL + Qt::Key_V);
      selectionChanged(0);
      menuEdit->addSeparator();
      menuEdit->addAction(tr("Instrument List..."), this, SLOT(startInstrumentListEditor()));
      menuEdit->addSeparator();
      menuEdit->addAction(tr("Page List..."), this, SLOT(startPageListEditor()));
      menuEdit->addSeparator();
      menuEdit->addAction(tr("Preferences..."), this, SLOT(startPreferenceDialog()));

      //---------------------
      //    Menu Create
      //---------------------

      QMenu* menuCreate = genCreateMenu();
      mb->addMenu(menuCreate);

      //---------------------
      //    Menu Notes
      //---------------------

      QMenu* menuNotes = mb->addMenu(tr("Notes"));

      menuNotes->addAction(tr("Input\tN"), this, SLOT(startNoteEntry()));
      menuNotes->addSeparator();

      QMenu* menuAddPitch = new QMenu(tr("Add Pitch"));
      QAction* a;
      a = menuAddPitch->addAction("A");
      a->setData(0);;
      a = menuAddPitch->addAction("B");
      a->setData(1);
      a = menuAddPitch->addAction("C");
      a->setData(2);
      a = menuAddPitch->addAction("D");
      a->setData(3);
      a = menuAddPitch->addAction("E");
      a->setData(4);
      a = menuAddPitch->addAction("F");
      a->setData(5);
      a = menuAddPitch->addAction("G");
      a->setData(6);

      menuNotes->addMenu(menuAddPitch);

      connect(menuAddPitch, SIGNAL(triggered(QAction*)), SLOT(cmdAddPitch(QAction*)));

      QMenu* menuAddIntervall = new QMenu(tr("Add Intervall"));

      a = menuAddIntervall->addAction(tr("Prime\t1"));
      a->setData(1);
      a = menuAddIntervall->addAction(tr("Sekunde above\t2"));
      a->setData(2);
      a = menuAddIntervall->addAction(tr("Terz above\t3"));
      a->setData(3);
      a = menuAddIntervall->addAction(tr("Quart above\t4"));
      a->setData(4);
      a = menuAddIntervall->addAction(tr("Quinte above\t5"));
      a->setData(5);
      a = menuAddIntervall->addAction(tr("Sexte above\t6"));
      a->setData(6);
      a = menuAddIntervall->addAction(tr("Septime above\t7"));
      a->setData(7);
      a = menuAddIntervall->addAction(tr("Oktave above\t8"));
      a->setData(8);
      a = menuAddIntervall->addAction(tr("None above\t9"));
      a->setData(9);
      menuAddIntervall->addSeparator();
      a = menuAddIntervall->addAction(tr("Sekunde below"));
      a->setData(12);
      a = menuAddIntervall->addAction(tr("Terz below"));
      a->setData(13);
      a = menuAddIntervall->addAction(tr("Quart below"));
      a->setData(14);
      a = menuAddIntervall->addAction(tr("Quinte below"));
      a->setData(15);
      a = menuAddIntervall->addAction(tr("Sexte below"));
      a->setData(16);
      a = menuAddIntervall->addAction(tr("Septime below"));
      a->setData(17);
      a = menuAddIntervall->addAction(tr("Oktave below"));
      a->setData(18);
      a = menuAddIntervall->addAction(tr("None below"));
      a->setData(19);

      menuNotes->addMenu(menuAddIntervall);
      connect(menuAddIntervall, SIGNAL(triggered(QAction*)), SLOT(cmdAddIntervall(QAction*)));

      QMenu* menuNtole = new QMenu(tr("N-Tole"));

      a = menuNtole->addAction("duole");
      a->setData(2);
      a->setShortcut(Qt::CTRL+Qt::Key_2);
      a = menuNtole->addAction("triole");
      a->setData(3);
      a->setShortcut(Qt::CTRL+Qt::Key_3);
      a = menuNtole->addAction("pentole");
      a->setData(5);
      a->setShortcut(Qt::CTRL+Qt::Key_5);

      menuNotes->addMenu(menuNtole);
      connect(menuNtole, SIGNAL(triggered(QAction*)), SLOT(cmdTuplet(QAction*)));

      //---------------------
      //    Menu Layout
      //---------------------

      QMenu* menuLayout = mb->addMenu(tr("&Layout"));

      menuLayout->addAction(tr("Page Settings..."), this, SLOT(showPageSettings()));
      menuLayout->addAction(tr("Reset Positions"),  this, SLOT(resetUserOffsets()));
      menuLayout->addAction(tr("Set Normal Staff Distances"),  canvas, SLOT(resetStaffOffsets()));
      menuLayout->addAction(tr("Reset Stretch"), this, SLOT(resetUserStretch()));
      menuLayout->addAction(tr("Breaks..."), this, SLOT(showLayoutBreakPalette()));

      //---------------------
      //    Menu Style
      //---------------------

      QMenu* menuStyle = mb->addMenu(tr("&Style"));
      menuStyle->addAction(tr("Edit Style..."), this, SLOT(editStyle()));
      menuStyle->addAction(tr("Edit Text Style..."), this, SLOT(editTextStyle()));
      menuStyle->addSeparator();
      menuStyle->addAction(QIcon(openIcon), tr("Load Style"), this, SLOT(loadStyle()));
      menuStyle->addAction(QIcon(saveIcon), tr("Save Style"), this, SLOT(saveStyle()));

      //---------------------
      //    Menu Display
      //---------------------

      menuDisplay = mb->addMenu(tr("&Display"));

      padId = new QAction(tr("Pad"), this);
      padId->setShortcut(Qt::Key_F10);
      padId->setShortcutContext(Qt::ApplicationShortcut);
      padId->setCheckable(true);
      connect(padId, SIGNAL(toggled(bool)), SLOT(showPad(bool)));
      menuDisplay->addAction(padId);

      playId = new QAction(tr("PlayPanel"), this);
      playId->setCheckable(true);
      playId->setShortcut(Qt::Key_F11);
      playId->setShortcutContext(Qt::ApplicationShortcut);
      connect(playId, SIGNAL(toggled(bool)), SLOT(showPlayPanel(bool)));
      menuDisplay->addAction(playId);

      navigatorId = new QAction(tr("Navigator"), this);
      navigatorId->setCheckable(true);
      navigatorId->setShortcut(Qt::Key_F12);
      navigatorId->setShortcutContext(Qt::ApplicationShortcut);
      connect(navigatorId, SIGNAL(toggled(bool)), SLOT(showNavigator(bool)));
      menuDisplay->addAction(navigatorId);

      menuDisplay->addSeparator();

      transportId = new QAction(tr("Transport Toolbar"), this);
      transportId->setCheckable(true);
      menuDisplay->addAction(transportId);
      connect(transportId, SIGNAL(toggled(bool)), transportTools, SLOT(setVisible(bool)));

      inputId = new QAction(tr("Note Input Toolbar"), this);
      inputId->setCheckable(true);
      inputId->setChecked(true);
      menuDisplay->addAction(inputId);
      connect(inputId, SIGNAL(toggled(bool)), entryTools, SLOT(setVisible(bool)));

      // if we have no sequencer, disable transport and play panel
      if (!seq->isRunning()) {
            transportId->setEnabled(false);
            playId->setEnabled(false);
            }

      menuDisplay->addSeparator();
      visibleId = menuDisplay->addAction(tr("Show Invisible"), this, SLOT(showInvisibleClicked()));
      visibleId->setCheckable(true);

      //---------------------
      //    Menu Help
      //---------------------

      mb->addSeparator();
      QMenu* menuHelp = mb->addMenu(tr("&Help"));

      menuHelp->addAction(tr("Browser"),  this, SLOT(helpBrowser()), Qt::Key_F1);
      menuHelp->addAction(tr("&About"),   this, SLOT(about()));
      menuHelp->addAction(tr("About&Qt"), this, SLOT(aboutQt()));
      menuHelp->addSeparator();
      menuHelp->addAction(whatsThis);

      setCentralWidget(mainWindow);
      connect(canvas, SIGNAL(magChanged()), SLOT(updateMag()));

      loadInstrumentTemplates();
      preferencesChanged();
      connect(seq, SIGNAL(started()), SLOT(seqStarted()));
      connect(seq, SIGNAL(stopped()), SLOT(seqStopped()));
      loadScoreList();
      }

//---------------------------------------------------------
//   setEntry
//---------------------------------------------------------

void MuseScore::setEntry(bool flag, int i)
      {
      if (pad)
            pad->setOn(flag, i);
      switch (i) {
            case PAD_NOTE1:  entryAction[0]->setChecked(flag);  break;
            case PAD_NOTE2:  entryAction[1]->setChecked(flag);  break;
            case PAD_NOTE4:  entryAction[2]->setChecked(flag);  break;
            case PAD_NOTE8:  entryAction[3]->setChecked(flag);  break;
            case PAD_NOTE16: entryAction[4]->setChecked(flag);  break;
            case PAD_NOTE32: entryAction[5]->setChecked(flag);  break;
            case PAD_NOTE64: entryAction[6]->setChecked(flag);  break;
            case PAD_DOT:    entryAction[7]->setChecked(flag);  break;
            case PAD_TIE:    entryAction[8]->setChecked(flag);  break;
            case PAD_REST:   entryAction[9]->setChecked(flag);  break;
            case PAD_SHARP2: entryAction[10]->setChecked(flag);  break;
            case PAD_SHARP:  entryAction[11]->setChecked(flag); break;
            case PAD_NAT:    entryAction[12]->setChecked(flag); break;
            case PAD_FLAT:   entryAction[13]->setChecked(flag); break;
            case PAD_FLAT2:  entryAction[14]->setChecked(flag); break;
            case PAD_VOICE0: voiceAction[0]->setChecked(flag); break;
            case PAD_VOICE1: voiceAction[1]->setChecked(flag); break;
            case PAD_VOICE2: voiceAction[2]->setChecked(flag); break;
            case PAD_VOICE3: voiceAction[3]->setChecked(flag); break;
            }
      }

//---------------------------------------------------------
//   magChanged
//---------------------------------------------------------

void MuseScore::magChanged(int idx)
      {
      qreal mag = canvas->mag();
      QSizeF s = canvas->fsize();
      switch(idx) {
            case 0:  mag = 0.25; break;
            case 1:  mag = 0.5;  break;
            case 2:  mag = 0.75; break;
            case 3:  mag = 1.0;  break;
            case 4:  mag = 1.5;  break;
            case 5:  mag = 2.0;  break;
            case 6:  mag = 4.0;  break;
            case 7:  mag = 8.0;  break;
            case 8:  mag = 16.0; break;
            case 9:      // page width
                  mag *= s.width() / cs->pageFormat()->width();
                  canvas->setOffset(0.0, 0.0);
                  break;
            case 10:     // page
                  {
                  double mag1 = s.width()  / cs->pageFormat()->width();
                  double mag2 = s.height() / cs->pageFormat()->height();
                  mag  *= (mag1 > mag2) ? mag2 : mag1;
                  canvas->setOffset(0.0, 0.0);
                  }
                  break;
            case 11:    // double page
                  {
                  double mag1 = s.width() / (cs->pageFormat()->width()*2+50.0);
                  double mag2 = s.height() / cs->pageFormat()->height();
                  mag  *= (mag1 > mag2) ? mag2 : mag1;
                  canvas->setOffset(0.0, 0.0);
                  }
                  break;
            case 12:    // original size
                  break;
            }
      canvas->setMag(mag);
      }

//---------------------------------------------------------
//   updateMag
//---------------------------------------------------------

void MuseScore::updateMag()
      {
      qreal val = canvas->mag();
      if (cs)
            cs->setMag(val);
      QString s;
      s.setNum(val * 100, 'f', 1);
      s += "%";
      mag->setEditText(s);
      }

//---------------------------------------------------------
//   padVisible
//---------------------------------------------------------

void MuseScore::padVisible(bool flag)
      {
      padId->setChecked(flag);
      }

//---------------------------------------------------------
//   navigatorVisible
//---------------------------------------------------------

void MuseScore::navigatorVisible(bool flag)
      {
      navigatorId->setChecked(flag);
      }

//---------------------------------------------------------
//   helpBrowser
//---------------------------------------------------------

void MuseScore::helpBrowser()
      {
      QString lang(getenv("LANG"));
      QFileInfo mscoreHelp(mscoreGlobalShare + QString("/doc/man-") + lang + QString(".pdf"));
      if (!mscoreHelp.isReadable()) {
            mscoreHelp.setFile(mscoreGlobalShare + QString("/doc/man-en.pdf"));
            if (!mscoreHelp.isReadable()) {
                  QString info(tr("MuseScore manual not found at: "));
                  info += mscoreHelp.filePath();
                  QMessageBox::critical(this, tr("MuseScore: Open Help"), info);
                  return;
                  }
            }
      QString url("file://" + mscoreHelp.filePath());
      QDesktopServices::openUrl(url);
      }

//---------------------------------------------------------
//   about
//---------------------------------------------------------

void MuseScore::about()
      {
      QMessageBox* mb = new QMessageBox();
      mb->setWindowTitle(QString("MuseScore"));
      mb->setText(tr("Linux Music Score Editor\n"
       "Version " VERSION "\n"
       "(C) Copyright 2002-2005 Werner Schweer and others\n"
       "see http://mscore.sourceforge.net/ for new versions\n"
       "and more information\n"
       "Published under the GNU Public Licence"));
      mb->exec();
      }

//---------------------------------------------------------
//   aboutQt
//---------------------------------------------------------

void MuseScore::aboutQt()
      {
      QMessageBox::aboutQt(this, QString("MuseScore"));
      }

//---------------------------------------------------------
//   openRecentMenu
//---------------------------------------------------------

void MuseScore::openRecentMenu()
      {
      genRecentPopup(openRecent);
      }

//---------------------------------------------------------
//   selectScore
//---------------------------------------------------------

void MuseScore::selectScore(QAction* action)
      {
      int id = action->data().toInt();
      if (id < 0)
            return;
      QString project = getScore(id).mid(2);
      if (project.isEmpty())
            return;
      loadScoreFile(project);
      }

void MuseScore::loadScoreFile(const QString&)
      {
      if (checkDirty())
            return;
printf("TODO      loadFile(s, false);\n");
      }

//---------------------------------------------------------
//   selectionChanged
//---------------------------------------------------------

void MuseScore::selectionChanged(int state)
      {
      cutId->setEnabled(state);
      copyId->setEnabled(state);
      pasteId->setEnabled(state);
      }

//---------------------------------------------------------
//   appendScore
//    append score to project list
//---------------------------------------------------------

void MuseScore::appendScore(Score* score)
      {
      scoreList.push_back(score);
      tab->addTab(score->projectName());

      if (scoreList.size() > 1)
            tab->show();
      else
            tab->hide();

      QString name(score->filePath());
      for (int i = 0; i < PROJECT_LIST_LEN; ++i) {
            if (projectList[i].name.isEmpty())
                  break;
            if (name == projectList[i].name) {
                  int dst = i;
                  int src = i+1;
                  int n   = PROJECT_LIST_LEN - i - 1;
                  for (int k = 0; k < n; ++k)
                        projectList[dst++] = projectList[src++];
                  projectList[dst].name = "";
                  break;
                  }
            }
      ProjectList* s = &projectList[PROJECT_LIST_LEN - 2];
      ProjectList* d = &projectList[PROJECT_LIST_LEN - 1];
      for (int i = 0; i < PROJECT_LIST_LEN-1; ++i)
            *d-- = *s--;
      projectList[0].name = name;
      }

//---------------------------------------------------------
//   usage
//---------------------------------------------------------

static void usage(const char* prog, const char*)
      {
      printVersion(prog);
      fprintf(stderr, "usage: %s flags scorefile\n   Flags:\n", prog);
      fprintf(stderr, "   -v        print version\n");
      fprintf(stderr, "   -d        debug mode\n");
      fprintf(stderr, "   -s        no internal synthesizer\n");
      fprintf(stderr, "   -m        no midi\n");
      fprintf(stderr, "   -L        layout debug\n");
      }

//---------------------------------------------------------
//   editTextStyle
//---------------------------------------------------------

void MuseScore::editTextStyle()
      {
      if (textStyleDialog == 0) {
            textStyleDialog = new TextStyleDialog(this);
            connect(textStyleDialog, SIGNAL(textStyleChanged()), SLOT(textStyleChanged()));
            }
      textStyleDialog->show();
      }

//---------------------------------------------------------
//   textStyleChanged
//---------------------------------------------------------

void MuseScore::textStyleChanged()
      {
      if (cs)
            cs->textStyleChanged();
      }

//---------------------------------------------------------
//   saveScoreList
//---------------------------------------------------------

void MuseScore::saveScoreList()
      {
      QFile f(QDir::homePath() + "/.mscorePrj");
      if (!f.open(QIODevice::WriteOnly))
            return;
      int n = scoreList.size();
      QTextStream out(&f);
      for (int i = PROJECT_LIST_LEN-1; i >= 0; --i) {
            if (projectList[i].name.isEmpty())
                  continue;
            bool loaded  = false;
            bool current = false;
            for (int k = 0; k < n; ++k) {
                  if (scoreList[k]->filePath() == projectList[i].name) {
                        loaded = true;
                        if (scoreList[k] == cs)
                              current = true;
                        break;
                        }
                  }
            if (current)
                  out << '+';
            else if (loaded)
                  out << '*';
            else
                  out << ' ';
            out << " " << projectList[i].name << '\n';
            }
      f.close();
      }

//---------------------------------------------------------
//   loadScoreList
//    read list of "Recent Scores"
//---------------------------------------------------------

void MuseScore::loadScoreList()
      {
      QFile f(QDir::homePath() + "/.mscorePrj");
      if (!f.open(QIODevice::ReadOnly))
            return;

      for (int i = 0; i < PROJECT_LIST_LEN; ++i) {
            QByteArray buffer = f.readLine(512);
            int n = buffer.size();
            if (n <= 0)
                  break;
            if (n && buffer[n-1] == '\n')
                  buffer[n-1] = 0;
            if (strlen(buffer) >= 3) {
                  projectList[i].name    = QString(buffer.data() + 2);
                  projectList[i].current = (buffer[0] == '+');
                  projectList[i].loaded  = (buffer[0] != ' ');
                  }
            else
                  break;
            }
      f.close();
      }

//---------------------------------------------------------
//   genRecentPopup
//---------------------------------------------------------

void MuseScore::genRecentPopup(QMenu* popup) const
      {
      popup->clear();
      for (int i = 0; i < PROJECT_LIST_LEN; ++i) {
            if (projectList[i].name.isEmpty())
                  break;
            const char* path = projectList[i].name.toLatin1().data();
            const char* p = strrchr(path, '/');
            if (p == 0)
                  p = path;
            else
                  ++p;
            QAction* action = popup->addAction(QString(p));
            action->setData(i);
            }
      }

//---------------------------------------------------------
//   getTopScore
//---------------------------------------------------------

QString MuseScore::getScore(int idx) const
      {
      if (idx >= PROJECT_LIST_LEN)
            idx = PROJECT_LIST_LEN - 1;
      return projectList[idx].name;
      }

//---------------------------------------------------------
//   setCurrentScore
//---------------------------------------------------------

void MuseScore::setCurrentScore(int idx)
      {
      if (tab->currentIndex() != idx) {
            tab->setCurrentIndex(idx);  // will call setCurrentScore() again
            return;
            }
      if (cs) {
            //
            // remember "global" values:
            //
            cs->setMag(canvas->mag());
            cs->setXoffset(canvas->xoffset());
            cs->setYoffset(canvas->yoffset());
//            cs->setSpatium(_spatium);
            }

      cs = scoreList[idx];
      cs->clearViewer();
      cs->addViewer(canvas);

      undoAction->setEnabled(!cs->undoEmpty());
      redoAction->setEnabled(!cs->redoEmpty());

      _spatium = cs->spatium();
      canvas->setMag(cs->mag());
      canvas->setXoffset(cs->xoffset());
      canvas->setYoffset(cs->yoffset());

      setWindowTitle("MuseScore: " + cs->projectName());
      canvas->setScore(cs);
      seq->setScore(cs);
      if (playPanel)
            playPanel->setScore(cs);

      cs->setUpdateAll();
      cs->endCmd(false);

      connect(cs, SIGNAL(selectionChanged(int)), SLOT(selectionChanged(int)));
//      initSymbols();
      }

//---------------------------------------------------------
//   signalHandler
//---------------------------------------------------------

#if 1
static void signalHandler(int)
      {
      printf("fp exception\n");
      abort();
      }
#endif

//---------------------------------------------------------
//   doRedo
//---------------------------------------------------------

void MuseScore::doRedo()
      {
      if (cs)
            cs->doRedo();
      }

//---------------------------------------------------------
//   doUndo
//---------------------------------------------------------

void MuseScore::doUndo()
      {
      if (cs)
            cs->doUndo();
      }

//---------------------------------------------------------
//   printFile
//---------------------------------------------------------

void MuseScore::printFile()
      {
      if (cs)
            cs->printFile();
      }

//---------------------------------------------------------
//   cmdAddPitch
//---------------------------------------------------------

void MuseScore::cmdAddPitch(QAction* action)
      {
      if (cs)
            cs->cmdAddPitch(action->data().toInt());
      }

//---------------------------------------------------------
//   midiReceived
//---------------------------------------------------------

void MuseScore::midiReceived()
      {
      if (cs)
            cs->midiReceived();
      }

//---------------------------------------------------------
//   cmdTuplet
//---------------------------------------------------------

void MuseScore::cmdTuplet(QAction* action)
      {
      if (cs)
            cs->cmdTuplet(action->data().toInt());
      }

//---------------------------------------------------------
//   cmdAddIntervall
//---------------------------------------------------------

void MuseScore::cmdAddIntervall(QAction* action)
      {
      if (cs)
            cs->cmdAddIntervall(action->data().toInt());
      }

//---------------------------------------------------------
//   cmdAddTitle
//---------------------------------------------------------

void MuseScore::cmdAddTitle()
      {
      if (cs)
            cs->cmdAddTitle();
      }

//---------------------------------------------------------
//   cmdAddSubTitle
//---------------------------------------------------------

void MuseScore::cmdAddSubTitle()
      {
      if (cs)
            cs->cmdAddSubTitle();
      }

//---------------------------------------------------------
//   cmdAddComposer
//---------------------------------------------------------

void MuseScore::cmdAddComposer()
      {
      if (cs)
            cs->cmdAddComposer();
      }

//---------------------------------------------------------
//   cmdAddPoet
//---------------------------------------------------------

void MuseScore::cmdAddPoet()
      {
      if (cs)
            cs->cmdAddPoet();
      }

//---------------------------------------------------------
//   addLyrics
//---------------------------------------------------------

void MuseScore::addLyrics()
      {
      if (cs)
            cs->addLyrics();
      }

//---------------------------------------------------------
//   addExpression
//---------------------------------------------------------

void MuseScore::addExpression()
      {
      if (cs)
            cs->addExpression();
      }

//---------------------------------------------------------
//   addTechnik
//---------------------------------------------------------

void MuseScore::addTechnik()
      {
      if (cs)
            cs->addTechnik();
      }

//---------------------------------------------------------
//   addTempo
//---------------------------------------------------------

void MuseScore::addTempo()
      {
      if (cs)
            cs->addTempo();
      }

//---------------------------------------------------------
//   addMetronome
//---------------------------------------------------------

void MuseScore::addMetronome()
      {
      if (cs)
            cs->addMetronome();
      }

//---------------------------------------------------------
//   startNoteEntry
//---------------------------------------------------------

void MuseScore::startNoteEntry()
      {
      if (cs)
            cs->startNoteEntry();
      }

//---------------------------------------------------------
//   resetUserStretch
//---------------------------------------------------------

void MuseScore::resetUserStretch()
      {
      if (cs)
            cs->resetUserStretch();
      }

#if 0
//---------------------------------------------------------
//   systemBreak
//---------------------------------------------------------

void MuseScore::systemBreak()
      {
      if (cs)
            cs->systemBreak();
      }

//---------------------------------------------------------
//   pageBreak
//---------------------------------------------------------

void MuseScore::pageBreak()
      {
      if (cs)
            cs->pageBreak();
      }
#endif

//---------------------------------------------------------
//   resetUserOffsets
//---------------------------------------------------------

void MuseScore::resetUserOffsets()
      {
      if (cs)
            cs->resetUserOffsets();
      }

//---------------------------------------------------------
//   midiNoteReceived
//---------------------------------------------------------

void MuseScore::midiNoteReceived(int pitch, bool chord)
      {
      if (cs)
            cs->midiNoteReceived(pitch, chord);
      }

//---------------------------------------------------------
//   showInvisibleClicked
//---------------------------------------------------------

void MuseScore::showInvisibleClicked()
      {
      if (cs)
            cs->setShowInvisible(visibleId->isChecked());
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
      cs->pages()->update();
      canvas->setMag(cs->mag());
      cs->doLayout();
      cs->setUpdateAll();
      cs->textStyleChanged();    // fix text styles (center, right etc.)
      canvas->updateNavigator(true);
      cs->endCmd(false);
      }

//---------------------------------------------------------
//   startPageListEditor
//---------------------------------------------------------

void MuseScore::startPageListEditor()
      {
      if (pageListEdit == 0) {
            pageListEdit = new PageListEditor(cs);
            }
      pageListEdit->updateList();
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
      if (editStyleWin == 0) {
            editStyleWin = new EditStyle(0);
            }
      editStyleWin->setValues(cs);
      editStyleWin->show();
      }

//---------------------------------------------------------
//   importMusicXml
//---------------------------------------------------------

void MuseScore::importMusicXml()
      {
      QString name = QFileDialog::getOpenFileName(
         this,
         tr("MuseScore: Import MusicXML"),
         ".",
         QString("MusicXml files (*.xml *.xml.gz *.xml.bz2);; All files (*)")
         );
      if (name.isEmpty())
            return;
      Score* score = new Score();
      score->read(name);
      appendScore(score);
      tab->setCurrentIndex(scoreList.size() - 1);
      }

//---------------------------------------------------------
//   showPlayPanel
//---------------------------------------------------------

void MuseScore::showPlayPanel(bool visible)
      {
      if (noSeq)
            return;

      if (playPanel == 0) {
            playPanel = new PlayPanel();
            connect(playPanel, SIGNAL(volChange(float)),    seq, SLOT(setVolume(float)));
            connect(playPanel, SIGNAL(relTempoChanged(int)),seq, SLOT(setRelTempo(int)));
            connect(playPanel, SIGNAL(posChange(int)),      seq, SLOT(setPos(int)));
            connect(playPanel, SIGNAL(rewindTriggered()),   seq, SLOT(rewindStart()));
            connect(playPanel, SIGNAL(close()),                  SLOT(closePlayPanel()));
            connect(playPanel, SIGNAL(stopToggled(bool)),        SLOT(setStop(bool)));
            connect(playPanel, SIGNAL(playToggled(bool)),        SLOT(setPlay(bool)));

            bool playing = seq->isPlaying();
            playPanel->setStop(!playing);
            playPanel->setPlay(playing);
            playPanel->setVolume(seq->volume());
            playPanel->setTempo(cs->tempomap->tempo(0));
            playPanel->setRelTempo(cs->tempomap->relTempo());
            playPanel->enableSeek(!seq->isPlaying());
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
//   showPad
//---------------------------------------------------------

void MuseScore::showPad(bool visible)
      {
      if (pad == 0) {
            pad = new Pad(0);
            connect(pad, SIGNAL(keyEvent(QKeyEvent*)), canvas, SLOT(keyPressEvent(QKeyEvent*)));
            connect(pad, SIGNAL(close()), SLOT(closePad()));
            cs->setPadState();
            }
      pad->setShown(visible);
      padId->setChecked(visible);
      }

//---------------------------------------------------------
//   keyPadToggled
//---------------------------------------------------------

void MuseScore::keyPadToggled(int key)
      {
      int padno = -1;
      if (key >= PAD_N0 && key <= PAD_N4)
            padno = key - PAD_N0;
      if (key == PAD_PLUS) {
            padno = padState.pad + 1;
            if (padno == 5)
                  padno = 0;
            }
      if (padno >= 0) {
            setPadNo(padno);
            padState.pad = padno;
            cs->setPadState();
            return;
            }
      cs->padToggle(padTrans[padState.pad][key].cmd);
      }

//---------------------------------------------------------
//   closePad
//---------------------------------------------------------

void MuseScore::closePad()
      {
      padId->setChecked(false);
      }

//---------------------------------------------------------
//   setPadNo
//---------------------------------------------------------

void MuseScore::setPadNo(int padno)
      {
      if (pad)
            pad->setPadNo(padno);
      }

//---------------------------------------------------------
//   cmdAppendMeasure
//---------------------------------------------------------

void MuseScore::cmdAppendMeasure()
      {
      if (cs)
            cs->cmdAppendMeasure();
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
//   cmdAppendMeasures
//---------------------------------------------------------

void MuseScore::cmdAppendMeasures(int n)
      {
      if (cs) {
	      printf("append %d measures\n", n);
            cs->cmdAppendMeasures(n);
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
	mscore->cmdAppendMeasures(n);
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
//   speakerToggled
//---------------------------------------------------------

void MuseScore::speakerToggled(bool val)
      {
      _speakerEnabled = val;
      }

//---------------------------------------------------------
//   midiinEnabled
//---------------------------------------------------------

bool MuseScore::midiinEnabled() const
      {
      return preferences.enableMidiInput && _midiinEnabled;
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

void MuseScore::removeTab(int i)
      {
      int n = scoreList.size();
      if (n <= 1)
            return;
      std::vector<Score*>::iterator ii = scoreList.begin() + i;
      scoreList.erase(ii);
      tab->removeTab(i);
      cs = 0;
      if (i >= (n-1))
            i = 0;
      setCurrentScore(i);
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
//      feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
      signal(SIGFPE, signalHandler);

      padState.pitch = 60;
      setDefaultStyle();
      QApplication app(argc, argv);

#ifndef MINGW32
      appDpiX = QX11Info::appDpiX();
      appDpiY = QX11Info::appDpiY();
#endif

      int c;
      while ((c = getopt(argc, argv, "vdLsm")) != EOF) {
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
                  default:
                        usage(argv[0], "bad argument");
                        return -1;
                  }
            }
      argc -= optind;
      ++argc;

      haveMidi = !initMidi();
      preferences.read();

      QApplication::setFont(QFont(QString("helvetica"), 11, QFont::Normal));

      if (debugMode) {
            if (haveMidi)
                  printf("midi devices found\n");
            }
      mscoreGlobalShare = getSharePath();
      if (debugMode) {
            printf("global share: <%s>\n", mscoreGlobalShare.toLocal8Bit().data());
            }

      static QTranslator translator;
      QFile ft(":mscore.qm");
      if (ft.exists()) {
            if (debugMode)
                  printf("locale file found\n");
            if (translator.load(":/mscore.qm")) {
                  if (debugMode)
                        printf("locale file loaded\n");
                  }
            qApp->installTranslator(&translator);
            }
      else {
            if (debugMode) {
                  printf("locale file not found for locale <%s>\n",
                     QLocale::system().name().toLatin1().data());
                  }
            }

      int fontId = QFontDatabase::addApplicationFont(":/fonts/emmentaler_20.otf");
      if (fontId == -1) {
            fprintf(stderr, "Mscore: fatal error: cannot load internal font\n");
            exit(-1);
            }

      seq = new Seq();
      if (!noSeq) {
            if (seq->init()) {
                  printf("sequencer init failed\n");
                  noSeq = true;
                  }
            }
      //
      // avoid font problems by overriding the environment
      //    fall back to "C" locale
      //
      setenv("LANG", "mops", 1);
      QLocale::setDefault(QLocale(QLocale::C));

      //-----------------------------------------
      //  sanity check
      //  check for score font
      //-----------------------------------------

#if 1
      QFont f;
      f.setFamily("Emmentaler");
      f.setPixelSize(20);
      QFontInfo fi(f);

      if (!fi.exactMatch()) {
            //
            // sometimes i do not get an exact match, but the
            // Emmentaler font is found in the font database.
            // I cannot find out why the font did not match. This
            // happens for example with current cvs version of fontconfig.
            //
            // produce some debugging output:
            //
            printf("Emmentaler not found (<%s><%d>)\n", fi.family().toLatin1().data(), fi.style());
            QFontDatabase fdb;
            QStringList families = fdb.families();
            foreach (QString family, families) {
                  if (family == "Emmentaler") {
                        printf("  found <%s>\n", family.toLatin1().data());
                        QStringList styles = fdb.styles(family);
                        foreach (QString style, styles) {
                              printf("    Style <%s>\n", style.toLatin1().data());

                              int w = fdb.weight(family, style);
                              printf("      weight %d\n", w);

                              bool b = fdb.isSmoothlyScalable(family, style);
                              printf("      smooth scalable %d\n", b);

                              b = fdb.isBitmapScalable(family, style);
                              printf("      bitmap scalable %d\n", b);

                              b = fdb.isScalable(family, style);
                              printf("      scalable %d\n", b);

                              b = fdb.isFixedPitch(family, style);
                              printf("      fixedPitch %d\n", b);

                              b = fdb.bold(family, style);
                              printf("      bold %d\n", b);

                              QList<int> sizes = fdb.smoothSizes(family, style);
                              printf("      sizes: ");
                              foreach (int s, sizes)
                                    printf("%d ", s);
                              printf("\n");
                              }
                        }
                  }
            }
#endif

      //-------------------------------
      //  load scores
      //-------------------------------

      initSymbols();
      genIcons();
      mscore = new MuseScore();

      int currentScore = 0;
      int idx = 0;
      bool scoreCreated = false;
      if (argc < 2) {
            for (int i = 0; i < PROJECT_LIST_LEN; ++i) {
                  if (projectList[i].name.isEmpty())
                        break;
                  if (projectList[i].loaded) {
                        Score* score = new Score();
                        scoreCreated = true;
                        score->read(projectList[i].name);
                        if (projectList[i].current)
                              currentScore = idx;
                        mscore->appendScore(score);
                        ++idx;
                        }
                  }
            }
      else {
            while (argc > 1) {
                  QString name = argv[optind++];
                  --argc;
                  if (!name.isEmpty()) {
                        Score* score = new Score();
                        scoreCreated = true;
                        score->read(name);
                        mscore->appendScore(score);
                        }
                  }
            }

      if (!scoreCreated)
            // start with empty score:
            mscore->appendScore(new Score());

      mscore->resize(scoreSize);
      mscore->move(scorePos);
      mscore->setCurrentScore(currentScore);
      mscore->showNavigator(preferences.showNavigator);
      mscore->showPad(preferences.showPad);
      if (mscore->getKeyPad())
            mscore->getKeyPad()->move(preferences.padPos);
      mscore->showPlayPanel(preferences.showPlayPanel);
      if (mscore->getPlayPanel())
            mscore->getPlayPanel()->move(preferences.playPanelPos);

      int rfd = getMidiReadFd();
      if (rfd != -1) {
            QSocketNotifier* sn = new QSocketNotifier(rfd,
               QSocketNotifier::Read,  mscore);
            sn->connect(sn, SIGNAL(activated(int)), mscore, SLOT(midiReceived()));
            }

      mscore->show();
      return qApp->exec();
      }


