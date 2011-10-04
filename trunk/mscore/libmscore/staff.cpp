//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "mscore.h"
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
#include "cleflist.h"
#include "timesig.h"
#include "instrtemplate.h"

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
      _keymap         = new KeyList;
      (*_keymap)[0]   = KeySigEvent(0);                  // default to C major
      _staffType      = _score->staffTypes()[PITCHED_STAFF_TYPE];
      _show           = true;
      _small          = false;
      _invisible      = false;
      _barLineSpan    = 1;
      _updateKeymap   = true;
      _linkedStaves   = 0;
      _initialClef    = ClefTypeList(CLEF_G, CLEF_G);
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
      delete _keymap;
      }

//---------------------------------------------------------
//   Staff::clefTypeList
//---------------------------------------------------------

ClefTypeList Staff::clefTypeList(int tick) const
      {
      ClefTypeList ctl = _initialClef;
      int track  = idx() * VOICES;
      for (Segment* s = score()->firstSegment(); s; s = s->next1()) {
            if (s->tick() > tick)
                  break;
            if (s->subtype() != SegClef)
                  continue;
            if (s->element(track) && !s->element(track)->generated())
                  ctl = static_cast<Clef*>(s->element(track))->clefTypeList();
            }
      return ctl;
      }

//---------------------------------------------------------
//   Staff::clef
//---------------------------------------------------------

ClefType Staff::clef(int tick) const
      {
      Clef* clef = 0;
      foreach(Clef* c, clefs) {
            if (c->segment()->tick() > tick)
                  break;
            clef = c;
            }
      return clef == 0 ? CLEF_G : clef->clefType();
      }

ClefType Staff::clef(Segment* segment) const
      {
      ClefType ct = CLEF_G;
      int track = idx() * VOICES;
      for (;;) {
            segment = segment->prev1(SegClef);
            if (segment == 0)
                  break;
            if (segment->element(track)) {
                  ct = static_cast<Clef*>(segment->element(track))->clefType();
                  break;
                  }
            }
      return ct;
      }

//---------------------------------------------------------
//   timeStretch
//---------------------------------------------------------

Fraction Staff::timeStretch(int tick) const
      {
      TimeSig* timesig = timeSig(tick);
      return timesig == 0 ? Fraction(1,1) : timesig->stretch();
      }

//---------------------------------------------------------
//   timeSig
//---------------------------------------------------------

TimeSig* Staff::timeSig(int tick) const
      {
      TimeSig* timesig = 0;
      foreach(TimeSig* ts, timesigs) {
            if (ts->segment()->tick() > tick)
                  break;
            timesig = ts;
            }
      return timesig;
      }

//---------------------------------------------------------
//   clefsGreater
//---------------------------------------------------------

static bool clefsGreater(const Clef* a, const Clef* b)
      {
      return a->segment()->tick() < b->segment()->tick();
      }

//---------------------------------------------------------
//   addClef
//---------------------------------------------------------

void Staff::addClef(Clef* clef)
      {
      if (clef->generated())
            return;
      if (clef->segment()->measure() == 0)
            abort();
      int tick = 0;
      if (!clefs.isEmpty()) {
            tick = clefs.back()->segment()->tick();
            }
      clefs.append(clef);
      if (clef->segment()->tick() < tick)
            qSort(clefs.begin(), clefs.end(), clefsGreater);
      }

//---------------------------------------------------------
//   timesigsGreater
//---------------------------------------------------------

static bool timesigsGreater(const TimeSig* a, const TimeSig* b)
      {
      return a->segment()->tick() < b->segment()->tick();
      }

//---------------------------------------------------------
//   addTimeSig
//---------------------------------------------------------

void Staff::addTimeSig(TimeSig* timesig)
      {
      int tick = 0;
      if (!timesigs.isEmpty())
            tick = timesigs.back()->segment()->tick();
      timesigs.append(timesig);
      if (timesig->segment()->tick() < tick)
            qSort(timesigs.begin(), timesigs.end(), timesigsGreater);
      }

//---------------------------------------------------------
//   removeClef
//---------------------------------------------------------

void Staff::removeClef(Clef* clef)
      {
      clefs.removeOne(clef);
      }

//---------------------------------------------------------
//   removeTimeSig
//---------------------------------------------------------

void Staff::removeTimeSig(TimeSig* timesig)
      {
      timesigs.removeOne(timesig);
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
                  _clefList.read(e, _score);
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
/*      if (!_clefList->empty()) {
            ClefType ct = clef(0);
            if (ct == CLEF_PERC2 || ct == CLEF_PERC)
                  _staffType = staffTypes[PERCUSSION_STAFF_TYPE];
            }
*/
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

qreal Staff::height() const
      {
      return (lines()-1) * spatium() * _staffType->lineDistance().val();
      }

//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

qreal Staff::spatium() const
      {
      return _score->spatium() * mag();
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Staff::mag() const
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

      setKey(tick, ke);
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
//      if (useTablature())
//            return part()->instr()->tablature()->strings();
      return _staffType->lines();
      }

//---------------------------------------------------------
//   setLines
//---------------------------------------------------------

void Staff::setLines(int val)
      {
      if (val == lines())
            return;
      //
      // create new staff type
      //
      StaffType* st = _staffType->clone();
      st->setLines(val);
      _staffType = st;
      score()->staffTypes().append(st);
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
//   useTablature
//---------------------------------------------------------

bool Staff::useTablature() const
      {
      return _staffType->group() == TAB_STAFF;
      }

//---------------------------------------------------------
//   setUseTablature
//---------------------------------------------------------

#if 0
void Staff::setUseTablature(bool val)
      {
      _staffType = score()->staffTypes()[val ? TAB_STAFF_TYPE : PITCHED_STAFF_TYPE];
      }
#endif

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
//            _clefList->clear();
            switch(_staffType->group()) {
                  case TAB_STAFF:        ct = ClefType(score()->styleI(ST_tabClef)); break;
                  case PITCHED_STAFF:    ct = CLEF_G; break;      // TODO: use preferred clef for instrument
                  case PERCUSSION_STAFF: ct = CLEF_PERC; break;
                  }
            }
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Staff::init(const InstrumentTemplate* t, int cidx)
      {
      if (cidx > MAX_STAVES) {
            setLines(5);
            setSmall(false);
            setInitialClef(t->clefIdx[0]);
            }
      else {
            if (useTablature())
                  setLines(t->tablature->strings());
            else
                  setLines(t->staffLines[cidx]);
            setSmall(t->smallStaff[cidx]);
            setInitialClef(t->clefIdx[cidx]);
            setBracket(0, t->bracket[cidx]);
            setBracketSpan(0, t->bracketSpan[cidx]);
            setBarLineSpan(t->barlineSpan[cidx]);
            }

      StaffType* st;
      if (t->useTablature && t->tablature)
            st = score()->staffTypes().at(TAB_STAFF_TYPE);
      else if (t->useDrumset)
            st = score()->staffTypes().at(PERCUSSION_STAFF_TYPE);
      else
            st = score()->staffTypes().at(PITCHED_STAFF_TYPE);

// printf("init %d %d\n", _initialClef._concertClef, _initialClef._transposingClef);

      if (t->staffLines[cidx] != st->lines()) {
            // create new staff type:
            st = st->clone();
            st->setLines(t->staffLines[cidx]);
            score()->staffTypes().append(st);
            }
      setStaffType(st);
      }

