//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include <assert.h>

#include "globals.h"
#include "staff.h"
#include "part.h"
#include "clef.h"
#include "xml.h"
#include "score.h"
#include "bracket.h"
#include "keysig.h"
#include "segment.h"
#include "style.h"
#include "measure.h"

//---------------------------------------------------------
//   idx
//---------------------------------------------------------

int Staff::idx() const
      {
      return _score->staffIdx(this);
      }

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
      while (!_brackets.isEmpty() && (_brackets.last()._bracket == NO_BRACKET))
            _brackets.removeLast();
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
            foreach(Staff* s, _score->staves()) {
                  if (s == this)
                        s->_brackets.append(b);
                  else
                        s->_brackets.append(BracketItem());
                  }
            }
      }

//---------------------------------------------------------
//   cleanupBrackets
//---------------------------------------------------------

void Staff::cleanupBrackets()
      {
      int index = idx();
      int n = _score->nstaves();
      for (int i = 0; i < _brackets.size(); ++i) {
            if (_brackets[i]._bracket != NO_BRACKET) {
                  int span = _brackets[i]._bracketSpan;
                  if (span > (n - index)) {
                        span = n - index;
                        _brackets[i]._bracketSpan = span;
                        }
                  }
            }
      for (int i = 0; i < _brackets.size(); ++i) {
            if (_brackets[i]._bracket != NO_BRACKET) {
                  int span = _brackets[i]._bracketSpan;
                  if (span <= 1)
                        _brackets[i] = BracketItem();
                  else {
                        // delete all other brackets with same span
                        for (int k = i + 1; k < _brackets.size(); ++k) {
                              if (span == _brackets[k]._bracketSpan)
                                    _brackets[k] = BracketItem();
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   trackName
//---------------------------------------------------------

QString Staff::trackName() const
      {
      return _part->trackName();
      }

//---------------------------------------------------------
//   Staff
//---------------------------------------------------------

Staff::Staff(Score* s, Part* p, int rs)
      {
      _score        = s;
      _rstaff       = rs;
      _part         = p;
      _clefList     = new ClefList;
      _keymap       = new KeyList;
      (*_keymap)[0] = 0;                  // default to C major
      _show         = true;
      _lines        = 5;
      _small        = false;
      _slashStyle   = false;
      _barLineSpan  = 1;
      _invisible    = false;
      }

//---------------------------------------------------------
//   ~Staff
//---------------------------------------------------------

Staff::~Staff()
      {
      delete _clefList;
      delete _keymap;
      _keymap   = 0;      // DEBUG
      _clefList = 0;
      }

//---------------------------------------------------------
//   Staff::clef
//---------------------------------------------------------

int Staff::clef(int tick) const
      {
      return _clefList->clef(tick);
      }

//---------------------------------------------------------
//   Staff::key
//---------------------------------------------------------

KeySigEvent Staff::key(int tick) const
      {
      return _keymap->key(tick);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Staff::write(Xml& xml) const
      {
      xml.stag("Staff");
      if (lines() != 5)
            xml.tag("lines", lines());
      if (small() && !xml.excerptmode)    // switch small staves to normal ones when extracting part
            xml.tag("small", small());
      if (invisible())
            xml.tag("invisible", invisible());
      if (slashStyle())
            xml.tag("slashStyle", slashStyle());
      _clefList->write(xml, "cleflist");
      _keymap->write(xml, "keylist");
      foreach(const BracketItem& i, _brackets) {
            xml.tagE("bracket type=\"%d\" span=\"%d\"", i._bracket, i._bracketSpan);
            }
      if (_barLineSpan != 1)
            xml.tag("barLineSpan", _barLineSpan);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Staff::read(QDomElement e)
      {
      setLines(5);
      setSmall(false);
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            int v = e.text().toInt();
            if (tag == "lines")
                  setLines(v);
            else if (tag == "small")
                  setSmall(v);
            else if (tag == "invisible")
                  setInvisible(v);
            else if (tag == "slashStyle")
                  setSlashStyle(v);
            else if (tag == "cleflist")
                  _clefList->read(e, _score);
            else if (tag == "keylist")
                  _keymap->read(e, _score);
            else if (tag == "bracket") {
                  BracketItem b;
                  b._bracket = e.attribute("type", "-1").toInt();
                  b._bracketSpan = e.attribute("span", "0").toInt();
                  _brackets.append(b);
                  }
            else if (tag == "barLineSpan")
                  _barLineSpan = v;
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   changeKeySig
///   Change key signature.
/// change key signature at tick into subtype st for all staves
/// in response to gui command (drop keysig on measure or keysig)
//---------------------------------------------------------

void Staff::changeKeySig(int tick, KeySigEvent st)
      {
printf("Staff::changeKeySig "); st.print(); printf("\n");
      KeySigEvent ot = _keymap->key(tick);
      if (ot == st) {
printf("Staff::changeKeySig: no change\n");
            return;                 // no change
            }

      iKeyList ki     = _keymap->find(tick);
      KeySigEvent oval = ki != _keymap->end() ? ki->second : KeySigEvent();
      bool removeFlag  = st == _keymap->key(tick-1);
      KeySigEvent nval = removeFlag ? KeySigEvent() : st;

      _score->undoChangeKey(this, tick, oval, nval);

      //---------------------------------------------
      //    if the next keysig has the same subtype
      //    then its unnecessary and must be removed
      //---------------------------------------------

      Measure* measure = _score->tick2measure(tick);
      if (!measure) {
            printf("measure for tick %d not found!\n", tick);
            return;
            }
      Measure* m = measure;
      if (m->prevMeasure())
            m = m->prevMeasure();
      int track = idx() * VOICES;
      for (Segment* segment = measure->first(); segment; segment = segment->next1()) {
            if (segment->subtype() != SegKeySig)
                  continue;
            //
            // we assume keySigs are only in first track (voice 0)
            //
            KeySig* e = static_cast<KeySig*>(segment->element(track));
            int etick = segment->tick();
            if (!e || (etick < tick))
                  continue;
            KeySigEvent cst = e->keySignature();
            if ((cst != st) && (etick > tick))
                  break;
            _score->undoRemoveElement(e);
            m->cmdRemoveEmptySegment(segment);
            if (etick > tick)
                  break;
            }

      //---------------------------------------------
      // insert new keysig symbols
      //---------------------------------------------

      if (!removeFlag) {
            KeySig* keysig = new KeySig(_score);
            keysig->setTrack(idx() * VOICES);
            keysig->setTick(tick);
            keysig->changeType(st);

            SegmentType stype = Segment::segmentType(KEYSIG);
            Segment* s = measure->findSegment(stype, tick);
            if (!s) {
                  s = measure->createSegment(stype, tick);
                  _score->undoAddElement(s);
                  }
            keysig->setParent(s);
            _score->undoAddElement(keysig);
            }
      _score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   changeClef
//---------------------------------------------------------

void Staff::changeClef(int tick, int st)
      {
      int ot = _clefList->clef(tick);
      if (ot == st)
            return;                 // no change

      // automatically turn this part into a drum part
      // if clef at tick zero is changed to percussion clef

      if (tick == 0 && st == CLEF_PERC)
            part()->setUseDrumset(true);

      iClefEvent ki   = _clefList->find(tick);
      int oval        = ki != _clefList->end() ? ki->second : NO_CLEF;
      bool removeFlag = st == _clefList->clef(tick-1);
      int nval        = removeFlag ? NO_CLEF : st;

      _score->undoChangeClef(this, tick, oval, nval);

      //---------------------------------------------
      //    if the next clef has the same subtype
      //    then its unnecessary and must be removed
      //---------------------------------------------

      Measure* measure = _score->tick2measure(tick);
      if (!measure) {
            printf("measure for tick %d not found!\n", tick);
            return;
            }
      Measure* m = measure;
      if (m->prevMeasure())
            m = m->prevMeasure();
      for (; m; m = m->nextMeasure()) {
            bool found = false;
            //
            // we assume Clefs are only in first track of staff (voice 0)
            //
            int track = idx() * VOICES;
            for (Segment* segment = m->first(); segment; segment = segment->next()) {
                  if (segment->subtype() != SegClef)
                        continue;
                  int etick = segment->tick();
                  Clef* e = static_cast<Clef*>(segment->element(track));
                  if (!e || (etick < tick))
                        continue;
                  int est = e->subtype();
                  if ((est != st) && (etick > tick) && !e->generated()) {
                        found = true;
                        break;
                        }
                  if (!e->generated()) {
                        _score->undoRemoveElement(e);
                        m->cmdRemoveEmptySegment(segment);
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
            clef->setTrack(idx() * VOICES);
            clef->setTick(tick);
            clef->setSubtype(st);

            //
            // if clef is at measure beginning, move to end of
            // previous measure
            //
            if (measure->tick() == tick && (tick != 0))
                  measure = measure->prevMeasure();
            SegmentType stype = Segment::segmentType(CLEF);
            Segment* s = measure->findSegment(stype, tick);
            if (!s) {
                  s = measure->createSegment(stype, tick);
                  _score->undoAddElement(s);
                  }
            clef->setParent(s);
            _score->undoAddElement(clef);
            }
      _score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   height
//---------------------------------------------------------

double Staff::height() const
      {
      return 4 * _score->spatium() * mag();
      }

//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

double Staff::spatium() const
      {
      return _score->spatium() * mag();
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double Staff::mag() const
      {
      return _small ? score()->styleD(ST_smallStaffMag) : 1.0;
      }

//---------------------------------------------------------
//   setKey
//---------------------------------------------------------

void Staff::setKey(int tick, int st)
      {
      KeySigEvent ke;
      ke.setAccidentalType(st);
      (*_keymap)[tick] = ke;
      }

void Staff::setKey(int tick, const KeySigEvent& st)
      {
      (*_keymap)[tick] = st;
      }

//---------------------------------------------------------
//   setClef
//---------------------------------------------------------

void Staff::setClef(int tick, int clef)
      {
      (*_clefList)[tick] = clef;
      }

