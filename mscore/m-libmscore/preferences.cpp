//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: preferences.cpp 3689 2010-11-08 10:55:30Z wschweer $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include "score.h"
#include "preferences.h"
// #include "note.h"
// #include "sym.h"
// #include "page.h"
// #include "file.h"

extern void readShortcuts();

//---------------------------------------------------------
//   Preferences
//---------------------------------------------------------

Preferences preferences;

Preferences::Preferences()
      {
      init();
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Preferences::init()
      {
      // set fallback defaults:

      bgUseColor         = true;
      fgUseColor         = false;
      bgWallpaper        = QString();
      fgWallpaper        = ""; // ":/data/paper5.png";
      fgColor.setRgb(255, 255, 255);
      bgColor.setRgb(0x76, 0x76, 0x6e);

      selectColor[0].setRgb(0, 0, 255);     //blue
      selectColor[1].setRgb(0, 150, 0);     //green
      selectColor[2].setRgb(230, 180, 50);  //yellow
      selectColor[3].setRgb(200, 0, 200);   //purple
      defaultColor   = Color(0,0,0);

      layoutBreakColor         = Color(0, 255, 0);

      midiExpandRepeats        = true;
      playRepeats              = true;

      warnPitchRange          = true;
      };

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Preferences::write()
      {
#if 0
      dirty = false;
      QSettings s;

      s.setValue("bgUseColor",         bgUseColor);
      s.setValue("fgUseColor",         fgUseColor);
      s.setValue("bgWallpaper",        bgWallpaper);
      s.setValue("fgWallpaper",        fgWallpaper);
      s.setValue("fgColor",            fgColor);
      s.setValue("bgColor",            bgColor);

      s.setValue("selectColor1",       selectColor[0]);
      s.setValue("selectColor2",       selectColor[1]);
      s.setValue("selectColor3",       selectColor[2]);
      s.setValue("selectColor4",       selectColor[3]);
      s.setValue("dropColor",          dropColor);
      s.setValue("defaultColor",       defaultColor);

      s.setValue("soundFont",          soundFont);
      s.setValue("lPort",              lPort);
      s.setValue("rPort",              rPort);
      s.setValue("showNavigator",      showNavigator);
      s.setValue("showPlayPanel",      showPlayPanel);
      s.setValue("showStatusBar",      showStatusBar);

      s.setValue("useAlsaAudio",       useAlsaAudio);
      s.setValue("useJackAudio",       useJackAudio);
      s.setValue("useJackMidi",        useJackMidi);
      s.setValue("usePortaudioAudio",  usePortaudioAudio);
      s.setValue("midiPorts",          midiPorts);
      s.setValue("rememberLastMidiConnections", rememberLastMidiConnections);

      s.setValue("alsaDevice",         alsaDevice);
      s.setValue("alsaSampleRate",     alsaSampleRate);
      s.setValue("alsaPeriodSize",     alsaPeriodSize);
      s.setValue("alsaFragments",      alsaFragments);
      s.setValue("portaudioDevice",    portaudioDevice);
      s.setValue("portMidiInput",   portMidiInput);

      s.setValue("layoutBreakColor",   layoutBreakColor);
      s.setValue("antialiasedDrawing", antialiasedDrawing);
      s.setValue("workingDirectory",   workingDirectory);

      s.setValue("midiExpandRepeats",  midiExpandRepeats);
      s.setValue("playRepeats",        playRepeats);

      s.setValue("proximity",          proximity);
      s.setValue("language",           language);

      s.setValue("spatium", spatium);
      s.setValue("mag", mag);
      s.setValue("tuning", tuning);
      s.setValue("masterGain", masterGain);
      s.setValue("chorusGain", chorusGain);
      s.setValue("reverbGain", reverbGain);
      s.setValue("reverbRoomSize", reverbRoomSize);
      s.setValue("reverbDamp", reverbDamp);
      s.setValue("reverbWidth", reverbWidth);

      s.setValue("followSong", followSong);

      s.beginGroup("PlayPanel");
      s.setValue("pos", playPanelPos);
      s.endGroup();
#endif
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Preferences::read()
      {
#if 0
      QSettings s;

      bgUseColor      = s.value("bgUseColor", true).toBool();
      fgUseColor      = s.value("fgUseColor", false).toBool();
      bgWallpaper     = s.value("bgWallpaper").toString();
      fgWallpaper     = s.value("fgWallpaper", ":/data/paper5.png").toString();
      fgColor         = s.value("fgColor", Color(255, 255, 255)).value<Color>();
      bgColor         = s.value("bgColor", Color(0x76, 0x76, 0x6e)).value<Color>();

      selectColor[0]  = s.value("selectColor1", Color(Qt::blue)).value<Color>();     //blue
      selectColor[1]  = s.value("selectColor2", Color(0, 150, 0)).value<Color>();    //green
      selectColor[2]  = s.value("selectColor3", Color(230, 180, 50)).value<Color>(); //yellow
      selectColor[3]  = s.value("selectColor4", Color(200, 0, 200)).value<Color>();  //purple

      defaultColor    = s.value("defaultColor", Color(Qt::black)).value<Color>();
      dropColor       = s.value("dropColor",    Color(Qt::red)).value<Color>();

      lPort           = s.value("lPort").toString();
      rPort           = s.value("rPort").toString();

      soundFont       = s.value("soundFont", mscoreGlobalShare+"/sound/TimGM6mb.sf2").toString();
      if (soundFont == ":/data/piano1.sf2") {
            // silently change to new default sound font
            soundFont = mscoreGlobalShare + "/sound/TimGM6mb.sf2";
            }
      showNavigator   = s.value("showNavigator", true).toBool();
      showStatusBar   = s.value("showStatusBar", true).toBool();
      showPlayPanel   = s.value("showPlayPanel", false).toBool();

#if defined(Q_WS_MAC) || defined(__MINGW32__)
      useAlsaAudio       = s.value("useAlsaAudio", false).toBool();
      useJackAudio       = s.value("useJackAudio", false).toBool();
      useJackMidi        = s.value("useJackMidi",  false).toBool();
      usePortaudioAudio  = s.value("usePortaudioAudio", true).toBool();
#else
      useAlsaAudio       = s.value("useAlsaAudio", true).toBool();
      useJackAudio       = s.value("useJackAudio", false).toBool();
      useJackMidi        = s.value("useJackMidi",  false).toBool();
      usePortaudioAudio  = s.value("usePortaudioAudio", false).toBool();
#endif

      alsaDevice         = s.value("alsaDevice", "default").toString();
      alsaSampleRate     = s.value("alsaSampleRate", 48000).toInt();
      alsaPeriodSize     = s.value("alsaPeriodSize", 1024).toInt();
      alsaFragments      = s.value("alsaFragments", 3).toInt();
      portaudioDevice    = s.value("portaudioDevice", -1).toInt();
      portMidiInput      = s.value("portMidiInput", "").toString();
      layoutBreakColor   = s.value("layoutBreakColor", Color(Qt::green)).value<Color>();
      antialiasedDrawing = s.value("antialiasedDrawing", true).toBool();

      QString path = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
      workingDirectory   = s.value("workingDirectory", path).toString();

      midiExpandRepeats        = s.value("midiExpandRepeats", true).toBool();
      playRepeats              = s.value("playRepeats", true).toBool();
      midiPorts                = s.value("midiPorts", 2).toInt();
      rememberLastMidiConnections = s.value("rememberLastMidiConnections", true).toBool();
      proximity                = s.value("proximity", 6).toInt();
      language                 = s.value("language", "system").toString();

      spatium                = s.value("spatium", SPATIUM20).toDouble();
      mag                    = s.value("mag", 1.0).toDouble();
      tuning                 = s.value("tuning", 440.0).toDouble();
      masterGain             = s.value("masterGain", 0.2).toDouble();
      chorusGain             = s.value("chorusGain", 0.5).toDouble();
      reverbGain             = s.value("reverbGain", 0.5).toDouble();
      reverbRoomSize         = s.value("reverbRoomSize", 0.5).toDouble();
      reverbDamp             = s.value("reverbDamp", 0.5).toDouble();
      reverbWidth            = s.value("reverbWidth", 1.0).toDouble();

      warnPitchRange         = s.value("warnPitchRange", true).toBool();
      followSong             = s.value("followSong", true).toBool();

      s.beginGroup("PlayPanel");
      playPanelPos = s.value("pos", QPoint(100, 300)).toPoint();
      s.endGroup();

      readShortcuts();
#endif
      }
