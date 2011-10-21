//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2007 Werner Schweer and others
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

#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/element.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/note.h"
#include "libmscore/rest.h"
#include "libmscore/chord.h"
#include "libmscore/barline.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"

//---------------------------------------------------------
//   LNote
//---------------------------------------------------------

struct LNote {
      int pitch;
      int len;

      LNote(int p, int l) : pitch(p), len(l) {}
      };

//---------------------------------------------------------
//   Lilypond
//---------------------------------------------------------

class Lilypond {
      Score* score;
      QString data;
      int ci;
      int line;
      int tick;
      Part* part;
      Staff* staff;
      Measure* measure;
      int relpitch;
      int curLen;

      bool lookup(QChar c);
      QChar lookup();
      void error(const char* txt);
      LNote scanNote();
      void addNote(const LNote&);
      void addRest();
      void scanRest();
      void scanCmd();
      void cmdRelative();
      void cmdTime();
      void cmdClef();
      void createMeasure();

   public:
      Lilypond(Score* s) {
            score    = s;
            relpitch = -1;
            curLen   = MScore::division;
            }
      bool read(const QString&);
      void convert();
      };

//---------------------------------------------------------
//   importLilypond
//    return true on success
//---------------------------------------------------------

bool MuseScore::importLilypond(Score* score, const QString& name)
      {
      Lilypond ly(score);
      if (!ly.read(name))
            return false;
      ly.convert();
      return true;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool Lilypond::read(const QString& name)
      {
      QFile fp(name);
      if (!fp.open(QIODevice::ReadOnly)) {
            qDebug("cannot open file <%s>\n", qPrintable(name));
            return false;
            }
      QByteArray ba = fp.readAll();
      fp.close();
      data = QString::fromUtf8(ba.data());
      return true;
      }

//---------------------------------------------------------
//   error
//---------------------------------------------------------

void Lilypond::error(const char* txt)
      {
      qDebug("Lilypond parse error: %s\n", txt);
      }

//---------------------------------------------------------
//   lookup
//---------------------------------------------------------

bool Lilypond::lookup(QChar c)
      {
      while (!data[ci].isNull()) {
            if (data[ci] == c)
                  return true;
            if (data[ci] == '\n')
                  ++line;
            ++ci;
            }
      return false;
      }

QChar Lilypond::lookup()
      {
      while (data[ci].isSpace()) {
            if (data[ci] == '\n')
                  ++line;
            ++ci;
            }
      return data[ci];
      }

//---------------------------------------------------------
//   scanNote
//---------------------------------------------------------

LNote Lilypond::scanNote()
      {
      QChar c   = lookup();
      int pitch = 48;

      switch (c.toAscii()) {
            case 'a':   pitch += 9; break;
            case 'b':   pitch += 11; break;
            case 'c':   break;
            case 'd':   pitch += 2; break;
            case 'e':   pitch += 4; break;
            case 'f':   pitch += 5; break;
            case 'g':   pitch += 7; break;
            }
      ++ci;
qDebug("scanNote pitch %d relpitch %d\n", pitch, relpitch);
      if (relpitch > 0) {
            if (pitch < relpitch) {
                  while ((relpitch - pitch) > 5)
                        pitch += 12;
                  }
            else if (pitch > relpitch) {
                  while ((pitch - relpitch) > 5)
                        pitch -= 12;
                  }
            }
      int octave = 0;
      for (;;) {
            if (data[ci] == ' ' || data[ci] == '\n')
                  break;
            if (data[ci] == '\'') {
                  ++octave;
                  ++ci;
                  }
            else if (data[ci] == ',') {
                  --octave;
                  ++ci;
                  }
            else if (data[ci].isLetterOrNumber()) {
                  QString buffer;
                  while (data[ci].isLetterOrNumber())
                        buffer.append(data[ci++]);
                  int len = buffer.toInt();
                  switch(len) {
                        case 1:  curLen = MScore::division * 4; break;
                        case 2:  curLen = MScore::division * 2; break;
                        case 4:  curLen = MScore::division;     break;
                        case 8:  curLen = MScore::division / 2; break;
                        case 16: curLen = MScore::division / 4; break;
                        case 32: curLen = MScore::division / 8; break;
                        case 64: curLen = MScore::division / 16; break;
                        default:
                              qDebug("illegal note len %d\n", len);
                              break;
                        }
                  }
            else if (data[ci] == '.') {
                  curLen += (curLen / 2);
                  ++ci;
                  }
            }
      pitch += octave * 12;
      if (relpitch > 0)
            relpitch = pitch;
      return LNote(pitch, curLen);
      }

//---------------------------------------------------------
//   scanRest
//---------------------------------------------------------

void Lilypond::scanRest()
      {
      ++ci;
      for (;;) {
            if (data[ci] == ' ' || data[ci] == '\n')
                  break;
            if (data[ci].isLetterOrNumber()) {
                  QString buffer;
                  while (data[ci].isLetterOrNumber())
                        buffer.append(data[ci++]);
                  int len = buffer.toInt();
                  switch(len) {
                        case 1:  curLen = MScore::division * 4; break;
                        case 2:  curLen = MScore::division * 2; break;
                        case 4:  curLen = MScore::division;     break;
                        case 8:  curLen = MScore::division / 2; break;
                        case 16: curLen = MScore::division / 4; break;
                        case 32: curLen = MScore::division / 8; break;
                        case 64: curLen = MScore::division / 16; break;
                        default:
                              qDebug("illegal note len %d\n", len);
                              break;
                        }
                  }
            else if (data[ci] == '.') {
                  curLen += (curLen / 2);
                  ++ci;
                  }
            }
      }

//---------------------------------------------------------
//   createMeasure
//    create new measure if necessary
//---------------------------------------------------------

void Lilypond::createMeasure()
      {
      if (tick >= measure->tick() + measure->ticks()) {
            measure = new Measure(score);
            measure->setTick(tick);
            score->add(measure);
            }
      }

//---------------------------------------------------------
//   addNote
//---------------------------------------------------------

void Lilypond::addNote(const LNote& lnote)
      {
      createMeasure();

      Segment* segment = new Segment(measure);
      segment->setSubtype(SegChordRest);
      segment->setTick(tick);
      segment->setParent(measure);
      measure->add(segment);

      Chord* chord = new Chord(score);
      chord->setTrack(staff->idx() * VOICES);
      chord->setParent(segment);
      Duration d;
      d.setVal(lnote.len);
      chord->setDurationType(d);

      segment->add(chord);

      Note* note = new Note(score);
      note->setPitch(lnote.pitch);
      note->setTpcFromPitch();
      note->setParent(chord);
      note->setTrack(staff->idx() * VOICES);
      chord->add(note);
      tick += lnote.len;
      }

//---------------------------------------------------------
//   addRest
//---------------------------------------------------------

void Lilypond::addRest()
      {
      createMeasure();

      Segment* segment = new Segment(measure);
      segment->setSubtype(SegChordRest);
      segment->setTick(tick);
      segment->setParent(measure);
      measure->add(segment);

      Rest* rest = new Rest(score);
      rest->setTrack(staff->idx() * VOICES);
      rest->setParent(segment);
      Duration d;
      d.setVal(curLen);
      rest->setDurationType(d);
      segment->add(rest);
      tick += curLen;
      }

//---------------------------------------------------------
//   cmdRelative
//---------------------------------------------------------

void Lilypond::cmdRelative()
      {
      LNote note = scanNote();
      relpitch   = note.pitch;
qDebug("cmdRelative %d\n", relpitch);
      }

//---------------------------------------------------------
//   cmdTime
//---------------------------------------------------------

void Lilypond::cmdTime()
      {
      qDebug("cmdTime\n");
      }

//---------------------------------------------------------
//   cmdClef
//---------------------------------------------------------

void Lilypond::cmdClef()
      {
      qDebug("cmdClef\n");
      }

//---------------------------------------------------------
//   scanCmd
//---------------------------------------------------------

void Lilypond::scanCmd()
      {
      QString buffer;
      while (!data[ci].isNull() && data[ci] != ' ' && data[ci] != '\n')
            buffer.append(data[ci++]);
      if (buffer == "relative")
            cmdRelative();
      else if (buffer == "time")
            cmdTime();
      else if (buffer == "clef")
            cmdClef();
      else
            error("unknown cmd");
      }

//---------------------------------------------------------
//   convert
//---------------------------------------------------------

void Lilypond::convert()
      {
      ci   = 0;
      line = 0;
      tick = 0;

      part = new Part(score);
      score->appendPart(part);

      staff = new Staff(score, part, 0);
      score->staves().push_back(staff);
      part->staves()->push_back(staff);

      measure = new Measure(score);
      measure->setTick(tick);
      score->add(measure);

      QChar c = lookup();
      if (c == '\\') {
            ++ci;
            scanCmd();
            }
      if (!lookup('{'))
            error("{ expected");
      ++ci;
      for (QChar c = lookup(); !c.isNull() && c != '}'; c = lookup()) {
            switch(c.toAscii()) {
                  case 'a' ... 'g':
                        {
                        LNote note = scanNote();
                        addNote(note);
                        }
                        break;
                  case 'r':
                        scanRest();
                        addRest();
                        break;
                  default:
                        qDebug("unexpected char <%c>\n", c.toAscii());
                        ++ci;
                        break;
                  }
            }
      }

