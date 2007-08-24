//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: mscore.h,v 1.54 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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
class Pad;
class PaletteBox;
class Xml;
class MagBox;

extern QString mscoreGlobalShare;
static const int PROJECT_LIST_LEN = 6;

//
// MuseScore _state
//

enum {
      STATE_NORMAL,
      STATE_NOTE_ENTRY,
      STATE_EDIT,
      STATE_PLAY
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
//   TabBar
//---------------------------------------------------------

class TabBar : public QTabBar {
      Q_OBJECT

      virtual void mouseDoubleClickEvent(QMouseEvent* ev) {
            emit doubleClick(currentIndex());
            QTabBar::mouseDoubleClickEvent(ev);
            }
   signals:
      void doubleClick(int);

   public:
      TabBar() : QTabBar() {}
      };


//---------------------------------------------------------
//   Shortcut
//    hold the basic values for configurable shortcuts
//---------------------------------------------------------

class Shortcut {
   public:
      const char* xml;        //! xml tag name for configuration file
      QString descr;          //! descriptor, shown in editor
      QKeySequence key;       //! shortcut
      Qt::ShortcutContext context;
      QString text;
      QString help;
      QIcon* icon;
      QAction* action;        //! cached action

      Shortcut();
      Shortcut(const char* name, const char* d, const QKeySequence& k = QKeySequence(),
         Qt::ShortcutContext cont = Qt::ApplicationShortcut,
         const char* txt = 0, const char* h = 0, QIcon* i = 0);
      Shortcut(const Shortcut& c);
      };

//---------------------------------------------------------
//   MuseScore
//---------------------------------------------------------

class MuseScore : public QMainWindow {
      Q_OBJECT

      int _state;
      Score* cs;              // current score
      Canvas* canvas;
      QVBoxLayout* layout;
      TabBar* tab;
      QToolButton* removeTabButton;

      QMenu* menuDisplay;
      QMenu* menuEdit;
      QMenu* openRecent;

      MagBox* mag;
      QActionGroup* transportAction;

      QAction* playId;
      QAction* navigatorId;
      QAction* visibleId;
      QAction* transportId;
      QAction* inputId;

      PreferenceDialog* preferenceDialog;
      QToolBar* fileTools;
      QToolBar* transportTools;
      QToolBar* entryTools;
      QToolBar* voiceTools;
      EditStyle* editStyleWin;
      InstrumentsDialog* instrList;
      TextStyleDialog* textStyleDialog;
      MeasuresDialog* measuresDialog;
      InsertMeasuresDialog* insertMeasuresDialog;

      PlayPanel* playPanel;
      InstrumentListEditor* iledit;
      PageListEditor* pageListEdit;
      MeasureListEditor* measureListEdit;
      PageSettings* pageSettings;

      QWidget* symbolDialog;
      QWidget* clefPalette;
      QWidget* keyPalette;
      QWidget* timePalette;
      QWidget* linePalette;
      QWidget* bracketPalette;
      QWidget* barPalette;
      QWidget* fingeringPalette;
      QWidget* noteAttributesPalette;
      QWidget* accidentalsPalette;
      QWidget* dynamicsPalette;
      QWidget* layoutBreakPalette;
      QStatusBar* _statusBar;
      QLabel* _modeText;

      Pad* pad;
      PaletteBox* paletteBox;
      QList<Score*> scoreList;
      bool _midiinEnabled;
      bool _speakerEnabled;

      virtual void closeEvent(QCloseEvent*);

      void playVisible(bool flag);
      void navigatorVisible(bool flag);
      void launchBrowser(const QString whereTo);

      void addScore(const QString& name);
      void saveScoreList();
      void loadScoreList();
      void loadInstrumentTemplates();
      void editInstrList();
      void symbolMenu();
      void clefMenu();
      void keyMenu();
      void timeMenu();
      void dynamicsMenu();
      void loadFile();
      bool saveFile();
      bool saveAs();
      void newFile();
      void newFileFromTemplate();
      void fingeringMenu();
      QString createDefaultName() const;

   private slots:
      void helpBrowser();
      void about();
      void aboutQt();
      void padVisible(bool);
      void openRecentMenu();
      void selectScore(QAction*);
      void selectionChanged(int);
      void startPreferenceDialog();
      void startInstrumentListEditor();
      void startPageListEditor();
      void preferencesChanged();
      void editStyle();
      void saveStyle();
      void loadStyle();
      void editTextStyle();
      void seqStarted();
      void seqStopped();
      void closePlayPanel();

      void lineMenu();
      void bracketMenu();
      void barMenu();
      void noteAttributesMenu();
      void accidentalsMenu();
      void midiReceived();
      void cmdAppendMeasures();
      void cmdInsertMeasures();
      void resetUserStretch();
      void showLayoutBreakPalette();
      void resetUserOffsets();
      void magChanged(double);
      void showPageSettings();
      void pageSettingsChanged();
      void showInvisibleClicked();
      void midiinToggled(bool);
      void speakerToggled(bool);
      void removeTab(int);
      void removeTab();
      void cmd(QAction*);
      void clipboardChanged();

   public slots:
      void setCurrentScore(int);
      void showPlayPanel(bool);
      void showPad(bool);
      void showPalette(bool);
      void showNavigator(bool);

   public:
      MuseScore();
      Canvas* getCanvas() { return canvas; }
      bool checkDirty(Score*);
      void clearScore();
      bool saveFile(QFileInfo&);
      bool saveFile(QFile*);
      PlayPanel* getPlayPanel() const { return playPanel; }
      Pad* getKeyPad() const          { return pad; }
      QMenu* genCreateMenu();
      void appendScore(Score*);
      void midiNoteReceived(int pitch, bool chord);
      void showElementContext(Element* el);
	void cmdAppendMeasures(int);
      bool midiinEnabled() const;
      bool playEnabled() const;
      Score* currentScore() const { return cs; }
      void setState(int);
      static Shortcut sc[];
      static Shortcut scSeq[];
      void incMag();
      void decMag();
      void setMag(double);
      void readSettings();
      void writeSettings();
      };

//---------------------------------------------------------
//   MagValidator
//---------------------------------------------------------

class MagValidator : public QValidator {
      Q_OBJECT

      virtual State validate(QString&, int&) const;

   public:
      MagValidator(QObject* parent = 0);
      };

//---------------------------------------------------------
//   MagBox
//---------------------------------------------------------

class MagBox : public QComboBox {
      Q_OBJECT

      double txt2mag(const QString&);

   signals:
      void magChanged(double);

   private slots:
      void indexChanged(int idx);

   public:
      MagBox(QWidget* parent = 0);
      void setMag(double);
      };

extern QMenu* genCreateMenu(QWidget* parent);
extern MuseScore* mscore;

extern QAction* getAction(const char*);
extern QMap<QString, Shortcut*> shortcuts;
extern QMap<QString, Shortcut*> shortcutsSeq;

#endif

