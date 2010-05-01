//=============================================================================
//  MusE Score
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

#ifndef __MSCORE_H__
#define __MSCORE_H__

#include "globals.h"
#include "ui_measuresdialog.h"
#include "ui_insertmeasuresdialog.h"
#include "ui_aboutbox.h"
#include "singleapp/src/qtsingleapplication.h"
#include "updatechecker.h"

class ScoreView;
class Element;
class ToolButton;
class PreferenceDialog;
class InstrumentsDialog;
class Instrument;
class MidiFile;
class TextStyleDialog;
class PlayPanel;
class InstrumentListEditor;
class Inspector;
class MeasureListEditor;
class Score;
class PageSettings;
class PaletteBox;
class Palette;
class PaletteScrollArea;
class TimeDialog;
class Xml;
class MagBox;
class NewWizard;
class ExcerptsDialog;
class SynthControl;
class PianorollEditor;
class Staff;
class ScoreView;
class ScoreTab;
class QScriptEngineDebugger;
class Drumset;
class TextTools;
class ScriptEngine;
class KeyEditor;
class ChordStyleEditor;
class UndoGroup;
class Navigator;
class LanguageItem;

extern QString mscoreGlobalShare;
static const int PROJECT_LIST_LEN = 6;
extern bool playRepeats;

//---------------------------------------------------------
//   LanguageItem
//---------------------------------------------------------

struct LanguageItem {
      QString key;
      QString name;
      QString handbook;
      LanguageItem(const QString k, const QString n) {
            key = k;
            name = n;
            handbook = QString::null;
            }
      LanguageItem(const QString k, const QString n, const QString h) {
            key = k;
            name = n;
            handbook = h;
            }
      };

extern QList<LanguageItem> languages;

//---------------------------------------------------------
//   AboutBoxDialog
//---------------------------------------------------------

class AboutBoxDialog : public QDialog, Ui::AboutBox {
      Q_OBJECT

   public:
      AboutBoxDialog();
      };

//---------------------------------------------------------
//   InsertMeasuresDialog
//   Added by DK, 05.08.07
//---------------------------------------------------------

class InsertMeasuresDialog : public QDialog, public Ui::InsertMeasuresDialogBase {
      Q_OBJECT

   private slots:
      virtual void accept();

   public:
      InsertMeasuresDialog(QWidget* parent = 0);
      };

//---------------------------------------------------------
//   MeasuresDialog
//---------------------------------------------------------

class MeasuresDialog : public QDialog, public Ui::MeasuresDialogBase {
      Q_OBJECT

   private slots:
      virtual void accept();

   public:
      MeasuresDialog(QWidget* parent = 0);
      };

//---------------------------------------------------------
//   NoteButton
//---------------------------------------------------------

class NoteButton : public QToolButton {
      Q_OBJECT
      virtual QSize sizeHint() const;

   public:
      NoteButton(QWidget* parent = 0);
      };

//---------------------------------------------------------
//   Shortcut
//    hold the basic values for configurable shortcuts
//---------------------------------------------------------

class Shortcut {
   public:
      int state;              //! shortcut is valid in this Mscore state
      const char* xml;        //! xml tag name for configuration file
      QString descr;          //! descriptor, shown in editor
      QKeySequence key;       //! shortcut
      QKeySequence::StandardKey standardKey;
      Qt::ShortcutContext context;
      QString text;           //! text as shown on buttons or menus
      QString help;           //! ballon help
      int icon;
      QAction* action;        //! cached action
      bool translated;

      Shortcut();
      Shortcut(int state, const char* name, const char* d, QKeySequence::StandardKey sk = QKeySequence::UnknownKey,
         Qt::ShortcutContext cont = Qt::ApplicationShortcut,
         const char* txt = 0, const char* h = 0, int i = -1);
      Shortcut(int state, const char* name, const char* d, const QKeySequence& k = QKeySequence(),
         Qt::ShortcutContext cont = Qt::ApplicationShortcut,
         const char* txt = 0, const char* h = 0, int i = -1);
      Shortcut(const Shortcut& c);
      };


//---------------------------------------------------------
//   MuseScoreApplication (mac only)
//---------------------------------------------------------

class MuseScoreApplication : public QtSingleApplication {
   public:  
      QStringList paths;
      MuseScoreApplication(const QString &id, int &argc, char **argv) : QtSingleApplication(id, argc, argv){};
      bool event(QEvent *ev);
      };

//---------------------------------------------------------
//   MuseScore
//---------------------------------------------------------

class MuseScore : public QMainWindow {
      Q_OBJECT

      ScoreState _sstate;
      UndoGroup* _undoGroup;
      UpdateChecker* ucheck;
      QList<Score*> scoreList;
      Score* cs;              // current score
      ScoreView* cv;             // current viewer

      QVBoxLayout* layout;    // main window layout
      QSplitter* splitter;
      ScoreTab* tab1;
      ScoreTab* tab2;
      Navigator* navigator;
      QSplitter* mainWindow;

      QMenu* menuDisplay;
      QMenu* openRecent;

      MagBox* mag;
      QAction* playId;

      QProgressBar* _progressBar;
      PreferenceDialog* preferenceDialog;
      QToolBar* cpitchTools;
      QToolBar* fileTools;
      QToolBar* transportTools;
      QToolBar* entryTools;
      TextTools* _textTools;
      QToolBar* voiceTools;
      InstrumentsDialog* instrList;
      MeasuresDialog* measuresDialog;
      InsertMeasuresDialog* insertMeasuresDialog;
      QMenu* _fileMenu;
      QMenu* menuEdit;
      QMenu* menuCreate;
      QMenu* menuNotes;
      QMenu* menuLayout;
      QMenu* menuStyle;

      QWidget* searchDialog;
      QComboBox* searchCombo;

      PlayPanel* playPanel;
      InstrumentListEditor* iledit;
      SynthControl* synthControl;
      Inspector* inspector;
      MeasureListEditor* measureListEdit;
      PageSettings* pageSettings;

      QWidget* symbolDialog;

      PaletteScrollArea* clefPalette;
      PaletteScrollArea* keyPalette;
      KeyEditor* keyEditor;
      ChordStyleEditor* chordStyleEditor;
      TimeDialog* timePalette;
      PaletteScrollArea* linePalette;
      PaletteScrollArea* bracketPalette;
      PaletteScrollArea* barPalette;
      PaletteScrollArea* fingeringPalette;
      PaletteScrollArea* noteAttributesPalette;
      PaletteScrollArea* accidentalsPalette;
      PaletteScrollArea* dynamicsPalette;
      PaletteScrollArea* layoutBreakPalette;
      QStatusBar* _statusBar;
      QLabel* _modeText;
      QLabel* _positionLabel;
      NewWizard* newWizard;

      PaletteBox* paletteBox;
      Palette* drumPalette;
      Drumset* drumset;                   // drumset associated with drumPalette

      bool _midiinEnabled;
      bool _speakerEnabled;
      QString lastOpenPath;
      QList<QString> plugins;
      ScriptEngine* se;
      QString pluginPath;

      QScriptEngineDebugger* debugger;

      QTimer* autoSaveTimer;
      QSignalMapper* pluginMapper;

      PianorollEditor* pianorollEditor;
      bool _splitScreen;
      bool _horizontalSplit;

      QString rev;
      //---------------------

      virtual void closeEvent(QCloseEvent*);

      virtual void dragEnterEvent(QDragEnterEvent*);
      virtual void dropEvent(QDropEvent*);

      void playVisible(bool flag);
      void launchBrowser(const QString whereTo);

      void loadScoreList();
      void editInstrList();
      void symbolMenu();
      void clefMenu();
      void showKeyEditor();
      void timeMenu();
      void dynamicsMenu();
      void loadFile();
      void saveFile();
      void fingeringMenu();
      void registerPlugin(const QString& pluginPath);
      void pluginExecuteFunction(int idx, const char* functionName);
      void startInspector();
      void midiinToggled(bool);
      void speakerToggled(bool);
      void undo();
      void redo();
      void showPalette(bool);
      void showPlayPanel(bool);
      void showNavigator(bool);
      void showMixer(bool);
      void showSynthControl(bool);
      void helpBrowser();
      void splitWindow(bool horizontal);
      void removeSessionFile();
      void editChordStyle();
      void startExcerptsDialog();

   private slots:
      void autoSaveTimerTimeout();
      void helpBrowser1();
      void about();
      void aboutQt();
      void openRecentMenu();
      void selectScore(QAction*);
      void selectionChanged(int);
      void startPreferenceDialog();
      void preferencesChanged();
      void seqStarted();
      void seqStopped();
      void closePlayPanel();
      void lineMenu();
      void bracketMenu();
      void barMenu();
      void noteAttributesMenu();
      void accidentalsMenu();
      void cmdAppendMeasures();
      void cmdInsertMeasures();
      void showLayoutBreakPalette();
      void magChanged(int);
      void showPageSettings();
      void removeTab(int);
      void removeTab();
      void cmd(QAction*);
      void clipboardChanged();
      void drumPaletteSelected(int);
      void endSearch();
      void closeSynthControl();

   public slots:
      void dirtyChanged(Score*);
      void setPos(int tick);
      void searchTextChanged(const QString& s);
      void pluginTriggered(int);
      void handleMessage(const QString& message);
      void setCurrentScoreView(ScoreView*);
      void setCurrentScoreView(int);
      void setNormalState()    { changeState(STATE_NORMAL); }
      void setEditState()      { changeState(STATE_EDIT); }
      void setNoteEntryState() { changeState(STATE_NOTE_ENTRY); }
      void setPlayState()      { changeState(STATE_PLAY); }
      void setSearchState()    { changeState(STATE_SEARCH); }
      void checkForUpdate();
      void registerPlugin(QAction*);
      QMenu* fileMenu() const  { return _fileMenu; }

   public:
      MuseScore();
      bool checkDirty(Score*);
      PlayPanel* getPlayPanel() const { return playPanel; }
      QMenu* genCreateMenu(QWidget* parent = 0);
      int appendScore(Score*);
      void midiNoteReceived(int pitch, bool chord);
      void showElementContext(Element* el);
	    void cmdAppendMeasures(int);
      bool midiinEnabled() const;
      bool playEnabled() const;
      Score* currentScore() const { return cs; }
      static Shortcut sc[];
      static Shortcut scSeq[];
      void incMag();
      void decMag();
      void readSettings();
      void writeSettings();
      void play(Element* e) const;
      void play(Element* e, int pitch) const;
      bool loadPlugin(const QString& filename);
      QString createDefaultName() const;
      void startAutoSave();
      void updateDrumset();
      double getMag(ScoreView*) const;
      void setMag(double);
      bool noScore() const { return scoreList.isEmpty(); }
      void setDrumPalette(Palette* p) { drumPalette = p; }
      TextTools* textTools();
      void updateTabNames();
      QProgressBar* showProgressBar();
      void hideProgressBar();
      void updateRecentScores(Score*);
      QFileDialog* saveAsDialog();
      QFileDialog* saveCopyDialog();

      QString lastSaveCopyDirectory;
      QString lastSaveDirectory;
      SynthControl* getSynthControl() const { return synthControl; }
      void editInPianoroll(Staff* staff);
      PianorollEditor* getPianorollEditor() const { return pianorollEditor; }
      void writeSessionFile(bool);
      bool restoreSession(bool);
      ScoreView* currentScoreView() const { return cv; }
      bool splitScreen() const { return _splitScreen; }
      void setCurrentView(int tabIdx, int idx);
      void loadPlugins();
      void unloadPlugins();
      ScoreState state() const { return _sstate; }
      void changeState(ScoreState);
      bool readLanguages(const QString& path);
      void setRevision(QString& r){rev = r;}
      QString revision(){return rev;}
      void newFile();
      bool hasToCheckForUpdate();
      static bool unstable();
      bool eventFilter(QObject *, QEvent *);
      };

extern QMenu* genCreateMenu(QWidget* parent);
extern MuseScore* mscore;
extern QString dataPath;

extern Shortcut* getShortcut(const char* id);
extern QAction* getAction(Shortcut*);
extern QAction* getAction(const char*);
extern QMap<QString, Shortcut*> shortcuts;
extern Shortcut* midiActionMap[128];
extern void setMscoreLocale(QString localeName);

#endif

