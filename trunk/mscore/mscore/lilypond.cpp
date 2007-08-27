//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
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

#include "score.h"
#include "element.h"
#include "layout.h"
#include "part.h"
#include "staff.h"
#include "note.h"
#include "rest.h"
#include "chord.h"
#include "barline.h"

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
      QByteArray data;
      char* cp;
      int line;
      int tick;
      Part* part;
      Staff* staff;
      Measure* measure;
      int relpitch;
      int curLen;

      bool lookup(char c);
      char lookup();
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
            curLen   = division;
            }
      bool read(const QString&);
      void convert();
      };

//---------------------------------------------------------
//   importLilypond
//    return true on success
//---------------------------------------------------------

bool Score::importLilypond(const QString& name)
      {
      Lilypond ly(this);
      if (!ly.read(name))
            return false;
      ly.convert();

      _saved = false;
      _created = true;
      return true;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool Lilypond::read(const QString& name)
      {
      QFile fp(name);
      if (!fp.open(QIODevice::ReadOnly)) {
            printf("cannot open file <%s>\n", qPrintable(name));
            return false;
            }
      data = fp.readAll();
      fp.close();
      return true;
      }

//---------------------------------------------------------
//   error
//---------------------------------------------------------

void Lilypond::error(const char* txt)
      {
      printf("Lilypond parse error: %s\n", txt);
      }

//---------------------------------------------------------
//   lookup
//---------------------------------------------------------

bool Lilypond::lookup(char c)
      {
      while (*cp) {
            if (*cp == c)
                  return true;
            if (*cp == '\n')
                  ++line;
            ++cp;
            }
      return false;
      }

char Lilypond::lookup()
      {
      while (*cp) {
            if (*cp != ' ' && *cp != '\n')
                  return *cp;
            ++cp;
            }
      return false;
      }

//---------------------------------------------------------
//   scanNote
//---------------------------------------------------------

LNote Lilypond::scanNote()
      {
      char c    = lookup();
      int pitch = 48;

      switch (c) {
            case 'a':   pitch += 9; break;
            case 'b':   pitch += 11; break;
            case 'c':   break;
            case 'd':   pitch += 2; break;
            case 'e':   pitch += 4; break;
            case 'f':   pitch += 5; break;
            case 'g':   pitch += 7; break;
            }
      ++cp;
      printf("scanNote pitch %d relpitch %d\n", pitch, relpitch);
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
            if (*cp == ' ' || *cp == '\n')
                  break;
            if (*cp == '\'') {
                  ++octave;
                  ++cp;
                  }
            else if (*cp == ',') {
                  --octave;
                  ++cp;
                  }
            else if (isalnum(*cp)) {
                  char buffer[16];
                  char* d = buffer;
                  while (isalnum(*cp))
                        *d++ = *cp++;
                  *d = 0;
                  int len = atoi(buffer);
                  switch(len) {
                        case 1:  curLen = division * 4; break;
                        case 2:  curLen = division * 2; break;
                        case 4:  curLen = division;     break;
                        case 8:  curLen = division / 2; break;
                        case 16: curLen = division / 4; break;
                        case 32: curLen = division / 8; break;
                        case 64: curLen = division / 16; break;
                        default:
                              printf("illegal note len %d\n", len);
                              break;
                        }
                  }
            else if (*cp == '.') {
                  curLen += (curLen / 2);
                  ++cp;
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
      ++cp;
      for (;;) {
            if (*cp == ' ' || *cp == '\n')
                  break;
            if (isalnum(*cp)) {
                  char buffer[16];
                  char* d = buffer;
                  while (isalnum(*cp))
                        *d++ = *cp++;
                  *d = 0;
                  int len = atoi(buffer);
                  switch(len) {
                        case 1:  curLen = division * 4; break;
                        case 2:  curLen = division * 2; break;
                        case 4:  curLen = division;     break;
                        case 8:  curLen = division / 2; break;
                        case 16: curLen = division / 4; break;
                        case 32: curLen = division / 8; break;
                        case 64: curLen = division / 16; break;
                        default:
                              printf("illegal note len %d\n", len);
                              break;
                        }
                  }
            else if (*cp == '.') {
                  curLen += (curLen / 2);
                  ++cp;
                  }
            }
      }

//---------------------------------------------------------
//   createMeasure
//    create new measure if necessary
//---------------------------------------------------------

void Lilypond::createMeasure()
      {
      if (tick >= measure->tick() + measure->tickLen()) {
            BarLine* barLine = new BarLine(score);
            barLine->setParent(measure);
            barLine->setStaff(staff);
            measure->setEndBarLine(barLine);

            measure = new Measure(score);
            measure->setTick(tick);
            measure->setTickLen(division * 4);
            score->mainLayout()->push_back(measure);
            }
      }

//---------------------------------------------------------
//   addNote
//---------------------------------------------------------

void Lilypond::addNote(const LNote& lnote)
      {
      createMeasure();

      Segment* segment = new Segment(measure);
      segment->setSubtype(Segment::SegChordRest);
      segment->setTick(tick);
      segment->setParent(measure);
      measure->add(segment);

      Chord* chord = new Chord(score);
      chord->setStaff(staff);
      chord->setParent(segment);
      chord->setTickLen(lnote.len);
      chord->setTick(tick);

      segment->add(chord);

      Note* note = new Note(score);
      note->setPitch(lnote.pitch);
      note->setParent(chord);
      note->setStaff(staff);
      note->setTickLen(lnote.len);
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
      segment->setSubtype(Segment::SegChordRest);
      segment->setTick(tick);
      segment->setParent(measure);
      measure->add(segment);

      Rest* rest = new Rest(score);
      rest->setStaff(staff);
      rest->setParent(segment);
      rest->setTickLen(curLen);
      rest->setTick(tick);

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
printf("cmdRelative %d\n", relpitch);
      }

//---------------------------------------------------------
//   cmdTime
//---------------------------------------------------------

void Lilypond::cmdTime()
      {
      printf("cmdTime\n");
      }

//---------------------------------------------------------
//   cmdClef
//---------------------------------------------------------

void Lilypond::cmdClef()
      {
      printf("cmdClef\n");
      }

//---------------------------------------------------------
//   scanCmd
//---------------------------------------------------------

void Lilypond::scanCmd()
      {
      char buffer[32];
      char* d = buffer;
      while (*cp && *cp != ' ' && *cp != '\n')
            *d++ = *cp++;
      *d = 0;
      if (strcmp(buffer, "relative") == 0)
            cmdRelative();
      else if (strcmp(buffer, "time") == 0)
            cmdTime();
      else if (strcmp(buffer, "clef") == 0)
            cmdClef();

      else
            error("unknown cmd");
      }

//---------------------------------------------------------
//   convert
//---------------------------------------------------------

void Lilypond::convert()
      {
      cp   = data.data();
      line = 0;
      tick = 0;

      part = new Part(score);
      score->parts()->push_back(part);

      staff = new Staff(score, part, 0);
      score->staves().push_back(staff);
      part->staves()->push_back(staff);

      measure = new Measure(score);
      measure->setTick(tick);
      measure->setTickLen(division * 4);
      score->mainLayout()->push_back(measure);

      char c = lookup();
      if (c == '\\') {
            ++cp;
            scanCmd();
            }
      if (!lookup('{'))
            error("{ expected");
      ++cp;
      for (char c = lookup(); c && c != '}'; c = lookup()) {
            switch(c) {
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
                        printf("unexpected char <%c>\n", c);
                        ++cp;
                        break;
                  }
            }
      BarLine* barLine = new BarLine(score);
      barLine->setParent(measure);
      barLine->setStaff(staff);
      measure->setEndBarLine(barLine);
      }

