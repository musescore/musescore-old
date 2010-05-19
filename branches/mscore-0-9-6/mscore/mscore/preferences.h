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

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

#include "ui_prefsdialog.h"
#include "globals.h"

struct Shortcut;

enum SessionStart {
      EMPTY_SESSION, LAST_SESSION, NEW_SESSION, SCORE_SESSION
      };

//---------------------------------------------------------
//   MidiRemote
//---------------------------------------------------------

struct MidiRemote {
      int channel;
      int type;         // -1 : inactive, 0 : noteOn, 1 : ctrl
      int data;         // pitch or controller number
      };

//---------------------------------------------------------
//   Preferences
//---------------------------------------------------------

struct Preferences {
      bool bgUseColor;
      bool fgUseColor;
      QString bgWallpaper;
      QString fgWallpaper;
      QColor fgColor;
      QColor bgColor;
      QColor selectColor[VOICES];
      QColor defaultColor;
      QColor dropColor;
      bool enableMidiInput;
      bool playNotes;         // play notes on click
      QString soundFont;      // sound font used by synthesizer
      QString lPort;          // audio port left
      QString rPort;          // audio port right
      bool showNavigator;
      bool showPlayPanel;
      bool showStatusBar;
      QPoint playPanelPos;

      bool useAlsaAudio;
      bool useJackAudio;
      bool usePortaudioAudio;
      bool useJackMidi;
      int midiPorts;
      bool rememberLastMidiConnections;

      QString alsaDevice;
      int alsaSampleRate;
      int alsaPeriodSize;
      int alsaFragments;
      int portaudioDevice;
      QString portMidiInput;

      QColor layoutBreakColor;
      bool antialiasedDrawing;
      SessionStart sessionStart;
      QString startScore;
      QString workingDirectory;
      bool showSplashScreen;

      MidiRemote rewind, play, stop;
      MidiRemote len1, len2, len4, len8, len16, len32;
      MidiRemote len3, len6, len12, len24;
      bool midiExpandRepeats;

      bool playRepeats;
      QString instrumentList;  // file path of instrument templates
      bool alternateNoteEntryMethod;
      int proximity;    // proximity for selecting elements on canvas
      bool autoSave;
      int autoSaveTime;
      bool pngScreenShot;
      QString language;
      int iconWidth, iconHeight;
      int noteEntryIconWidth, noteEntryIconHeight;
      QString applicationFont;
      QString style;
      bool replaceFractions;
      bool replaceCopyrightSymbol;
      QPrinter::PageSize paperSize;
      double paperWidth, paperHeight;     // only valid if paperSize is QPrinter::Custom
      bool landscape;
      bool twosided;
      double spatium;
      
      //update
      int checkUpdateStartup;
      
      float tuning;                 // synthesizer master tuning offset (440Hz)
      float masterGain;            // synthesizer master gain
      float chorusGain;
      float reverbGain;
      float reverbRoomSize;
      float reverbDamp;
      float reverbWidth;

      int defaultPlayDuration;      // len of note play during note entry
      QString importStyleFile;
      QByteArray importCharset;
      bool warnPitchRange;
      bool followSong;

      bool dirty;

      Preferences();
      void write();
      void read();
      void init();
      };

//---------------------------------------------------------
//   ShortcutItem
//---------------------------------------------------------

class ShortcutItem : public QTreeWidgetItem {

      bool operator<(const QTreeWidgetItem&) const;

   public:
      ShortcutItem() : QTreeWidgetItem() {}
      };

//---------------------------------------------------------
//   PreferenceDialog
//---------------------------------------------------------

class PreferenceDialog : public QDialog, private Ui::PrefsDialogBase {
      Q_OBJECT

      QMap<QString, Shortcut*> localShortcuts;
      bool shortcutsChanged;

      void apply();
      bool sfChanged;
      void updateSCListView();
      void setUseMidiOutput(bool);
      void updateValues(Preferences*);

   private slots:
      void buttonBoxClicked(QAbstractButton*);
      void bgClicked(bool);
      void fgClicked(bool);
      void selectFgWallpaper();
      void selectBgWallpaper();
      void selectWorkingDirectory();
      void selectInstrumentList();
      void selectStartWith();
      void playPanelCurClicked();
      void resetShortcutClicked();
      void clearShortcutClicked();
      void defineShortcutClicked();
      void portaudioApiActivated(int idx);
      void resetAllValues();
      void paperSizeChanged(double);
      void pageFormatSelected(int);
      void landscapeToggled(bool);
      void styleFileButtonClicked();

   signals:
      void preferencesChanged();

   public:
      PreferenceDialog(QWidget* parent);
      };

extern Preferences preferences;
extern QString appStyleSheet();
extern bool useALSA, useJACK, usePortaudio;
#endif
