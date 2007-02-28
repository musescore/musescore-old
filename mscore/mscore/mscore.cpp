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
#include "sym.h"

#include "padids.h"
#include "pad.h"
#include "alsa.h"
#include "pagesettings.h"
#include "listedit.h"
#include "editstyle.h"
#include "playpanel.h"
#include "page.h"
#include "partedit.h"

QTextStream cout(stdout);
QTextStream eout(stderr);

PadState padState;
QString mscoreGlobalShare;
QString language;

static unsigned int startMag = 3;   // 100%, synchronize with canvas default

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
double DPI, DPMM;

QMap<QString, Shortcut*> shortcuts;

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
      // fprintf(stderr, "%s: Linux Music Score Editor; Version %s\n", prog, VERSION);
      cout << prog << ": Linux Music Score Editor; Version " << VERSION << endl;
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void MuseScore::closeEvent(QCloseEvent* ev)
      {
      for (QList<Score*>::iterator i = scoreList.begin(); i != scoreList.end(); ++i) {
            Score* score = *i;
            if (score->dirty()) {
                  QString s(tr("%1 contains unsaved data\n"
                    "Save Current Score?"));
                  s = s.arg(score->projectName());
                  int n = QMessageBox::warning(this, tr("MuseScore"),
                     s,
                     tr("&Save"), tr("&Nosave"), tr("&Abort"), 0, 2);
                  if (n == 0)
                        saveFile();
                  else if (n == 2) {
                        ev->ignore();
                        return;
                        }
                  }
            }
      saveScoreList();
      seq->exit();
      ev->accept();
      //
      // close all toplevel windows
      //
      if (pageListEdit)
            pageListEdit->close();
      if (pad)
            pad->close();
      if (playPanel)
            playPanel->close();
      if (symbolPalette1)
            symbolPalette1->close();
      if (clefPalette)
            clefPalette->close();
      if (keyPalette)
            keyPalette->close();
      if (timePalette)
            timePalette->close();
      if (linePalette)
            linePalette->close();
      if (bracketPalette)
            bracketPalette->close();
      if (barPalette)
            barPalette->close();
      if (fingeringPalette)
            fingeringPalette->close();
      if (noteAttributesPalette)
            noteAttributesPalette->close();
      if (accidentalsPalette)
            accidentalsPalette->close();
      if (dynamicsPalette)
            dynamicsPalette->close();
      if (layoutBreakPalette)
            layoutBreakPalette->close();
      if (preferenceDialog)
            preferenceDialog->close();
      if (iledit)
            iledit->close();
      if (editStyleWin)
            editStyleWin->close();
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
      getAction("midi-on")->setEnabled(preferences.enableMidiInput);
      _statusBar->setShown(preferences.showStatusBar);
      }

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

MuseScore::MuseScore()
   : QMainWindow()
      {
      mscore = this;
      setIconSize(QSize(ICON_HEIGHT, ICON_HEIGHT));
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

      _modeText = new QLabel;
      _modeText->setAutoFillBackground(true);
      QPalette p(_modeText->palette());
      p.setColor(QPalette::Window, QColor(176, 190, 242));
      _modeText->setPalette(p);
      _statusBar = new QStatusBar;
      _statusBar->addPermanentWidget(_modeText, 0);
      setStatusBar(_statusBar);
      setState(STATE_NORMAL);
      _statusBar->setShown(preferences.showStatusBar);

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
         << "redo" << "append-measure" << "duole" << "triole" << "pentole"
         << "note-c" << "note-d" << "note-e" << "note-f" << "note-g"
         << "note-a" << "note-b"
         << "chord-c" << "chord-d" << "chord-e" << "chord-f" << "chord-g"
         << "chord-a" << "chord-b"
         << "stretch+" << "stretch-"
         << "instruments" << "clefs" << "keys" << "symbols" << "times" << "dynamics"
         << "lyrics" << "fingering" << "expression" << "technik" << "tempo"
         << "metronome" << "cut" << "copy" << "paste"
         << "beam-start" << "beam-mid" << "no-beam" << "beam32"
         << "file-open" << "file-new" << "file-save" << "file-save-as" << "file-close"
         << "export-midi" << "export-xml" << "import-midi" << "import-xml" << "quit"
         << "toggle-statusbar"
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
      QHBoxLayout* hbox = new QHBoxLayout;
      hbox->addWidget(tab);
      hbox->addStretch(100);
      canvas = new Canvas;
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

      transportAction = new QActionGroup(this);
      transportAction->setExclusive(true);

      a = getAction("rewind");
      transportAction->addAction(a);
      connect(a, SIGNAL(triggered()), seq, SLOT(rewindStart()));

      a = getAction("stop");
      a->setCheckable(true);
      a->setChecked(true);
      transportAction->addAction(a);
      connect(a, SIGNAL(toggled(bool)), this, SLOT(setStop(bool)));

      a = getAction("play");
      a->setCheckable(true);
      transportAction->addAction(a);
      connect(a, SIGNAL(toggled(bool)), this, SLOT(setPlay(bool)));

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
      transportTools->addAction(getAction("stop"));
      transportTools->addAction(getAction("play"));

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
         << "pad-note-16" << "pad-note-32" << "pad-note-64" << "pad-dot"
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
//            pal.setColor(QPalette::BrightText, preferences.selectColor[i].light(140));
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

      QMenu* menuFile = mb->addMenu(tr("&File"));

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
      menuFile->addAction(getAction("export-midi"));
      menuFile->addAction(getAction("export-xml"));
      menuFile->addAction(getAction("import-midi"));
      menuFile->addAction(getAction("import-xml"));

      menuFile->addSeparator();
      menuFile->addAction(getAction("print"));
      menuFile->addSeparator();
      menuFile->addAction(getAction("quit"));

      //---------------------
      //    Menu Edit
      //---------------------

      menuEdit = mb->addMenu(tr("&Edit"));
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
      menuEdit->addAction(tr("Part List..."), this, SLOT(startInstrumentListEditor()));
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

      a = getAction("note-input");
      connect(a, SIGNAL(triggered()), SLOT(startNoteEntry()));
      menuNotes->addAction(a);
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

      QMenu* menuAddIntervall = new QMenu(tr("Add Intervall"));
      for (int i = 1; i < 10; ++i) {
            char buffer[16];
            sprintf(buffer, "intervall%d", i);
            a = getAction(buffer);
            ag->addAction(a);
            menuAddIntervall->addAction(a);
            }
      menuAddIntervall->addSeparator();
      for (int i = 2; i < 10; ++i) {
            char buffer[16];
            sprintf(buffer, "intervall-%d", i);
            a = getAction(buffer);
            ag->addAction(a);
            menuAddIntervall->addAction(a);
            }
      menuNotes->addMenu(menuAddIntervall);

      QMenu* menuNtole = new QMenu(tr("Tuples"));
      menuNtole->addAction(getAction("duole"));
      menuNtole->addAction(getAction("triole"));
      menuNtole->addAction(getAction("pentole"));
      menuNotes->addMenu(menuNtole);

      //---------------------
      //    Menu Layout
      //---------------------

      QMenu* menuLayout = mb->addMenu(tr("&Layout"));

      menuLayout->addAction(tr("Page Settings..."), this, SLOT(showPageSettings()));
      menuLayout->addAction(tr("Reset Positions"),  this, SLOT(resetUserOffsets()));
      menuLayout->addAction(tr("Set Normal Staff Distances"),  canvas, SLOT(resetStaffOffsets()));
      menuLayout->addAction(getAction("stretch+"));
      menuLayout->addAction(getAction("stretch-"));

      menuLayout->addAction(tr("Reset Stretch"), this, SLOT(resetUserStretch()));
      menuLayout->addAction(tr("Breaks..."), this, SLOT(showLayoutBreakPalette()));

      //---------------------
      //    Menu Style
      //---------------------

      QMenu* menuStyle = mb->addMenu(tr("&Style"));
      menuStyle->addAction(tr("Edit Style..."), this, SLOT(editStyle()));
      menuStyle->addAction(tr("Edit Text Style..."), this, SLOT(editTextStyle()));
      menuStyle->addSeparator();
      menuStyle->addAction(fileOpenIcon, tr("Load Style"), this, SLOT(loadStyle()));
      menuStyle->addAction(fileSaveIcon, tr("Save Style"), this, SLOT(saveStyle()));

      //---------------------
      //    Menu Display
      //---------------------

      menuDisplay = mb->addMenu(tr("&Display"));

      padId = getAction("toggle-pad");
      padId->setCheckable(true);
      connect(padId, SIGNAL(toggled(bool)), SLOT(showPad(bool)));
      menuDisplay->addAction(padId);

      playId = getAction("toggle-playpanel");
      playId->setCheckable(true);
      connect(playId, SIGNAL(toggled(bool)), SLOT(showPlayPanel(bool)));
      menuDisplay->addAction(playId);

      navigatorId = getAction("toggle-navigator");
      navigatorId->setCheckable(true);
      connect(navigatorId, SIGNAL(toggled(bool)), SLOT(showNavigator(bool)));
      menuDisplay->addAction(navigatorId);

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

      loadInstrumentTemplates();
      preferencesChanged();
      connect(seq, SIGNAL(started()), SLOT(seqStarted()));
      connect(seq, SIGNAL(stopped()), SLOT(seqStopped()));
      loadScoreList();

      QClipboard* cb = QApplication::clipboard();
      connect(cb, SIGNAL(dataChanged()), SLOT(clipboardChanged()));
      connect(cb, SIGNAL(selectionChanged()), SLOT(clipboardChanged()));
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
            addItem(tr(magTable[i]), i);
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
      double cw = canvas->fsize().width();
      double ch = canvas->fsize().height();

      PageFormat* pf = mscore->currentScore()->pageFormat();
      double nmag = canvas->mag();
      int mn = sizeof(magTable)/sizeof(*magTable);
      bool found = false;
      for (int i = 0; i < mn; ++i) {
            if (magTable[i] != s)
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
      QPixmap pm(":/data/splash.jpg");
      QSplashScreen* sc = new QSplashScreen(pm);
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
      QString s = getScore(id).mid(2);
      if (s.isEmpty())
            return;
      Score* score = new Score();
      score->read(s);
      appendScore(score);
      tab->setCurrentIndex(scoreList.size() - 1);
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
      getAction("file-close")->setEnabled(scoreList.size() > 1);
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
      if (textStyleDialog == 0)
            textStyleDialog = new TextStyleDialog(this);
      textStyleDialog->show();
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
            }

      cs = scoreList[idx];
      cs->clearViewer();
      cs->addViewer(canvas);

      getAction("undo")->setEnabled(!cs->undoEmpty());
      getAction("redo")->setEnabled(!cs->redoEmpty());
      visibleId->setChecked(cs->showInvisible());

      cs->setSpatium(cs->mainLayout()->spatium());
      setMag(cs->mag());
      canvas->setXoffset(cs->xoffset());
      canvas->setYoffset(cs->yoffset());

      setWindowTitle("MuseScore: " + cs->projectName());
      canvas->setScore(cs, cs->mainLayout());
      seq->setScore(cs);
      if (playPanel)
            playPanel->setScore(cs);

      cs->endCmd(false);
      cs->layout();

      connect(cs, SIGNAL(selectionChanged(int)), SLOT(selectionChanged(int)));
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
//   midiReceived
//---------------------------------------------------------

void MuseScore::midiReceived()
      {
      if (cs)
            cs->midiReceived();
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
//TODO      cs->pages()->update();
      setMag(cs->mag());
      cs->layout();
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
            connect(playPanel, SIGNAL(closed()),                 SLOT(closePlayPanel()));
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
            connect(pad, SIGNAL(closed()), SLOT(closePad()));
            cs->setPadState();
            }
      pad->setShown(visible);
      padId->setChecked(visible);
      }

//---------------------------------------------------------
//   closePad
//---------------------------------------------------------

void MuseScore::closePad()
      {
      padId->setChecked(false);
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
      QList<Score*>::iterator ii = scoreList.begin() + i;
      if (checkDirty(*ii))
            return;
      scoreList.erase(ii);
      tab->removeTab(i);
      cs = 0;
      if (i >= (n-1))
            i = 0;
      setCurrentScore(i);
      getAction("file-close")->setEnabled(scoreList.size() > 1);
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

#ifdef __MINGW32__
      appDpiX = 75;
      appDpiY = 75;
#else
      appDpiX = QX11Info::appDpiX();
      appDpiY = QX11Info::appDpiY();
#endif
      DPI  = appDpiX;     // drawing resolution
      DPMM = DPI / INCH;  // dots/mm

      _spatium = 20.0 / 72.0 * DPI / 4.0;

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

      //
      // initialize shortcut hash table
      //
      for (unsigned i = 0;; ++i) {
            if (MuseScore::sc[i].xml == 0)
                  break;
            shortcuts[MuseScore::sc[i].xml] = new Shortcut(MuseScore::sc[i]);
            }

      haveMidi = !initMidi();
      preferences.read();

      QSplashScreen* sc = 0;
      if (preferences.showSplashScreen) {
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

      //
      //  load internal fonts
      //
      int fontId = QFontDatabase::addApplicationFont(":/fonts/mscore_20.otf");
      if (fontId == -1) {
            fprintf(stderr, "Mscore: fatal error: cannot load internal font\n");
            exit(-1);
            }
      fontId = QFontDatabase::addApplicationFont(":/fonts/mscore1_20.otf");
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
#ifndef __MINGW32__
      setenv("LANG", "mops", 1);
#endif
      QLocale::setDefault(QLocale(QLocale::C));

      //-----------------------------------------
      //  sanity check
      //  check for score font
      //-----------------------------------------

#if 1
      QFont f;
      f.setFamily("MScore");
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
            printf("no exact match for font Mscore found (<%s><%d>)\n", fi.family().toLatin1().data(), fi.style());
            QFontDatabase fdb;
            QStringList families = fdb.families();
            foreach (QString family, families) {
                  if (family == "MScore") {
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
      new MuseScore();

      int currentScore = 0;
      int idx = 0;
      bool scoreCreated = false;
      if (argc < 2) {
            switch (preferences.sessionStart) {
                  case LAST_SESSION:
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
      mscore->getCanvas()->setFocus(Qt::OtherFocusReason);
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

      if (cmd == "instruments")
            editInstrList();
      else if (cmd == "clefs")
            clefMenu();
      else if (cmd == "keys")
            keyMenu();
      else if (cmd == "symbols")
            symbolMenu1();
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
      else if (cmd == "export-midi")
            exportMidi();
      else if (cmd == "export-xml")
            exportMusicXml();
      else if (cmd == "import-midi")
            importMidi();
      else if (cmd == "import-xml")
            importMusicXml();
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
      bool flag = ms->hasFormat("application/mscore/symbol")
            ||    ms->hasFormat("application/mscore/staff")
            ||    ms->hasFormat("application/mscore/system")
            ||    ms->hasFormat("application/mscore/symbols");
      // TODO: depends on selection state
      getAction("paste")->setEnabled(flag);
      }

//---------------------------------------------------------
//   setState
//---------------------------------------------------------

void MuseScore::setState(int val)
      {
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
            }
      }
