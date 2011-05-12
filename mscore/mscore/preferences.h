//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "globals.h"

enum SessionStart {
      EMPTY_SESSION, LAST_SESSION, NEW_SESSION, SCORE_SESSION
      };

// midi remote control values:
enum {
      RMIDI_REWIND,
      RMIDI_TOGGLE_PLAY,
      RMIDI_PLAY,
      RMIDI_STOP,
      RMIDI_NOTE1,
      RMIDI_NOTE2,
      RMIDI_NOTE4,
      RMIDI_NOTE8,
      RMIDI_NOTE16,
      RMIDI_NOTE32,
      RMIDI_NOTE64,
      RMIDI_REST,
      RMIDI_DOT,
      RMIDI_DOTDOT,
      RMIDI_TIE,
      RMIDI_NOTE_EDIT_MODE,
      MIDI_REMOTES
      };

enum MuseScoreStyleType {
      STYLE_DARK,
      STYLE_LIGHT,
      STYLE_NATIVE
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
      int iconHeight, iconWidth;
      QColor selectColor[VOICES];
      QColor defaultColor;
      QColor dropColor;
      bool enableMidiInput;
      bool playNotes;         // play notes on click
      QString soundFont;      // default sound font used by synthesizer
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
      QString defaultStyle;
      bool showSplashScreen;

      bool useMidiRemote;
      MidiRemote midiRemote[MIDI_REMOTES];

      bool midiExpandRepeats;

      bool playRepeats;
      QString instrumentList; // file path of instrument templates
      bool alternateNoteEntryMethod;
      int proximity;          // proximity for selecting elements on canvas
      bool autoSave;
      int autoSaveTime;
      double pngResolution;
      bool pngTransparent;
      QString language;
      bool replaceFractions;
      bool replaceCopyrightSymbol;
      QPrinter::PageSize paperSize;
      double paperWidth, paperHeight;     // only valid if paperSize is QPrinter::Custom
      bool landscape;
      bool twosided;
      double spatium;
      double mag;

      //update
      int checkUpdateStartup;

      float tuning;                 // synthesizer master tuning offset (440Hz)
      float masterGain;             // synthesizer master gain
      float chorusGain;
      float reverbGain;
      float reverbRoomSize;
      float reverbDamp;
      float reverbWidth;

      int defaultPlayDuration;      // len of note play during note entry
      QString importStyleFile;
      QString importCharset;
      bool warnPitchRange;
      bool followSong;
      bool useOsc;
      int oscPort;
      bool singlePalette;
      QString styleName;
      int globalStyle;        // 0 - dark, 1 - light

      QString myScoresPath;
      QString myStylesPath;
      QString myImagesPath;
      QString myTemplatesPath;
      QString myPluginsPath;
      QString mySoundFontsPath;

      double nudgeStep;       // in spatium units (default 0.1)
      double nudgeStep10;     // Ctrl + cursor key (default 1.0)
      double nudgeStep50;     // Alt  + cursor key (default 5.0)

      int hRaster, vRaster;
      bool nativeDialogs;

      int exportAudioSampleRate;

      QString profile;

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

extern Preferences preferences;
extern QString appStyleSheet();
extern bool useALSA, useJACK, usePortaudio;
#endif
