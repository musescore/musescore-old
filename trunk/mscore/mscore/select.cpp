//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: select.cpp,v 1.47 2006/03/22 12:04:14 wschweer Exp $
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

//---------------------------------------------------------
//   element
//---------------------------------------------------------

Element* Selection::element() const
      {
      if (state == SEL_SINGLE)
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
      if (state == SEL_STAFF || state == SEL_SYSTEM)
            cs->setUpdateAll();
      return clear();
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

QRectF Selection::clear()
      {
      QRectF r;
      for (ciElement i = _el.begin(); i != _el.end(); ++i) {
            r |= (*i)->abbox();
            (*i)->setSelected(false);
            r |= (*i)->abbox();
            }
      _el.clear();
      state = SEL_NONE;
      return r;
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Selection::remove(Element* el)
      {
      _el.remove(el);
      updateState();
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Selection::add(Element* el)
      {
      _el.push_back(el);
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
//   select
//    staff is valid, if obj is of type MEASURE
//---------------------------------------------------------

void Score::select(Element* obj, int state, int staff)
      {
// printf("select element <%s> staff %d\n", obj ? obj->name() : "", obj ? obj->staffIdx() : -1);

      if (!(state & Qt::ShiftModifier) || !obj)
            refresh |= sel->deselectAll(this);
      if (!obj) {
            sel->state   = SEL_NONE;
            padState.len = 0;
            emit selectionChanged(int(SEL_NONE));
            return;
            }
      if (obj->type() == MEASURE) {
            refresh       |= QRectF(0, 0, 10000, 10000);   // hack
            Measure* m     = (Measure*)obj;
            int tickStart  = m->tick();
            int tickEnd    = tickStart + m->tickLen();
            int staffStart = staff;
            int staffEnd   = staff+1;

            if ((state & Qt::ShiftModifier)
               && (sel->state == SEL_SYSTEM || sel->state == SEL_STAFF)) {
                  if (sel->tickStart < tickStart)
                        tickStart = sel->tickStart;
                  if (sel->tickEnd > tickEnd)
                        tickEnd = sel->tickEnd;
                  if (sel->state == SEL_STAFF) {
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
            else if ((state & Qt::ShiftModifier) && (sel->state == SEL_SYSTEM))
                  selState = SEL_SYSTEM;

            if (selState == SEL_SYSTEM) {
                  sel->staffStart = 0;
                  sel->staffEnd = nstaves();
                  }
            if (sel->state != selState) {
                  sel->deselectAll(this);
                  sel->state = selState;
                  emit selectionChanged(int(sel->state));
                  }
            //
            //  select all elements in range
            //
            int startTrack = staffStart * VOICES;
            int endTrack   = staffEnd * VOICES;

            for (Measure* m = _layout->first(); m; m = m->next()) {
                  int ms = m->tick();
                  int me = ms + m->tickLen();
                  if (me < tickStart)
                        continue;
                  if (ms >= tickEnd)
                        break;
                  for (int st = startTrack; st < endTrack; ++st) {
                        for (Segment* segment = m->first(); segment; segment = segment->next()) {
                              if (segment->tick() < tickStart)
                                    continue;
                              if (segment->tick() >= tickEnd)
                                    break;
                              Element* e = segment->element(st);
                              if (!e)
                                    continue;
                              e->setSelected(true);
                              sel->elements()->push_back(e);
                              }
                        }
//                  for (int st = sbar; st < ebar; ++st) {
//                        for (Segment* segment = m->first(); segment; segment = segment->next()) {
//TODO                              if (el->lyrics()) {
//                                    el->lyrics()->setSelected(true);
//                                    sel->elements()->push_back(el->lyrics());
//                                    }
//                              }
//                        }
                  }
            padState.len = 0;
            return;
            }
      refresh |= obj->abbox();
      sel->add(obj);
      ::setPadState(obj);
      cis->staff = obj->staffIdx();
      cis->voice = obj->voice();   //TODO
      emit selectionChanged(int(sel->state));
      }

//---------------------------------------------------------
//   findSelectableElement
//---------------------------------------------------------

/**
 If found, return selectable Element in this Score near Canvas relative point \a pp.
*/

Element* Score::findSelectableElement(const QPointF& pp)
      {
      for (ciPage ip = pages()->begin(); ip != pages()->end(); ++ip) {
            const Page* page = *ip;
            if (!page->contains(pp))
                  continue;
            QPointF p = pp - page->pos();  // transform to page relative

            // Page Element?
            const ElementList* el = page->pel();
            for (ciElement i = el->begin(); i != el->end(); ++i) {
                  if ((*i)->contains(p))
                        return *i;
                  }

            // Element in System/Measure selected?
            SystemList* sl = page->systems();
            for (ciSystem s = sl->begin(); s != sl->end(); ++s) {
                  Element* element = (*s)->findSelectableElement(p);
                  if (element)
                        return element;
                  }
            // System/Measure selected?
            for (ciSystem s = sl->begin(); s != sl->end(); ++s) {
                  QPointF ppp  = p - (*s)->pos();   // system relative
                  BarLine* bar = (*s)->getBarLine();
                  if (bar && bar->contains(ppp))
                        return bar;
                  MeasureList* ml = (*s)->measures();
                  for (ciMeasure im = ml->begin(); im != ml->end(); ++im) {
                        Measure* m = *im;
                        const ElementList* pel = m->pel();
                        for (ciElement ie = pel->begin(); ie != pel->end(); ++ie) {
                              if ((*ie)->contains(p))
                                    return *ie;
                              }
                        QPointF pppp = ppp - m->pos();  // measure relative

                        // if on a staff in a measure, select the measure
                        int idx  = (*s)->y2staff(p.y());
                        double x = m->bbox().x();
                        double w = m->bbox().width();
                        if (idx >= 0 && pppp.x() > x && pppp.x() < (x + w))
                              return m;
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   lassoSelect
//---------------------------------------------------------

QRegion Canvas::lassoSelect()
      {
      QRegion r;
      _score->select(0, 0, 0);
      QRectF lr(lasso->abbox());

      int tracks = _score->nstaves() * VOICES;
      for (ciPage ip = _score->pages()->begin(); ip != _score->pages()->end(); ++ip) {
            const Page* page = *ip;
            if (page->abbox().intersects(lasso->abbox())) {
                  const ElementList* el = page->pel();
                  for (ciElement i = el->begin(); i != el->end(); ++i) {
                        if (lr.contains((*i)->abbox()))
                              _score->select(*i, Qt::ShiftModifier, 0);
                        }

                  SystemList* sl = page->systems();
                  for (ciSystem s = sl->begin(); s != sl->end(); ++s) {
                        for (ciMeasure im = (*s)->measures()->begin(); im != (*s)->measures()->end(); ++im) {
                              Measure* measure = *im;
                              for (const Segment* segment = measure->first(); segment; segment = segment->next()) {
                                    for (int track = 0; track < tracks; ++track) {
                                          Element* e = segment->element(track);
                                          if (e) {
                                                if (e->type() == CHORD) {
                                                      Chord* chord = (Chord*)e;
                                                      NoteList* notes = chord->noteList();
                                                      for (ciNote in = notes->begin(); in != notes->end(); ++in) {
                                                            Note* note = in->second;
                                                            if (lr.contains(note->abbox()))
                                                                  _score->select(note, Qt::ShiftModifier, 0);
                                                            if (note->tieFor() && lr.contains(note->tieFor()->abbox()))
                                                                  _score->select(note->tieFor(), Qt::ShiftModifier, 0);
                                                            if (note->accidental() && lr.contains(note->accidental()->abbox()))
                                                                  _score->select(note->accidental(), Qt::ShiftModifier, 0);
                                                            }
                                                      QList<NoteAttribute*>* al = chord->getAttributes();
                                                      for (ciAttribute ia = al->begin(); ia != al->end(); ++ia) {
                                                            if (lr.contains((*ia)->abbox()))
                                                                  _score->select(*ia, Qt::ShiftModifier, 0);
                                                            }
                                                      }
                                                else if (e->type() == REST) {
                                                      if (lr.contains(e->abbox()))
                                                            _score->select(e, Qt::ShiftModifier, 0);
                                                      QList<NoteAttribute*>* al = ((Rest*)e)->getAttributes();
                                                      for (ciAttribute ia = al->begin(); ia != al->end(); ++ia) {
                                                            if (lr.contains((*ia)->abbox()))
                                                                  _score->select(*ia, Qt::ShiftModifier, 0);
                                                            }

                                                      }
                                                else {
                                                      if (lr.contains(e->abbox()))
                                                            _score->select(e, Qt::ShiftModifier, 0);
                                                      }
                                                }
                                          }
                                    }
                              }
                        }
                  }
            }
      return r;
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
      sel->clear();
      int tracks = nstaves() * VOICES;
      ElementList* el = sel->elements();

      for (Measure* m = _layout->first(); m; m = m->next()) {
            for (Segment* s = m->first(); s; s = s->next()) {
                  for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                        LyricsList* ll = s->lyricsList(staffIdx);
                        foreach(Lyrics* l, *ll) {
                              if (l && l->selected())
                                    el->push_back(l);
                              }
                        }
                  for (int track = 0; track < tracks; ++track) {
                        Element* e = s->element(track);
                        if (e == 0)
                              continue;
                        if (e->selected())
                              el->push_back(e);
                        if (e->type() == CHORD) {
                              Chord* chord = (Chord*)e;
                              const NoteList* nl = chord->noteList();
                              for (ciNote in = nl->begin(); in != nl->end(); ++in) {
                                    Note* note = in->second;
                                    if (note->selected())
                                          el->push_back(note);
                                    if (note->tieFor() && note->tieFor()->selected()) {
                                          el->push_back(note->tieFor());
                                          }
                                    foreach(Text* f, note->fingering()) {
                                          if (f->selected())
                                                el->push_back(f);
                                          }
                                    }
                                QList<NoteAttribute*>* al = chord->getAttributes();
                                for (ciAttribute i = al->begin(); i != al->end(); ++i) {
                                    NoteAttribute* a = *i;
                                if (a->selected())
                                      el->push_back(a);
                                }
                              }
                        else if (e->type() == REST) {
                              Rest* rest = (Rest*)e;
                                QList<NoteAttribute*>* al = rest->getAttributes();
                                for (ciAttribute i = al->begin(); i != al->end(); ++i) {
                                      NoteAttribute* a = *i;
                                if (a->selected())
                                      el->push_back(a);
                                }
                              }
                        }
                  }
            const ElementList* l = m->el();
            for (ciElement i = l->begin(); i != l->end(); ++i) {
                  if ((*i)->selected())
                        el->push_back(*i);
                  }
            l = m->pel();
            for (ciElement i = l->begin(); i != l->end(); ++i) {
                  if ((*i)->selected())
                        el->push_back(*i);
                  }
            }
      sel->updateState();
      emit selectionChanged(int(sel->state));
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
      if (state == SEL_NONE) {
            printf("NONE\n");
            return;
            }
      if (state == SEL_SINGLE)
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
      if (n == 0)
            state = SEL_NONE;
      else if (n == 1) {
            state = SEL_SINGLE;
            Element* obj = element();
            ::setPadState(obj);
            cis->staff = obj->staffIdx();
            cis->voice = obj->voice();   //TODO
            }
      else
            state = SEL_MULT;
      }

//---------------------------------------------------------
//   mimeType
//---------------------------------------------------------

const char* Selection::mimeType() const
      {
      switch (state) {
            default:
            case SEL_NONE:
                  return 0;
            case SEL_SINGLE:
                  return "application/mscore/symbol";
            case SEL_MULT:
                  return "application/mscore/symbols";
            case SEL_STAFF:
                  return "application/mscore/staff";
            case SEL_SYSTEM:
                  return "application/mscore/system";
            }
      }

//---------------------------------------------------------
//   mimeData
//---------------------------------------------------------

QByteArray Selection::mimeData() const
      {
      QByteArray a;
      switch (state) {
            case SEL_SINGLE:
                  a = element()->mimeData();
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

      for (int staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
            xml.stag(QString("Staff id=\"%1\"").arg(staffIdx));
            for (Measure* m = _score->scoreLayout()->first(); m; m = m->next()) {
                  int ms = m->tick();
                  int me = ms + m->tickLen();
                  if (me < tickStart)
                        continue;
                  if (ms >= tickEnd)
                        break;
                  m->write(xml, 0, staffIdx);
                  }
            xml.etag("Staff");
            }
      buffer.close();
      return buffer.buffer();
      }
