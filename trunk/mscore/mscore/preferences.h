//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: preferences.h,v 1.20 2006/04/06 13:03:11 wschweer Exp $
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

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

#include "ui_prefsdialog.h"
#include "globals.h"

struct Shortcut;

enum SessionStart {
      LAST_SESSION, NEW_SESSION, SCORE_SESSION
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
      bool cursorBlink;
      bool bgUseColor;
      bool fgUseColor;
      QString bgWallpaper;
      QString fgWallpaper;
      QColor fgColor;
      QColor bgColor;
      QColor selectColor[VOICES];
      QColor dropColor;
      bool enableMidiInput;
      bool playNotes;         // play notes on click
      QString soundFont;      // sound font used by synthesizer
      QString lPort;          // audio port left
      QString rPort;          // audio port right
      Direction stemDir[VOICES];
      bool showNavigator;
      bool showPlayPanel;
      bool showStatusBar;
      QPoint playPanelPos;
      bool useAlsaAudio;
      bool useJackAudio;
      bool usePortaudioAudio;
      QString alsaDevice;
      int alsaSampleRate;
      int alsaPeriodSize;
      int alsaFragments;
      int portaudioDevice;
      QColor layoutBreakColor;
      bool antialiasedDrawing;
      SessionStart sessionStart;
      QString startScore;
      QString imagePath;
      QString workingDirectory;
      QString lastSaveDirectory;
      bool showSplashScreen;
      MidiRemote rewind, play, stop;
      MidiRemote len1, len2, len4, len8, len16, len32;
      MidiRemote len3, len6, len12, len24;
      bool midiExpandRepeats;
      bool playRepeats;
      QString instrumentList;  // file path of instrument templates
      bool alternateNoteEntryMethod;
      bool useMidiOutput;
      int midiPorts;
      bool midiAutoConnect;
      int rtcTicks;
      int proximity;    // proximity for selecting elements on canvas
      bool autoSave;
      int autoSaveTime;

      bool dirty;

      Preferences();
      void write();
      void read();
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

   private slots:
      void buttonBoxClicked(QAbstractButton*);
      void bgClicked(bool);
      void fgClicked(bool);
      void selectFgWallpaper();
      void selectBgWallpaper();
      void selectSoundFont();
      void selectImagePath();
      void selectWorkingDirectory();
      void selectInstrumentList();
      void selectStartWith();
      void playPanelCurClicked();
      void resetShortcutClicked();
      void clearShortcutClicked();
      void defineShortcutClicked();
      void portaudioApiActivated(int idx);
      void useMidiOutputClicked();
      void useSynthesizerClicked();

   signals:
      void preferencesChanged();

   public:
      PreferenceDialog(QWidget* parent);
      };

extern Preferences preferences;
extern QString appStyleSheet;
extern bool useALSA, useJACK, usePortaudio;
#endif
