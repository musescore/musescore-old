//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#ifndef __IMPORTGTP_H__
#define __IMPORTGTP_H__

#include "al/fraction.h"

class Score;
class Note;
class Segment;

static const int GP_MAX_LYRIC_LINES = 5;
static const int GP_MAX_TRACK_NUMBER = 32;
static const int GP_MAX_STRING_NUMBER = 7;

struct GpTrack {
      int patch;
      uchar volume, pan, chorus, reverb, phase, tremolo;
      };

struct GpBar {
      Fraction timesig;
      int keysig;
      QString marker;
      };

//---------------------------------------------------------
//   GuitarPro
//---------------------------------------------------------

class GuitarPro {
      static const char* errmsg[];
      int version;

      Score* score;
      QFile* f;
      int curPos;

      void skip(qint64 len);
      void read(void* p, qint64 len);
      int readUChar();
      int readChar();
      QString readPascalString(int);
      QString readWordPascalString();
      int readInt();
      QString readDelphiString();
      void readNote(int string, Note*);
      void readChromaticGraph();
      void readMixChange();
      void readColumnEffects();
      void readChordDiagram(Segment*);

   public:
      QString title, subtitle, artist, album, composer;
      QString transcriber, instructions;
      QStringList comments;
      GpTrack channelDefaults[GP_MAX_TRACK_NUMBER * 2];
      int numTracks;
      int numBars;
      QList<GpBar> bars;

      enum GuitarProError { GP_NO_ERROR, GP_UNKNOWN_FORMAT,
         GP_EOF, GP_BAD_NUMBER_OF_STRINGS
            };

      GuitarPro(Score*);
      ~GuitarPro();
      void read(QFile*);
      QString error(GuitarProError n) const { return QString(errmsg[int(n)]); }
      };


#endif



