//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
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
#include "tablature.h"
#include "stafftype.h"
#include "undo.h"

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
      _score          = s;
      _rstaff         = rs;
      _part           = p;
      _clefList       = new ClefList;
      _keymap         = new KeyList;
      (*_keymap)[0]   = KeySigEvent(0);                  // default to C major
      _staffType      = _score->staffTypes()[PITCHED_STAFF_TYPE];
      _show           = true;
      _small          = false;
      _invisible      = false;
      _barLineSpan    = 1;
      _updateClefList = true;
      _updateKeymap   = true;
      _linkedStaves   = 0;
      }

//---------------------------------------------------------
//   ~Staff
//---------------------------------------------------------

Staff::~Staff()
      {
      if (_linkedStaves) {
            _linkedStaves->remove(this);
            if (_linkedStaves->isEmpty())
                  delete _linkedStaves;
            }
      delete _clefList;
      delete _keymap;
      _keymap   = 0;      // DEBUG
      _clefList = 0;
      }

//---------------------------------------------------------
//   Staff::clef
//---------------------------------------------------------

ClefType Staff::clef(int tick) const
      {
      ClefTypeList ctl = _clefList->clef(tick);
      ClefType ct = score()->concertPitch() ? ctl._concertClef : ctl._transposingClef;
      return ct;
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
      int idx = score()->staffIdx(this);
      xml.stag(QString("Staff id=\"%1\"").arg(idx+1));
      if (linkedStaves()) {
            Score* s = score();
            if (s->parentScore())
                  s = s->parentScore();
            foreach(Staff* staff, linkedStaves()->staves()) {
                  if ((staff->score() == s) && (staff != this))
                        xml.tag("linkedTo", s->staffIdx(staff) + 1);
                  }
            }
      xml.tag("type", score()->staffTypes().indexOf(_staffType));
      if (small() && !xml.excerptmode)    // switch small staves to normal ones when extracting part
            xml.tag("small", small());
      if (invisible())
            xml.tag("invisible", invisible());
      foreach(const BracketItem& i, _brackets)
            xml.tagE("bracket type=\"%d\" span=\"%d\"", i._bracket, i._bracketSpan);
      if (_barLineSpan != 1)
            xml.tag("barLineSpan", _barLineSpan);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Staff::read(QDomElement e)
      {
      setSmall(false);
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            int v = e.text().toInt();
            if (tag == "type") {
                  StaffType* st = score()->staffTypes().value(v);
                  if (st)
                        _staffType = st;
                  }
            else if (tag == "lines")
                  ;                       // obsolete: setLines(v);
            else if (tag == "small")
                  setSmall(v);
            else if (tag == "invisible")
                  setInvisible(v);
            else if (tag == "slashStyle")
                  ;                       // obsolete: setSlashStyle(v);
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
            else if (tag == "linkedTo") {
                  v -= 1;
                  //
                  // if this is an excerpt, link staff to parentScore()
                  //
                  if (score()->parentScore()) {
                        linkTo(score()->parentScore()->staff(v));
                        }
                  else {
                        int idx = score()->staffIdx(this);
                        if (v < idx)
                              linkTo(score()->staff(v));
                        }
                  }
            else
                  domError(e);
            }
      //
      // for compatibility with old scores:
      //
      if (!_clefList->empty()) {
            ClefType ct = clef(0);
            if (ct == CLEF_PERC2 || ct == CLEF_PERC)
                  _staffType = staffTypes[PERCUSSION_STAFF_TYPE];
            }
      }

#if 0
//---------------------------------------------------------
//   changeKeySig
///   Change key signature.
/// change key signature at tick into subtype st for all staves
/// in response to gui command (drop keysig on measure or keysig)
//---------------------------------------------------------

void Staff::changeKeySig(int tick, KeySigEvent st)
      {
// printf("Staff::changeKeySig "); st.print(); printf("\n");

      Measure* measure = _score->tick2measure(tick);
      if (!measure) {
            printf("measure for tick %d not found!\n", tick);
            return;
            }
      Segment* s = measure->findSegment(SegKeySig, tick);
      if (!s) {
            s = new Segment(measure, SegKeySig, tick);
            _score->undoAddElement(s);
            }
      int track = idx() * VOICES;
      KeySig* ks = static_cast<KeySig*>(s->element(track));

      KeySig* nks = new KeySig(score());
      nks->setTrack(track);
      nks->changeKeySigEvent(st);
      nks->setParent(s);

      if (ks)
            _score->undoChangeElement(ks, nks);
      else
            _score->undoAddElement(nks);
      }
#endif

//---------------------------------------------------------
//   height
//---------------------------------------------------------

double Staff::height() const
      {
      return (lines()-1) * spatium() * _staffType->lineDistance().val();
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
//   removeKey
//---------------------------------------------------------

void Staff::removeKey(int tick)
      {
      _keymap->erase(tick);
      }

//---------------------------------------------------------
//   setClef
//---------------------------------------------------------

void Staff::setClef(int tick, const ClefTypeList& cl)
      {
      _clefList->setClef(tick, cl);
      }

void Staff::setClef(int tick, const ClefType& ct)
      {
      _clefList->setClef(tick, ClefTypeList(ct, ct));
      }

//---------------------------------------------------------
//   channel
//---------------------------------------------------------

int Staff::channel(int tick,  int voice) const
      {
      if (_channelList[voice].isEmpty())
            return 0;
      QMap<int, int>::const_iterator i = _channelList[voice].lowerBound(tick);
      if (i == _channelList[voice].begin())
            return _channelList[voice].begin().value();
      --i;
      return i.value();
      }

//---------------------------------------------------------
//   lines
//---------------------------------------------------------

int Staff::lines() const
      {
      if (useTablature())
            return part()->instr()->tablature()->strings();
      return _staffType->lines();
      }

//---------------------------------------------------------
//   slashStyle
//---------------------------------------------------------

bool Staff::slashStyle() const
      {
      return _staffType->slashStyle();
      }

//---------------------------------------------------------
//   setSlashStyle
//---------------------------------------------------------

void Staff::setSlashStyle(bool val)
      {
      _staffType->setSlashStyle(val);
      }

//---------------------------------------------------------
//   setLines
//---------------------------------------------------------

void Staff::setLines(int val)
      {
      _staffType->setLines(val);
      }

//---------------------------------------------------------
//   useTablature
//---------------------------------------------------------

bool Staff::useTablature() const
      {
      return _staffType->group() == TAB_STAFF;
      }

//---------------------------------------------------------
//   setUseTablature
//---------------------------------------------------------

void Staff::setUseTablature(bool val)
      {
      _staffType = score()->staffTypes()[val ? TAB_STAFF_TYPE : PITCHED_STAFF_TYPE];
      }

//---------------------------------------------------------
//   linkTo
//---------------------------------------------------------

void Staff::linkTo(Staff* staff)
      {
      if (!_linkedStaves) {
            if (staff->linkedStaves()) {
                  _linkedStaves = staff->linkedStaves();
                  }
            else {
                  _linkedStaves = new LinkedStaves;
                  _linkedStaves->add(staff);
                  staff->setLinkedStaves(_linkedStaves);
                  }
            _linkedStaves->add(this);
            }
      else {
            printf("Staff::linkTo: staff already linked\n");
            abort();
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void LinkedStaves::add(Staff* staff)
      {
      _staves.append(staff);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void LinkedStaves::remove(Staff* staff)
      {
      _staves.removeOne(staff);
      }

//---------------------------------------------------------
//   primaryStaff
///   if there are linked staves, the primary staff is
///   the one who is played back
//---------------------------------------------------------

bool Staff::primaryStaff() const
      {
      QList<Staff*> s;
      if (!_linkedStaves)
            return true;
      foreach(Staff* staff, _linkedStaves->staves()) {
            if (staff->score() == score())
                  s.append(staff);
            }
      return s.front() == this;
      }

//---------------------------------------------------------
//   setStaffType
//---------------------------------------------------------

void Staff::setStaffType(StaffType* st)
      {
      if (_staffType == st)
            return;
      _staffType = st;

      //
      //    check for right clef-type and fix
      //    if necessary
      //
      ClefType ct = clef(0);
      StaffGroup csg = clefTable[ct].staffGroup;

      if (_staffType->group() != csg) {
            _clefList->clear();
            switch(_staffType->group()) {
                  case TAB_STAFF:        ct = CLEF_TAB2; break;
                  case PITCHED_STAFF:    ct = CLEF_G; break;      // TODO: use preferred clef for instrument
                  case PERCUSSION_STAFF: ct = CLEF_PERC; break;
                  }
            }
      }

