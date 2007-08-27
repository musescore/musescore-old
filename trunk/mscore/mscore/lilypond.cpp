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
#include "chord.h"
#include "barline.h"

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

      bool lookup(char c);
      char lookup();
      void error(const char* txt);
      void scanNote(char c);

   public:
      Lilypond(Score* s) { score = s; }
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

void Lilypond::scanNote(char c)
      {
      int pitch = 48;

      int octave = 0;
      for (;;) {
            ++cp;
            if (*cp == ' ' || *cp == '\n')
                  break;
            if (*cp == '\'')
                  ++octave;
            }

      pitch += octave * 12;
      switch (c) {
            case 'a':   pitch += 9; break;
            case 'b':   pitch += 11; break;
            case 'c':   break;
            case 'd':   pitch += 2; break;
            case 'e':   pitch += 4; break;
            case 'f':   pitch += 5; break;
            case 'g':   pitch += 7; break;
            }

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

      Segment* segment = new Segment(measure);
      segment->setSubtype(Segment::SegChordRest);
      segment->setTick(tick);
      segment->setParent(measure);
      measure->add(segment);

      Chord* chord = new Chord(score);
      chord->setStaff(staff);
      chord->setParent(segment);
      chord->setTickLen(division);
      chord->setTick(tick);

      segment->add(chord);

      Note* note   = new Note(score);
      note->setPitch(pitch);
      note->setParent(chord);
      note->setStaff(staff);
      note->setTickLen(division);
      chord->add(note);
      tick += division;
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

      if (!lookup('{'))
            error("{ expected");
      ++cp;
      for (char c = lookup(); c && c != '}'; c = lookup()) {
            switch(c) {
                  case 'a':
                  case 'b':
                  case 'c':
                  case 'd':
                  case 'e':
                  case 'f':
                  case 'g':
                        scanNote(c);
                        break;
                  default:
                        error("unexpected char");
                        ++cp;
                        break;
                  }
            }
      BarLine* barLine = new BarLine(score);
      barLine->setParent(measure);
      barLine->setStaff(staff);
      measure->setEndBarLine(barLine);
      }

