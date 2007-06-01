//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2007 Werner Schweer (ws@seh.de)
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

#include "musedata.h"
#include "score.h"
#include "layout.h"
#include "part.h"
#include "staff.h"
#include "barline.h"
#include "clef.h"
#include "key.h"
#include "note.h"
#include "chord.h"
#include "rest.h"
#include "text.h"
#include "bracket.h"
#include "tuplet.h"

//---------------------------------------------------------
//   musicalAttribute
//---------------------------------------------------------

void MuseData::musicalAttribute(QString s, Part* part)
      {
      QStringList al = s.mid(3).split(" ", QString::SkipEmptyParts);
      foreach(QString item, al) {
            if (item.startsWith("K:")) {
                  int key = item.mid(2).toInt();
                  foreach(Staff* staff, *(part->staves()))
                        (*staff->keymap())[curTick] = key;
                  }
            else if (item.startsWith("Q:")) {
                  _division = item.mid(2).toInt();
                  }
            else if (item.startsWith("T:")) {
                  QStringList tl = item.mid(2).split("/");
                  if (tl.size() != 2) {
                        printf("bad time sig <%s>\n", qPrintable(item));
                        continue;
                        }
                  int z = tl[0].toInt();
                  int n = tl[1].toInt();
                  if ((z > 0) && (n > 0))
                        score->sigmap->add(curTick, z, n);
                  }
            else if (item.startsWith("X:"))
                  ;
            else if (item[0] == 'C') {
                  int staffIdx = 1;
                  int col = 2;
                  if (item[1].isDigit()) {
                        staffIdx = item.mid(1,1).toInt();
                        col = 3;
                        }
                  staffIdx -= 1;
                  int clef = item.mid(col).toInt();
                  int mscoreClef = CLEF_G;
                  switch(clef) {
                        case 4:  mscoreClef = CLEF_G; break;
                        case 22: mscoreClef = CLEF_F; break;
                        case 13: mscoreClef = CLEF_C3; break;
                        case 14: mscoreClef = CLEF_C2; break;
                        case 15: mscoreClef = CLEF_C1; break;
                        default:
                              printf("unknown clef %d\n", clef);
                              break;
                        }
                  Staff* staff = part->staff(staffIdx);
                  staff->clef()->setClef(curTick, mscoreClef);
                  }
            else
                  printf("unknown $key <%s>\n", qPrintable(item));
            }
      }

//---------------------------------------------------------
//   readChord
//---------------------------------------------------------

void MuseData::readChord(Part* part, const QString& s)
      {
      //                       a  b   c  d  e  f  g
      static int table[7]  = { 9, 11, 0, 2, 4, 5, 7 };

      int step  = s[1].toAscii() - 'A';
      int alter = 0;
      int octave = 0;
      for (int i = 2; i < 4; ++i) {
            if (s[i] == '#')
                  alter += 1;
            else if (s[i] == 'f')
                  alter -= 1;
            else if (s[i].isDigit()) {
                  octave = s.mid(i,1).toInt();
                  break;
                  }
            }
      int staffIdx = 0;
      if (s.size() >= 24) {
            if (s[23].isDigit())
                  staffIdx = s.mid(23,1).toInt() - 1;
            }
      Staff* staff = part->staff(staffIdx);

      int pitch = table[step] + alter + (octave + 1) * 12;
      if (pitch < 0)
            pitch = 0;
      if (pitch > 127)
            pitch = 127;

      Chord* chord = (Chord*)chordRest;
      Note* note = new Note(score);
      note->setPitch(pitch);
      note->setStaff(staff);
      note->setTick(chord->tick());
      note->setVoice(voice);
      chord->add(note);
      }

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

void MuseData::readNote(Part* part, const QString& s)
      {
      //                       a  b   c  d  e  f  g
      static int table[7]  = { 9, 11, 0, 2, 4, 5, 7 };

      int step  = s[0].toAscii() - 'A';
      int alter = 0;
      int octave = 0;
      for (int i = 1; i < 3; ++i) {
            if (s[i] == '#')
                  alter += 1;
            else if (s[i] == 'f')
                  alter -= 1;
            else if (s[i].isDigit()) {
                  octave = s.mid(i,1).toInt();
                  break;
                  }
            }
      Direction dir = AUTO;
      if (s.size() >= 23) {
            if (s[22] == 'u')
                  dir = UP;
            else if (s[22] == 'd')
                  dir = DOWN;
            }

      int staffIdx = 0;
      if (s.size() >= 24) {
            if (s[23].isDigit())
                  staffIdx = s.mid(23,1).toInt() - 1;
            }
      Staff* staff = part->staff(staffIdx);
      int gstaff = score->staff(staff);

      int pitch = table[step] + alter + (octave + 1) * 12;
      if (pitch < 0)
            pitch = 0;
      if (pitch > 127)
            pitch = 127;
      int ticks = s.mid(5, 3).toInt();
      ticks     = (ticks * ::division + _division/2) / _division;
      int tick  = curTick;
      curTick  += ticks;

      Tuplet* tuplet = 0;
      if (s.size() >= 22) {
            int a = 1;
            int b = 1;
            if (s[19] != ' ') {
                  a = s[19].toAscii() - '0';
                  if (a == 3 && s[20] != ':')
                        b = 2;
                  else {
                        b = s[21].toAscii() - '0';
                        }
                  }
            if (a == 3 && b == 2) {       // triplet
                  if (chordRest && chordRest->tuplet() && ntuplet) {
                        tuplet = chordRest->tuplet();
                        }
                  else {
                        tuplet = new Tuplet(score);
                        tuplet->setStaff(staff);
                        tuplet->setBaseLen(ticks * a / b);

                        ntuplet = a;
                        tuplet->setNormalNotes(b);
                        tuplet->setActualNotes(a);

                        measure->add(tuplet);
                        }
                  }
            else if (a == 1 && b == 1)
                  ;
            else
                  printf("unsupported tuple %d/%d\n", a, b);
            }

      Chord* chord = new Chord(score, tick);
      chordRest = chord;
      chord->setStaff(staff);
      chord->setStemDirection(dir);
      if (tuplet) {
            chord->setTuplet(tuplet);
            tuplet->add(chord);
            --ntuplet;
            }
      chord->setTickLen(ticks);

      Segment* segment = measure->getSegment(chord);

      voice = 0;
      for (; voice < VOICES; ++voice) {
            Element* e = segment->element(gstaff * VOICES + voice);
            if (e == 0) {
                  chord->setVoice(voice);
                  segment->add(chord);
                  break;
                  }
            }
      if (voice == VOICES) {
            printf("cannot allocate voice\n");
            delete chord;
            return;
            }
      Note* note = new Note(score);
      note->setPitch(pitch);
      note->setStaff(staff);
      note->setTick(tick);
      note->setVoice(voice);
      chord->add(note);
      }

//---------------------------------------------------------
//   readRest
//---------------------------------------------------------

void MuseData::readRest(Part* part, const QString& s)
      {
      int ticks = s.mid(5, 3).toInt();
      ticks     = (ticks * ::division + _division/2) / _division;

      int tick  = curTick;
      curTick  += ticks;

      int staffIdx = 0;
      if (s.size() >= 24) {
            if (s[23].isDigit())
                  staffIdx = s.mid(23,1).toInt() - 1;
            }
      Staff* staff = part->staff(staffIdx);
      int gstaff = score->staff(staff);

      Rest* rest = new Rest(score, tick, ticks);
      chordRest = rest;
      rest->setStaff(staff);
      Segment* segment = measure->getSegment(rest);

      voice = 0;
      for (; voice < VOICES; ++voice) {
            Element* e = segment->element(gstaff * VOICES + voice);
            if (e == 0) {
                  rest->setVoice(voice);
                  segment->add(rest);
                  break;
                  }
            }
      if (voice == VOICES) {
            printf("cannot allocate voice\n");
            delete rest;
            return;
            }
      }

//---------------------------------------------------------
//   readBackup
//---------------------------------------------------------

void MuseData::readBackup(const QString& s)
      {
      int ticks = s.mid(5, 3).toInt();
      ticks     = (ticks * ::division + _division/2) / _division;
      if (s[0] == 'b')
            curTick  -= ticks;
      else
            curTick += ticks;
      }

//---------------------------------------------------------
//   createMeasure
//---------------------------------------------------------

Measure* MuseData::createMeasure()
      {
      ScoreLayout* la = score->mainLayout();
      for (Measure* m = la->first(); m; m = m->next()) {
            int st = m->tick();
            int l  = m->tickLen();
            if (curTick == st)
                  return m;
            if (curTick > st && curTick < (st+l)) {
                  // irregular measure
                  int z, n;
                  score->sigmap->timesig(st, z, n);
                  score->sigmap->add(st, curTick - st, z, n);
                  score->sigmap->add(curTick, z, n);
                  m->setTickLen(curTick - st);
                  break;
                  }
            if (curTick < st + l) {
                  printf("cannot create measure at %d\n", curTick);
                  return 0;
                  }
            }
      Measure* measure  = new Measure(score);
      measure->setTick(curTick);

      for (iStaff i = score->staves()->begin(); i != score->staves()->end(); ++i) {
            Staff* s = *i;
	      if (s->isTop()) {
      	      BarLine* barLine = new BarLine(score);
            	barLine->setStaff(s);
	            measure->setEndBarLine(barLine);
      	      }
            }
      la->push_back(measure);
      return measure;
      }

//---------------------------------------------------------
//   readPart
//---------------------------------------------------------

void MuseData::readPart(QStringList sl, Part* part)
      {
      int line = 10;
      QString s;
      for (; line < sl.size(); ++line) {
            s = sl[line];
            if (!s.isEmpty() && s[0] == '$')
                  break;
            }
      if (line >= sl.size()) {
            printf(" $ not found in part\n");
            return;
            }
      curTick = 0;
      measure = 0;
      measure = createMeasure();
      for (; line < sl.size(); ++line) {
            s = sl[line];
// printf("%6d: <%s>\n", curTick, qPrintable(s));
            char c = s[0].toAscii();
            switch(c) {
                  case 'A':
                  case 'B':
                  case 'C':
                  case 'D':
                  case 'E':
                  case 'F':
                  case 'G':
                        readNote(part, s);
                        break;
                  case ' ':         // chord
                        readChord(part, s);
                        break;
                  case 'r':
                        readRest(part, s);
                        break;
                  case 'g':         // grace note
                  case 'c':         // cue note
                  case 'f':         // basso continuo
                        break;
                  case 'b':         // backspace
                  case 'i':         // forward space
                        readBackup(s);
                        break;
                  case 'm':         // measure line / bar line
                        measure = createMeasure();
                        break;
                  case '*':         // musical direction
                        break;
                  case 'P':         // print suggestion
                        break;
                  case 'S':         // sound record
                        break;
                  case '$':
                        musicalAttribute(s, part);
                        break;
                  default:
                        printf("unknown record <%s>\n", qPrintable(s));
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   countStaves
//---------------------------------------------------------

int MuseData::countStaves(const QStringList& sl)
      {
      int staves = 1;
      for (int i = 10; i < sl.size(); ++i) {
            QString s = sl[i];
            char c = s[0].toAscii();
            switch(c) {
                  case 'A':
                  case 'B':
                  case 'C':
                  case 'D':
                  case 'E':
                  case 'F':
                  case 'G':
                  case 'r':
                        {
                        int staffIdx = 1;
                        if (s.size() >= 24) {
                              if (s[23].isDigit())
                                    staffIdx = s.mid(23,1).toInt();
                              }
                        if (staffIdx > staves)
                              staves = staffIdx;
                        }
                        break;
                  }
            }
      return staves;
      }

//---------------------------------------------------------
//   read
//    return false on error
//---------------------------------------------------------

bool MuseData::read(const QString& name)
      {
      QFile fp(name);
      if (!fp.open(QIODevice::ReadOnly)) {
            printf("cannot open file <%s>\n", qPrintable(name));
            return false;
            }
      QTextStream ts(&fp);
      QStringList part;
      bool commentMode = false;
      for (;;) {
            QString s(ts.readLine());
            if (s.isNull())
                  break;
            if (s.isEmpty()) {
                  if (!commentMode)
                        part.append(QString(""));
                  continue;
                  }
            if (s[0] == '&') {
                  commentMode = !commentMode;
                  continue;
                  }
            if (commentMode)
                  continue;
            if (s[0] == '@')
                  continue;
            if (s[0] == '/') {
                  parts.append(part);

                  Part* mpart = new Part(score);
                  int staves  = countStaves(part);
                  for (int i = 0; i < staves; ++i) {
                        Staff* staff = new Staff(score, mpart, i);
                        mpart->insertStaff(staff);
                        score->staves()->push_back(staff);
                        if ((staves == 2) && (i == 0)) {
printf("set bracket\n");
                              staff->setBracket(0, BRACKET_AKKOLADE);
                              staff->setBracketSpan(0, 2);
                              }
                        }
                  score->parts()->push_back(mpart);
                  mpart->setLongName(part[8]);
                  part.clear();
                  continue;
                  }
            if (s[0] == 'a') {
                  part.back().append(s.mid(1));
                  continue;
                  }
            part.append(s);
            }
      fp.close();
      return true;
      }

//---------------------------------------------------------
//   convert
//---------------------------------------------------------

void MuseData::convert()
      {
      for (int pn = 0; pn < parts.size(); ++pn) {
            Part* part = (*score->parts())[pn];
            readPart(parts[pn], part);
            }
#if 0
      // crash if system does not fit on page (too many instruments)

      Measure* measure = score->tick2measure(0);
      if (measure) {
            Text* text = new Text(score);
            text->setSubtype(TEXT_TITLE);
            text->setText(parts[0][6]);
            text->setText("mops");
            measure->add(text);
            text = new Text(score);
            text->setSubtype(TEXT_SUBTITLE);
            text->setText(parts[0][6]);
            measure->add(text);
            }
#endif
      }

//---------------------------------------------------------
//   importMuseData
//    return true on success
//---------------------------------------------------------

bool Score::importMuseData(const QString& name)
      {
      MuseData md(this);
      if (!md.read(name))
            return false;
      md.convert();

      _saved = false;
      _created = true;
      return true;
      }

//---------------------------------------------------------
//   readMuseData
//---------------------------------------------------------

void Score::readMuseData(QString name)
      {
      info.setFile(name);
      _fileDivision = ::division;
      importMuseData(name);
      int measureNo = 0;
      for (Element* m = _layout->first(); m; m = m->next()) {
            Measure* measure = (Measure*)m;
            measure->setNo(measureNo);
            if (!measure->irregular())
                  ++measureNo;
            }
      layout();
      }

