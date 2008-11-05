//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: select.cpp,v 1.47 2006/03/22 12:04:14 wschweer Exp $
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
#include "padstate.h"
#include "system.h"
#include "select.h"
#include "sig.h"
#include "utils.h"
#include "text.h"
#include "segment.h"
#include "input.h"
#include "measure.h"
#include "layout.h"
#include "page.h"
#include "barline.h"
#include "xml.h"
#include "lyrics.h"
#include "values.h"
#include "tuplet.h"

//---------------------------------------------------------
//   Selection
//---------------------------------------------------------

Selection::Selection(Score* s)
      {
      _score = s;
      _state = SEL_NONE;
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

bool Selection::isStartActive() const {
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
      if (!_activeSegment)
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
      sel->remove(obj);
      obj->setSelected(false);
      }

//---------------------------------------------------------
//   updateSelectedElements
//---------------------------------------------------------

void Score::updateSelectedElements(SelState state)
      {
      setUpdateAll();
      QList<Element*>* el = sel->elements();
      foreach(Element* e, *el)
            e->setSelected(false);
      el->clear();

      //
      //  select all elements in range
      //
      if (state == SEL_SYSTEM) {
            sel->staffStart = 0;
            sel->staffEnd = nstaves();
            }
      // assert:
      if (sel->staffStart < 0 || sel->staffStart >= nstaves() || sel->staffEnd < 0 || sel->staffEnd > nstaves()
         || sel->staffStart >= sel->staffEnd) {
            printf("updateSelectedElements: bad staff selection %d - %d\n", sel->staffStart, sel->staffEnd);
            sel->staffStart = 0;
            sel->staffEnd   = 1;
            }
      int startTrack = sel->staffStart * VOICES;
      int endTrack   = sel->staffEnd * VOICES;

      for (int st = startTrack; st < endTrack; ++st) {
            for (Segment* s = sel->startSegment(); s && (s != sel->endSegment()); s = s->nextCR()) {
                  Element* e = s->element(st);
                  if (!e)
                        continue;
                  e->setSelected(true);
                  sel->elements()->append(e);
                  }
            }
      }

//---------------------------------------------------------
//   select
//    staff is valid, if obj is of type MEASURE
//---------------------------------------------------------

void Score::select(Element* e, SelectType type, int staffIdx)
      {
//printf("select element <%s> type %d(state %d) staff %d\n",
//   e ? e->name() : "", type, sel->state(), e ? e->staffIdx() : -1);

      SelState selState = sel->state();

      if (type == SELECT_SINGLE) {
            refresh |= sel->deselectAll(this);
            if (e && (e->type() == MEASURE)) {
                  select(e, SELECT_RANGE, staffIdx);
                  return;
                  }
            if (e == 0) {
                  selState = SEL_NONE;
                  _padState.len = 0;
                  updateAll = true;
                  }
            else {
                  refresh |= e->abbox();
                  if (e->selected()) {
                        sel->remove(e);
                        selState = SEL_NONE;
                        }
                  else {
                        sel->add(e);
                        _is.track = e->track();
                        selState = SEL_SINGLE;
                        }
                  if (e->type() == NOTE || e->type() == REST) {
                        ChordRest* cr = static_cast<ChordRest*>(e);
                        if (e->type() == NOTE)
                              cr = static_cast<ChordRest*>(e->parent());
                        _is.setPos(cr->tick());
                        _is.cr = cr;
                        emit posChanged(_is.pos());
                        }
                  }
            }
      else if (type == SELECT_ADD) {
            if (e->type() == MEASURE) {
                  Measure* m = static_cast<Measure*>(e);
                  int tick  = m->tick();
                  int etick = tick + m->tickLen();
                  if (sel->state() == SEL_NONE) {
                        sel->setStartSegment(m->tick2segment(tick));
                        sel->setEndSegment(tick2segment(etick));
                        }
                  else if (sel->state() == SEL_SYSTEM) {
                        if (tick < sel->tickStart()) {
                              sel->setStartSegment(m->tick2segment(tick));
                              }
                        else if (etick >= sel->tickEnd())
                              sel->setEndSegment(tick2segment(etick));
                        }
                  else {
                        select(0, SELECT_SINGLE, 0);
                        return;
                        }
                  updateAll = true;
                  selState = SEL_SYSTEM;
                  updateSelectedElements(selState);
                  _padState.len = 0;
                  }
            else {
                  if (sel->state() == SEL_STAFF || sel->state() == SEL_SYSTEM) {
                        select(0, SELECT_SINGLE, 0);
                        return;
                        }
                  else {
                        refresh |= e->abbox();
                        sel->add(e);
                        _padState.len = 0;
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
                  if (sel->state() == SEL_NONE) {
                        sel->staffStart = staffIdx;
                        sel->staffEnd = staffIdx + 1;
                        sel->setStartSegment(m->tick2segment(tick));
                        sel->setEndSegment(tick2segment(etick));
                        }
                  else if (sel->state() == SEL_STAFF) {
                        if (staffIdx < sel->staffStart)
                              sel->staffStart = staffIdx;
                        else if (staffIdx >= sel->staffEnd)
                              sel->staffEnd = staffIdx + 1;
                        if (tick < sel->tickStart()) {
                              sel->setStartSegment(m->tick2segment(tick));
                              activeIsFirst = true;
                              }
                        else if (etick >= sel->tickEnd())
                              sel->setEndSegment(tick2segment(etick));
                        else {
                              if (sel->activeSegment() == sel->startSegment()) {
                                    sel->setStartSegment(m->tick2segment(tick));
                                    activeIsFirst = true;
                                    }
                              else
                                    sel->setEndSegment(tick2segment(etick));
                              }
                        }
                  else if (sel->state() == SEL_SINGLE) {
                        Segment* seg = 0;
                        Element* oe = sel->element();
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
                        printf("SELECT_RANGE: measure: sel state %d\n", sel->state());
                        }
                  }
            else if (e->type() == NOTE || e->type() == REST || e->type() == CHORD) {
                  if (e->type() == NOTE)
                        e = static_cast<Note*>(e)->chord();
                  ChordRest* cr = static_cast<ChordRest*>(e);

                  if (sel->state() == SEL_NONE) {
                        sel->staffStart = e->staffIdx();
                        sel->staffEnd   = sel->staffStart + 1;
                        sel->setStartSegment(cr->segment());
                        sel->setEndSegment(cr->segment()->nextCR());
                        }
                  else if (sel->state() == SEL_SINGLE) {
                        Element* oe = sel->element();
                        if (oe->type() == NOTE || oe->type() == REST || oe->type() == CHORD) {
                              if (oe->type() == NOTE)
                                    oe = oe->parent();
                              ChordRest* ocr = static_cast<ChordRest*>(oe);
                              sel->staffStart = oe->staffIdx();
                              sel->staffEnd   = sel->staffStart + 1;
                              sel->setStartSegment(ocr->segment());
                              sel->setEndSegment(ocr->segment()->nextCR());
                              if (!sel->endSegment())
                                    sel->setEndSegment(ocr->segment()->next());

                              staffIdx = cr->staffIdx();
                              int tick = cr->tick();
                              if (staffIdx < sel->staffStart)
                                    sel->staffStart = staffIdx;
                              else if (staffIdx >= sel->staffEnd)
                                    sel->staffEnd = staffIdx + 1;
                              if (tick < sel->tickStart()) {
                                    sel->setStartSegment(cr->segment());
                                    activeIsFirst = true;
                                    }
                              else if (tick >= sel->tickEnd())
                                    sel->setEndSegment(cr->segment()->nextCR());
                              else {
                                    if (sel->activeSegment() == sel->startSegment()) {
                                          sel->setStartSegment(cr->segment());
                                          activeIsFirst = true;
                                          }
                                    else
                                          sel->setEndSegment(cr->segment()->nextCR());
                                    }
                              }
                        else {
printf("select: TODO\n");
                              }
                        }
                  else if (sel->state() == SEL_STAFF) {
                        staffIdx = cr->staffIdx();
                        int tick = cr->tick();
                        if (staffIdx < sel->staffStart)
                              sel->staffStart = staffIdx;
                        else if (staffIdx >= sel->staffEnd)
                              sel->staffEnd = staffIdx + 1;
                        if (tick < sel->tickStart()) {
                              if (sel->activeSegment() == sel->endSegment())
                                    sel->setEndSegment(sel->startSegment());
                              sel->setStartSegment(cr->segment());
                              activeIsFirst = true;
                              }
                        else if (tick >= sel->tickEnd()) {
                              if (sel->activeSegment() == sel->startSegment())
                                    sel->setStartSegment(sel->endSegment());
                              sel->setEndSegment(cr->segment()->nextCR());
                              }
                        else {
                              if (sel->activeSegment() == sel->startSegment()) {
                                    sel->setStartSegment(cr->segment());
                                    activeIsFirst = true;
                                    }
                              else
                                    sel->setEndSegment(cr->segment()->nextCR());
                              }
                        }
                  else {
                        printf("sel state %d\n", sel->state());
                        }
                  selState = SEL_STAFF;
                  if (!sel->endSegment())
                        sel->setEndSegment(cr->segment()->next());
                  }
            else {
                  select(e, SELECT_SINGLE, staffIdx);
                  return;
                  }

// printf("range %d-%d %d-%d\n", sel->staffStart, sel->staffEnd, sel->tickStart(), sel->tickEnd());
            if (activeIsFirst)
                  sel->setActiveSegment(sel->startSegment());
            else
                  sel->setActiveSegment(sel->endSegment());

            sel->activeTrack = activeTrack;

            selState = SEL_STAFF;
            updateSelectedElements(selState);
            _padState.len = 0;
            }

      sel->setState(selState);
      emit selectionChanged(int(sel->state()));
      }

//---------------------------------------------------------
//   lassoSelect
//---------------------------------------------------------

void Score::lassoSelect(const QRectF& bbox)
      {
      select(0, SELECT_SINGLE, 0);
      QRectF lr(bbox.normalized());
      QList<const Element*> el = _layout->items(lr);
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

      foreach(const Element* e, *(sel->elements())) {
            if (e->type() == NOTE || e->type() == REST) {
                  ++noteRestCount;
                  if (e->type() == NOTE)
                        e = e->parent();
                  Segment* seg = static_cast<const ChordRest*>(e)->segment();
                  if ((startSegment == 0) || (e->tick() < startSegment->tick()))
                        startSegment = seg;
                  if ((endSegment == 0) || (e->tick() > endSegment->tick()))
                        endSegment = seg;
                  int idx = e->staffIdx();
                  if (idx < startStaff)
                        startStaff = idx;
                  if (idx > endStaff)
                        endStaff = idx;
                  }
            }
      if (noteRestCount > 0) {
            endSegment = endSegment->nextCR();
            sel->setRange(startSegment, endSegment, startStaff, endStaff+1);
            if (sel->state() != SEL_STAFF) {
                  sel->setState(SEL_STAFF);
                  emit selectionChanged(int(sel->state()));
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
      for(MeasureBase* m = _layout->first(); m; m = m->next())
            m->collectElements(l);
      foreach(const Page* page, _layout->pages())
            page->collectElements(l);
      sel->clear();
      foreach(const Element* e, l) {
            if (e->selected()) {
                  sel->append(const_cast<Element*>(e));
                  }
            }
      sel->updateState();
      emit selectionChanged(int(sel->state()));
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
            int n = _el.size();
            if (n == 0)
                  setState(SEL_NONE);
            else if (n == 1) {
                  setState(SEL_SINGLE);
                  Element* e = element();
                  if (e->type() == NOTE || e->type() == REST) {
                        if (!e->score()->noteEntryMode())
                              e->score()->setPadState(e);
                        if (e->type() == NOTE)
                              e = e->parent();
                        e->score()->setInputTrack(e->track());
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
                  a = staffMimeData();
                  break;
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

