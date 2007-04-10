//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: staff.cpp,v 1.11 2006/03/28 14:58:58 wschweer Exp $
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

#include "globals.h"
#include "staff.h"
#include "part.h"
#include "clef.h"
#include "xml.h"
#include "score.h"
#include "bracket.h"
#include "key.h"
#include "segment.h"

//---------------------------------------------------------
//   bracket
//---------------------------------------------------------

int Staff::bracket(int idx) const
      {
      if (idx < _brackets.size())
            return _brackets[idx]._bracket;
      return NO_BRACKET;
      }

//---------------------------------------------------------
//   bracketSpan
//---------------------------------------------------------

int Staff::bracketSpan(int idx) const
      {
      if (idx < _brackets.size())
            return _brackets[idx]._bracketSpan;
      return 0;
      }

//---------------------------------------------------------
//   setBracket
//---------------------------------------------------------

void Staff::setBracket(int idx, int val)
      {
      if (idx >= _brackets.size()) {
            for (int i = _brackets.size(); i <= idx; ++i)
                  _brackets.append(BracketItem());
            }
      _brackets[idx]._bracket = val;
      }

//---------------------------------------------------------
//   setBracketSpan
//---------------------------------------------------------

void Staff::setBracketSpan(int idx, int val)
      {
      if (idx >= _brackets.size()) {
            for (int i = _brackets.size(); i <= idx; ++i)
                  _brackets.append(BracketItem());
            }
      _brackets[idx]._bracketSpan = val;
      }

//---------------------------------------------------------
//   addBracket
//---------------------------------------------------------

void Staff::addBracket(BracketItem b)
      {
      if (!_brackets.isEmpty() && _brackets[0]._bracket == NO_BRACKET) {
            _brackets[0] = b;
            }
      else {
            //
            // create new bracket level
            //
            foreach(Staff* s, *_score->staves()) {
                  if (s == this)
                        s->_brackets.append(b);
                  else
                        s->_brackets.append(BracketItem());
                  }
            }
      }

//---------------------------------------------------------
//   isTopSplit
//---------------------------------------------------------

bool Staff::isTopSplit() const
      {
      return _part->nstaves() > 1 && isTop();
      }

//---------------------------------------------------------
//   trackName
//---------------------------------------------------------

QString Staff::trackName() const
      {
      return _part->trackName();
      }

//---------------------------------------------------------
//   longName
//---------------------------------------------------------

const QTextDocument& Staff::longName() const
      {
      return _part->longName();
      }

//---------------------------------------------------------
//   shortName
//---------------------------------------------------------

const QTextDocument& Staff::shortName() const
      {
      return _part->shortName();
      }

//---------------------------------------------------------
//   midiChannel
//---------------------------------------------------------

int Staff::midiChannel() const
      {
      return _part->midiChannel();
      }

//---------------------------------------------------------
//   midiProgram
//---------------------------------------------------------

int Staff::midiProgram() const
      {
      return _part->midiProgram();
      }

//---------------------------------------------------------
//   volume
//---------------------------------------------------------

int Staff::volume() const
      {
      return _part->volume();
      }

//---------------------------------------------------------
//   reverb
//---------------------------------------------------------

int Staff::reverb() const
      {
      return _part->reverb();
      }

//---------------------------------------------------------
//   chorus
//---------------------------------------------------------

int Staff::chorus() const
      {
      return _part->chorus();
      }

//---------------------------------------------------------
//   Staff
//---------------------------------------------------------

Staff::Staff(Score* s, Part* p, int rs)
      {
      _score  = s;
      _rstaff = rs;
      _part   = p;
      _clef   = new ClefList;
      _keymap = new KeyList;
      (*_keymap)[0] = 0;
      _show   = true;
      }

//---------------------------------------------------------
//   ~Staff
//---------------------------------------------------------

Staff::~Staff()
      {
      delete _clef;
      delete _keymap;
      }

//---------------------------------------------------------
//   Staff::key
//---------------------------------------------------------

int Staff::key(int tick) const
      {
      return _clef->clef(tick);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Staff::write(Xml& xml) const
      {
      xml.stag("Staff");
      _clef->write(xml, "cleflist");
      _keymap->write(xml, "keylist");
      foreach(const BracketItem& i, _brackets) {
            xml.tagE("bracket type=\"%d\" span=\"%d\"", i._bracket, i._bracketSpan);
            }
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Staff::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            if (tag == "cleflist")
                  _clef->read(node, _score);
            else if (tag == "keylist")
                  _keymap->read(node, _score);
            else if (tag == "bracket") {
                  BracketItem b;
                  b._bracket = e.attribute("type", "-1").toInt();
                  b._bracketSpan = e.attribute("span", "0").toInt();
                  _brackets.append(b);
                  }
            else
                  printf("Mscore:Staff: unknown tag %s\n", tag.toLatin1().data());
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void StaffList::remove(Staff* p)
      {
      if (removeAll(p) < 1)
            printf("StaffList::remove(%p): not found\n", p);
      }

//---------------------------------------------------------
//   idx
//---------------------------------------------------------

int Staff::idx() const
      {
      return _score->staff(this);
      }

//---------------------------------------------------------
//   changeKeySig
//
// change key signature at tick into subtype st for all staves
// in response to gui command (drop keysig on measure or keysig)
//---------------------------------------------------------

void Staff::changeKeySig(int tick, int st)
      {
      int ot = _keymap->key(tick);
      if (ot == st)
            return;                 // no change

      int oval = -1000;
      iKeyEvent ki = _keymap->find(tick);
      if (ki != _keymap->end()) {
            oval = ki->second;
            _keymap->erase(ki);
            }

      bool removeFlag = st == _keymap->key(tick);
      int nval = -1000;
      if (!removeFlag) {
            (*_keymap)[tick] = st;
            nval = st;
            }
      _score->undoOp(UndoOp::ChangeKeySig, this, tick, oval, nval);

      Measure* measure = _score->tick2measure(tick);
      if (!measure) {
            printf("measure for tick %d not found!\n", tick);
            return;
            }

      //---------------------------------------------
      //    if the next keysig has the same subtype
      //    then its unnecessary and must be removed
      //---------------------------------------------

      for (Measure* m = measure; m; m = m->next()) {
            bool found = false;
            for (Segment* segment = m->first(); segment; segment = segment->next()) {
                  if (segment->subtype() != Segment::SegKeySig)
                        continue;
                  //
                  // we assume keySigs are only in first track (voice 0)
                  //
                  int track = idx() * VOICES;
                  KeySig* e = (KeySig*)segment->element(track);
                  int etick = segment->tick();
                  if (!e || (etick < tick))
                        continue;
                  if ((e->subtype() != st) && (etick > tick)) {
                        found = true;
                        break;
                        }
                  _score->undoRemoveElement(e);
                  m->cmdRemoveEmptySegment(segment);
                  if (etick > tick) {
                        found = true;
                        break;
                        }
                  }
            if (found)
                  break;
            }

      //---------------------------------------------
      // insert new keysig symbols
      //---------------------------------------------

      if (!removeFlag) {
            KeySig* keysig = new KeySig(_score);
            keysig->setStaff(this);
            keysig->setTick(tick);
            keysig->setSubtype(st);

            Segment::SegmentType stype = Segment::segmentType(KEYSIG);
            Segment* s = measure->findSegment(stype, tick);
            if (!s) {
                  s = measure->createSegment(stype, tick);
                  _score->undoAddElement(s);
                  }
            keysig->setParent(s);
            _score->undoAddElement(keysig);
            }
      _score->layout();
      }

//---------------------------------------------------------
//   changeClef
//---------------------------------------------------------

void Staff::changeClef(int tick, int st)
      {
      // bool small = st & clefSmallBit;
      st &= ~clefSmallBit;
      int ot = _clef->clef(tick);
      if (ot == st)
            return;                 // no change

      int oval = NO_CLEF;
      iClefEvent ki = _clef->find(tick);
      if (ki != _clef->end()) {
            oval = ki->second;
            _clef->erase(ki);
            }

      bool removeFlag = st == _clef->clef(tick);
      int nval = NO_CLEF;
      if (!removeFlag) {
            (*_clef)[tick] = st;
            nval = st;
            }
      _score->undoOp(UndoOp::ChangeClef, this, tick, oval, nval);

      Measure* measure = _score->tick2measure(tick);
      if (!measure) {
            printf("measure for tick %d not found!\n", tick);
            return;
            }

      //---------------------------------------------
      //    if the next clef has the same subtype
      //    then its unnecessary and must be removed
      //---------------------------------------------

      for (Measure* m = measure; m; m = m->next()) {
            bool found = false;
            for (Segment* segment = m->first(); segment; segment = segment->next()) {
                  if (segment->subtype() != Segment::SegClef)
                        continue;
                  //
                  // we assume Clefs are only in first track (voice 0)
                  //
                  int track = idx() * VOICES;
                  Clef* e = (Clef*)segment->element(track);
                  int etick = segment->tick();
                  if (!e || (etick < tick))
                        continue;
                  int est = e->subtype() & ~clefSmallBit;
                  if ((est != st) && (etick > tick)) {
                        found = true;
                        break;
                        }
                  _score->undoRemoveElement(e);
                  m->cmdRemoveEmptySegment(segment);
                  if (etick > tick) {
                        found = true;
                        break;
                        }
                  }
            if (found)
                  break;
            }

      //---------------------------------------------
      // insert new clef symbol
      //---------------------------------------------

      if (!removeFlag) {
            Clef* clef = new Clef(_score);
            clef->setStaff(this);
            clef->setTick(tick);
            clef->setSubtype(st);

            Segment::SegmentType stype = Segment::segmentType(CLEF);
            Segment* s = measure->findSegment(stype, tick);
            if (!s) {
                  s = measure->createSegment(stype, tick);
                  _score->undoAddElement(s);
                  }
            clef->setParent(s);
            _score->undoAddElement(clef);
            }
      _score->layout();
      }

