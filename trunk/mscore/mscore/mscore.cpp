//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: mscore.cpp,v 1.105 2006/09/15 09:34:57 wschweer Exp $
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

// #include <signal.h>
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

QPaintDevice* pdev;
double PDPI, DPI, DPMM;
double SPATIUM;

QTextStream cout(stdout);
QTextStream eout(stderr);

QString mscoreGlobalShare;

static unsigned int startMag = 3;   // 100%, synchronize with canvas default

const char* eventRecordFile;

struct ProjectItem {
      QString name;
      bool current;
      bool loaded;
      };

QList<ProjectItem*> projectList;

QMap<QString, Shortcut*> shortcuts;
bool converterMode = false;

static const char* outFileName;
static QString localeName;

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
      return QCoreApplication::applicationDirPath() + QString("/../" INSTALL_NAME);
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

      cout << prog << ": Linux Music Score Editor; Version " << VERSION
           << "  Build " << revision << endl;
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void MuseScore::closeEvent(QCloseEvent* ev)
      {
      for (QList<Score*>::iterator i = scoreList.begin(); i != scoreList.end(); ++i) {
            Score* score = *i;
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
      transportId->setChecked(seq->isRunning());
      transportTools->setShown(seq->isRunning());
      getAction("midi-on")->setEnabled(preferences.enableMidiInput);
      _statusBar->setShown(preferences.showStatusBar);
      }

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

MuseScore::MuseScore()
   : QMainWindow()
      {
      setIconSize(QSize(ICON_HEIGHT, ICON_HEIGHT));
      setWindowTitle(QString("MuseScore"));
      cs                    = 0;
      editStyleWin          = 0;
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
      lastOpenPath          = preferences.workingDirectory;

      _modeText = new QLabel;
      _modeText->setAutoFillBackground(true);
      QPalette p(_modeText->palette());
      p.setColor(QPalette::Window, QColor(176, 190, 242));
      _modeText->setPalette(p);
      _statusBar = new QStatusBar;
      _statusBar->addPermanentWidget(_modeText, 0);
      setStatusBar(_statusBar);

      QAction* a;

      // otherwise unused actions:
      //   must be added somewere to work

      QActionGroup* ag = new QActionGroup(this);
      ag->setExclusive(false);
      QStringList sl;
      sl << "page-prev" << "page-next" << "page-top" << "page-end"
         << "add-tie" << "add-slur" << "add-hairpin" << "add-hairpin-reverse"
         << "escape" << "delete" << "rest" << "pitch-up" << "pitch-down"
         << "pitch-up-octave" << "pitch-down-octave"
         << "move-up" << "move-down" << "up-chord" << "down-chord"
         << "top-chord" << "bottom-chord" << "next-chord" << "prev-chord"
         << "next-measure" << "prev-measure" << "print" << "undo"
         << "redo" << "append-measure" << "append-measures" << "insert-measure" << "insert-measures"
         << "insert-hbox" << "insert-vbox" << "append-hbox" << "append-vbox"
         << "duplet" << "triplet" << "quintuplet"
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
         << "file-open" << "file-new" << "file-save" << "file-save-as" << "file-close"
         << "quit"
         << "toggle-statusbar" << "note-input" << "pitch-spell"
         << "rewind" << "play" << "pause" <<"repeat"
         << "play-next-measure" << "play-next-chord" << "play-prev-measure" << "play-prev-chord"
         << "seek-begin" << "seek-end"
         << "load-style" << "save-style" << "select-all" << "transpose"
         << "reset-beammode"
         ;

      foreach(const QString s, sl) {
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
      tab = new TabBar;
      tab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      QHBoxLayout* hbox = new QHBoxLayout;
      hbox->addWidget(tab);
      hbox->addStretch(100);

      removeTabButton = new QToolButton;
      removeTabButton->setIcon(QIcon(QPixmap(":/data/tab_remove.png")));
      hbox->addWidget(removeTabButton);
      connect(removeTabButton, SIGNAL(clicked()), SLOT(removeTab()));

      canvas = new Canvas;
      canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      layout->addLayout(hbox);
      layout->addWidget(canvas);

      connect(tab, SIGNAL(currentChanged(int)), SLOT(setCurrentScore(int)));
      connect(tab, SIGNAL(doubleClick(int)), SLOT(removeTab(int)));

      QAction* whatsThis = QWhatsThis::createAction(this);

      //---------------------------------------------------
      //    Transport Action
      //---------------------------------------------------

      a  = getAction("midi-on");
      a->setCheckable(true);
      a->setEnabled(preferences.enableMidiInput);
      a->setChecked(_midiinEnabled);
      connect(a, SIGNAL(triggered(bool)), SLOT(midiinToggled(bool)));

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
      transportTools->addAction(getAction("sound-on"));
      transportTools->addAction(getAction("midi-on"));
      transportTools->addSeparator();
      transportTools->addAction(getAction("rewind"));
      transportTools->addAction(getAction("pause"));
      transportTools->addAction(getAction("play"));
      transportTools->addSeparator();
      a = getAction("repeat");
      a->setChecked(preferences.playRepeats);
      transportTools->addAction(a);

      a = getAction("mag");
      fileTools->addAction(a);
      connect(a, SIGNAL(triggered()), canvas, SLOT(magCanvas()));

      mag = new MagBox;
      connect(mag, SIGNAL(magChanged(double)), SLOT(magChanged(double)));
      fileTools->addWidget(mag);
      addToolBarBreak();

      //-------------------------------
      //    Note Entry Tool Bar
      //-------------------------------

      entryTools = addToolBar(tr("Note Entry"));
      entryTools->setIconSize(QSize(ICON_WIDTH, ICON_HEIGHT));

      QStringList sl1;
      sl1 << "pad-note-1" << "pad-note-2" << "pad-note-4" << "pad-note-8"
         << "pad-note-16" << "pad-note-32" << "pad-note-64" << "pad-dot" << "pad-dotdot"
         << "pad-tie" << "pad-rest" << "pad-sharp2" << "pad-sharp"
         << "pad-nat" << "pad-flat" << "pad-flat2";

      foreach(const QString s, sl1) {
            QAction* a = getAction(s.toLatin1().data());
            a->setCheckable(true);
            ag->addAction(a);
            entryTools->addAction(a);
            if (s == "pad-tie" || s == "pad-rest")
                  entryTools->addSeparator();
            }

      sl1.clear();
      sl1 << "pad-appoggiatura" << "pad-acciaccatura";
      sl1 << "beam-start" << "beam-mid" << "no-beam" << "beam32" << "auto-beam";

      foreach(const QString s, sl1) {
            QAction* a = getAction(s.toLatin1().data());
            a->setCheckable(true);
            ag->addAction(a);
            }

      a = getAction("flip");
      ag->addAction(a);
      entryTools->addAction(a);
      entryTools->addSeparator();

      QStringList sl2;
      sl2 << "voice-1" << "voice-2" << "voice-3" << "voice-4";
      QActionGroup* vag = new QActionGroup(this);
      vag->setExclusive(true);
      int i = 0;
      foreach(const QString s, sl2) {
            QAction* a = getAction(s.toLatin1().data());
            a->setCheckable(true);
            vag->addAction(a);
            QToolButton* tb = new QToolButton(this);
            tb->setDefaultAction(a);
            tb->setToolButtonStyle(Qt::ToolButtonIconOnly);
            tb->setAutoFillBackground(true);
            QPalette pal = tb->palette();
            QColor c(preferences.selectColor[i].light(180));
            pal.setColor(QPalette::Window, preferences.selectColor[i].light(180));
            pal.setColor(QPalette::Button, preferences.selectColor[i].light(150));
            tb->setPalette(pal);
            entryTools->addWidget(tb);
            ++i;
            }
      connect(vag, SIGNAL(triggered(QAction*)), SLOT(cmd(QAction*)));

      //---------------------
      //    Menus
      //---------------------

      QMenuBar* mb = menuBar();

      //---------------------
      //    Menu File
      //---------------------

      QMenu* menuFile = mb->addMenu(tr("&Score"));
      menuFile->setObjectName("Score");

      menuFile->addAction(getAction("file-new"));
      menuFile->addAction(getAction("file-open"));
      openRecent = menuFile->addMenu(fileOpenIcon, tr("Open &Recent"));
      connect(openRecent, SIGNAL(aboutToShow()), SLOT(openRecentMenu()));
      connect(openRecent, SIGNAL(triggered(QAction*)), SLOT(selectScore(QAction*)));
      menuFile->addSeparator();
      menuFile->addAction(getAction("file-save"));
      menuFile->addAction(getAction("file-save-as"));
      menuFile->addAction(getAction("file-close"));

      menuFile->addSeparator();
      menuFile->addAction(getAction("print"));
      menuFile->addSeparator();
      menuFile->addAction(getAction("quit"));

      //---------------------
      //    Menu Edit
      //---------------------

      QMenu* menuEdit = mb->addMenu(tr("&Edit"));
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
      menuEdit->addAction(tr("Excerpts..."), this, SLOT(startExcerptsDialog()));
      menuEdit->addSeparator();
      menuEdit->addAction(tr("Inspector..."), this, SLOT(startPageListEditor()));
      menuEdit->addSeparator();
      menuEdit->addAction(tr("Preferences..."), this, SLOT(startPreferenceDialog()));

      //---------------------
      //    Menu Create
      //---------------------

      QMenu* menuCreate = genCreateMenu(mb);
      mb->setObjectName("Create");
      mb->addMenu(menuCreate);

      //---------------------
      //    Menu Notes
      //---------------------

      QMenu* menuNotes = mb->addMenu(tr("Notes"));
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
      menuNtole->addAction(getAction("quintuplet"));
      menuNotes->addMenu(menuNtole);

      menuNotes->addSeparator();
      menuNotes->addAction(getAction("transpose"));

      //---------------------
      //    Menu Layout
      //---------------------

      QMenu* menuLayout = mb->addMenu(tr("&Layout"));
      menuLayout->setObjectName("Layout");

      menuLayout->addAction(tr("Page Settings..."), this, SLOT(showPageSettings()));
      menuLayout->addAction(tr("Reset Positions"),  this, SLOT(resetUserOffsets()));
      menuLayout->addAction(tr("Set Normal Staff Distances"),  canvas, SLOT(resetStaffOffsets()));
      menuLayout->addAction(getAction("stretch+"));
      menuLayout->addAction(getAction("stretch-"));

      menuLayout->addAction(tr("Reset Stretch"), this, SLOT(resetUserStretch()));
      menuLayout->addAction(getAction("reset-beammode"));
      menuLayout->addAction(tr("Breaks..."), this, SLOT(showLayoutBreakPalette()));

      //---------------------
      //    Menu Style
      //---------------------

      QMenu* menuStyle = mb->addMenu(tr("&Style"));
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

      menuHelp->addAction(tr("Manual"),  this, SLOT(helpBrowser()), Qt::Key_F1);
      menuHelp->addAction(tr("&About"),   this, SLOT(about()));
      menuHelp->addAction(tr("About&Qt"), this, SLOT(aboutQt()));
      menuHelp->addSeparator();
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
      setState(STATE_NORMAL);
      }

//---------------------------------------------------------
//   magTable
//---------------------------------------------------------

static const char* magTable[] = {
     "25%", "50%", "75%", "100%", "150%", "200%", "400%", "800%", "1600%",
      QT_TRANSLATE_NOOP("magTable","PgeWidth"),
      QT_TRANSLATE_NOOP("magTable","Page"),
      QT_TRANSLATE_NOOP("magTable","DblPage"),
     };

//---------------------------------------------------------
//   MagBox
//---------------------------------------------------------

MagBox::MagBox(QWidget* parent)
   : QComboBox(parent)
      {
      setEditable(true);
      setInsertPolicy(QComboBox::InsertAtBottom);
      setToolTip(tr("Mag"));
      setWhatsThis(tr("Zoom Canvas"));
      setValidator(new MagValidator(this));

      for (unsigned int i =  0; i < sizeof(magTable)/sizeof(*magTable); ++i) {
            QString ts(QCoreApplication::translate("magTable", magTable[i]));
            addItem(ts, i);
            if (i == startMag)
                  setCurrentIndex(i);
            }
      connect(this, SIGNAL(currentIndexChanged(int)), SLOT(indexChanged(int)));
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void MagBox::setMag(double val)
      {
      for (int i = 0; i < count(); ++i) {
            if (txt2mag(itemText(i)) == val) {
                  setCurrentIndex(i);
                  return;
                  }
            }
      blockSignals(true);
      setCurrentIndex(count()-1);
      setItemText(count()-1, QString("%1%").arg(val * 100));
      blockSignals(false);
      }

//---------------------------------------------------------
//   indexChanged
//---------------------------------------------------------

void MagBox::indexChanged(int idx)
      {
      int mn = sizeof(magTable)/sizeof(*magTable);
      double mag = txt2mag(itemText(idx));
      if (idx > mn) {
            setItemText(idx-1, QString("%1%").arg(mag*100.0));
            blockSignals(true);
            setCurrentIndex(idx-1);
            removeItem(idx);
            blockSignals(false);
            --idx;
            }
      else if (idx == mn)
            setItemText(idx, QString("%1%").arg(mag*100.0));
      emit magChanged(mag);
      }

//---------------------------------------------------------
//   txt2mag
//---------------------------------------------------------

double MagBox::txt2mag(const QString& s)
      {
      Canvas* canvas = mscore->getCanvas();
      double cw      = canvas->fsize().width();
      double ch      = canvas->fsize().height();

      PageFormat* pf = mscore->currentScore()->pageFormat();
      double nmag    = canvas->mag();
      int mn         = sizeof(magTable)/sizeof(*magTable);
      bool found     = false;
      for (int i = 0; i < mn; ++i) {
            QString ts(QCoreApplication::translate("magTable", magTable[i]));
            if (ts != s)
                  continue;
            switch(i) {
                  case 9:      // page width
                        nmag *= cw / (pf->width() * DPI);
                        canvas->setOffset(0.0, 0.0);
                        found = true;
                        break;
                  case 10:     // page
                        {
                        double mag1 = cw  / (pf->width() * DPI);
                        double mag2 = ch / (pf->height() * DPI);
                        nmag  *= (mag1 > mag2) ? mag2 : mag1;
                        canvas->setOffset(0.0, 0.0);
                        found = true;
                        }
                        break;
                  case 11:    // double page
                        {
                        double mag1 = cw / (pf->width()*2*DPI+50.0);
                        double mag2 = ch / (pf->height() * DPI);
                        nmag  *= (mag1 > mag2) ? mag2 : mag1;
                        canvas->setOffset(0.0, 0.0);
                        found = true;
                        }
                        break;
                  }
            if (found)
                  break;
            }

      if (!found) {
            QString d;
            for (int i = 0; i < s.size(); ++i) {
                  QChar c = s[i];
                  if (c.isDigit() || c == '.')
                        d.append(c);
                  }
            bool ok;
            double nnmag = d.toDouble(&ok);
            if (ok)
                  nmag = nnmag / 100;
            else
                  printf("bad mag entry <%s>\n", s.toLatin1().data());
            }
      return nmag;
      }

//---------------------------------------------------------
//   MagValidator
//---------------------------------------------------------

MagValidator::MagValidator(QObject* parent)
   : QValidator(parent)
      {
      }

//---------------------------------------------------------
//   validate
//---------------------------------------------------------

QValidator::State MagValidator::validate(QString& input, int& /*pos*/) const
      {
      QComboBox* cb = (QComboBox*)parent();
      int mn = sizeof(magTable)/sizeof(*magTable);
      for (int i = 0; i < mn; ++i) {
            if (input == cb->itemText(i))
                  return QValidator::Acceptable;
            }
      QString d;
      for (int i = 0; i < input.size(); ++i) {
            QChar c = input[i];
            if (c.isDigit() || c == '.')
                  d.append(c);
            else if (c != '%')
                  return QValidator::Invalid;
            }
      if (d.isEmpty())
            return QValidator::Intermediate;
      bool ok;
      double nmag = d.toDouble(&ok);
      if (!ok)
            return QValidator::Invalid;
      if (nmag < 25.0 || nmag > 1600.0)
            return QValidator::Intermediate;
      return QValidator::Acceptable;
      }

//---------------------------------------------------------
//   magChanged
//---------------------------------------------------------

void MuseScore::magChanged(double mag)
      {
      canvas->setMag(mag);
      }

//---------------------------------------------------------
//   MuseScore
//---------------------------------------------------------

void MuseScore::setMag(double val)
      {
      canvas->setMag(val);
      mag->setMag(val);
      }

//---------------------------------------------------------
//   incMag
//---------------------------------------------------------

void MuseScore::incMag()
      {
      qreal _mag = canvas->mag() * 1.7;
      if (_mag > 16.0)
            _mag = 16.0;
      setMag(_mag);
      }

//---------------------------------------------------------
//   decMag
//---------------------------------------------------------

void MuseScore::decMag()
      {
      qreal nmag = canvas->mag() / 1.7;
      if (nmag < 0.05)
            nmag = 0.05;
      setMag(nmag);
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
      QString lang(localeName.left(2));
      QFileInfo mscoreHelp(mscoreGlobalShare + QString("/man/") + lang + QString("/index.html"));
      if (!mscoreHelp.isReadable()) {
            mscoreHelp.setFile(mscoreGlobalShare + QString("/man/man/en/index.html"));
            if (!mscoreHelp.isReadable()) {
                  QString info(tr("MuseScore online manual not found at: "));
                  info += mscoreHelp.filePath();
                  QMessageBox::critical(this, tr("MuseScore: Open Help"), info);
                  return;
                  }
            }
      QString url("file://" + mscoreHelp.filePath());
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
      ProjectItem* item = (ProjectItem*)action->data().value<void*>();
      if (item) {
            Score* score = new Score();
            score->read(item->name);
            appendScore(score);
            tab->setCurrentIndex(scoreList.size() - 1);
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
      scoreList.push_back(score);
      tab->addTab(score->name());

      bool showTabBar = scoreList.size() > 1;
      tab->setVisible(showTabBar);
      removeTabButton->setVisible(showTabBar);

      QString name(score->filePath());

      for (int i = 0; i < projectList.size(); ++i) {
            if (projectList[i]->name == name) {
                  delete projectList[i];
                  projectList.removeAt(i);
                  break;
                  }
            }
      if (projectList.size() >= PROJECT_LIST_LEN) {
            ProjectItem* item = projectList.takeLast();
            delete item;
            }
      ProjectItem* item = new ProjectItem;
      item->name = name;
      projectList.prepend(item);
      getAction("file-close")->setEnabled(scoreList.size() > 1);
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
        "   -s        no internal synthesizer\n"
        "   -m        no midi\n"
        "   -L        layout debug\n"
        "   -I        dump midi input\n"
        "   -O        dump midi output\n"
        "   -o file   export to 'file'; format depends on file extension\n");
      }

//---------------------------------------------------------
//   editTextStyle
//---------------------------------------------------------

void MuseScore::editTextStyle()
      {
      TextStyleDialog dialog(this, cs);
      dialog.exec();
      }

//---------------------------------------------------------
//   saveScoreList
//---------------------------------------------------------

void MuseScore::saveScoreList()
      {
      QSettings settings;

      int n  = scoreList.size();
      int pn = projectList.size();
      for (int i = pn - 1; i >= 0; --i) {
            bool loaded  = false;
            bool current = false;
            for (int k = 0; k < n; ++k) {
                  if (scoreList[k]->filePath() == projectList[i]->name) {
                        loaded = true;
                        if (scoreList[k] == cs)
                              current = true;
                        break;
                        }
                  }
            QString s;
            if (current)
                  s += "+";
            else if (loaded)
                  s += "*";
            else
                  s += " ";
            s += projectList[i]->name;
            settings.setValue(QString("recent-%1").arg(i), s);
            }
      }

//---------------------------------------------------------
//   loadScoreList
//    read list of "Recent Scores"
//---------------------------------------------------------

void MuseScore::loadScoreList()
      {
      QSettings s;
      for (int i = 0; i < PROJECT_LIST_LEN; ++i) {
            QString buffer = s.value(QString("recent-%1").arg(i)).toString();
            int n = buffer.size();
            if (n >= 2) {
                  ProjectItem* item = new ProjectItem;
                  item->name    = buffer.mid(1);
                  item->current = (buffer[0] == '+');
                  item->loaded  = (buffer[0] != ' ');
                  projectList.append(item);
                  }
            }
      }

//---------------------------------------------------------
//   openRecentMenu
//---------------------------------------------------------

void MuseScore::openRecentMenu()
      {
      openRecent->clear();
      for (int i = 0; i < projectList.size(); ++i) {
            const ProjectItem* item = projectList[i];
            QFileInfo fi(item->name);
            QAction* action = openRecent->addAction(fi.baseName());
            action->setData(QVariant::fromValue<void*>((void*)item));
            }
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
            cs->clearViewer();
            }

      cs = scoreList[idx];
      cs->clearViewer();
      cs->addViewer(canvas);

      getAction("undo")->setEnabled(!cs->undoEmpty());
      getAction("redo")->setEnabled(!cs->redoEmpty());
      getAction("file-save")->setEnabled(cs->isSavable());

      visibleId->setChecked(cs->showInvisible());

      cs->setSpatium(cs->layout()->spatium());
      setMag(cs->mag());
      canvas->setOffset(cs->xoffset(), cs->yoffset());

      setWindowTitle("MuseScore: " + cs->name());
      canvas->setScore(cs, cs->layout());
      seq->setScore(cs);
      if (playPanel)
            playPanel->setScore(cs);

      cs->start();
      cs->setLayoutAll(true);
      cs->end();

      connect(cs, SIGNAL(selectionChanged(int)), SLOT(selectionChanged(int)));
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
//   resetUserStretch
//---------------------------------------------------------

void MuseScore::resetUserStretch()
      {
      if (cs)
            cs->resetUserStretch();
      }

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
      cs->start();
      canvas->updateNavigator(true);
      cs->setLayoutAll(true);
      cs->end();
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
      editStyleWin->setScore(cs);
      editStyleWin->show();
      }

//---------------------------------------------------------
//   showPlayPanel
//---------------------------------------------------------

void MuseScore::showPlayPanel(bool visible)
      {
      if (noSeq)
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
      if (n <= 1)
            return;
      QList<Score*>::iterator ii = scoreList.begin() + i;
      if (checkDirty(*ii))
            return;
      scoreList.erase(ii);
      tab->removeTab(i);
      cs = 0;
      if (i >= (n-1))
            i = 0;
      setCurrentScore(i);
      bool showTabBar = scoreList.size() > 1;
      getAction("file-close")->setEnabled(showTabBar);
      tab->setVisible(showTabBar);
      removeTabButton->setVisible(showTabBar);
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
      // feclearexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
      // feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);

      QApplication app(argc, argv);
      QCoreApplication::setOrganizationName("MusE");
      QCoreApplication::setOrganizationDomain("muse.org");
      QCoreApplication::setApplicationName("MuseScore");
      qApp->setStyleSheet(appStyleSheet);
      qApp->setWindowIcon(windowIcon);

      setDefaultStyle();

      int c;
      while ((c = getopt(argc, argv, "vdLsmiIOo:")) != EOF) {
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
                  default:
                        usage(argv[0], "bad argument");
                        return -1;
                  }
            }
      argc -= optind;
      ++argc;

      pdev = new QPrinter(QPrinter::HighResolution);
      QWidget wi(0);

      PDPI = wi.logicalDpiX();         // physical resolution
      DPI  = pdev->logicalDpiX();      // logical drawing resolution
      DPMM = DPI / INCH;  // dots/mm

      // HACK:
      // without setting the application font to a pixel size,
      // menu entries are garbled
#if 1
      QFont f = qApp->font();
      double size = f.pointSizeF();
      if (size > 0.0) {
            int px = lrint(size * PDPI / PPI);
            f.setPixelSize(px);
            qApp->setFont(f);
            }
#endif

      // rastral size of font is 20pt = 20/72 inch = 20*DPI/72 dots
      //   staff has 5 lines = 4 * _spatium
      _spatium = SPATIUM20  * DPI;     // 20.0 / 72.0 * DPI / 4.0;

      initSymbols();

      mscoreGlobalShare = getSharePath();
      if (debugMode) {
            printf("global share: <%s>\n", mscoreGlobalShare.toLocal8Bit().data());
            }

      //
      // set translator before preferences are read to get
      //    translations for all shortcuts
      //
      localeName = QLocale::system().name();
      QTranslator translator;
      QString lp = mscoreGlobalShare + "locale/" + QString("mscore_") + localeName;
      if (debugMode)
            cout << "load translator <" << lp << ">\n";

      translator.load(lp);
      app.installTranslator(&translator);
      //
      // initialize shortcut hash table
      //
      for (unsigned i = 0;; ++i) {
            if (MuseScore::sc[i].xml == 0)
                  break;
            shortcuts[MuseScore::sc[i].xml] = new Shortcut(MuseScore::sc[i]);
            }
      preferences.read();
      QSplashScreen* sc = 0;
      if (!converterMode && preferences.showSplashScreen) {
            QPixmap pm(":/data/splash.jpg");
            sc = new QSplashScreen(pm);
            sc->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
            QSize s(pm.size());
            QBitmap bm(s);
            bm.clear();
            QPainter p;
            p.begin(&bm);
            p.setBrush(Qt::SolidPattern);
            p.drawRoundRect(QRect(QPoint(0, 0), s), s.height()/6, s.width()/6);
            p.end();
            sc->setMask(bm);
            sc->show();
            app.processEvents();
            }

      //
      //  load internal fonts
      //
      int fontId = QFontDatabase::addApplicationFont(":/fonts/mscore_20.otf");
      if (fontId == -1) {
            fprintf(stderr, "Mscore: fatal error: cannot load internal font\n");
            if (!debugMode)
                  exit(-1);
            }
      fontId = QFontDatabase::addApplicationFont(":/fonts/mscore1_20.otf");
      if (fontId == -1) {
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

      //-------------------------------
      //  load scores
      //-------------------------------

      initSymbols();    // again!?!
      genIcons();
      initDrumset();

      mscore = new MuseScore();
      gscore = new Score;
      mscore->readSettings();

      int currentScore = 0;
      int idx = 0;
      bool scoreCreated = false;
      if (argc < 2) {
            switch (preferences.sessionStart) {
                  case LAST_SESSION:
                        foreach(const ProjectItem* item, projectList) {
                              if (item->loaded) {
                                    Score* score = new Score();
                                    scoreCreated = true;
                                    score->read(item->name);
                                    if (item->current)
                                          currentScore = idx;
                                    mscore->appendScore(score);
                                    ++idx;
                                    }
                              }
                        break;
                  case NEW_SESSION:
                        break;
                  case SCORE_SESSION:
                        Score* score = new Score();
                        scoreCreated = true;
                        score->read(preferences.startScore);
                        mscore->appendScore(score);
                        break;
                  }
            }
      else {
            while (argc > 1) {
                  QString name = QString::fromLocal8Bit(argv[optind++]);
                  --argc;
                  if (!name.isEmpty()) {
                        Score* score = new Score();
                        scoreCreated = true;
                        score->read(name);
                        mscore->appendScore(score);
                        }
                  }
            }

      if (!scoreCreated) {
            // start with empty score:
            Score* score = new Score();
            score->fileInfo()->setFile(mscore->createDefaultName());
            score->setCreated(true);
            mscore->appendScore(score);
            }

      mscore->setCurrentScore(currentScore);
      mscore->showNavigator(preferences.showNavigator);

      mscore->showPlayPanel(preferences.showPlayPanel);
      if (mscore->getPlayPanel())
            mscore->getPlayPanel()->move(preferences.playPanelPos);

      mscore->getCanvas()->setFocus(Qt::OtherFocusReason);

      if (converterMode) {
            QString fn(outFileName);
            Score* cs = mscore->currentScore();
            cs->layout();

            bool rv;
            if (fn.endsWith(".msc")) {
                  QFileInfo fi(fn);
                  rv = mscore->saveFile(fi);
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
      return qApp->exec();
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void MuseScore::cmd(QAction* a)
      {
      QString cmd(a->data().toString());
      Shortcut* sc = getShortcut(cmd.toAscii().data());
      if ((sc->state & _state) == 0) {
            printf("cmd <%s> not valid in state %d\n", qPrintable(cmd), _state);
            return;
            }
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
      else if (cmd == "file-close")
            removeTab(scoreList.indexOf(cs));
      else if (cmd == "file-save-as")
            saveAs();
      else if (cmd == "file-new")
            newFile();
      else if (cmd == "quit")
            close();
      else if (cmd == "fingering")
            fingeringMenu();
      else if (cmd == "toggle-statusbar") {
            QAction* a = getAction("toggle-statusbar");
            preferences.showStatusBar = a->isChecked();
            _statusBar->setShown(preferences.showStatusBar);
            preferences.write();
            }
      else if (cmd == "append-measures")
            cmdAppendMeasures();
      else if (cmd == "insert-measures")
            cmdInsertMeasures();
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
//   setState
//---------------------------------------------------------

void MuseScore::setState(int val)
      {
      foreach (Shortcut* s, shortcuts) {
            if (s->action) {
                  if (s->state & val)
                        s->action->setShortcut(s->key);
                  else
                        s->action->setShortcut(0);
                  }
            }

      switch(val) {
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
      _state = val;
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
      descr   = qApp->translate("MuseScore", d);
      help    = qApp->translate("MuseScore", h);
      text    = qApp->translate("MuseScore", txt);
      }

Shortcut::Shortcut(const Shortcut& c)
      {
      state   = c.state;
      xml     = c.xml;
      key     = c.key;
      context = c.context;
      icon    = c.icon;
      action  = c.action;
      descr   = qApp->translate("MuseScore", c.descr.toLatin1().data());
      help    = qApp->translate("MuseScore", c.help.toLatin1().data());
      text    = qApp->translate("MuseScore", c.text.toLatin1().data());
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
      settings.endGroup();
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void MuseScore::readSettings()
      {
      QSettings settings;
      settings.beginGroup("MainWindow");
      resize(settings.value("size", QSize(950, 500)).toSize());
      move(settings.value("pos", QPoint(10, 10)).toPoint());
      mscore->showPalette(settings.value("showPanel", "0").toBool());
      settings.endGroup();
      }

//---------------------------------------------------------
//   play
//    play note for 300 msec
//---------------------------------------------------------

void MuseScore::play(Element* e) const
      {
      if (mscore->playEnabled() && e->type() == NOTE) {
            Note* note = (Note*) e;
            seq->startNote(note->staff()->part(), note->pitch(), 80, 300);
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
      versionLabel->setText("Version: " VERSION);
      revisionLabel->setText(QString("Revision: %1").arg(revision));
      }

