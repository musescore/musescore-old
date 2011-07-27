//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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
#include "scoreview.h"
#include "note.h"
#include "chord.h"
#include "rest.h"
#include "score.h"
#include "slur.h"
#include "system.h"
#include "select.h"
#include "al/sig.h"
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
#include "beam.h"
#include "textline.h"

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
      _staffStart     = 0;
      _staffEnd       = 0;
      _activeTrack    = 0;
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
      if(_endSegment)
          return _endSegment->tick();
      else{ // endsegment == 0 if end of score
          Measure* m = _score->lastMeasure();
          return m->tick() + m->tickLen();
          }
      }

//---------------------------------------------------------
//   isStartActive
//---------------------------------------------------------

bool Selection::isStartActive() const
      {
      return activeSegment() && activeSegment()->tick() == tickStart();
      }

//---------------------------------------------------------
//   isEndActive
//---------------------------------------------------------

bool Selection::isEndActive() const {
      return activeSegment() && activeSegment()->tick() == tickEnd();
      }

//---------------------------------------------------------
//   element
//---------------------------------------------------------

Element* Selection::element() const
      {
      return _el.size() == 1 ? _el[0] : 0;
      }

//---------------------------------------------------------
//   activeCR
//---------------------------------------------------------

ChordRest* Selection::activeCR() const
      {
      if ((_state != SEL_RANGE) || !_activeSegment)
            return 0;
      if (_activeSegment == _startSegment)
            return firstChordRest(_activeTrack);
      else
            return lastChordRest(_activeTrack);
      }

//---------------------------------------------------------
//   firstChordRest
//---------------------------------------------------------

ChordRest* Selection::firstChordRest(int track) const
      {
      ChordRest* cr = 0;
      for (ciElement i = _el.begin(); i != _el.end(); ++i) {
            Element* el = *i;
            if (el->type() == NOTE)
                  el = el->parent();
            if (el->isChordRest()) {
                  if (track != -1 && el->track() != track)
                        continue;
                  if (cr) {
                        if (el->tick() < cr->tick())
                              cr = static_cast<ChordRest*>(el);
                        }
                  else
                        cr = static_cast<ChordRest*>(el);
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
            if (el->type() == NOTE)
                  el = ((Note*)el)->chord();
            if (el->isChordRest() && static_cast<ChordRest*>(el)->segment()->subtype() == SegChordRest) {
                  if (track != -1 && el->track() != track)
                        continue;
                  if (cr) {
                        if (((ChordRest*)el)->tick() >= cr->tick())
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

void Selection::deselectAll()
      {
      if (_state == SEL_RANGE)
            _score->setUpdateAll();
      clear();
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Selection::clear()
      {
      foreach(Element* e, _el) {
            _score->addRefresh(e->abbox());
            e->setSelected(false);
            _score->addRefresh(e->abbox());
            }
      _el.clear();
      setStartSegment(0);
      setEndSegment(0);
      setState(SEL_NONE);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Selection::remove(Element* el)
      {
      _el.removeOne(el);
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

void Score::deselect(Element* el)
      {
      refresh |= el->abbox();
      _selection.remove(el);
      el->setSelected(false);
      }

//---------------------------------------------------------
//   updateSelectedElements
//---------------------------------------------------------

void Score::updateSelectedElements()
      {
      setUpdateAll();
      foreach(Element* e, _selection.elements())
            e->setSelected(false);
      _selection.clearElements();

      // assert:
      if (_selection.staffStart() < 0 || _selection.staffStart() >= nstaves() || _selection.staffEnd() < 0 || _selection.staffEnd() > nstaves()
         || _selection.staffStart() >= _selection.staffEnd()) {
            printf("updateSelectedElements: bad staff selection %d - %d\n", _selection.staffStart(), _selection.staffEnd());
            _selection.setStaffStart(0);
            _selection.setStaffEnd(1);
            }
      int startTrack = _selection.staffStart() * VOICES;
      int endTrack   = _selection.staffEnd() * VOICES;

      for (int st = startTrack; st < endTrack; ++st) {
            for (Segment* s = _selection.startSegment(); s && (s != _selection.endSegment()); s = s->next1()) {
                  Element* e = s->element(st);
                  if (!e)
                        continue;
                  if (e->type() == CHORD) {
                        Chord* chord = static_cast<Chord*>(e);
                        foreach(Note* note, chord->notes()) {
                              note->setSelected(true);
                              _selection.append(note);
                              }
                        }
                  else {
                        e->setSelected(true);
                        _selection.append(e);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   select
//    staffIdx is valid, if element is of type MEASURE
//---------------------------------------------------------

void Score::select(Element* e, SelectType type, int staffIdx)
      {
      if (debugMode)
            printf("select element <%s> type %d(state %d) staff %d\n",
               e ? e->name() : "", type, selection().state(), e ? e->staffIdx() : -1);

      SelState selState = _selection.state();

      if (type == SELECT_SINGLE) {
            _selection.deselectAll();
            if (e == 0) {
                  selState = SEL_NONE;
                  _updateAll = true;
                  }
            else {
                  if (e->type() == MEASURE) {
                        select(e, SELECT_RANGE, staffIdx);
                        return;
                        }
                  refresh |= e->abbox();
                  _selection.add(e);
                  _is.track = e->track();
                  selState = SEL_LIST;
                  if (e->type() == NOTE || e->type() == REST || e->type() == CHORD) {
                        if (e->type() == NOTE)
                              e = e->parent();
                        _is._segment = static_cast<ChordRest*>(e)->segment();
                        emit posChanged(_is.tick());
                        }
                  }
            _selection.setActiveSegment(0);
            _selection.setActiveTrack(0);
            }
      else if (type == SELECT_ADD) {
            if (e->type() == MEASURE) {
                  Measure* m = static_cast<Measure*>(e);
                  int tick  = m->tick();
                  int etick = tick + m->tickLen();
                  if (_selection.state() == SEL_NONE) {
                        _selection.setStartSegment(m->tick2segment(tick, true));
                        _selection.setEndSegment(tick2segment(etick));
                        }
                  else {
                        select(0, SELECT_SINGLE, 0);
                        return;
                        }
                  _updateAll = true;
                  selState = SEL_RANGE;
                  _selection.setStaffStart(0);
                  _selection.setStaffEnd(nstaves());
                  updateSelectedElements();
                  }
            else {
                  if (_selection.state() == SEL_RANGE) {
                        select(0, SELECT_SINGLE, 0);
                        return;
                        }
                  else {
                        refresh |= e->abbox();
                        if (_selection.elements().contains(e))
                              _selection.remove(e);
                        else {
                            _selection.add(e);
                            selState = SEL_LIST;
                            }
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
                  if (_selection.state() == SEL_NONE) {
                        _selection.setStaffStart(staffIdx);
                        _selection.setStaffEnd(staffIdx + 1);
                        _selection.setStartSegment(m->tick2segment(tick, true));
                        _selection.setEndSegment(tick2segment(etick, true));
                        }
                  else if (_selection.state() == SEL_RANGE) {
                        if (staffIdx < _selection.staffStart())
                              _selection.setStaffStart(staffIdx);
                        else if (staffIdx >= _selection.staffEnd())
                              _selection.setStaffEnd(staffIdx + 1);
                        if (tick < _selection.tickStart()) {
                              _selection.setStartSegment(m->tick2segment(tick, true));
                              activeIsFirst = true;
                              }
                        else if (etick >= _selection.tickEnd())
                              _selection.setEndSegment(tick2segment(etick, true));
                        else {
                              if (_selection.activeSegment() == _selection.startSegment()) {
                                    _selection.setStartSegment(m->tick2segment(tick, true));
                                    activeIsFirst = true;
                                    }
                              else
                                    _selection.setEndSegment(tick2segment(etick, true));
                              }
                        }
                  else if (_selection.isSingle()) {
                        Segment* seg = 0;
                        Element* oe = _selection.element();
                        bool reverse = false;
                        int ticks = 0;
                        if (oe->isChordRest())
                              ticks = static_cast<ChordRest*>(oe)->ticks();
                        if (tick < oe->tick())
                              seg = m->first();
                        else if (etick >= oe->tick() + ticks) {
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
                        printf("SELECT_RANGE: measure: sel state %d\n", _selection.state());
                        }
                  }
            else if (e->type() == NOTE || e->type() == REST || e->type() == CHORD) {
                  if (e->type() == NOTE)
                        e = static_cast<Note*>(e)->chord();
                  ChordRest* cr = static_cast<ChordRest*>(e);

                  if (_selection.state() == SEL_NONE) {
                        _selection.setStaffStart(e->staffIdx());
                        _selection.setStaffEnd(_selection.staffStart() + 1);
                        _selection.setStartSegment(cr->segment());
                        activeTrack = cr->track();
                        _selection.setEndSegment(cr->segment()->nextCR(cr->track()));
                        }
                  else if (_selection.isSingle()) {
                        Element* oe = _selection.element();
                        if (oe && (oe->type() == NOTE || oe->type() == REST || oe->type() == CHORD)) {
                              if (oe->type() == NOTE)
                                    oe = oe->parent();
                              ChordRest* ocr = static_cast<ChordRest*>(oe);
                              _selection.setStaffStart(oe->staffIdx());
                              _selection.setStaffEnd(_selection.staffStart() + 1);
                              _selection.setStartSegment(ocr->segment());
                              _selection.setEndSegment(ocr->segment()->nextCR(ocr->track()));
                              if (!_selection.endSegment())
                                    _selection.setEndSegment(ocr->segment()->next());

                              staffIdx = cr->staffIdx();
                              int tick = cr->tick();
                              if (staffIdx < _selection.staffStart())
                                    _selection.setStaffStart(staffIdx);
                              else if (staffIdx >= _selection.staffEnd())
                                    _selection.setStaffEnd(staffIdx + 1);
                              if (tick < _selection.tickStart()) {
                                    _selection.setStartSegment(cr->segment());
                                    activeIsFirst = true;
                                    }
                              else if (tick >= _selection.tickEnd())
                                    _selection.setEndSegment(cr->segment()->nextCR(cr->track()));
                              else {
                                    if (_selection.activeSegment() == _selection.startSegment()) {
                                          _selection.setStartSegment(cr->segment());
                                          activeIsFirst = true;
                                          }
                                    else
                                          _selection.setEndSegment(cr->segment()->nextCR(cr->track()));
                                    }
                              }
                        else {
                              select(e, SELECT_SINGLE, 0);
                              return;
                              }
                        }
                  else if (_selection.state() == SEL_RANGE) {
                        staffIdx = cr->staffIdx();
                        int tick = cr->tick();
                        if (staffIdx < _selection.staffStart())
                              _selection.setStaffStart(staffIdx);
                        else if (staffIdx >= _selection.staffEnd())
                              _selection.setStaffEnd(staffIdx + 1);
                        if (tick < _selection.tickStart()) {
                              if (_selection.activeSegment() == _selection.endSegment())
                                    _selection.setEndSegment(_selection.startSegment());
                              _selection.setStartSegment(cr->segment());
                              activeIsFirst = true;
                              }
                        else if (_selection.endSegment() && tick >= _selection.tickEnd()) {
                              if (_selection.activeSegment() == _selection.startSegment())
                                    _selection.setStartSegment(_selection.endSegment());
                              Segment* s = cr->segment()->nextCR(cr->track());
                              _selection.setEndSegment(s);
                              }
                        else {
                              if (_selection.activeSegment() == _selection.startSegment()) {
                                    _selection.setStartSegment(cr->segment());
                                    activeIsFirst = true;
                                    }
                              else {
                                    _selection.setEndSegment(cr->segment()->nextCR(cr->track()));
                                    }
                              }
                        }
                  else {
                        printf("sel state %d\n", _selection.state());
                        }
                  selState = SEL_RANGE;
                  if (!_selection.endSegment())
                        _selection.setEndSegment(cr->segment()->nextCR());
                  if (!_selection.startSegment())
                        _selection.setStartSegment(cr->segment());
                  }
            else {
                  select(e, SELECT_SINGLE, staffIdx);
                  return;
                  }

            if (activeIsFirst)
                  _selection.setActiveSegment(_selection.startSegment());
            else
                  _selection.setActiveSegment(_selection.endSegment());

            _selection.setActiveTrack(activeTrack);

            selState = SEL_RANGE;
            updateSelectedElements();
            }

      _selection.setState(selState);
      emit selectionChanged(int(_selection.state()));
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
                  if (e->type() != MEASURE && e->selectable())
                        select(const_cast<Element*>(e), SELECT_ADD, 0);
                  }
            }
      }

//---------------------------------------------------------
//   setRange
//---------------------------------------------------------

void Selection::setRange(Segment* a, Segment* b, int c, int d)
      {
      _startSegment  = a;
      _endSegment    = b;
      _activeSegment = b;
      _staffStart    = c;
      _staffEnd      = d;
      }

//---------------------------------------------------------
//   lassoSelectEnd
//---------------------------------------------------------

void Score::lassoSelectEnd()
      {
      int noteRestCount     = 0;
      Segment* startSegment = 0;
      Segment* endSegment   = 0;
      int startStaff        = 0x7fffffff;
      int endStaff          = 0;
      int endTrack          = 0;

      if (_selection.elements().isEmpty()) {
            _selection.setState(SEL_NONE);
            return;
            }
      _selection.setState(SEL_LIST);

      foreach(const Element* e, _selection.elements()) {
            if (e->type() != NOTE && e->type() != REST)
                  continue;
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
      if (noteRestCount > 0) {
            endSegment = endSegment->nextCR(endTrack);
            _selection.setRange(startSegment, endSegment, startStaff, endStaff+1);
            if (_selection.state() != SEL_RANGE) {
                  _selection.setState(SEL_RANGE);
                  emit selectionChanged(int(_selection.state()));
                  }
            }
      _updateAll = true;
      }

//---------------------------------------------------------
//   searchSelectedElements
//    "ElementList selected"
//---------------------------------------------------------

/**
 Rebuild list of selected Elements.
*/
static void collectSelectedElements(void* data, Element* e)
      {
      QList<const Element*>* l = static_cast<QList<const Element*>*>(data);
      if (e->selected())
            l->append(e);
      }

void Score::searchSelectedElements()
      {
      _selection.searchSelectedElements();
      emit selectionChanged(int(_selection.state()));
      }

void Selection::searchSelectedElements()
      {
      _el.clear();
      _score->scanElements(&_el, collectSelectedElements);
      updateState();
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
      switch(_state) {
            case SEL_NONE:   printf("NONE\n"); return;
            case SEL_RANGE:  printf("RANGE\n"); break;
            case SEL_LIST:   printf("LIST\n"); break;
            }
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
      if (n == 0) {
            setState(SEL_NONE);
            return;
            }
      if (_state == SEL_NONE)
            _state = SEL_LIST;

      Element* e = element();
      if (e && (e->type() == NOTE || e->type() == REST)) {
            if (!_score->noteEntryMode())
                  _score->setPadState(e);
            e->setSelected(true);
            if (e->type() == NOTE)
                  e = e->parent();
            _score->setInputTrack(e->track());
            }
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
            case SEL_LIST:
                  return isSingle() ? mimeSymbolFormat : mimeSymbolListFormat;
            case SEL_RANGE:
                  return mimeStaffListFormat;
            }
      }

//---------------------------------------------------------
//   mimeData
//---------------------------------------------------------

QByteArray Selection::mimeData() const
      {
      QByteArray a;
      switch (_state) {
            case SEL_LIST:
                  if (isSingle()) {
                        Element* e = element();
                        if (e->type() == TEXTLINE_SEGMENT)
                              e = static_cast<TextLineSegment*>(e)->textLine();
                        a = e->mimeData(QPointF());
                        }
                  break;
            case SEL_NONE:
                  break;
            case SEL_RANGE:
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
      xml.clipboardmode = true;

      int slurId = 1;
      foreach(Element* el, *_score->gel()) {
            if (el->type() == SLUR)
                  static_cast<Slur*>(el)->setId(slurId++);
            }
      int beamId = 0;
      foreach(Beam* beam, _score->beams())
            beam->setId(beamId++);
      for (Measure* m = _score->firstMeasure(); m; m = m->nextMeasure()) {
            foreach(Tuplet* tuplet, *m->tuplets())
                  tuplet->setId(-1);
            }

      int ticks  = tickEnd() - tickStart();
      int staves = staffEnd() - staffStart();
      xml.stag(QString("StaffList tick=\"%1\" len=\"%2\" staff=\"%3\" staves=\"%4\"").arg(tickStart()).arg(ticks).arg(staffStart()).arg(staves));
      Segment* seg1 = _startSegment;
      Segment* seg2 = _endSegment;

      for (int staffIdx = staffStart(); staffIdx < staffEnd(); ++staffIdx) {
            xml.stag(QString("Staff id=\"%1\"").arg(staffIdx));

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

            int startTrack = staffIdx * VOICES;
            int endTrack   = startTrack + VOICES;
            for (Segment* seg = seg1; seg && seg != seg2; seg = seg->next1()) {
                  if (seg->subtype() == SegEndBarLine)
                        continue;
                  if (seg->subtype() == SegTimeSig)
                        continue;
                  for (int track = startTrack; track < endTrack; ++track) {
                        Element* e = seg->element(track);
                        if (e == 0 || e->generated())
                              continue;
                        if (e->isChordRest()) {
                              ChordRest* cr = static_cast<ChordRest*>(e);
                              Tuplet* tuplet = cr->tuplet();
                              if (tuplet && tuplet->elements().front() == cr) {
                                    QList<Tuplet*> tl;
                                    tl.prepend(tuplet);
                                    Tuplet* t = tuplet;
                                    while (t->tuplet()) {
                                          t = t->tuplet();
                                          tl.prepend(t);
                                          }
                                    foreach(Tuplet* t, tl) {
                                          if (t->id() == -1) {
                                                tuplet->setId(xml.tupletId++);
                                                tuplet->write(xml);
                                                }
                                          }
                                    }
                              foreach(Slur* slur, cr->slurFor()) {
                                    if (slur->startElement()->tick() >= tickStart()
                                       && slur->endElement()->tick() < tickEnd()) {
                                          slur->write(xml);
                                          }
                                    }
                              }
                        if (e->type() == CHORD) {
                              Chord* c = static_cast<Chord*>(e);
                              c->write(xml, tickStart(), tickEnd());
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

      if (_state == SEL_LIST) {
            foreach(Element* e, _el) {
                  if (e->type() == NOTE)
                        nl.append(static_cast<Note*>(e));
                  }
            }
      else if (_state == SEL_RANGE) {
            for (int staffIdx = staffStart(); staffIdx < staffEnd(); ++staffIdx) {
                  int startTrack = staffIdx * VOICES;
                  int endTrack   = startTrack + VOICES;
                  for (Segment* seg = _startSegment; seg && seg != _endSegment; seg = seg->next1()) {
                        if (!(seg->subtype() & (SegChordRest | SegGrace)))
                              continue;
                        for (int track = startTrack; track < endTrack; ++track) {
                              Element* e = seg->element(track);
                              if (e == 0 || e->type() != CHORD)
                                    continue;
                              Chord* c = static_cast<Chord*>(e);
                              nl.append(c->notes());
                              }
                        }
                  }
            }
      return nl;
      }

//---------------------------------------------------------
//   ElementPattern
//---------------------------------------------------------

struct ElementPattern {
      QList<Element*> el;
      int type;
      int subtype;
      int staff;
      int voice;
      const System* system;
      };

//---------------------------------------------------------
//   collectMatch
//---------------------------------------------------------

static void collectMatch(void* data, Element* e)
      {
      ElementPattern* p = static_cast<ElementPattern*>(data);
/*      if (p->type == e->type() && p->subtype != e->subtype())
            printf("%s subtype %d does not match\n", e->name(), e->subtype());
      */
      if ((p->type != e->type()) || (p->subtype != e->subtype()))
            return;
      if ((p->staff != -1) && (p->staff != e->staffIdx()))
            return;
      if (e->type() == CHORD || e->type() == REST || e->type() == NOTE || e->type() == LYRICS) {
            if (p->voice != -1 && p->voice != e->voice())
                  return;
            }
      if (p->system) {
            Element* ee = e;
            do {
                  if (ee->type() == SYSTEM) {
                        if (p->system != ee)
                              return;
                        break;
                        }
                  ee = ee->parent();
                  } while (ee);
            }
      p->el.append(e);
      }

//---------------------------------------------------------
//   selectSimilar
//---------------------------------------------------------

void Score::selectSimilar(Element* e, bool sameStaff)
      {
      ElementPattern pattern;
      pattern.type    = e->type();
      pattern.subtype = e->subtype();
      pattern.staff   = sameStaff ? e->staffIdx() : -1;
      pattern.voice   = -1;
      pattern.system  = 0;

      scanElements(&pattern, collectMatch);

      select(0, SELECT_SINGLE, 0);
      foreach(Element* e, pattern.el)
            select(e, SELECT_ADD, 0);
      }

//---------------------------------------------------------
//   SelectDialog
//---------------------------------------------------------

SelectDialog::SelectDialog(const Element* _e, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      e = _e;
      type->setText(e->name());
      }

//---------------------------------------------------------
//   setPattern
//---------------------------------------------------------

void SelectDialog::setPattern(ElementPattern* p)
      {
      p->type    = e->type();
      p->subtype = e->subtype();
      p->staff   = sameStaff->isChecked() ? e->staffIdx() : -1;
      p->voice   = sameVoice->isChecked() ? e->voice() : -1;
      p->system  = 0;
      if (sameSystem->isChecked()) {
            do {
                  if (e->type() == SYSTEM) {
                        p->system = static_cast<const System*>(e);
                        break;
                        }
                  e = e->parent();
                  } while (e);
            }
      }

//---------------------------------------------------------
//   selectElementDialog
//---------------------------------------------------------

void Score::selectElementDialog(Element* e)
      {
      SelectDialog sd(e, 0);
      if (sd.exec()) {
            ElementPattern pattern;
            sd.setPattern(&pattern);
            scanElements(&pattern, collectMatch);
            if (sd.doReplace()) {
                  select(0, SELECT_SINGLE, 0);
                  foreach(Element* ee, pattern.el)
                        select(ee, SELECT_ADD, 0);
                  }
            else if (sd.doSubtract()) {
                  QList<Element*> sl(_selection.elements());
                  foreach(Element* ee, pattern.el)
                        sl.removeOne(ee);
                  select(0, SELECT_SINGLE, 0);
                  foreach(Element* ee, sl)
                        select(ee, SELECT_ADD, 0);
                  }
            else if (sd.doAdd()) {
                  foreach(Element* ee, pattern.el)
                        select(ee, SELECT_ADD, 0);
                  }
            }
      }

//---------------------------------------------------------
//   isInMiddleOfTuplet
//---------------------------------------------------------

static bool isInMiddleOfTuplet(Element* e)
      {
      if (e == 0 || !e->isChordRest())
            return false;
      ChordRest* cr = static_cast<ChordRest*>(e);
      if (!cr->tuplet())
            return false;
      Tuplet* tuplet = cr->tuplet();
      while (tuplet) {
            if (tuplet->elements().front() == e)
                  return false;
            if (tuplet->elements().back() == e)
                  return false;
            tuplet = tuplet->tuplet();
            }
      return true;
      }

//---------------------------------------------------------
//   canCopy
//    return false if range selection intersects a tuplet
//---------------------------------------------------------

bool Selection::canCopy() const
      {
      if (_state != SEL_RANGE)
            return true;

      for (int staffIdx = _staffStart; staffIdx != _staffEnd; ++staffIdx) {
            if (isInMiddleOfTuplet(_startSegment->element(staffIdx)))
                  return false;
            if (_endSegment && isInMiddleOfTuplet(_endSegment->element(staffIdx)))
                  return false;
            }
      return true;
      }

