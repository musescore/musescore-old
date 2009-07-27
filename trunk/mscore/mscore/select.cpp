//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

/**
 \file
 Implementation of class Selection plus other selection related functions.
*/

#include "globals.h"
#include "canvas.h"
#include "note.h"
#include "chord.h"
#include "rest.h"
#include "score.h"
#include "slur.h"
#include "system.h"
#include "select.h"
#include "sig.h"
#include "utils.h"
#include "text.h"
#include "segment.h"
#include "input.h"
#include "measure.h"
#include "page.h"
#include "barline.h"
#include "xml.h"
#include "lyrics.h"
#include "limits.h"
#include "tuplet.h"

//---------------------------------------------------------
//   Selection
//---------------------------------------------------------

Selection::Selection(Score* s)
      {
      _score         = s;
      _state         = SEL_NONE;
      _startSegment  = 0;
      _endSegment    = 0;
      _activeSegment = 0;
      staffStart     = 0;
      staffEnd       = 0;
      activeTrack    = 0;
      }

//---------------------------------------------------------
//   tickStart
//---------------------------------------------------------

int Selection::tickStart() const
      {
      return _startSegment->tick();
      }

//---------------------------------------------------------
//   tickEnd
//---------------------------------------------------------

int Selection::tickEnd() const
      {
      return _endSegment->tick();
      }

//---------------------------------------------------------
//   isStartActive
//---------------------------------------------------------

bool Selection::isStartActive() const
      {
      return activeSegment() && activeSegment()->tick() == startSegment()->tick();
      }

//---------------------------------------------------------
//   isEndActive
//---------------------------------------------------------

bool Selection::isEndActive() const {
      return activeSegment() && activeSegment()->tick() == endSegment()->tick();
      }

//---------------------------------------------------------
//   element
//---------------------------------------------------------

Element* Selection::element() const
      {
      if (_state == SEL_SINGLE)
            return _el.front();
      return 0;
      }

//---------------------------------------------------------
//   activeCR
//---------------------------------------------------------

ChordRest* Selection::activeCR() const
      {
      if ((_state != SEL_STAFF && _state != SEL_SYSTEM) || !_activeSegment)
            return 0;
      if (_activeSegment == _startSegment)
            return firstChordRest(activeTrack);
      else
            return lastChordRest(activeTrack);
      }

//---------------------------------------------------------
//   firstChordRest
//---------------------------------------------------------

ChordRest* Selection::firstChordRest(int track) const
      {
      ChordRest* cr = 0;
      for (ciElement i = _el.begin(); i != _el.end(); ++i) {
            Element* el = *i;
            if (el->type() == NOTE) {
                  el = ((Note*)el)->chord();
                  }
            if (el->isChordRest()) {
                  if (track != -1 && el->track() != track)
                        continue;
                  if (cr) {
                        if (((ChordRest*)el)->tick() < cr->tick())
                              cr = (ChordRest*)el;
                        }
                  else
                        cr = (ChordRest*)el;
                  }
            }
      return cr;
      }

//---------------------------------------------------------
//   lastChordRest
//---------------------------------------------------------

ChordRest* Selection::lastChordRest(int track) const
      {
      ChordRest* cr = 0;
      for (ciElement i = _el.begin(); i != _el.end(); ++i) {
            Element* el = *i;
            if (el->type() == NOTE) {
                  el = ((Note*)el)->chord();
                  }
            if (el->isChordRest()) {
                  if (track != -1 && el->track() != track)
                        continue;
                  if (cr) {
                        if (((ChordRest*)el)->tick() > cr->tick())
                              cr = (ChordRest*)el;
                        }
                  else
                        cr = (ChordRest*)el;
                  }
            }
      return cr;
      }

//---------------------------------------------------------
//   deselectAll
//---------------------------------------------------------

QRectF Selection::deselectAll(Score* cs)
      {
      QRectF r;
      if (_state == SEL_STAFF || _state == SEL_SYSTEM)
            cs->setUpdateAll();
      return clear();
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

QRectF Selection::clear()
      {
      QRectF r;
      foreach(Element* e, _el) {
            r |= e->abbox();
            e->setSelected(false);
            r |= e->abbox();
            }
      _el.clear();
      setState(SEL_NONE);
      return r;
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Selection::remove(Element* el)
      {
      _el.removeAll(el);
      el->setSelected(false);
      updateState();
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Selection::add(Element* el)
      {
      _el.append(el);
      update();
      }

//---------------------------------------------------------
//   deselect
//---------------------------------------------------------

void Score::deselect(Element* obj)
      {
      refresh |= obj->abbox();
      selection()->remove(obj);
      obj->setSelected(false);
      }

//---------------------------------------------------------
//   updateSelectedElements
//---------------------------------------------------------

void Score::updateSelectedElements(SelState state)
      {
      setUpdateAll();
      QList<Element*>* el = selection()->elements();
      foreach(Element* e, *el)
            e->setSelected(false);
      el->clear();

      //
      //  select all elements in range
      //
      if (state == SEL_SYSTEM) {
            _selection->staffStart = 0;
            _selection->staffEnd = nstaves();
            }
      // assert:
      if (_selection->staffStart < 0 || _selection->staffStart >= nstaves() || _selection->staffEnd < 0 || _selection->staffEnd > nstaves()
         || _selection->staffStart >= _selection->staffEnd) {
            printf("updateSelectedElements: bad staff selection %d - %d\n", _selection->staffStart, _selection->staffEnd);
            _selection->staffStart = 0;
            _selection->staffEnd   = 1;
            }
      int startTrack = _selection->staffStart * VOICES;
      int endTrack   = _selection->staffEnd * VOICES;

      for (int st = startTrack; st < endTrack; ++st) {
            for (Segment* s = _selection->startSegment(); s && (s != _selection->endSegment()); s = s->next1()) {
                  Element* e = s->element(st);
                  if (!e)
                        continue;
                  e->setSelected(true);
                  _selection->elements()->append(e);
                  }
            }
      }

//---------------------------------------------------------
//   select
//    staff is valid, if obj is of type MEASURE
//---------------------------------------------------------

void Score::select(Element* e, SelectType type, int staffIdx)
      {
// printf("select element <%s> type %d(state %d) staff %d\n",
//   e ? e->name() : "", type, selection()->state(), e ? e->staffIdx() : -1);

      SelState selState = _selection->state();

      if (type == SELECT_SINGLE) {
            refresh |= _selection->deselectAll(this);
            if (e == 0) {
                  selState = SEL_NONE;
                  if (!noteEntryMode())
                        //_is.duration.setVal(Duration::V_INVALID);
                  updateAll = true;
                  }
            else {
                  if (e->type() == MEASURE) {
                        select(e, SELECT_RANGE, staffIdx);
                        return;
                        }
                  refresh |= e->abbox();
                  if (e->selected()) {
                        _selection->remove(e);
                        selState = SEL_NONE;
                        }
                  else {
                        _selection->add(e);
                        _is.track = e->track();
                        selState = SEL_SINGLE;
                        }
                  if (e->type() == NOTE || e->type() == REST) {
                        _is.cr = static_cast<ChordRest*>(e->type() == NOTE ? e->parent() : e);
                        emit posChanged(_is.pos());
                        }
                  }
            }
      else if (type == SELECT_ADD) {
            if (e->type() == MEASURE) {
                  Measure* m = static_cast<Measure*>(e);
                  int tick  = m->tick();
                  int etick = tick + m->tickLen();
                  if (_selection->state() == SEL_NONE) {
                        _selection->setStartSegment(m->tick2segment(tick, true));
                        _selection->setEndSegment(tick2segment(etick));
                        }
                  else if (_selection->state() == SEL_SYSTEM) {
                        if (tick < _selection->tickStart()) {
                              _selection->setStartSegment(m->tick2segment(tick, true));
                              }
                        else if (etick >= _selection->tickEnd())
                              _selection->setEndSegment(tick2segment(etick));
                        }
                  else {
                        select(0, SELECT_SINGLE, 0);
                        return;
                        }
                  updateAll = true;
                  selState = SEL_SYSTEM;
                  updateSelectedElements(selState);
                  //_is.duration.setVal(Duration::V_INVALID);
                  }
            else {
                  if (_selection->state() == SEL_STAFF || _selection->state() == SEL_SYSTEM) {
                        select(0, SELECT_SINGLE, 0);
                        return;
                        }
                  else {
                        refresh |= e->abbox();
                        _selection->add(e);
                        //_is.duration.setVal(Duration::V_INVALID);
                        selState = SEL_MULT;
                        }
                  }
            }
      else if (type == SELECT_RANGE) {
            bool activeIsFirst = false;
            int activeTrack = e->track();
            if (e->type() == MEASURE) {
                  Measure* m = static_cast<Measure*>(e);
                  int tick  = m->tick();
                  int etick = tick + m->tickLen();
                  activeTrack = staffIdx * VOICES;
                  if (_selection->state() == SEL_NONE) {
                        _selection->staffStart = staffIdx;
                        _selection->staffEnd = staffIdx + 1;
                        _selection->setStartSegment(m->tick2segment(tick, true));
                        _selection->setEndSegment(tick2segment(etick));
                        }
                  else if (_selection->state() == SEL_STAFF) {
                        if (staffIdx < _selection->staffStart)
                              _selection->staffStart = staffIdx;
                        else if (staffIdx >= _selection->staffEnd)
                              _selection->staffEnd = staffIdx + 1;
                        if (tick < _selection->tickStart()) {
                              _selection->setStartSegment(m->tick2segment(tick, true));
                              activeIsFirst = true;
                              }
                        else if (etick >= _selection->tickEnd())
                              _selection->setEndSegment(tick2segment(etick));
                        else {
                              if (_selection->activeSegment() == _selection->startSegment()) {
                                    _selection->setStartSegment(m->tick2segment(tick, true));
                                    activeIsFirst = true;
                                    }
                              else
                                    _selection->setEndSegment(tick2segment(etick));
                              }
                        }
                  else if (_selection->state() == SEL_SINGLE) {
                        Segment* seg = 0;
                        Element* oe = _selection->element();
                        bool reverse = false;
                        if (tick < oe->tick())
                              seg = m->first();
                        else if (etick >= oe->tick() + oe->tickLen()) {
                              seg = m->last();
                              reverse = true;
                              }
                        int track = staffIdx * VOICES;
                        Element* el = 0;
                        // find first or last chord/rest in measure
                        for (;;) {
                              el = seg->element(track);
                              if (el && el->isChordRest())
                                    break;
                              if (reverse)
                                    seg = seg->prev1();
                              else
                                    seg = seg->next1();
                              if (!seg)
                                    break;
                              }
                        if (el)
                              select(el, SELECT_RANGE, staffIdx);
                        return;
                        }
                  else {
                        printf("SELECT_RANGE: measure: sel state %d\n", _selection->state());
                        }
                  }
            else if (e->type() == NOTE || e->type() == REST || e->type() == CHORD) {
                  if (e->type() == NOTE)
                        e = static_cast<Note*>(e)->chord();
                  ChordRest* cr = static_cast<ChordRest*>(e);

                  if (_selection->state() == SEL_NONE) {
                        _selection->staffStart = e->staffIdx();
                        _selection->staffEnd   = _selection->staffStart + 1;
                        _selection->setStartSegment(cr->segment());
                        _selection->setEndSegment(cr->segment()->nextCR(cr->track()));
                        }
                  else if (_selection->state() == SEL_SINGLE) {
                        Element* oe = _selection->element();
                        if (oe->type() == NOTE || oe->type() == REST || oe->type() == CHORD) {
                              if (oe->type() == NOTE)
                                    oe = oe->parent();
                              ChordRest* ocr = static_cast<ChordRest*>(oe);
                              _selection->staffStart = oe->staffIdx();
                              _selection->staffEnd   = _selection->staffStart + 1;
                              _selection->setStartSegment(ocr->segment());
                              _selection->setEndSegment(ocr->segment()->nextCR(ocr->track()));
                              if (!_selection->endSegment())
                                    _selection->setEndSegment(ocr->segment()->next());

                              staffIdx = cr->staffIdx();
                              int tick = cr->tick();
                              if (staffIdx < _selection->staffStart)
                                    _selection->staffStart = staffIdx;
                              else if (staffIdx >= _selection->staffEnd)
                                    _selection->staffEnd = staffIdx + 1;
                              if (tick < _selection->tickStart()) {
                                    _selection->setStartSegment(cr->segment());
                                    activeIsFirst = true;
                                    }
                              else if (tick >= _selection->tickEnd())
                                    _selection->setEndSegment(cr->segment()->nextCR(cr->track()));
                              else {
                                    if (_selection->activeSegment() == _selection->startSegment()) {
                                          _selection->setStartSegment(cr->segment());
                                          activeIsFirst = true;
                                          }
                                    else
                                          _selection->setEndSegment(cr->segment()->nextCR(cr->track()));
                                    }
                              }
                        else {
// printf("select: TODO\n");
                              select(e, SELECT_SINGLE, 0);
                              return;
                              }
                        }
                  else if (_selection->state() == SEL_STAFF) {
                        staffIdx = cr->staffIdx();
                        int tick = cr->tick();
                        if (staffIdx < _selection->staffStart)
                              _selection->staffStart = staffIdx;
                        else if (staffIdx >= _selection->staffEnd)
                              _selection->staffEnd = staffIdx + 1;
                        if (tick < _selection->tickStart()) {
                              if (_selection->activeSegment() == _selection->endSegment())
                                    _selection->setEndSegment(_selection->startSegment());
                              _selection->setStartSegment(cr->segment());
                              activeIsFirst = true;
                              }
                        else if (tick >= _selection->tickEnd()) {
                              if (_selection->activeSegment() == _selection->startSegment())
                                    _selection->setStartSegment(_selection->endSegment());
                              _selection->setEndSegment(cr->segment()->nextCR(cr->track()));
                              }
                        else {
                              if (_selection->activeSegment() == _selection->startSegment()) {
                                    _selection->setStartSegment(cr->segment());
                                    activeIsFirst = true;
                                    }
                              else
                                    _selection->setEndSegment(cr->segment()->nextCR(cr->track()));
                              }
                        }
                  else {
                        printf("sel state %d\n", _selection->state());
                        }
                  selState = SEL_STAFF;
                  if (!_selection->endSegment())
                        _selection->setEndSegment(cr->segment()->next());
                  }
            else {
                  select(e, SELECT_SINGLE, staffIdx);
                  return;
                  }

// printf("range %d-%d %d-%d\n", _selection->staffStart, _selection->staffEnd, _selection->tickStart(), _selection->tickEnd());
            if (activeIsFirst)
                  _selection->setActiveSegment(_selection->startSegment());
            else
                  _selection->setActiveSegment(_selection->endSegment());

            _selection->activeTrack = activeTrack;

            selState = SEL_STAFF;
            updateSelectedElements(selState);
            //_is.duration.setVal(Duration::V_INVALID);
            }
      _selection->setState(selState);
      emit selectionChanged(int(_selection->state()));
      }

//---------------------------------------------------------
//   lassoSelect
//---------------------------------------------------------

void Score::lassoSelect(const QRectF& bbox)
      {
      select(0, SELECT_SINGLE, 0);
      QRectF lr(bbox.normalized());
      QList<const Element*> el = items(lr);
      for (int i = 0; i < el.size(); ++i) {
            const Element* e = el.at(i);
            e->itemDiscovered = 0;
            if (lr.contains(e->abbox())) {
                  if (e->type() != MEASURE)
                        select(const_cast<Element*>(e), SELECT_ADD, 0);
                  }
            }
      }

//---------------------------------------------------------
//   setRange
//---------------------------------------------------------

void Selection::setRange(Segment* a, Segment* b, int c, int d)
      {
      _startSegment = a;
      _endSegment   = b;
      _activeSegment = b;
      staffStart = c;
      staffEnd   = d;
      }

//---------------------------------------------------------
//   lassoSelectEnd
//---------------------------------------------------------

void Score::lassoSelectEnd(const QRectF& /*bbox*/)
      {
      int noteRestCount     = 0;
      Segment* startSegment = 0;
      Segment* endSegment   = 0;
      int startStaff        = 0x7fffffff;
      int endStaff          = 0;
      int endTrack          = 0;

      foreach(const Element* e, *(_selection->elements())) {
            if (e->type() == NOTE || e->type() == REST) {
                  ++noteRestCount;
                  if (e->type() == NOTE)
                        e = e->parent();
                  Segment* seg = static_cast<const ChordRest*>(e)->segment();
                  if ((startSegment == 0) || (e->tick() < startSegment->tick()))
                        startSegment = seg;
                  if ((endSegment == 0) || (e->tick() > endSegment->tick())) {
                        endSegment = seg;
                        endTrack = e->track();
                        }
                  int idx = e->staffIdx();
                  if (idx < startStaff)
                        startStaff = idx;
                  if (idx > endStaff)
                        endStaff = idx;
                  }
            }
      if (noteRestCount > 0) {
            endSegment = endSegment->nextCR(endTrack);
            _selection->setRange(startSegment, endSegment, startStaff, endStaff+1);
            if (_selection->state() != SEL_STAFF) {
                  _selection->setState(SEL_STAFF);
                  emit selectionChanged(int(_selection->state()));
                  }
            }
      updateAll = true;
      }

//---------------------------------------------------------
//   searchSelectedElements
//    "ElementList selected"
//---------------------------------------------------------

/**
 Rebuild list of selected Elements.
*/

void Score::searchSelectedElements()
      {
      QList<const Element*> l;
      for(MeasureBase* m = first(); m; m = m->next())
            m->collectElements(l);
      foreach(const Page* page, pages())
            page->collectElements(l);
      _selection->clear();
      foreach(const Element* e, l) {
            if (e->selected())
                  _selection->append(const_cast<Element*>(e));
            }
      _selection->updateState();
      emit selectionChanged(int(_selection->state()));
      }

//---------------------------------------------------------
//   update
//---------------------------------------------------------

/**
 Set select flag for all Elements in select list.
*/

void Selection::update()
      {
      for (ciElement i = _el.begin(); i != _el.end(); ++i)
            (*i)->setSelected(true);
      updateState();
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Selection::dump()
      {
      printf("Selection dump: ");
      if (_state == SEL_NONE) {
            printf("NONE\n");
            return;
            }
      if (_state == SEL_SINGLE)
            printf("SINGLE\n");
      else
            printf("MULTI\n");
      for (ciElement i = _el.begin(); i != _el.end(); ++i)
            printf("  %p %s\n", *i, (*i)->name());
      }

//---------------------------------------------------------
//   updateState
//---------------------------------------------------------

/**
 Update cis and padState.
*/

void Selection::updateState()
      {
      int n = _el.size();
      if (_state == SEL_NONE || _state == SEL_SINGLE || _state == SEL_MULT) {
            if (n == 0)
                  setState(SEL_NONE);
            else if (n == 1) {
                  setState(SEL_SINGLE);
                  Element* e = element();
                  if (e->type() == NOTE || e->type() == REST) {
                        if (!_score->noteEntryMode())
                              _score->setPadState(e);
                        if (e->type() == NOTE)
                              e = e->parent();
                        _score->setInputTrack(e->track());
                        }
                  }
            else
                  setState(SEL_MULT);
            }
      else if (n == 0)
            setState(SEL_NONE);
      }

//---------------------------------------------------------
//   mimeType
//---------------------------------------------------------

QString Selection::mimeType() const
      {
      switch (_state) {
            default:
            case SEL_NONE:
                  return QString();
            case SEL_SINGLE:
                  return mimeSymbolFormat;
            case SEL_MULT:
                  return mimeSymbolListFormat;
            case SEL_STAFF:
                  return mimeStaffListFormat;
            case SEL_SYSTEM:
                  return mimeMeasureListFormat;
            }
      }

//---------------------------------------------------------
//   mimeData
//---------------------------------------------------------

QByteArray Selection::mimeData() const
      {
      QByteArray a;
      switch (_state) {
            case SEL_SINGLE:
                  a = element()->mimeData(QPointF());
                  break;
            default:
            case SEL_NONE:
                  break;
            case SEL_MULT:
                  break;
            case SEL_STAFF:
            case SEL_SYSTEM:
                  a = staffMimeData();
                  break;
            }
      return a;
      }

//---------------------------------------------------------
//   staffMimeData
//---------------------------------------------------------

QByteArray Selection::staffMimeData() const
      {
      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      Xml xml(&buffer);
      xml.header();
      xml.noSlurs = true;
      xml.clipboardmode = true;

      int ticks  = tickEnd() - tickStart();
      int staves = staffEnd - staffStart;
      xml.stag(QString("StaffList tick=\"%1\" len=\"%2\" staff=\"%3\" staves=\"%4\"").arg(tickStart()).arg(ticks).arg(staffStart).arg(staves));
      Segment* seg1 = _startSegment;
      Segment* seg2 = _endSegment;

      for (int staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
            xml.stag(QString("Staff id=\"%1\"").arg(staffIdx));
            int startTrack = staffIdx * VOICES;
            int endTrack   = startTrack + VOICES;
            for (Segment* seg = seg1; seg && seg != seg2; seg = seg->next1()) {
                  if (seg->subtype() == Segment::SegEndBarLine)
                        continue;
                  if (seg->subtype() == Segment::SegTimeSig)
                        continue;
                  for (int track = startTrack; track < endTrack; ++track) {
                        Element* e = seg->element(track);
                        if (e == 0 || e->generated())
                              continue;
                        if (e->isChordRest()) {
                              ChordRest* cr = static_cast<ChordRest*>(e);
                              Tuplet* tuplet = cr->tuplet();
                              if (tuplet && tuplet->elements().front() == cr) {
                                    tuplet->setId(xml.tupletId++);
                                    tuplet->write(xml);
                                    }
                              }
                        if (e->type() == CHORD) {
                              Chord* c = static_cast<Chord*>(e);
                              c->write(xml, _startSegment->tick(), _endSegment->tick());
                              }
                        else if (e->type() == REST) {
                              Rest* r = static_cast<Rest*>(e);
                              r->write(xml);
                              }
                        else
                              e->write(xml);
                        if ((track % VOICES) == 0) {
                              int staffIdx = track / VOICES;
                              LyricsList* ll = seg->lyricsList(staffIdx);
                              foreach(Lyrics* l, *ll) {
                                    if (l)
                                          l->write(xml);
                                    }
                              }
                        }
                  }
            for (MeasureBase* mb = seg1->measure(); mb; mb = mb->next()) {
                  if (mb->type() != MEASURE)
                        continue;
                  Measure* m = static_cast<Measure*>(mb);
                  if (seg2 && m->tick() >= seg2->tick())
                        break;
                  foreach(Element* e, *m->el()) {
                        if (e->type() == HARMONY) {
                              if ((e->staffIdx() != staffIdx) || (e->tick() < seg1->tick()))
                                    continue;
                              if (seg2 && (e->tick() >= seg2->tick()))
                                    continue;
                              e->write(xml);
                              }
                        }
                  }
            xml.etag();
            }
      xml.etag();
      buffer.close();
      return buffer.buffer();
      }

//---------------------------------------------------------
//   noteList
//---------------------------------------------------------

QList<Note*> Selection::noteList() const
      {
      QList<Note*>nl;

      if (_state == SEL_SINGLE || _state == SEL_MULT) {
            foreach(Element* e, _el) {
                  if (e->type() == NOTE)
                        nl.append(static_cast<Note*>(e));
                  }
            }
      else if (_state == SEL_STAFF || _state == SEL_SYSTEM) {
            for (int staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
                  int startTrack = staffIdx * VOICES;
                  int endTrack   = startTrack + VOICES;
                  for (Segment* seg = _startSegment; seg && seg != _endSegment; seg = seg->next1()) {
                        if (seg->subtype() != Segment::SegChordRest)
                              continue;
                        for (int track = startTrack; track < endTrack; ++track) {
                              Element* e = seg->element(track);
                              if (e == 0 || e->type() != CHORD)
                                    continue;
                              Chord* c = static_cast<Chord*>(e);
                              NoteList* notes = c->noteList();
                              for(iNote i = notes->begin(); i != notes->end(); ++i)
                                    nl.append(i->second);
                              }
                        }
                  }
            }
      return nl;
      }

