//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: mscore.h,v 1.54 2006/04/12 14:58:10 wschweer Exp $
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

#ifndef __MSCORE_H__
#define __MSCORE_H__

#include "globals.h"
#include "ui_measuresdialog.h"
#include "ui_insertmeasuresdialog.h"
#include "ui_aboutbox.h"

class Canvas;
class Element;
class ToolButton;
class PreferenceDialog;
class EditStyle;
class InstrumentsDialog;
class Instrument;
class MidiFile;
class TextStyleDialog;
class PlayPanel;
class InstrumentListEditor;
class PageListEditor;
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
class QScriptEmbeddedDebugger;
struct Drumset;
class TextTools;

extern QString mscoreGlobalShare;
static const int PROJECT_LIST_LEN = 6;
extern bool playRepeats;

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
      Qt::ShortcutContext context;
      QString text;           //! text as shown on buttons or menus
      QString help;           //! ballon help
      QIcon* icon;
      QAction* action;        //! cached action
      bool translated;

      Shortcut();
      Shortcut(int state, const char* name, const char* d, const QKeySequence& k = QKeySequence(),
         Qt::ShortcutContext cont = Qt::ApplicationShortcut,
         const char* txt = 0, const char* h = 0, QIcon* i = 0);
      Shortcut(const Shortcut& c);
      };

//---------------------------------------------------------
//   Command
//---------------------------------------------------------

struct Command {
      QAction* a;
      int data;
      };

//---------------------------------------------------------
//   MuseScore
//---------------------------------------------------------

class MuseScore : public QMainWindow {
      Q_OBJECT

      QList<Score*> scoreList;
      Score* cs;              // current score

      QQueue<Command> commandQueue;

      QVBoxLayout* layout;
      QTabWidget* tab;
      QToolButton* removeTabButton;

      QMenu* menuDisplay;
      QMenu* openRecent;

      Palette* notePalette;

      MagBox* mag;

      QAction* playId;
      QAction* navigatorId;
      QAction* transportId;
      QAction* inputId;

      PreferenceDialog* preferenceDialog;
      QToolBar* cpitchTools;
      QToolBar* fileTools;
      QToolBar* transportTools;
      QToolBar* entryTools;
      TextTools* _textTools;
      QToolBar* voiceTools;
      EditStyle* editStyleWin;
      InstrumentsDialog* instrList;
      MeasuresDialog* measuresDialog;
      InsertMeasuresDialog* insertMeasuresDialog;
      QMenu* menuEdit;
      QMenu* menuCreate;
      QMenu* menuNotes;
      QMenu* menuLayout;
      QMenu* menuStyle;

      PlayPanel* playPanel;
      InstrumentListEditor* iledit;
      PageListEditor* pageListEdit;
      MeasureListEditor* measureListEdit;
      PageSettings* pageSettings;

      QWidget* symbolDialog;

      PaletteScrollArea* clefPalette;
      PaletteScrollArea* keyPalette;
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
      QScriptEngine* se;
      QString pluginPath;
      QScriptEmbeddedDebugger* debugger;

      QTimer* autoSaveTimer;
      QSignalMapper* pluginMapper;

      //---------------------

      virtual void closeEvent(QCloseEvent*);

      virtual void dragEnterEvent(QDragEnterEvent*);
      virtual void dropEvent(QDropEvent*);

      void playVisible(bool flag);
      void navigatorVisible(bool flag);
      void launchBrowser(const QString whereTo);

      void addScore(const QString& name);
      void saveScoreList();
      void loadScoreList();
      void editInstrList();
      void symbolMenu();
      void clefMenu();
      void keyMenu();
      void timeMenu();
      void dynamicsMenu();
      void loadFile();
      void saveFile();
      void newFile();
      void fingeringMenu();
      void registerPlugin(const QString& pluginPath);
      void startPageListEditor();

   private slots:
      void autoSaveTimerTimeout();
      void helpBrowser();
      void helpBrowser1();
      void about();
      void aboutQt();
      void openRecentMenu();
      void selectScore(QAction*);
      void selectionChanged(int);
      void startPreferenceDialog();
      void showMixer(bool);
      void startExcerptsDialog();
      void preferencesChanged();
      void editStyle();
      void editTextStyle();
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
      void pageSettingsChanged();
      void midiinToggled(bool);
      void speakerToggled(bool);
      void removeTab(int);
      void removeTab();
      void cmd(QAction*);
      void clipboardChanged();
      void pluginTriggered(int);
      void drumPaletteSelected(int);

   public slots:
      void setCurrentScore(int);
      void showPlayPanel(bool);
      void showPalette(bool);
      void showNavigator(bool);
      void dirtyChanged(Score*);
      void changeState(int);
      void setPos(int tick);

   public:
      MuseScore();
      bool checkDirty(Score*);
      void clearScore();
      PlayPanel* getPlayPanel() const { return playPanel; }
      QMenu* genCreateMenu(QWidget* parent = 0);
      void appendScore(Score*);
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
      void loadPlugins();
      QString createDefaultName() const;
      void startAutoSave();
      void updateDrumset();
      double getMag(Canvas*) const;
      void setMag(double);
      bool noScore() const { return scoreList.isEmpty(); }
      void setDrumPalette(Palette* p) { drumPalette = p; }
      TextTools* textTools();
      };

extern QMenu* genCreateMenu(QWidget* parent);
extern MuseScore* mscore;
extern QString dataPath;

extern Shortcut* getShortcut(const char* id);
extern QAction* getAction(const char*);
extern QMap<QString, Shortcut*> shortcuts;
extern Shortcut* midiActionMap[128];
extern void setMscoreLocale(QString localeName);

#endif

