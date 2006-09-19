//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: mscore.h,v 1.54 2006/04/12 14:58:10 wschweer Exp $
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

#ifndef __MSCORE_H__
#define __MSCORE_H__

#include "globals.h"
#include "ui_measuresdialog.h"

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

extern QString mscoreGlobalShare;
static const int PROJECT_LIST_LEN = 6;

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
      TabBar(QWidget* parent) : QTabBar(parent) {}
      };

//---------------------------------------------------------
//   MuseScore
//---------------------------------------------------------

class MuseScore : public QMainWindow {
      Q_OBJECT

      Score* cs;              // current score
      Canvas* canvas;
      QVBoxLayout* layout;
      TabBar* tab;

      QMenu* menuDisplay;
      QMenu* menuEdit;
      QMenu* openRecent;

      QComboBox* mag;
      QActionGroup* transportAction;
      QAction* padId;
      QAction* playId;
      QAction* navigatorId;
      QAction* visibleId;
      QAction* transportId;
      QAction* inputId;
      QAction* entryAction[16];
      QAction* voiceAction[VOICES];
      QAction* cutId;
      QAction* copyId;
      QAction* pasteId;
      PreferenceDialog* preferenceDialog;
      QToolBar* fileTools;
      QToolBar* transportTools;
      QToolBar* entryTools;
      QToolBar* voiceTools;
      EditStyle* editStyleWin;
      InstrumentsDialog* instrList;
      TextStyleDialog* textStyleDialog;
      MeasuresDialog* measuresDialog;
      QAction* playAction;
      QAction* stopAction;
      PlayPanel* playPanel;
      InstrumentListEditor* iledit;
      PageListEditor* pageListEdit;
      MeasureListEditor* measureListEdit;
      PageSettings* pageSettings;

      QWidget* symbolPalette1;
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
      QAction* midiinAction;
      QAction* speakerAction;

      Pad* pad;
      std::vector<Score*> scoreList;
      bool _midiinEnabled;
      bool _speakerEnabled;

      virtual void closeEvent(QCloseEvent*);
      void loadScoreFile(const QString&);

      void playVisible(bool flag);
      void navigatorVisible(bool flag);
      void launchBrowser(const QString whereTo);

      void addScore(const QString& name);
      void genRecentPopup(QMenu*) const;
      void saveScoreList();
      void loadScoreList();
      void loadInstrumentTemplates();

   private slots:
      void padTriggered(QAction* action);
      void padTriggered(int);
      void helpBrowser();
      void about();
      void aboutQt();
      void padVisible(bool);
      void openRecentMenu();
      void selectScore(QAction*);
      void quitDoc();
      void updateMag();
      void selectionChanged(int);
      void startPreferenceDialog();
      void startInstrumentListEditor();
      void startPageListEditor();
      void preferencesChanged();
      void setStop(bool);
      void setPlay(bool);
      void exportMidi();
      void importMidi();
      void importMusicXml();
      void exportMusicXml();
      void loadFile();
      bool saveFile();
      bool saveAs();
      void newFile();
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
      void fingeringMenu();
      void noteAttributesMenu();
      void accidentalsMenu();
      void doRedo();
      void doUndo();
      void printFile();
      void cmdAddPitch(QAction*);
      void cmdAddIntervall(QAction*);
      void cmdTuplet(QAction*);
      void midiReceived();
      void cmdAddTitle();
      void cmdAddSubTitle();
      void cmdAddComposer();
      void cmdAddPoet();
      void addLyrics();
      void addExpression();
      void addTechnik();
      void addTempo();
      void addMetronome();
      void cmdAppendMeasure();
      void cmdAppendMeasures();
      void resetUserStretch();
      void showLayoutBreakPalette();
      void resetUserOffsets();
      void magChanged(int);
      void showPageSettings();
      void pageSettingsChanged();
      void textStyleChanged();
      void showInvisibleClicked();
      void closePad();
      void midiinToggled(bool);
      void speakerToggled(bool);
      void removeTab(int);

   public slots:
      void setCurrentScore(int);
      void editInstrList();
      void symbolMenu1();
      void clefMenu();
      void keyMenu();
      void timeMenu();
      void dynamicsMenu();
      void startNoteEntry();
      void showPlayPanel(bool);
      void showPad(bool);
      void showNavigator(bool);
      void keyPadToggled(int key);

   public:
      MuseScore();
      Canvas* getCanvas() { return canvas; }
      bool checkDirty();
      void clearScore();
      bool saveFile(QFileInfo&);
      bool saveFile(QFile*);
      PlayPanel* getPlayPanel() const { return playPanel; }
      Pad* getKeyPad() const          { return pad; }
      QMenu* genCreateMenu(QWidget* parent);
      void appendScore(Score*);
      QString getScore(int idx) const;
      void midiNoteReceived(int pitch, bool chord);
      void showElementContext(Element* el);
      void setPadNo(int padno);
	void cmdAppendMeasures(int);
      void setEntry(bool val, int i);
      bool midiinEnabled() const;
      bool playEnabled() const;
      };

extern QMenu* genCreateMenu(QWidget* parent);
extern MuseScore* mscore;

#endif

