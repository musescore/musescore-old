//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: select.cpp,v 1.47 2006/03/22 12:04:14 wschweer Exp $
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
//   firstChordRest
//---------------------------------------------------------

ChordRest* Selection::firstChordRest() const
      {
      ChordRest* cr = 0;
      for (ciElement i = _el.begin(); i != _el.end(); ++i) {
            Element* el = *i;
            if (el->type() == NOTE) {
                  el = ((Note*)el)->chord();
                  }
            if (el->isChordRest()) {
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

ChordRest* Selection::lastChordRest() const
      {
      ChordRest* cr = 0;
      for (ciElement i = _el.begin(); i != _el.end(); ++i) {
            Element* el = *i;
            if (el->type() == NOTE) {
                  el = ((Note*)el)->chord();
                  }
            if (el->isChordRest()) {
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
      _state = SEL_NONE;
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

void Score::updateSelectedElements()
      {
      setUpdateAll();
      QList<Element*>* el = sel->elements();
      foreach(Element* e, *el)
            e->setSelected(false);
      el->clear();

      //
      //  select all elements in range
      //
      int startTrack = sel->staffStart * VOICES;
      int endTrack   = sel->staffEnd * VOICES;
      if (sel->state() == SEL_SYSTEM) {
            startTrack = 0;
            endTrack = nstaves() * VOICES;
            }

      for (MeasureBase* mb = _layout->first(); mb; mb = mb->next()) {
            int ms = mb->tick();
            int me = ms + mb->tickLen();
            if (me < sel->tickStart)
                  continue;
            if (ms >= sel->tickEnd)
                  break;
            if (mb->type() == MEASURE) {
                  Measure* m = (Measure*)mb;
                  for (int st = startTrack; st < endTrack; ++st) {
                        for (Segment* segment = m->first(); segment; segment = segment->next()) {
                              if (segment->tick() < sel->tickStart)
                                    continue;
                              if (segment->tick() >= sel->tickEnd)
                                    break;
                              Element* e = segment->element(st);
                              if (!e)
                                    continue;
                              e->setSelected(true);
                              sel->elements()->append(e);
                              }
                        }
                  }
//            for (int st = sbar; st < ebar; ++st) {
//                  for (Segment* segment = m->first(); segment; segment = segment->next()) {
//TODO                        if (el->lyrics()) {
//                              el->lyrics()->setSelected(true);
//                              sel->elements()->push_back(el->lyrics());
//                              }
//                        }
//                  }
            }
      }

//---------------------------------------------------------
//   select
//    staff is valid, if obj is of type MEASURE
//---------------------------------------------------------

void Score::select(Element* obj, int state, int staff)
      {
// printf("select element <%s> staff %d\n", obj ? obj->name() : "", obj ? obj->staffIdx() : -1);

      if (!(state & Qt::ShiftModifier) || !obj) {
            refresh |= sel->deselectAll(this);
            if (!obj) {
                  sel->setState(SEL_NONE);
                  _padState.len = 0;
                  emit selectionChanged(int(SEL_NONE));
                  updateAll = true;
                  return;
                  }
            }

      if (obj->type() == MEASURE) {
            updateAll      = true;
            Measure* m     = (Measure*)obj;
            int tickStart  = m->tick();
            int tickEnd    = tickStart + m->tickLen();
            int staffStart = staff;
            int staffEnd   = staff+1;

            if ((state & Qt::ShiftModifier)
               && (sel->state() == SEL_SYSTEM || sel->state() == SEL_STAFF)) {
                  if (sel->tickStart < tickStart)
                        tickStart = sel->tickStart;
                  if (sel->tickEnd > tickEnd)
                        tickEnd = sel->tickEnd;
                  if (sel->state() == SEL_STAFF) {
                        if (sel->staffStart < staffStart)
                              staffStart = sel->staffStart;
                        else if (sel->staffEnd > staffEnd)
                              staffEnd = sel->staffEnd;
                        }
                  }
            sel->staffStart = staffStart;
            sel->staffEnd   = staffEnd;
            sel->tickStart  = tickStart;
            sel->tickEnd    = tickEnd;

            SelState selState = SEL_STAFF;

            if (state & Qt::ControlModifier)
                  selState = SEL_SYSTEM;
            else if ((state & Qt::ShiftModifier) && (sel->state() == SEL_SYSTEM))
                  selState = SEL_SYSTEM;

            if (selState == SEL_SYSTEM) {
                  sel->staffStart = 0;
                  sel->staffEnd   = nstaves();
                  }
            if (sel->state() != selState) {
                  sel->setState(selState);
                  emit selectionChanged(int(sel->state()));
                  }
            updateSelectedElements();
            _padState.len = 0;
            return;
            }
      if ((obj->type() == NOTE || obj->type() == REST) && (sel->state() == SEL_STAFF)) {
            if (obj->type() == NOTE)
                  obj = obj->parent();
            ChordRest* cr = static_cast<ChordRest*>(obj);
            if (cr->tick() < sel->tickStart)
                  sel->tickStart = cr->tick();
            if (cr->tick() > sel->tickEnd)
                  sel->tickEnd = cr->tick();
            if (cr->staffIdx() < sel->staffStart)
                  sel->staffStart = cr->staffIdx();
            if (cr->staffIdx() > sel->staffEnd)
                  sel->staffEnd = cr->staffIdx();
            updateSelectedElements();
            return;
            }

      refresh |= obj->abbox();
      if (obj->selected()) {
            sel->remove(obj);
            }
      else {
            sel->add(obj);
            _is.track = obj->track();
            }
      setPadState(obj);
      emit selectionChanged(int(sel->state()));
      }

//---------------------------------------------------------
//   lassoSelect
//---------------------------------------------------------

void Score::lassoSelect(const QRectF& bbox)
      {
      select(0, 0, 0);
      QRectF lr(bbox.normalized());
      QList<const Element*> el = _layout->items(lr);
      for (int i = 0; i < el.size(); ++i) {
            const Element* e = el.at(i);
            e->itemDiscovered = 0;
            if (lr.contains(e->abbox())) {
                  if (e->type() != MEASURE)
                        select(const_cast<Element*>(e), Qt::ShiftModifier, 0);
                  }
            }
      }

//---------------------------------------------------------
//   setRange
//---------------------------------------------------------

void Selection::setRange(int a, int b, int c, int d)
      {
      tickStart = a;
      tickEnd   = b;
      staffStart = c;
      staffEnd   = d;
      }

//---------------------------------------------------------
//   lassoSelectEnd
//---------------------------------------------------------

void Score::lassoSelectEnd(const QRectF& /*bbox*/)
      {
      int noteRestCount = 0;
      int startTick     = 0x7fffffff;
      int endTick       = 0;
      int startStaff    = 0x7fffffff;
      int endStaff      = 0;

      foreach(const Element* e, *(sel->elements())) {
            if (e->type() == NOTE || e->type() == REST) {
                  ++noteRestCount;
                  if (e->type() == NOTE)
                        e = e->parent();
                  if (e->tick() < startTick)
                        startTick = e->tick();
                  if (e->tick() > endTick)
                        endTick = e->tick();
                  int idx = e->staffIdx();
                  if (idx < startStaff)
                        startStaff = idx;
                  if (idx > endStaff)
                        endStaff = idx;
                  }
            }
      Segment* seg = tick2segment(endTick);
      if (seg) {
            seg = seg->next1();
            if (seg)
                  endTick = seg->tick();
            }
      if (noteRestCount > 1) {
            sel->setState(SEL_STAFF);
            sel->setRange(startTick, endTick, startStaff, endStaff+1);
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
                  _state = SEL_NONE;
            else if (n == 1) {
                  _state     = SEL_SINGLE;
                  Element* e = element();
                  if (e->type() == NOTE || e->type() == REST) {
                        e->score()->setPadState(e);
                        if (e->type() == NOTE)
                              e = e->parent();
                        e->score()->setInputTrack(e->track());
                        }
                  }
            else
                  _state = SEL_MULT;
            }
      else if (n == 0)
            _state = SEL_NONE;
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

      int ticks  = tickEnd - tickStart;
      int staves = staffEnd - staffStart;
      xml.stag(QString("StaffList tick=\"%1\" len=\"%2\" staff=\"%3\" staves=\"%4\"").arg(tickStart).arg(ticks).arg(staffStart).arg(staves));
      Segment* seg1 = _score->tick2segment(tickStart);
      Segment* seg2 = _score->tick2segment(tickEnd);

      for (int staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
            xml.stag(QString("Staff id=\"%1\"").arg(staffIdx));
            int startTrack = staffIdx * VOICES;
            int endTrack   = startTrack + VOICES;
            for (Segment* seg = seg1; seg && seg != seg2; seg = seg->next1()) {
                  for (int track = startTrack; track < endTrack; ++track) {
                        Element* e = seg->element(track);
                        if (e && !e->generated())
                              e->write(xml);
                        }
                  }
            xml.etag();
            }
      xml.etag();
      buffer.close();
      return buffer.buffer();
      }

